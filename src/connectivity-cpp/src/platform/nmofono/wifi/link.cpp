/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include "link.h"
#include "access-point.h"
#include "grouped-access-point.h"
#include<cassert>

using namespace platform::nmofono::wifi;
namespace networking = connectivity::networking;
namespace dbus = core::dbus;
namespace fdo = org::freedesktop;
namespace NM = fdo::NetworkManager;

using core::Property;


struct Link::Private
{
    Private(const NM::Interface::Device &dev,
            const NM::Interface::NetworkManager &nm,
            KillSwitch::Ptr killSwitch);

    core::Property<std::uint32_t> characteristics_;
    core::Property<connectivity::networking::Link::Status> status_;
    core::Property<std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>>> rawAccessPoints;
    core::Property<std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>>> groupedAccessPoints;
    core::Property<std::shared_ptr<connectivity::networking::wifi::AccessPoint>> activeAccessPoint;

    org::freedesktop::NetworkManager::Interface::Device dev;
    org::freedesktop::NetworkManager::Interface::Device::Wireless wireless;
    org::freedesktop::NetworkManager::Interface::NetworkManager nm;

    KillSwitch::Ptr killSwitch;
    // hack hack
    std::vector<core::ScopedConnection> switchConnection;

    std::map<AccessPoint::Key, std::shared_ptr<GroupedAccessPoint>> grouper;
    bool disabled;
    std::uint32_t lastState;
    std::string name;
    std::shared_ptr<org::freedesktop::NetworkManager::Interface::ActiveConnection> activeConnection;
    bool connecting;

    std::mutex updateActiveConnectionMutex;
};

Link::Private::Private(const NM::Interface::Device &dev,
                       const NM::Interface::NetworkManager &nm,
                       KillSwitch::Ptr killSwitch)
    : dev(dev),
      wireless(this->dev.object),
      nm(nm),
      killSwitch(killSwitch),
      disabled(true),
      lastState(NM_STATE_UNKNOWN),
      connecting(false)
{}

Link::Link(const NM::Interface::Device& dev,
           const NM::Interface::NetworkManager& nm,
           platform::nmofono::KillSwitch::Ptr killSwitch)
    : p(new Private(dev, nm, killSwitch)) {
    p->characteristics_.set(Link::Characteristics::empty);
    p->status_.set(Status::disabled);

    p->name = p->dev.device_interface->get();

    p->wireless.access_point_added->connect(std::bind(&Link::ap_added, this, std::placeholders::_1));
    p->wireless.access_point_removed->connect(std::bind(&Link::ap_removed, this, std::placeholders::_1));
    for (auto path : dev.get_access_points()) {
        this->ap_added(path);
    }

    updateDeviceState(p->dev.state->get());
    p->dev.state_changed->connect([this](NM::Interface::Device::Signal::StateChanged::ArgumentType args) {
        // StateChanged ( u: new_state, u: old_state, u: reason )
        std::uint32_t new_state = std::get<0>(args);
        // std::uint32_t old_state = std::get<1>(args);
        // std::uint32_t reason    = std::get<2>(args);
        updateDeviceState(new_state);
    });
    p->switchConnection.emplace_back(p->killSwitch->state().changed().connect([this](platform::nmofono::KillSwitch::State){
        updateDeviceState(p->lastState);
    }));
}

Link::~Link()
{}

void Link::ap_added(const dbus::types::ObjectPath &path)
{
    try {
        for (auto ap : p->rawAccessPoints.get()) {
            if (std::dynamic_pointer_cast<AccessPoint>(ap)->object_path() == path) {
                // already in the list
                return;
            }
        }

        platform::nmofono::wifi::AccessPoint::Ptr shap;
        try {
            NM::Interface::AccessPoint ap(p->nm.service->object_for_path(path));
            shap = std::make_shared<platform::nmofono::wifi::AccessPoint>(ap);
        } catch(const std::exception &e) {
            std::cerr << __PRETTY_FUNCTION__ << ": failed to create AccessPoint proxy for "<< path.as_string() << ": " << std::endl
                      << "\t" << e.what() << std::endl
                      << "\tIgnoring." << std::endl;
            return;
        }

        auto list = p->rawAccessPoints.get();
        list.insert(shap);
        p->rawAccessPoints.set(list);

        auto k = AccessPoint::Key(shap);
        if(p->grouper.find(k) != p->grouper.end()) {
            p->grouper[k]->add_ap(shap);
        } else {
            p->grouper[k] = std::make_shared<GroupedAccessPoint>(shap);
        }
        update_grouped();
    } catch(const std::exception &e) {
        /// @bug dbus-cpp internal logic exploded
        // If this happens, indicator-network is in an unknown state with no clear way of
        // recovering. The only reasonable way out is a graceful exit.
        std::cerr << __PRETTY_FUNCTION__ << " Failed to run dbus service: " << e.what() << std::endl;
        exit(0);
    }
}

