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

#define slots
#include <qofono-qt5/qofonomanager.h>
#include <qofono-qt5/qofonomodem.h>
#undef slots

#include <notify-cpp/snapdecision/sim-unlock.h>
#include <sim-unlock-dialog.h>
#include <util/qhash-sharedptr.h>

#include <QMap>
#include <QList>
#include <NetworkManager.h>
#include <QDebug>
#include <algorithm>
#include <iostream>

using namespace std;

namespace nmofono {

class ManagerImpl::Private : public QObject
{
    Q_OBJECT
public:
    Manager& p;

    shared_ptr<OrgFreedesktopNetworkManagerInterface> nm;
    shared_ptr<QOfonoManager> m_ofono;

    Manager::FlightModeStatus m_flightMode = FlightModeStatus::on;
    bool m_unstoppableOperationHappening = false;
    Manager::NetworkingStatus m_status = NetworkingStatus::offline;
    uint32_t m_characteristics = 0;

    bool m_hasWifi = false;
    bool m_wifiEnabled = false;
    KillSwitch::Ptr m_killSwitch;

    QSet<Link::Ptr> m_nmLinks;
    QMap<QString, wwan::Modem::Ptr> m_ofonoLinks;

    SimUnlockDialog::Ptr m_unlockDialog;
    QList<wwan::Modem::Ptr> m_pendingUnlocks;

    Private(Manager& parent) :
        p(parent)
    {
    }

    void setUnstoppableOperationHappening(bool happening)
    {
        if (m_unstoppableOperationHappening == happening)
        {
            return;
        }

        m_unstoppableOperationHappening = happening;
        Q_EMIT p.unstoppableOperationHappeningUpdated(m_unstoppableOperationHappening);
    }

public Q_SLOTS:
    void updateHasWifi()
    {
        if (m_killSwitch->state() != KillSwitch::State::not_available)
        {
            m_hasWifi = true;
            if (m_killSwitch->state() == KillSwitch::State::unblocked)
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
        for (auto link : m_nmLinks)
        {
            if (link->type() == Link::Type::wifi)
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
        FlightModeStatus newStatus =
                flightMode ? FlightModeStatus::on : FlightModeStatus::off;

        if (m_flightMode == newStatus)
        {
            return;
        }

        m_flightMode = newStatus;
        Q_EMIT p.flightModeUpdated(m_flightMode);
    }

    void sim_unlock_ready()
    {
        if (!m_pendingUnlocks.empty())
        {
            auto modem = m_pendingUnlocks.front();
            m_pendingUnlocks.pop_front();
            if (modem->requiredPin() != wwan::Modem::PinType::none)
            {
                m_unlockDialog->unlock(modem);
            }
        }
    }

    void modems_changed(const QStringList& value)
    {
        QSet<QString> modemPaths(value.toSet());
        QSet<QString> currentModemPaths(m_ofonoLinks.keys().toSet());

        auto toRemove = currentModemPaths;
        toRemove.subtract(modemPaths);

        auto toAdd = modemPaths;
        toAdd.subtract(currentModemPaths);

        for (const auto& path : toRemove)
        {
            auto modem = m_ofonoLinks.take(path);
            if (m_pendingUnlocks.contains(modem))
            {
                m_unlockDialog->cancel();
                m_pendingUnlocks.removeOne(modem);
            }
        }

        for (const auto& path : toAdd)
        {
            auto modemInterface = make_shared<QOfonoModem>();
            modemInterface->setModemPath(path);

            auto modem = make_shared<wwan::Modem>(modemInterface);
            m_ofonoLinks[path] = modem;
        }

        Q_EMIT p.linksUpdated();
        m_unlockDialog->setShowSimIdentifiers(m_ofonoLinks.size() > 1);
    }
};

void
ManagerImpl::updateNetworkingStatus(uint status)
{
    switch(status) {
    case NM_STATE_UNKNOWN:
    case NM_STATE_ASLEEP:
    case NM_STATE_DISCONNECTED:
    case NM_STATE_DISCONNECTING:
    {
        d->m_status = NetworkingStatus::offline;
        break;
    }
    case NM_STATE_CONNECTING:
    {
        d->m_status = NetworkingStatus::connecting;
        break;
    }
    case NM_STATE_CONNECTED_LOCAL:
    case NM_STATE_CONNECTED_SITE:
    case NM_STATE_CONNECTED_GLOBAL:
    {
        d->m_status = NetworkingStatus::online;
        break;
    }
    }

    Q_EMIT statusUpdated(d->m_status);
}

ManagerImpl::ManagerImpl(const QDBusConnection& systemConnection) : d(new ManagerImpl::Private(*this))
{
    d->nm = make_shared<OrgFreedesktopNetworkManagerInterface>(NM_DBUS_SERVICE, NM_DBUS_PATH, systemConnection);

    d->m_unlockDialog = make_shared<SimUnlockDialog>();
    connect(d->m_unlockDialog.get(), &SimUnlockDialog::ready, d.get(), &Private::sim_unlock_ready);

    d->m_ofono = make_shared<QOfonoManager>();
    connect(d->m_ofono.get(), &QOfonoManager::modemsChanged, d.get(), &Private::modems_changed);
    d->modems_changed(d->m_ofono->modems());

    /// @todo add a watcher for the service
    /// @todo exceptions
    /// @todo offload the initialization to a thread or something
    /// @todo those Id() thingies

    d->m_killSwitch = make_shared<KillSwitch>(systemConnection);
    connect(d->m_killSwitch.get(), &KillSwitch::stateChanged, d.get(), &Private::updateHasWifi);

    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::DeviceAdded, this, &ManagerImpl::device_added);
    QList<QDBusObjectPath> devices(d->nm->GetDevices());
    for(const auto &path : devices) {
        device_added(path);
    }

    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::DeviceRemoved, this, &ManagerImpl::device_removed);
    updateNetworkingStatus(d->nm->state());
    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::PropertiesChanged, this, &ManagerImpl::nm_properties_changed);

