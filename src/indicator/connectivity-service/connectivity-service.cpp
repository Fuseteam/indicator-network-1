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

#include <connectivity-service/connectivity-service.h>
#include <NetworkingStatusAdaptor.h>
#include <NetworkingStatusPrivateAdaptor.h>
#include <dbus-types.h>

using namespace nmofono;
using namespace std;

namespace connectivity_service
{

class ConnectivityService::Private : public QObject
{
    Q_OBJECT
public:
    ConnectivityService& p;

    QDBusConnection m_connection;

    Manager::Ptr m_manager;

    shared_ptr<PrivateService> m_privateService;

    QStringList m_limitations;

    QString m_status;

    Private(ConnectivityService& parent, const QDBusConnection& connection) :
        p(parent), m_connection(connection)
    {
    }

    void notifyPropertyChanged( const QString& path,
                                const QString& interface,
                                const QStringList& propertyNames )
    {
        QDBusMessage signal = QDBusMessage::createSignal(
            path,
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged");
        signal << interface;
        QVariantMap changedProps;
        for(const auto& propertyName: propertyNames)
        {
            changedProps.insert(propertyName, p.property(qPrintable(propertyName)));
        }
        signal << changedProps;
        signal << QStringList();
        m_connection.send(signal);
    }

public Q_SLOTS:
    void flightModeUpdated()
    {
        notifyPropertyChanged(DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              { "FlightMode" });
    }

    void wifiEnabledUpdated()
    {
        notifyPropertyChanged(DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              { "WifiEnabled" });
    }

    void unstoppableOperationHappeningUpdated()
    {
        notifyPropertyChanged(DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              { "UnstoppableOperationHappening" });
    }

    void updateNetworkingStatus()
    {
        QStringList changed;

        QStringList old_limitations = m_limitations;
        QString old_status = m_status;

        switch (m_manager->status())
        {
            case Manager::NetworkingStatus::offline:
                m_status = "offline";
                break;
            case Manager::NetworkingStatus::connecting:
                m_status = "connecting";
                break;
            case Manager::NetworkingStatus::online:
                m_status = "online";
        }
        if (old_status != m_status)
        {
            changed << "Status";
        }

        QStringList limitations;
        auto characteristics = m_manager->characteristics();
        if ((characteristics
                & Link::Characteristics::is_bandwidth_limited) != 0)
        {
            // FIXME KNOWN TYPO
            limitations.push_back("bandwith");
        }
        m_limitations = limitations;
        if (old_limitations != m_limitations)
        {
            changed << "Limitations";
        }

        if (!changed.empty())
        {
            notifyPropertyChanged(DBusTypes::SERVICE_PATH,
                                  DBusTypes::SERVICE_INTERFACE,
                                  changed);
        }
    }
};

ConnectivityService::ConnectivityService(nmofono::Manager::Ptr manager, const QDBusConnection& connection)
    : d{new Private(*this, connection)}
{
    d->m_manager = manager;
    d->m_privateService = make_shared<PrivateService>(*this);

    // Memory is managed by Qt parent ownership
    new NetworkingStatusAdaptor(this);

    connect(d->m_manager.get(), &Manager::characteristicsUpdated, d.get(), &Private::updateNetworkingStatus);
    connect(d->m_manager.get(), &Manager::statusUpdated, d.get(), &Private::updateNetworkingStatus);
    connect(d->m_manager.get(), &Manager::flightModeUpdated, d.get(), &Private::flightModeUpdated);
    connect(d->m_manager.get(), &Manager::wifiEnabledUpdated, d.get(), &Private::wifiEnabledUpdated);
    connect(d->m_manager.get(), &Manager::unstoppableOperationHappeningUpdated, d.get(), &Private::unstoppableOperationHappeningUpdated);

    d->updateNetworkingStatus();

    if (!d->m_connection.registerObject(DBusTypes::SERVICE_PATH, this))
    {
        throw logic_error(
                "Unable to register NetworkingStatus object on DBus");
    }
    if (!d->m_connection.registerObject(DBusTypes::PRIVATE_PATH, d->m_privateService.get()))
    {
        throw logic_error(
                "Unable to register NetworkingStatus private object on DBus");
    }
    if (!d->m_connection.registerService(DBusTypes::DBUS_NAME))
    {
        throw logic_error(
                "Unable to register Connectivity service on DBus");
    }
}

ConnectivityService::~ConnectivityService()
{
    d->m_connection.unregisterService(DBusTypes::DBUS_NAME);
}

QStringList ConnectivityService::limitations() const
{
    return d->m_limitations;
}

QString ConnectivityService::status() const
{
    return d->m_status;
}

bool ConnectivityService::wifiEnabled() const
{
    return d->m_manager->wifiEnabled();
}

bool ConnectivityService::flightMode() const
{
    return (d->m_manager->flightMode() == Manager::FlightModeStatus::on);
}

bool ConnectivityService::unstoppableOperationHappening() const
{
    return d->m_manager->unstoppableOperationHappening();
}

PrivateService::PrivateService(ConnectivityService& parent) :
        p(parent)
{
    // Memory is managed by Qt parent ownership
    new PrivateAdaptor(this);
}

void PrivateService::UnlockAllModems()
{
    Q_EMIT p.unlockAllModems();
}

void PrivateService::UnlockModem(const QString &modem)
{
    Q_EMIT p.unlockModem(modem);
}

void PrivateService::SetFlightMode(bool enabled)
{
    p.d->m_manager->setFlightMode(enabled);
}

void PrivateService::SetWifiEnabled(bool enabled)
{
    p.d->m_manager->setWifiEnabled(enabled);
}

}

#include "connectivity-service.moc"
