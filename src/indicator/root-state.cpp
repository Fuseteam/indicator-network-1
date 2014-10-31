/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include "root-state.h"

#include <functional>

#include <connectivity/networking/wifi/link.h>
#include <connectivity/networking/wifi/access-point.h>

#include <menumodel-cpp/gio-helpers/util.h>

namespace networking = connectivity::networking;

class RootState::Private : public std::enable_shared_from_this<Private>
{
public:
    std::shared_ptr<networking::Manager> m_manager;
    ModemManager::Ptr m_modemManager;
    core::Property<Variant> m_state;

    std::string m_label;

    /// @todo multiple adapters etc..
    std::string m_networkingIcon;

    std::map<Modem::Ptr, std::string> m_cellularIcons;
    std::map<int, std::string>        m_modemTechIcons;

    Private() = delete;
    Private(std::shared_ptr<networking::Manager> manager, ModemManager::Ptr modemManager);
    void ConstructL();

    void modemsChanged(const std::set<Modem::Ptr> &modems);

    void updateModem(Modem::WeakPtr weakModem);

    void updateNetworkingIcon();

    Variant createIcon(const std::string name);
    void updateRootState();
};

RootState::Private::Private(std::shared_ptr<networking::Manager> manager, ModemManager::Ptr modemManager)
    : m_manager{manager},
      m_modemManager{modemManager}
{}

void
RootState::Private::ConstructL()
{
    auto that = shared_from_this();

    m_manager->flightMode().changed().connect([that](connectivity::networking::Manager::FlightModeStatus)
    {
        GMainLoopDispatch([that](){
            that->updateRootState();
        });
    });

    modemsChanged(m_modemManager->modems().get());
    // modem properties and signals already synced with GMainLoop
    m_modemManager->modems().changed().connect(std::bind(&Private::modemsChanged, this, std::placeholders::_1));

    m_manager->status().changed().connect([that](connectivity::networking::Manager::NetworkingStatus)
    {
        GMainLoopDispatch([that](){
            that->updateNetworkingIcon();
        });
    });

    m_manager->links().changed().connect([that](std::set<connectivity::networking::Link::Ptr>)
    {
        GMainLoopDispatch([that](){
            that->updateNetworkingIcon();
        });
    });


    // will also call updateRootState()
    updateNetworkingIcon();
}

void
RootState::Private::modemsChanged(const std::set<Modem::Ptr> &modems)
{
    std::set<Modem::Ptr> current;
    for (auto element : m_cellularIcons)
        current.insert(element.first);

    std::set<Modem::Ptr> removed;
    std::set_difference(current.begin(), current.end(),
                        modems.begin(), modems.end(),
                        std::inserter(removed, removed.begin()));

    std::set<Modem::Ptr> added;
    std::set_difference(modems.begin(), modems.end(),
                        current.begin(), current.end(),
                        std::inserter(added, added.begin()));
    for (auto modem : removed)
        m_cellularIcons.erase(modem);

    for (auto modem : added) {
        // modem properties and signals already synced with GMainLoop
        modem->online().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        modem->simStatus().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        modem->status().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        modem->technology().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        modem->strength().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        updateModem(modem);
    }
}

void
RootState::Private::updateModem(Modem::WeakPtr weakModem)
{
    auto modem = weakModem.lock();
    if (!modem) {
        std::cerr << std::string(__PRETTY_FUNCTION__) << ": modem expired" << std::endl;
        return;
    }

    m_modemTechIcons.erase(modem->index());
    m_cellularIcons[modem] = "";

    if (!modem->online().get()) {
        // modem offline, nothing to show
        updateRootState();
        return;
    }

    switch(modem->simStatus().get()) {
    case Modem::SimStatus::missing:
        // no need to show anything in the panel
        break;
    case Modem::SimStatus::error:
        m_cellularIcons[modem] = "simcard-error";
        break;
    case Modem::SimStatus::locked:
    case Modem::SimStatus::permanentlyLocked:
        m_cellularIcons[modem] = "simcard-locked";
        break;
    case Modem::SimStatus::ready:
    {
        switch (modem->status().get()) {
        case org::ofono::Interface::NetworkRegistration::Status::unregistered:
        case org::ofono::Interface::NetworkRegistration::Status::unknown:
        case org::ofono::Interface::NetworkRegistration::Status::searching:
            m_cellularIcons[modem] = "gsm-3g-disabled";
            break;
        case org::ofono::Interface::NetworkRegistration::Status::denied:
            /// @todo we might need network-error for this
            m_cellularIcons[modem] = "gsm-3g-disabled";
            break;
        case org::ofono::Interface::NetworkRegistration::Status::registered:
        case org::ofono::Interface::NetworkRegistration::Status::roaming:
            if (modem->strength().get() != 0) {
                m_cellularIcons[modem] = Modem::strengthIcon(modem->strength().get());
                m_modemTechIcons[modem->index()] = Modem::technologyIcon(modem->technology().get());
            } else {
                m_cellularIcons[modem] = "gsm-3g-no-service";

            }

            // we might have changed the modem tech icon which affects the networkingIcon.
            updateNetworkingIcon();
            break;
        }
        break;
    }
    case Modem::SimStatus::not_available:
        // no need to show anything in the panel
        break;
    }

    updateRootState();
}

