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
#include <nmofono/wwan/sim-manager.h>
#include <NetworkManagerActiveConnectionInterface.h>
#include <NetworkManagerDeviceInterface.h>
#include <NetworkManagerInterface.h>
#include <NetworkManagerSettingsInterface.h>
#include <NetworkManagerSettingsConnectionInterface.h>

#define slots
#include <qofono-qt5/qofonomanager.h>
#include <qofono-qt5/qofonomodem.h>
#undef slots
#include <ofono/dbus.h>

#include <notify-cpp/notification-manager.h>
#include <notify-cpp/snapdecision/sim-unlock.h>
#include <sim-unlock-dialog.h>
#include <util/qhash-sharedptr.h>

#include <QMap>
#include <QList>
#include <QRegularExpression>
#include <QSettings>
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

    bool m_flightMode = true;
    bool m_unstoppableOperationHappening = false;
    Manager::NetworkingStatus m_status = NetworkingStatus::offline;
    uint32_t m_characteristics = 0;

    bool m_hasWifi = false;
    bool m_wifiEnabled = false;
    KillSwitch::Ptr m_killSwitch;

    bool m_modemAvailable = false;

    QSet<Link::Ptr> m_nmLinks;
    QMap<QString, wwan::Modem::Ptr> m_ofonoLinks;

    SimUnlockDialog::Ptr m_unlockDialog;
    QList<wwan::Modem::Ptr> m_pendingUnlocks;

    HotspotManager::SPtr m_hotspotManager;

    bool m_mobileDataEnabled = false;
    bool m_mobileDataEnabledPending = false;

    wwan::Sim::Ptr m_simForMobileData;
    bool m_simForMobileDataPending = false;

    QList<wwan::Modem::Ptr> m_modems;
    QList<wwan::Sim::Ptr> m_sims;

    wwan::SimManager::Ptr m_simManager;

    QSettings *m_settings;

    Private(Manager& parent) :
        p(parent)
    {
         m_simManager = make_shared<wwan::SimManager>();
         m_sims = m_simManager->knownSims();
         connect(m_simManager.get(), &wwan::SimManager::simAdded, this, &Private::simAdded);

         m_settings = new QSettings(QSettings::IniFormat,
                                    QSettings::UserScope,
                                    "Ubuntu",
                                    "connectivityservice",
                                    this);
        QVariant ret = m_settings->value("MobileDataEnabled");
        if (ret.isNull())
        {
            /* This is the first time we are running on a system.
             *
             * We need to figure out the status of mobile data from looking
             * at the individual modems.
             */
            m_mobileDataEnabledPending = true;
            m_settings->setValue("MobileDataEnabled", true);
        }
        else
        {
            m_mobileDataEnabled = ret.toBool();
        }

        ret = m_settings->value("SimForMobileData");
        if (ret.isNull())
        {
            /* This is the first time we are running on a system.
             *
             * We need to figure out the SIM used for mobile data
             * from the individual modems.
             */
            m_simForMobileDataPending = true;
            m_settings->setValue("SimForMobileData", QString());
        }
        else
        {
            QString imsi = ret.toString();
            wwan::Sim::Ptr sim;
            for(auto i = m_sims.begin(); i != m_sims.end(); i++)
            {
                if ((*i)->imsi() == imsi) {
                    sim = *i;
                    break;
                }
            }
            m_simForMobileData = sim;
        }

    }

    void matchModemsAndSims()
    {
        for (wwan::Modem::Ptr modem: m_modems)
        {
            bool match = false;
            for(wwan::Sim::Ptr sim : m_sims)
            {
                if (sim->ofonoPath() == modem->ofonoPath())
                {
                    match = true;
                    modem->setSim(sim);
                    break;
                }
            }
            if (!match)
            {
                modem->setSim(wwan::Sim::Ptr());
            }
        }
    }

    void simAdded(wwan::Sim::Ptr sim)
    {
        m_sims.append(sim);
        matchModemsAndSims();
        Q_EMIT p.simsChanged();
    }

    void modemReady()
    {
        wwan::Modem *modem_raw = qobject_cast<wwan::Modem*>(sender());
        if (!modem_raw)
        {
            qWarning() << "modem cast failed.";
            return;
        }
        m_modems.append(m_ofonoLinks[modem_raw->name()]);
        matchModemsAndSims();
        Q_EMIT p.modemsChanged();
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

    void updateModemAvailable()
    {
        bool modemAvailable = !m_ofonoLinks.empty();

        if (m_modemAvailable == modemAvailable)
        {
            return;
        }

        m_modemAvailable = modemAvailable;
        Q_EMIT p.modemAvailableChanged(m_modemAvailable);
    }

    void setFlightMode(bool newStatus)
    {
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

    void modemReadyToUnlock(const QString& name)
    {
        auto modem = m_ofonoLinks[name];
        if (modem)
        {
            p.unlockModem(modem);
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
                disconnect(modem.get(), &wwan::Modem::readyToUnlock, this, &Private::modemReadyToUnlock);
            }
            m_modems.removeAll(modem);
            Q_EMIT p.modemsChanged();
        }

        for (const auto& path : toAdd)
        {
            auto modemInterface = make_shared<QOfonoModem>();
            modemInterface->setModemPath(path);

            auto modem = make_shared<wwan::Modem>(modemInterface);
            m_ofonoLinks[path] = modem;
            connect(modem.get(), &wwan::Modem::readyToUnlock, this, &Private::modemReadyToUnlock);
            connect(modem.get(), &wwan::Modem::ready, this, &Private::modemReady);
        }

        Q_EMIT p.linksUpdated();
        m_unlockDialog->setShowSimIdentifiers(m_ofonoLinks.size() > 1);

        updateModemAvailable();
    }

    void setMobileDataEnabled(bool value) {
        if (m_mobileDataEnabled == value)
        {
            return;
        }

        m_settings->setValue("MobileDataEnabled", value);
        m_settings->sync();

        m_mobileDataEnabled = value;
        Q_EMIT p.mobileDataEnabledChanged(value);

        if (m_mobileDataEnabled)
        {
            for (wwan::Modem::Ptr modem : m_modems)
            {
                if (modem->sim() && modem->sim() == m_simForMobileData)
                {
                    modem->sim()->setMobileDataEnabled(true);
                }
            }
        }
        else
        {
            for (wwan::Modem::Ptr modem : m_modems)
            {
                if (modem->sim()) {
                    modem->sim()->setMobileDataEnabled(false);
                }
            }
        }
    }

    void setSimForMobileData(wwan::Sim::Ptr sim) {
        if (m_simForMobileData == sim)
        {
            return;
        }

        if (m_simForMobileData)
        {
            m_simForMobileData->setMobileDataEnabled(false);
        }

        if (!sim)
        {
            m_settings->setValue("SimForMobileData", "");
        }
        else
        {
            m_settings->setValue("SimForMobileData", sim->imsi());
        }
        m_settings->sync();

        m_simForMobileData = sim;
        if (m_simForMobileData)
        {
            m_simForMobileData->setMobileDataEnabled(p.mobileDataEnabled());
        }

        Q_EMIT p.simForMobileDataChanged();
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

ManagerImpl::ManagerImpl(notify::NotificationManager::SPtr notificationManager,
                         KillSwitch::Ptr killSwitch,
                         HotspotManager::SPtr hotspotManager,
                         const QDBusConnection& systemConnection) :
        d(new ManagerImpl::Private(*this))
{
    d->nm = make_shared<OrgFreedesktopNetworkManagerInterface>(NM_DBUS_SERVICE, NM_DBUS_PATH, systemConnection);

    d->m_unlockDialog = make_shared<SimUnlockDialog>(notificationManager);
    connect(d->m_unlockDialog.get(), &SimUnlockDialog::ready, d.get(), &Private::sim_unlock_ready);

    d->m_ofono = make_shared<QOfonoManager>();
    connect(d->m_ofono.get(), &QOfonoManager::modemsChanged, d.get(), &Private::modems_changed);
    d->modems_changed(d->m_ofono->modems());

    d->m_killSwitch = killSwitch;
    connect(d->m_killSwitch.get(), &KillSwitch::stateChanged, d.get(), &Private::updateHasWifi);

    d->m_hotspotManager = hotspotManager;
    connect(d->m_hotspotManager.get(), &HotspotManager::enabledChanged, this, &Manager::hotspotEnabledChanged);
    connect(d->m_hotspotManager.get(), &HotspotManager::ssidChanged, this, &Manager::hotspotSsidChanged);
    connect(d->m_hotspotManager.get(), &HotspotManager::passwordChanged, this, &Manager::hotspotPasswordChanged);
    connect(d->m_hotspotManager.get(), &HotspotManager::modeChanged, this, &Manager::hotspotModeChanged);
    connect(d->m_hotspotManager.get(), &HotspotManager::authChanged, this, &Manager::hotspotAuthChanged);
    connect(d->m_hotspotManager.get(), &HotspotManager::storedChanged, this, &Manager::hotspotStoredChanged);

    connect(d->m_hotspotManager.get(), &HotspotManager::reportError, this, &Manager::reportError);

    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::DeviceAdded, this, &ManagerImpl::device_added);
    QList<QDBusObjectPath> devices(d->nm->GetDevices());
    for(const auto &path : devices) {
        device_added(path);
    }

    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::DeviceRemoved, this, &ManagerImpl::device_removed);
    updateNetworkingStatus(d->nm->state());
    connect(d->nm.get(), &OrgFreedesktopNetworkManagerInterface::PropertiesChanged, this, &ManagerImpl::nm_properties_changed);

    connect(d->m_killSwitch.get(), &KillSwitch::flightModeChanged, d.get(), &Private::setFlightMode);
    d->setFlightMode(d->m_killSwitch->isFlightMode());

    /// @todo set by the default connections.
    d->m_characteristics = Link::Characteristics::empty;

    d->updateHasWifi();
}