    connect(d->m_killSwitch.get(), &KillSwitch::flightModeChanged, d.get(), &Private::setFlightMode);
    try
    {
        d->setFlightMode(d->m_killSwitch->isFlightMode());
    }
    catch (exception const& e)
    {
        cerr << __PRETTY_FUNCTION__ << ": " << e.what() << endl;
        cerr << "Failed to retrieve initial flight mode state, assuming state is false." << endl;
        d->setFlightMode(false);
    }

    /// @todo set by the default connections.
    d->m_characteristics = Link::Characteristics::empty;

    d->updateHasWifi();
}

void
ManagerImpl::nm_properties_changed(const QVariantMap &properties)
{
    auto stateIt = properties.find("State");
    if (stateIt != properties.cend())
    {
        updateNetworkingStatus(stateIt->toUInt());
    }
}

void
ManagerImpl::device_removed(const QDBusObjectPath &path)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        qDebug() << "Device Removed:" << path.path();
#endif
    Link::Ptr toRemove;
    for (auto dev : d->m_nmLinks)
    {
        auto wifiLink = dynamic_pointer_cast<wifi::WifiLinkImpl>(dev);
        if (wifiLink && wifiLink->device_path() == path)
        {
            toRemove = dev;
            break;
        }
    }
    if (toRemove)
    {
        d->m_nmLinks.remove(toRemove);
        Q_EMIT linksUpdated();
        d->updateHasWifi();
    }
}

void
ManagerImpl::device_added(const QDBusObjectPath &path)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << "Device Added:" << path.path();
#endif
    for (const auto &dev : d->m_nmLinks)
    {
        auto wifiLink = dynamic_pointer_cast<wifi::WifiLinkImpl>(dev);
        if (wifiLink && wifiLink->device_path() == path) {
            // already in the list
            return;
        }
    }

    Link::Ptr link;
    try {
        auto dev = make_shared<OrgFreedesktopNetworkManagerDeviceInterface>(
            NM_DBUS_SERVICE, path.path(), d->nm->connection());
        if (dev->deviceType() == NM_DEVICE_TYPE_WIFI) {
            link = make_shared<wifi::WifiLinkImpl>(dev,
                                                d->nm,
                                                d->m_killSwitch);
        }
    } catch (const exception &e) {
        qDebug() << __PRETTY_FUNCTION__ << ": failed to create Device proxy for "<< path.path() << ": ";
        qDebug() << "\t" << e.what();
        qDebug() << "\tIgnoring.";
        return;
    }

    if (link) {
        d->m_nmLinks.insert(link);
        Q_EMIT linksUpdated();
    }

    d->updateHasWifi();
}


void
ManagerImpl::setFlightMode(bool enabled)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << __PRETTY_FUNCTION__ << enabled;
#endif
    if (enabled == d->m_killSwitch->isFlightMode())
    {
        return;
    }

    d->setUnstoppableOperationHappening(true);

    if (!d->m_killSwitch->flightMode(enabled))
    {
        qWarning() << "Failed to change flightmode.";
    }
    d->setUnstoppableOperationHappening(false);
}

