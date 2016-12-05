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

#include <root-state.h>

#include <nmofono/wifi/wifi-link.h>
#include <nmofono/wifi/access-point.h>

#include <icons.h>
#include <util/localisation.h>

#include <functional>
#include <QDebug>

using namespace std;
using namespace nmofono;

class RootState::Private : public QObject
{
    Q_OBJECT

public:
    RootState& p;

    Manager::Ptr m_manager;
    Variant m_state;

    string m_label;

    QStringList m_networkingIcons;

    QMap<int, QString> m_cellularIcons;
    QMap<int, QString> m_modemTechIcons;

    int m_activeModem = -1;

    Private(RootState& parent, nmofono::Manager::Ptr manager);

    Variant createIcon(const string& name);

    void updateModems();

    void updateModem(const wwan::Modem& modem);

public Q_SLOTS:
    void updateNetworkingIcon();

    void updateRootState();
};

RootState::Private::Private(RootState& parent, nmofono::Manager::Ptr manager)
    : p{parent},
      m_manager{manager}
{
    connect(m_manager.get(), &nmofono::Manager::flightModeUpdated, this, &Private::updateRootState);

    connect(m_manager.get(), &nmofono::Manager::hotspotEnabledChanged, this, &Private::updateNetworkingIcon);
    connect(m_manager.get(), &Manager::statusUpdated, this, &Private::updateNetworkingIcon);
    connect(m_manager.get(), &Manager::linksUpdated, this, &Private::updateNetworkingIcon);

    connect(m_manager.get(), &nmofono::Manager::txChanged, this, &Private::updateNetworkingIcon);
    connect(m_manager.get(), &nmofono::Manager::rxChanged, this, &Private::updateNetworkingIcon);

    // will also call updateRootState()
    updateNetworkingIcon();
}

void
RootState::Private::updateModems()
{
    QMap<int, wwan::Modem::Ptr> modems;
    for (auto modem : m_manager->modemLinks())
    {
        modems[modem->index()] = modem;
    }

    QSet<int> current = m_cellularIcons.keys().toSet();
    QSet<int> updated = modems.keys().toSet();

    QSet<int> removed(current);
    removed.subtract(updated);

    QSet<int> added(updated);
    added.subtract(current);

    for (auto index : removed)
    {
        m_cellularIcons.remove(index);
    }

    for (auto index : added) {
        // modem properties and signals already synced with GMainLoop
        connect(modems[index].get(), &wwan::Modem::updated, this, &Private::updateNetworkingIcon);
    }

    m_activeModem = -1;
    for (auto modem : modems)
    {
        updateModem(*modem);
    }
}

void
RootState::Private::updateModem(const wwan::Modem& modem)
{
    int index = modem.index();

    QString newModemTechIcon;
    QString newCellularIcon;

    if (modem.online())
    {
        switch(modem.simStatus())
        {
        case wwan::Modem::SimStatus::missing:
            newCellularIcon = "no-simcard";
            break;
        case wwan::Modem::SimStatus::error:
            newCellularIcon = "simcard-error";
            break;
        case wwan::Modem::SimStatus::locked:
        case wwan::Modem::SimStatus::permanentlyLocked:
            newCellularIcon = "simcard-locked";
            break;
        case wwan::Modem::SimStatus::ready:
        {
            switch (modem.modemStatus())
            {
            case wwan::Modem::ModemStatus::unregistered:
            case wwan::Modem::ModemStatus::unknown:
            case wwan::Modem::ModemStatus::searching:
                newCellularIcon = "gsm-3g-disabled";
                break;
            case wwan::Modem::ModemStatus::denied:
                /// @todo we might need network-error for this
                newCellularIcon = "gsm-3g-disabled";
                break;
            case wwan::Modem::ModemStatus::registered:
            case wwan::Modem::ModemStatus::roaming:
                if (modem.strength() != 0) {
                    newCellularIcon = Icons::strengthIcon(modem.strength());
                    newModemTechIcon = Icons::bearerIcon(modem.bearer());
                } else {
                    newCellularIcon = "gsm-3g-no-service";
                }
                break;
            }
            break;
        }
        case wwan::Modem::SimStatus::not_available:
            newCellularIcon = "no-simcard";
            break;
        }
    }

    m_cellularIcons[index] = newCellularIcon;
    m_modemTechIcons[index] = newModemTechIcon;

    if (modem.dataEnabled())
    {
        m_activeModem = index;
    }
}

