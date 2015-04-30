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

#include <nmofono/wifi/wifi-link.h>
#include <nmofono/wifi/access-point.h>

#include <menumodel-cpp/gio-helpers/util.h>

using namespace std;
namespace networking = connectivity::networking;

class RootState::Private : public QObject
{
    Q_OBJECT

public:
    RootState& p;

    std::shared_ptr<networking::Manager> m_manager;
    ModemManager::Ptr m_modemManager;
    Variant m_state;
    networking::wifi::AccessPoint::Ptr m_activeAP;
    std::unique_ptr<QMetaObject::Connection> m_activeAP_conn;

    std::string m_label;

    /// @todo multiple adapters etc..
    std::string m_networkingIcon;

    std::map<Modem::Ptr, std::string> m_cellularIcons;
    std::map<int, std::string>        m_modemTechIcons;

    int m_activeModem = -1;

    Private() = delete;
    Private(RootState& parent, std::shared_ptr<networking::Manager> manager, ModemManager::Ptr modemManager);

    Variant createIcon(const std::string& name);

public Q_SLOTS:
    void updateModem(Modem::Ptr modem);

    void updateNetworkingIcon();

    void updateRootState();

void modemsChanged(const QList<Modem::Ptr> &modems);
};

RootState::Private::Private(RootState& parent, std::shared_ptr<networking::Manager> manager, ModemManager::Ptr modemManager)
    : p{parent},
      m_manager{manager},
      m_modemManager{modemManager}
{
    connect(m_manager.get(), &networking::Manager::flightModeUpdated, this, &Private::updateRootState);

    modemsChanged(m_modemManager->modems());
    // modem properties and signals already synced with GMainLoop
    connect(m_modemManager.get(), &ModemManager::modemsUpdated, this, &Private::modemsChanged);

    connect(m_manager.get(), &networking::Manager::statusUpdated, this, &Private::updateNetworkingIcon);
    connect(m_manager.get(), &networking::Manager::linksUpdated, this, &Private::updateNetworkingIcon);

    // will also call updateRootState()
    updateNetworkingIcon();
}

void
RootState::Private::modemsChanged(const QList<Modem::Ptr> &modems)
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

    m_activeModem = -1;
    for (auto modem : added) {
        // modem properties and signals already synced with GMainLoop
        connect(modem.get(), &Modem::updated, this, &Private::updateModem);
        updateModem(modem);
    }
}

void
RootState::Private::updateModem(Modem::Ptr modem)
{
    if (modem->dataEnabled())
    {
        m_activeModem = modem->index();
    }
    m_modemTechIcons.erase(modem->index());
    m_cellularIcons[modem] = "";

    if (!modem->online()) {
        // modem offline, nothing to show
        updateRootState();
        return;
    }

    switch(modem->simStatus()) {
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
        switch (modem->status()) {
        case Modem::Status::unregistered:
        case Modem::Status::unknown:
        case Modem::Status::searching:
            m_cellularIcons[modem] = "gsm-3g-disabled";
            break;
        case Modem::Status::denied:
            /// @todo we might need network-error for this
            m_cellularIcons[modem] = "gsm-3g-disabled";
            break;
        case Modem::Status::registered:
        case Modem::Status::roaming:
            if (modem->strength() != 0) {
                m_cellularIcons[modem] = Modem::strengthIcon(modem->strength()).toStdString();
                m_modemTechIcons[modem->index()] = Modem::technologyIcon(modem->bearer()).toStdString();
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

    switch (m_manager->status()) {
    case networking::Manager::NetworkingStatus::offline:
        m_networkingIcon = "nm-no-connection";
        //a11ydesc = _("Network (none)");
        break;
    case networking::Manager::NetworkingStatus::connecting:
        m_networkingIcon = "nm-no-connection";
        // some sort of connection animation
        break;
    case networking::Manager::NetworkingStatus::online:
        for (auto link : m_manager->links()) {
            if (link->status() != networking::Link::Status::online)
                continue;
            if (link->type() == networking::Link::Type::wifi) {

                auto wifiLink = std::dynamic_pointer_cast<networking::wifi::Link>(link);

                int strength = -1;
                bool secured = false;

                // check if the currently active AP has changed
                if (m_activeAP != wifiLink->activeAccessPoint())
                {
                    // locally store the active AP
                    m_activeAP = wifiLink->activeAccessPoint();
                    if (m_activeAP)
                    {
                        // connect updateNetworkingIcon() to changes in AP strength
                        auto c = QObject::connect(
                                m_activeAP.get(),
                                &networking::wifi::AccessPoint::strengthUpdated,
                                this,
                                &Private::updateNetworkingIcon);
                        if (m_activeAP_conn)
                        {
                            disconnect(*m_activeAP_conn);
                        }
                        m_activeAP_conn.reset(new QMetaObject::Connection(c));
                    }
                    else
                    {
                        // there is no active AP, so we disconnect from strength updates
                        if (m_activeAP_conn)
                        {
                            disconnect(*m_activeAP_conn);
                            m_activeAP_conn.release();
                        }
                    }
                }

                if (m_activeAP) {
                    strength = m_activeAP->strength();
                    secured  = m_activeAP->secured();
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
            if (m_activeModem != -1) {
                auto it = m_modemTechIcons.find(m_activeModem);
                if (it != m_modemTechIcons.end())
                {
                    m_networkingIcon = it->second;
                }
            }
        }
        break;
    }

    updateRootState();
}

Variant
RootState::Private::createIcon(const std::string& name)
{
    GError *error = nullptr;
    auto gicon = shared_ptr<GIcon>(g_icon_new_for_string(name.c_str(), &error), GObjectDeleter());
    if (error) {
        string message(error->message);
        g_error_free(error);
        throw std::runtime_error("Could not create GIcon: " + message);
    }

    Variant ret = Variant::fromGVariant(g_icon_serialize(gicon.get()));
    return ret;
}

void
RootState::Private::updateRootState()
{
    std::vector<std::string> icons;
    std::map<std::string, Variant> state;

    switch(m_manager->flightMode()) {
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
    for (auto modem : m_modemManager->modems()) {
        if (modem->status() == Modem::Status::roaming) {
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

    m_state = TypedVariant<std::map<std::string, Variant>>(state);
    Q_EMIT p.stateUpdated(m_state);
}

RootState::RootState(std::shared_ptr<connectivity::networking::Manager> manager, ModemManager::Ptr modemManager)
    : d{new Private(*this, manager, modemManager)}
{
}

RootState::~RootState()
{}

const Variant &
RootState::state() const
{
    return d->m_state;
}

#include "root-state.moc"