void Link::ap_removed(const dbus::types::ObjectPath &path)
{
    platform::nmofono::wifi::AccessPoint::Ptr shap;

    auto list = p->rawAccessPoints.get();
    for (const auto &ap : list) {
        if (std::dynamic_pointer_cast<AccessPoint>(ap)->object_path() == path) {
            shap = std::dynamic_pointer_cast<platform::nmofono::wifi::AccessPoint>(ap);
            list.erase(ap);
            break;
        }
    }
    if (!shap) {
        std::cerr << __PRETTY_FUNCTION__ << ": Tried to remove access point " << path.as_string() << " that has not been added." << std::endl;
        return;
    }
    p->rawAccessPoints.set(list);

    for (auto &e : p->grouper) {
        if (!e.second->has_object(path))
            continue;

        if(e.second->num_aps() == 1) {
            p->grouper.erase(e.first);
        } else {
            e.second->remove_ap(shap);
        }
        break;
    }
    update_grouped();
}

void Link::update_grouped() {
    std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>> new_grouped;
    for(auto &i : p->grouper) {
        new_grouped.insert(i.second);
    }
    p->groupedAccessPoints.set(new_grouped);
}

void
Link::enable()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    std::cout << "Link::enable()" << std::endl;
#endif

    try {
        if (p->killSwitch->state() != KillSwitch::State::unblocked) {
            // try to unblock. throws if fails.
            p->killSwitch->unblock();
        }
        p->nm.wireless_enabled->set(true);
        p->dev.autoconnect->set(true);
    } catch(std::runtime_error &e) {
        /// @todo when toggling enable()/disable() rapidly the default timeout of
        ///       1 second in dbus::core::Property is not long enough..
        ///       just ignore for now and get dbus-cpp to have larger timeout.
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
    }

#if 0
    /** @todo:
     * NetworkManager 0.9.9 allows doing this below and letting NM to figure out the best connection:
     *
     * m_nm.activate_connection(dbus::types::ObjectPath("/"),
     *                          m_dev.object->path(),
     *                          core::dbus::typ es::ObjectPath("/"));
     *
     * for now we get somewhat similar result by toggling the Autoconnect property,
     * but this breaks the possible setting we will have for autoconnecting later.
     */
    dev.autoconnect->set(true);
    disabled = false;
#endif
}

void
Link::disable()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    std::cout << "Link::disable()" << std::endl;
#endif

    /// @todo for now just disable wireless completely.
    ///        this only works properly when there is one wifi adapter on the system
    try {
        if (p->killSwitch->state() == KillSwitch::State::unblocked) {
            // block the device. that will disable it also
            p->killSwitch->block();
            return;
        }
        p->nm.wireless_enabled->set(false);
        p->dev.autoconnect->set(false);
    } catch(std::runtime_error &e) {
        /// @todo when toggling enable()/disable() rapidly the default timeout of
        ///       1 second in dbus::core::Property is not long enough..
        ///       just ignore for now and get dbus-cpp to have larger timeout.
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
    }

#if 0
    /** @todo remove this after NM 0.9.9 is available. check comment in enable() */
    dev.disconnect();
#endif
}

networking::Link::Type
Link::type() const
{
    return Type::wifi;
}

const Property<std::uint32_t>&
Link::characteristics() const
{
    return p->characteristics_;
}

const Property<networking::Link::Status>&
Link::status() const
{
    return p->status_;
}

networking::Link::Id
Link::id() const
{
    return 0;
}

std::string
Link::name() const
{
    return p->name;
}

const Property<std::set<std::shared_ptr<networking::wifi::AccessPoint> > >&
Link::rawAccessPoints() const
{
    return p->rawAccessPoints;
}

const core::Property<std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>>>&
Link::accessPoints() const {
    return p->groupedAccessPoints;
}

void
Link::connect_to(std::shared_ptr<networking::wifi::AccessPoint> accessPoint)
{
    try {
        p->connecting = true;
        std::vector<int8_t> ssid;
        std::shared_ptr<GroupedAccessPoint> ap = std::dynamic_pointer_cast<GroupedAccessPoint>(accessPoint);
        // The accesspoint interface does not provide this property so we need to coax it out of
        // derived classes.
        if(ap) {
            ssid = ap->raw_ssid();
        } else {
            std::shared_ptr<AccessPoint> bap = std::dynamic_pointer_cast<AccessPoint>(accessPoint);
            assert(bap);
            ssid = bap->raw_ssid();
        }
        NM::Interface::Connection *found = nullptr;
        auto connections = p->dev.get_available_connections();
        for (auto &con : connections) {
            for (auto map : con.get_settings()) {
                if (map.first == "802-11-wireless") {
                    for (auto conf : map.second) {
                        if (conf.first == "ssid") {
                            std::vector<int8_t> value;
                            value = conf.second.as<std::vector<std::int8_t>>();
                            if (value == ap->raw_ssid()) {
                                found = &con;
                                break;
                            }
                        }
                    }
                }
            }
        }

        /// @todo check the timestamps as there might be multiple ones that are suitable.
        /// @todo oh, and check more parameters than just the ssid

        core::dbus::types::ObjectPath ac;
        if (found) {
            ac = p->nm.activate_connection(found->object->path(),
                    p->dev.object->path(),
                    ap->object_path());
        } else {
            std::map<std::string, std::map<std::string, dbus::types::Variant> > conf;

            /// @todo getting the ssid multiple times over dbus is stupid.

            std::map<std::string, dbus::types::Variant> wireless_conf;
            wireless_conf["ssid"] = dbus::types::Variant::encode<std::vector<std::int8_t>>(ap->raw_ssid());

            conf["802-11-wireless"] = wireless_conf;
            auto ret = p->nm.add_and_activate_connection(conf,
                    p->dev.object->path(),
                    ap->object_path());
            ac = std::get<1>(ret);
        }
        updateActiveConnection(ac);
        p->connecting = false;
    } catch(const std::exception &e) {
        // @bug default timeout expired: LP(#1361642)
        // If this happens, indicator-network is in an unknown state with no clear way of
        // recovering. The only reasonable way out is a graceful exit.
        std::cerr << __PRETTY_FUNCTION__ << " Failed to activate connection: " << e.what() << std::endl;
        exit(0);
    }
}