bool
ManagerImpl::wifiEnabled() const
{
    return d->m_wifiEnabled;
}


void
ManagerImpl::setWifiEnabled(bool enabled)
{
    if (!d->m_hasWifi)
    {
        return;
    }

    if (d->m_wifiEnabled == enabled)
    {
        return;
    }

    d->setUnstoppableOperationHappening(true);
    // Disable hotspot before disabling WiFi
    if (!enabled && d->m_hotspotManager->enabled())
    {
        d->m_hotspotManager->setEnabled(false);
    }

    d->m_killSwitch->setBlock(!enabled);
    d->nm->setWirelessEnabled(enabled);
    d->setUnstoppableOperationHappening(false);
}

bool
ManagerImpl::modemAvailable() const
{
    return !d->m_ofonoLinks.empty();
}

bool
ManagerImpl::hotspotEnabled() const
{
    return d->m_hotspotManager->enabled();
}

void
ManagerImpl::setHotspotEnabled(bool enabled)
{
    if (d->m_hotspotManager->enabled() == enabled)
    {
        return;
    }

    if (enabled && d->m_flightMode)
    {
        qWarning() << __PRETTY_FUNCTION__ << "Cannot set hotspot enabled when flight mode is on";
        return;
    }

    d->setUnstoppableOperationHappening(true);

    if (enabled && !d->m_wifiEnabled)
    {
        d->m_killSwitch->setBlock(false);
        d->nm->setWirelessEnabled(true);
    }

    d->m_hotspotManager->setEnabled(enabled);
    d->setUnstoppableOperationHappening(false);
}

