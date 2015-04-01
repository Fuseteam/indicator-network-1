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
 *     Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <nmofono/manager-impl.h>
#include <nmofono/wifi/wifi-link-impl.h>
#include <NetworkManagerInterface.h>
#include <NetworkManagerDeviceInterface.h>

#include <NetworkManager.h>
#include <QDebug>
#include <iostream>

using namespace platform::nmofono;
namespace networking = connectivity::networking;

using namespace std;

namespace platform {
namespace nmofono {

class Manager::Private : public QObject
{
    Q_OBJECT
public:
    Manager& p;

    std::shared_ptr<OrgFreedesktopNetworkManagerInterface> nm;

    connectivity::networking::Manager::FlightModeStatus m_flightMode = FlightModeStatus::on;
    std::set<std::shared_ptr<connectivity::networking::Link>> m_links;
    connectivity::networking::Manager::NetworkingStatus m_status = NetworkingStatus::offline;
    uint32_t m_characteristics = 0;

    bool m_hasWifi = false;
    bool m_wifiEnabled = false;
    std::shared_ptr<KillSwitch> m_wifiKillSwitch;

    Private(Manager& parent) :
        p(parent)
    {
    }

public Q_SLOTS:
    void updateHasWifi()
    {
        if (m_wifiKillSwitch->state() != KillSwitch::State::not_available) {
            m_hasWifi = true;
            if (m_wifiKillSwitch->state() == KillSwitch::State::unblocked)
            {
                m_wifiEnabled = true;
            }
            else
            {
                m_wifiEnabled = false;
            }
            Q_EMIT p.hasWifiUpdated(m_hasWifi);
            Q_EMIT p.wifiEnabledUpdated(m_wifiEnabled);
            return;
        }

        // ok, killswitch not supported, but we still might have wifi devices
        bool haswifi = false;
        for (auto link : m_links)
        {
            if (link->type() == networking::Link::Type::wifi)
            {
                haswifi = true;
            }
        }
        m_hasWifi = haswifi;
        m_wifiEnabled = haswifi;
        Q_EMIT p.hasWifiUpdated(m_hasWifi);
        Q_EMIT p.wifiEnabledUpdated(m_wifiEnabled);
    }

    void setFlightMode(bool flightMode)
    {
        if (flightMode)
        {
            m_flightMode = networking::Manager::FlightModeStatus::on;
        }
        else
        {
            m_flightMode = networking::Manager::FlightModeStatus::off;
        }

        Q_EMIT p.flightModeUpdated(m_flightMode);
    }
};
}
}

void
Manager::updateNetworkingStatus(uint status)
{
    switch(status) {
    case NM_STATE_UNKNOWN:
    case NM_STATE_ASLEEP:
    case NM_STATE_DISCONNECTED:
    case NM_STATE_DISCONNECTING:
    {
        d->m_status = networking::Manager::NetworkingStatus::offline;
        break;
    }
    case NM_STATE_CONNECTING:
    {
        d->m_status = networking::Manager::NetworkingStatus::connecting;
        break;
    }
    case NM_STATE_CONNECTED_LOCAL:
    case NM_STATE_CONNECTED_SITE:
    case NM_STATE_CONNECTED_GLOBAL:
    {
        d->m_status = networking::Manager::NetworkingStatus::online;
        break;
    }
    }

    Q_EMIT statusUpdated(d->m_status);
}

Manager::Manager(const QDBusConnection& systemConnection) : d(new Manager::Private(*this))
{
    d->nm = std::make_shared<OrgFreedesktopNetworkManagerInterface>(NM_DBUS_SERVICE, NM_DBUS_PATH, systemConnection);

    /// @todo add a watcher for the service
    /// @todo exceptions
    /// @todo offload the initialization to a thread or something
    /// @todo those Id() thingies

    d->m_wifiKillSwitch = std::make_shared<KillSwitch>(systemConnection);
    connect(d->m_wifiKillSwitch.get(), &KillSwitch::stateChanged, d.get(), &Private::updateHasWifi);

    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::DeviceAdded, this, &Manager::device_added);
    QList<QDBusObjectPath> devices(d->nm->GetDevices());
    for(const auto &path : devices) {
        device_added(path);
    }

    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::DeviceRemoved, this, &Manager::device_removed);
    updateNetworkingStatus(d->nm->state());
    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::PropertiesChanged, this, &Manager::nm_properties_changed);

    connect(d->m_wifiKillSwitch.get(), &KillSwitch::flightModeChanged, d.get(), &Private::setFlightMode);
    try
    {
        d->setFlightMode(d->m_wifiKillSwitch->isFlightMode());
    }
    catch (std::exception const& e)
    {
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
        std::cerr << "Failed to retrieve initial flight mode state, assuming state is false." << std::endl;
        d->setFlightMode(false);
    }

    /// @todo set by the default connections.
    d->m_characteristics = networking::Link::Characteristics::empty;

    d->updateHasWifi();
}