const Property<std::shared_ptr<networking::wifi::AccessPoint> >&
Link::activeAccessPoint()
{
    return p->activeAccessPoint;
}

void
Link::updateDeviceState(std::uint32_t new_state)
{
    p->lastState = new_state;
    switch (new_state){
    case NM_DEVICE_STATE_DISCONNECTED:
    case NM_DEVICE_STATE_DEACTIVATING:
        if (p->connecting) {
            // ignore these while doing connect_to()
            break;
        }
        /* fallthrough */
    case NM_DEVICE_STATE_UNKNOWN:
    case NM_DEVICE_STATE_UNMANAGED:
    case NM_DEVICE_STATE_UNAVAILABLE:
    case NM_DEVICE_STATE_FAILED:
    {
        // make sure to set activeConnection before changing the status
        updateActiveConnection(core::dbus::types::ObjectPath("/"));

        switch(p->killSwitch->state().get()) {
        case KillSwitch::State::hard_blocked:
        case KillSwitch::State::soft_blocked:
            p->status_.set(Status::disabled);
            break;
        case KillSwitch::State::not_available:
        case KillSwitch::State::unblocked:
            p->status_.set(Status::offline);
        }
        break;
    }
    case NM_DEVICE_STATE_PREPARE:
    case NM_DEVICE_STATE_CONFIG:
    case NM_DEVICE_STATE_NEED_AUTH:
    case NM_DEVICE_STATE_IP_CONFIG:
    case NM_DEVICE_STATE_IP_CHECK:
    {
        // make sure to set activeConnection before changing the status
        auto path = p->dev.active_connection->get();
        // for some reason the path is not always set on these
        // states. Let's not clear the active connection as obviously
        // we have one.
        if (path != core::dbus::types::ObjectPath("/"))
            updateActiveConnection(path);
        p->status_.set(Status::connecting);
        break;
    }
    case NM_DEVICE_STATE_SECONDARIES:
    {
        // make sure to set activeConnection before changing the status
        updateActiveConnection(p->dev.active_connection->get());
        p->status_.set(Status::connected);
        break;
    }
    case NM_DEVICE_STATE_ACTIVATED:
    {
        // make sure to set activeConnection before changing the status
        updateActiveConnection(p->dev.active_connection->get());
        p->status_.set(Status::online);
        break;
    }}

}

/// '/' path means invalid.
void
Link::updateActiveConnection(const core::dbus::types::ObjectPath &path)
{
    std::lock_guard<std::mutex> lock(p->updateActiveConnectionMutex);

    // clear the one we have.
    if (path == core::dbus::types::ObjectPath("/")) {
        p->activeAccessPoint.set(connectivity::networking::wifi::AccessPoint::Ptr());
        p->activeConnection.reset();
        return;
    }

    // already up-to-date
    if (p->activeConnection && p->activeConnection->object->path() == path)
        return;

    try {
        p->activeConnection =
                std::make_shared<NM::Interface::ActiveConnection>(
                        p->dev.service->object_for_path(path));
        auto state = p->activeConnection->get_state();
        switch (state) {
        case NM::Interface::ActiveConnection::State::unknown:
        case NM::Interface::ActiveConnection::State::activating:
        case NM::Interface::ActiveConnection::State::activated:
        case NM::Interface::ActiveConnection::State::deactivating:
        case NM::Interface::ActiveConnection::State::deactivated:
            ;

            // for Wi-Fi devices specific_object is the AccessPoint object.
            auto ap_path = p->activeConnection->specific_object->get();
            for (auto &ap : p->groupedAccessPoints.get()) {
                auto shap =  std::dynamic_pointer_cast<GroupedAccessPoint>(ap);
                if (shap->has_object(ap_path)) {
                    p->activeAccessPoint.set(ap);
                    break;
                }
            }
        }
    } catch (std::exception &e) {
        std::cerr << "failed to get active connection:" << std::endl
                  << "\tpath: " << path.as_string() << std::endl
                  << "\t" << e.what() << std::endl;
    }
}


const core::dbus::types::ObjectPath& Link::device_path() const {
    return p->dev.object->path();
}