void
ManagerImpl::setFlightMode(bool enabled)
{
    if (enabled == d->m_killSwitch->isFlightMode())
    {
        return;
    }

    d->setUnstoppableOperationHappening(true);
    // Disable hotspot before enabling flight mode
    if (enabled && d->m_hotspotManager->enabled())
    {
        d->m_hotspotManager->setEnabled(false);
    }
    if (!d->m_killSwitch->flightMode(enabled))
    {
        qWarning() << "Failed to change flightmode.";
    }
    d->setUnstoppableOperationHappening(false);
}

bool
ManagerImpl::flightMode() const
{
    return d->m_flightMode;
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
            wifi::WifiLink::Ptr tmp = make_shared<wifi::WifiLinkImpl>(dev,
                                                d->nm,
                                                d->m_killSwitch);

            // We're not interested in showing access points
            if (tmp->name() != d->m_hotspotManager->interface())
            {
                tmp->setDisconnectWifi(d->m_hotspotManager->disconnectWifi());
                QObject::connect(d->m_hotspotManager.get(), &HotspotManager::disconnectWifiChanged,
                        tmp.get(), &wifi::WifiLink::setDisconnectWifi);

                link = tmp;
            }
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
                && (d->m_pendingUnlocks.size() == 0))
        {
            if (modem->isReadyToUnlock())
            {
                d->m_unlockDialog->unlock(modem);
            }
            else
            {
                modem->notifyWhenReadyToUnlock();
            }
        }
        else
        {
            d->m_pendingUnlocks.push_back(modem);
        }
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

bool
ManagerImpl::hotspotStored() const
{
    return d->m_hotspotManager->stored();
}

QByteArray
ManagerImpl::hotspotSsid() const
{
    return d->m_hotspotManager->ssid();
}

QString
ManagerImpl::hotspotPassword() const
{
    return d->m_hotspotManager->password();
}

QString
ManagerImpl::hotspotMode() const
{
    return d->m_hotspotManager->mode();
}

QString
ManagerImpl::hotspotAuth() const
{
    return d->m_hotspotManager->auth();
}

void
ManagerImpl::setHotspotSsid(const QByteArray& ssid)
{
    d->m_hotspotManager->setSsid(ssid);
}

void
ManagerImpl::setHotspotPassword(const QString& password)
{
    d->m_hotspotManager->setPassword(password);
}

void
ManagerImpl::setHotspotMode(const QString& mode)
{
    d->m_hotspotManager->setMode(mode);
}

void
ManagerImpl::setHotspotAuth(const QString& auth)
{
    d->m_hotspotManager->setAuth(auth);
}

bool
ManagerImpl::mobileDataEnabled() const
{
    return d->m_mobileDataEnabled;
}

void
ManagerImpl::setMobileDataEnabled(bool value)
{
    d->setMobileDataEnabled(value);
}

wwan::Sim::Ptr
ManagerImpl::simForMobileData() const
{
    return d->m_simForMobileData;
}

void
ManagerImpl::setSimForMobileData(wwan::Sim::Ptr sim)
{
    d->setSimForMobileData(sim);
}

QList<wwan::Modem::Ptr>
ManagerImpl::modems() const
{
    return d->m_modems;
}

QList<wwan::Sim::Ptr>
ManagerImpl::sims() const
{
    return d->m_sims;
}


}

#include "manager-impl.moc"