void
Manager::nm_properties_changed(const QVariantMap &properties)
{
    auto stateIt = properties.find("State");
    if (stateIt != properties.cend())
    {
        updateNetworkingStatus(stateIt->toUInt());
    }
}

void
Manager::device_removed(const QDBusObjectPath &path)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        qDebug() << "Device Removed:" << path.path();
#endif
    auto links = d->m_links;
    for (const auto &dev : links) {
        if (std::dynamic_pointer_cast<wifi::Link>(dev)->device_path() == path) {
            links.erase(dev);
            break;
        }
    }
    d->m_links = links;
    Q_EMIT linksUpdated(d->m_links);

    d->updateHasWifi();
}

void
Manager::device_added(const QDBusObjectPath &path)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << "Device Added:" << path.path();
#endif
    for (const auto &dev : d->m_links) {
        if (std::dynamic_pointer_cast<wifi::Link>(dev)->device_path() == path) {
            // already in the list
            return;
        }
    }

    connectivity::networking::Link::Ptr link;
    try {
        auto dev = std::make_shared<OrgFreedesktopNetworkManagerDeviceInterface>(
            NM_DBUS_SERVICE, path.path(), d->nm->connection());
        if (dev->deviceType() == NM_DEVICE_TYPE_WIFI) {
            link = std::make_shared<wifi::Link>(dev,
                                                d->nm,
                                                d->m_wifiKillSwitch);
        }
    } catch (const std::exception &e) {
        qDebug() << __PRETTY_FUNCTION__ << ": failed to create Device proxy for "<< path.path() << ": ";
        qDebug() << "\t" << e.what();
        qDebug() << "\tIgnoring.";
        return;
    }

    if (link) {
        d->m_links.insert(link);
        Q_EMIT linksUpdated(d->m_links);
    }

    d->updateHasWifi();
}


void
Manager::enableFlightMode()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
    if (!d->m_wifiKillSwitch->flightMode(true))
    {
        qWarning() << "Failed to enable flightmode.";
    }
}

void
Manager::disableFlightMode()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
    if (!d->m_wifiKillSwitch->flightMode(false))
    {
        qWarning() << "Failed to disable flightmode";
    }
}

networking::Manager::FlightModeStatus
Manager::flightMode() const
{
    // - connect to each individual URfkill.Killswitch interface
    // - make this property to reflect their combined state
    /// @todo implement flightmode status properly when URfkill gets the flightmode API
    return d->m_flightMode;
}

const std::set<std::shared_ptr<networking::Link> >&
Manager::links() const
{
    return d->m_links;
}

networking::Manager::NetworkingStatus
Manager::status() const
{
    return d->m_status;
}

uint32_t
Manager::characteristics() const
{
    return d->m_characteristics;
}

bool
Manager::hasWifi() const
{
    return d->m_hasWifi;
}

bool
Manager::wifiEnabled() const
{
    return d->m_wifiEnabled;
}


bool
Manager::enableWifi()
{
    if (!d->m_hasWifi)
    {
        return false;
    }

    if (d->m_wifiEnabled)
    {
        return false;
    }

    try {
        if (d->m_wifiKillSwitch->state() == KillSwitch::State::soft_blocked) {
            // try to unblock. throws if fails.
            d->m_wifiKillSwitch->unblock();
        }
        d->nm->setWirelessEnabled(true);
    } catch(std::runtime_error &e) {
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool
Manager::disableWifi()
{
    if (!d->m_hasWifi)
    {
        return false;
    }

    if (!d->m_wifiEnabled)
    {
        return false;
    }

    try {
        if (d->m_wifiKillSwitch->state() == KillSwitch::State::unblocked) {
            // block the device. that will disable it also
            d->m_wifiKillSwitch->block();
        }
        d->nm->setWirelessEnabled(false);
    } catch(std::runtime_error &e) {
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
        return false;
    }
    return true;
}

#include "manager-impl.moc"