Manager::FlightModeStatus
ManagerImpl::flightMode() const
{
    // - connect to each individual URfkill.Killswitch interface
    // - make this property to reflect their combined state
    /// @todo implement flightmode status properly when URfkill gets the flightmode API
    return d->m_flightMode;
}

bool
ManagerImpl::unstoppableOperationHappening() const
{
    return d->m_unstoppableOperationHappening;
}

QSet<Link::Ptr>
ManagerImpl::links() const
{
    QSet<Link::Ptr> result(d->m_nmLinks);
    for(auto i: d->m_ofonoLinks)
    {
        result.insert(i);
    }
    return result;
}

Manager::NetworkingStatus
ManagerImpl::status() const
{
    return d->m_status;
}

uint32_t
ManagerImpl::characteristics() const
{
    return d->m_characteristics;
}

bool
ManagerImpl::hasWifi() const
{
    return d->m_hasWifi;
}

bool
ManagerImpl::wifiEnabled() const
{
    return d->m_wifiEnabled;
}


bool
ManagerImpl::setWifiEnabled(bool enabled)
{
    if (!d->m_hasWifi)
    {
        return false;
    }

    if (d->m_wifiEnabled == enabled)
    {
        return false;
    }

    bool success = true;
    d->setUnstoppableOperationHappening(true);

    try
    {
        if (enabled)
        {
            if (d->m_killSwitch->state() == KillSwitch::State::soft_blocked)
            {
                // try to unblock. throws if fails.
                d->m_killSwitch->unblock();
            }
        }
        else
        {
            if (d->m_killSwitch->state() == KillSwitch::State::unblocked) {
                // block the device. that will disable it also
                d->m_killSwitch->block();
            }
        }
        d->nm->setWirelessEnabled(enabled);
    }
    catch (runtime_error &e)
    {
        qWarning() << __PRETTY_FUNCTION__ << ": " << e.what();
        success = false;
    }
    d->setUnstoppableOperationHappening(false);
    return success;
}

bool
ManagerImpl::roaming() const
{
    for (auto modem : d->m_ofonoLinks) {
        if (modem->modemStatus() == wwan::Modem::ModemStatus::roaming) {
            return true;
        }
    }

    return false;
}

void
ManagerImpl::unlockModem(wwan::Modem::Ptr modem)
{
    try {
        if (!d->m_ofonoLinks.values().contains(modem)
                || d->m_unlockDialog->modem() == modem
                || count(d->m_pendingUnlocks.begin(), d->m_pendingUnlocks.end(), modem) != 0)
            return;

        if (d->m_unlockDialog->state() == SimUnlockDialog::State::ready
                && d->m_pendingUnlocks.size() == 0)
            d->m_unlockDialog->unlock(modem);
        else
            d->m_pendingUnlocks.push_back(modem);
    } catch(const exception &e) {
        // Something unexpected has happened. As an example, unity8 might have
        // crashed taking the notification server with it. There is no graceful
        // and reliable way to recover so die and get restarted.
        // See also https://bugs.launchpad.net/unity-notifications/+bug/1238990
        qWarning() << __PRETTY_FUNCTION__ << " sim unlocking failed: " << QString::fromStdString(e.what());
    }
}

void
ManagerImpl::unlockAllModems()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << __PRETTY_FUNCTION__;
#endif
    for (auto& m : d->m_ofonoLinks)
    {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        qDebug() << "Unlocking " << m->simIdentifier();
#endif
        unlockModem(m);
    }
}

void
ManagerImpl::unlockModemByName(const QString &name)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << __PRETTY_FUNCTION__ ;
#endif
    auto it = d->m_ofonoLinks.find(name);
    if (it != d->m_ofonoLinks.cend())
    {
        unlockModem(it.value());
    }
}


QSet<wifi::WifiLink::Ptr>
ManagerImpl::wifiLinks() const
{
    QSet<wifi::WifiLink::Ptr> result;
    for(auto link: d->m_nmLinks)
    {
        if (link->type() == Link::Type::wifi)
        {
            result.insert(dynamic_pointer_cast<wifi::WifiLink>(link));
        }
    }
    return result;
}

QSet<wwan::Modem::Ptr>
ManagerImpl::modemLinks() const
{
    return d->m_ofonoLinks.values().toSet();
}

}

#include "manager-impl.moc"