void
RootState::Private::updateNetworkingIcon()
{
    m_networkingIcons.clear();

    updateModems();

    switch (m_manager->status()) {
    case Manager::NetworkingStatus::offline:
        m_networkingIcons << "nm-no-connection";
        //a11ydesc = _("Network (none)");
        break;
    case Manager::NetworkingStatus::connecting:
        m_networkingIcons << "nm-no-connection";
        // some sort of connection animation
        break;
    case Manager::NetworkingStatus::online:
        multimap<Link::Id, ethernet::EthernetLink::SPtr> sortedEthernetLinks;
        for (auto ethernetLink : m_manager->ethernetLinks())
        {
            sortedEthernetLinks.insert(make_pair(ethernetLink->id(), ethernetLink));
        }
        for (auto pair : sortedEthernetLinks)
        {
            auto ethernetLink = pair.second;

            connect(ethernetLink.get(), &ethernet::EthernetLink::statusUpdated, this,
                    &Private::updateNetworkingIcon, Qt::UniqueConnection);

            auto status = ethernetLink->status();
            m_networkingIcons << Icons::ethernetIcon(status);
        }

        for (auto wifiLink : m_manager->wifiLinks())
        {
            connect(wifiLink.get(), &wifi::WifiLink::statusUpdated, this,
                    &Private::updateNetworkingIcon, Qt::UniqueConnection);
            connect(wifiLink.get(), &wifi::WifiLink::signalUpdated, this,
                    &Private::updateNetworkingIcon, Qt::UniqueConnection);

            if (wifiLink->status() != Link::Status::online
                    && wifiLink->status() != Link::Status::connected)
            {
                continue;
            }

            auto signal = wifiLink->signal();
            if (signal != wifi::WifiLink::Signal::disconnected)
            {
                m_networkingIcons << Icons::wifiIcon(signal);
            }
        }

        if (m_manager->rx() && m_manager->tx())
        {
            m_networkingIcons << "transfer-progress";
        }
        else
        {
            if (m_manager->rx()){
                m_networkingIcons << "transfer-progress-download";
            }

            if (m_manager->tx()){
                m_networkingIcons << "transfer-progress-upload";
            }
        }

        // Splat WiFi icons if we are using the hotspot
        if (m_networkingIcons.isEmpty() || m_manager->hotspotEnabled())
        {
            m_networkingIcons.clear();

            if (m_activeModem != -1) {
                auto it = m_modemTechIcons.find(m_activeModem);
                if (it != m_modemTechIcons.end())
                {
                    m_networkingIcons << it.value();
                }
            }
        }

        if (m_manager->hotspotEnabled())
        {
            m_networkingIcons << "hotspot-active";
        }
        break;
    }

    updateRootState();
}

Variant
RootState::Private::createIcon(const string& name)
{
    GError *error = nullptr;
    auto gicon = shared_ptr<GIcon>(g_icon_new_for_string(name.c_str(), &error), GObjectDeleter());
    if (error) {
        string message(error->message);
        g_error_free(error);
        throw runtime_error("Could not create GIcon: " + message);
    }

    Variant ret = Variant::fromGVariant(g_icon_serialize(gicon.get()));
    return ret;
}

void
RootState::Private::updateRootState()
{
    vector<string> icons;
    map<string, Variant> state;

    if(m_manager->flightMode())
    {
        icons.push_back("airplane-mode");
    }

    multimap<int, QString, wwan::WwanLink::Compare> sorted;
    QMapIterator<int, QString> iconIt(m_cellularIcons);
    while (iconIt.hasNext()) {
        iconIt.next();
        sorted.insert(make_pair(iconIt.key(), iconIt.value()));
    }

    for (auto pair : sorted)
    {
        if (!pair.second.isEmpty())
        {
            icons.push_back(pair.second.toStdString());
        }
    }

    if (m_manager->roaming())
    {
        icons.push_back("network-cellular-roaming");
    }

    if (!m_networkingIcons.isEmpty()) {
        /* We're doing icon always right now so we have a fallback before everyone
           supports multi-icon.  We shouldn't set both in the future. */
        try {
            state["icon"] = createIcon(m_networkingIcons.first().toStdString());
        } catch (exception &e) {
            qWarning() << e.what();
        }

        for (const auto& icon: m_networkingIcons)
        {
            icons.push_back(icon.toStdString());
        }
    }

    if (!m_label.empty())
    {
        state["label"] = TypedVariant<string>(m_label);
    }

    // TRANSLATORS: this is the indicator title shown on the top header of the indicator area
    state["title"] = TypedVariant<string>(_("Network"));

    /// @todo state["accessibility-desc"] = TypedVariant<string>(a11ydesc);
    state["visible"] = TypedVariant<bool>(true); /// @todo is this really necessary/useful?


    if (!icons.empty()) {
        vector<Variant> iconVariants;
        for (auto name : icons) {
            try {
                iconVariants.push_back(createIcon(name));
            } catch (exception &e) {
                cerr << e.what();
            }
        }
        state["icons"] = TypedVariant<vector<Variant>>(iconVariants);
    }

    TypedVariant<map<string, Variant>> new_state(state);
    if (m_state == new_state)
    {
        return;
    }

    m_state = new_state;
    Q_EMIT p.stateUpdated(m_state);
}

RootState::RootState(Manager::Ptr manager)
    : d{new Private(*this, manager)}
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