void
RootState::Private::updateNetworkingIcon()
{
    m_networkingIcon.clear();

    switch (m_manager->status().get()) {
    case networking::Manager::NetworkingStatus::offline:
        m_networkingIcon = "nm-no-connection";
        //a11ydesc = _("Network (none)");
        break;
    case networking::Manager::NetworkingStatus::connecting:
        m_networkingIcon = "nm-no-connection";
        // some sort of connection animation
        break;
    case networking::Manager::NetworkingStatus::online:
        for (auto link : m_manager->links().get()) {
            if (link->status().get() != networking::Link::Status::online)
                continue;
            if (link->type() == networking::Link::Type::wifi) {

                auto wifiLink = std::dynamic_pointer_cast<networking::wifi::Link>(link);

                int strength = -1;
                bool secured = false;
                if (wifiLink->activeAccessPoint().get()) {
                    strength = wifiLink->activeAccessPoint().get()->strength().get();
                    secured  = wifiLink->activeAccessPoint().get()->secured();
                }

                /// @todo deal with a11ydesc
//                gchar *a11ydesc = nullptr;
//                if (secured) {
//                    a11ydesc = g_strdup_printf(_("Network (wireless, %d%%, secure)"), strength);
//                } else {
//                    a11ydesc = g_strdup_printf(_("Network (wireless, %d%%)"), strength);
//                }
//                m_a11ydesc = {a11ydesc};
//                g_free(a11ydesc);

                if (strength >= 80) {
                    m_networkingIcon = secured ? "nm-signal-100-secure" : "nm-signal-100";
                } else if (strength >= 60) {
                    m_networkingIcon = secured ? "nm-signal-75-secure" : "nm-signal-75";
                } else if (strength >= 40) {
                    m_networkingIcon = secured ? "nm-signal-50-secure" : "nm-signal-50";
                } else if (strength >= 20) {
                    m_networkingIcon = secured ? "nm-signal-25-secure" : "nm-signal-25";
                } else {
                    m_networkingIcon = secured ? "nm-signal-0-secure" : "nm-signal-0";
                }

            } else if (link->type() == networking::Link::Type::wwan) {
                /// @todo show the tech icon
            }
        }

        if (m_networkingIcon.empty()) {
            // seems we don't have an active wifi connection..
            // it must be a cellular one then.
            /// @todo need to revise this once the modems are part of the connectivity-api
            ///       this might get us wrong results on dual-sim
            m_networkingIcon = m_modemTechIcons[1];
        }
        break;
    }

    updateRootState();
}

Variant
RootState::Private::createIcon(const std::string name)
{
    GError *error = nullptr;
    auto gicon = g_icon_new_for_string(name.c_str(), &error);
    if (error) {
        g_error_free(error);
        throw std::runtime_error("Could not create GIcon: " + std::string(error->message));
    }

    Variant ret = Variant::fromGVariant(g_icon_serialize(gicon));
    /// @todo not sure about this one:
    g_object_unref(gicon);
    return ret;
}

void
RootState::Private::updateRootState()
{
    std::vector<std::string> icons;
    std::map<std::string, Variant> state;

    switch(m_manager->flightMode().get()) {
    case networking::Manager::FlightModeStatus::off:
        break;
    case networking::Manager::FlightModeStatus::on:
        icons.push_back("airplane-mode");
        break;
    }

    std::multimap<int, std::string, Modem::Compare> sorted;
    for (auto pair : m_cellularIcons) {
        sorted.insert(std::make_pair(pair.first->index(), pair.second));
    }
    for (auto pair : sorted) {
        if (!pair.second.empty())
            icons.push_back(pair.second);
    }

    // if any of the modems is roaming, show the roaming icon
    for (auto modem : m_modemManager->modems().get()) {
        if (modem->status().get() == org::ofono::Interface::NetworkRegistration::Status::roaming) {
            icons.push_back("network-cellular-roaming");
            break;
        }
    }

    if (!m_networkingIcon.empty()) {

        /* We're doing icon always right now so we have a fallback before everyone
           supports multi-icon.  We shouldn't set both in the future. */
        try {
            state["icon"] = createIcon(m_networkingIcon);
        } catch (std::exception &e) {
            std::cerr << e.what();
        }

        icons.push_back(m_networkingIcon);
    }

    if (!m_label.empty())
        state["label"] = TypedVariant<std::string>(m_label);

    // TRANSLATORS: this is the indicator title shown on the top header of the indicator area
    state["title"] = TypedVariant<std::string>(_("Network"));

    /// @todo state["accessibility-desc"] = TypedVariant<std::string>(a11ydesc);
    state["visible"] = TypedVariant<bool>(true); /// @todo is this really necessary/useful?


    if (!icons.empty()) {
        std::vector<Variant> iconVariants;
        for (auto name : icons) {
            try {
                iconVariants.push_back(createIcon(name));
            } catch (std::exception &e) {
                std::cerr << e.what();
            }
        }
        state["icons"] = TypedVariant<std::vector<Variant>>(iconVariants);
    }

    m_state.set(TypedVariant<std::map<std::string, Variant>>(state));
}

RootState::RootState(std::shared_ptr<connectivity::networking::Manager> manager, ModemManager::Ptr modemManager)
    : d{new Private(manager, modemManager)}
{
    d->ConstructL();
}

RootState::~RootState()
{}

const core::Property<Variant> &
RootState::state()
{
    return d->m_state;
}
