/*
 * Copyright © 2016 Canonical Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <nmofono/connection/available-connection.h>
#include <nmofono/ethernet/ethernet-link.h>
#include <util/dbus-property-cache.h>
#include <util/localisation.h>

#include <NetworkManagerDeviceWiredInterface.h>

#include <NetworkManager.h>
#include <iostream>

using namespace std;
using namespace nmofono;
using namespace nmofono::connection;

namespace nmofono
{
namespace ethernet
{

struct EthernetLink::Private : public QObject
{
Q_OBJECT

public:
    Private(EthernetLink& parent) :
            p(parent)
    {
    }

public Q_SLOTS:
    void
    updateStatus(uint new_state, uint, uint)
    {
        Status status = Status::disabled;

        switch (new_state)
        {
            case NM_DEVICE_STATE_DISCONNECTED:
            case NM_DEVICE_STATE_DEACTIVATING:
            case NM_DEVICE_STATE_UNMANAGED:
            case NM_DEVICE_STATE_UNAVAILABLE:
                status = Status::offline;
                break;
            case NM_DEVICE_STATE_UNKNOWN:
            case NM_DEVICE_STATE_FAILED:
                status = Status::failed;
                break;
            case NM_DEVICE_STATE_PREPARE:
            case NM_DEVICE_STATE_CONFIG:
            case NM_DEVICE_STATE_NEED_AUTH:
            case NM_DEVICE_STATE_IP_CONFIG:
            case NM_DEVICE_STATE_IP_CHECK:
                status = Status::connecting;
                break;
            case NM_DEVICE_STATE_SECONDARIES:
            case NM_DEVICE_STATE_ACTIVATED:
                status = Status::connected;
                break;
        }

        setStatus(status);

        updateActiveConnection();
    }

    void
    updateAvailableConnections()
    {
        auto availableConnectionPaths = m_dev->availableConnections().toSet();
        auto current(m_availableConnections.keys().toSet());

        auto toRemove(current);
        toRemove.subtract(availableConnectionPaths);

        auto toAdd(availableConnectionPaths);
        toAdd.subtract(current);

        if (toAdd.isEmpty() && toRemove.isEmpty())
        {
            return;
        }

        for (auto connectionPath : toRemove)
        {
            m_availableConnections.take(connectionPath);
        }

        for (auto connectionPath : toAdd)
        {
            m_availableConnections[connectionPath] = make_shared<AvailableConnection>(connectionPath, m_dev->connection());
        }

        if (m_preferredConnection)
        {
            if (!m_availableConnections.contains(m_preferredConnection->path()))
            {
                setPreferredConnection(AvailableConnection::SPtr());
            }
        }

        Q_EMIT p.availableConnectionsChanged();
    }

    void
    updateActiveConnection()
    {
        auto activeConnectionPath = m_dev->activeConnection();
        auto activeConnection = m_connectionManager->connection(activeConnectionPath);

        bool foundActiveConnection = false;
        if (activeConnection)
        {
            for (QMapIterator<QDBusObjectPath, AvailableConnection::SPtr> it(m_availableConnections); it.hasNext();)
            {
                it.next();

                auto availableConnection(it.value());
                if (availableConnection->connectionUuid() == activeConnection->uuid())
                {
                    setPreferredConnection(availableConnection);
                    foundActiveConnection = true;
                }
            }
        }

        // If we couldn't find an active connection, we'll have to fall back to the first available one.
        if (!m_preferredConnection && !foundActiveConnection && !m_availableConnections.isEmpty())
        {
            setPreferredConnection(m_availableConnections.first());
        }
    }

    void
    setPreferredConnection(AvailableConnection::SPtr availableConnection)
    {
        if (m_preferredConnection == availableConnection)
        {
            return;
        }

        m_preferredConnection = availableConnection;
        Q_EMIT p.preferredConnectionChanged(m_preferredConnection);
    }

    void
    setName(const QString& name)
    {
        if (m_name == name)
        {
            return;
        }

        m_name = name;
        Q_EMIT p.nameUpdated(m_name);
    }

    void
    setStatus(Status status)
    {
        if (m_status == status)
        {
            return;
        }

        m_status = status;
        Q_EMIT p.statusUpdated(m_status);
    }

    void
    setAutoConnect(bool autoConnect)
    {
        if (m_autoConnect == autoConnect)
        {
            return;
        }

        m_autoConnect = autoConnect;
        Q_EMIT p.autoConnectChanged(m_autoConnect);
    }

    void
    devicePropertyChanged(const QString& name, const QVariant& value)
    {
        if (name == "Autoconnect")
        {
            setAutoConnect(value.toBool());
        }
    }

public:
    EthernetLink& p;

    shared_ptr<OrgFreedesktopNetworkManagerDeviceInterface> m_dev;

    shared_ptr<OrgFreedesktopNetworkManagerInterface> m_nm;

    shared_ptr<OrgFreedesktopNetworkManagerSettingsInterface> m_settings;

    shared_ptr<OrgFreedesktopNetworkManagerDeviceWiredInterface> m_wired;

    shared_ptr<util::DBusPropertyCache> m_devicePropertyCache;

    ActiveConnectionManager::SPtr m_connectionManager;

    QMap<QDBusObjectPath, AvailableConnection::SPtr> m_availableConnections;

    AvailableConnection::SPtr m_preferredConnection;

    QString m_name;

    Id m_id = 0;

    Status m_status = Status::disabled;

    bool m_autoConnect = false;
};

EthernetLink::EthernetLink(shared_ptr<OrgFreedesktopNetworkManagerDeviceInterface> dev, shared_ptr<OrgFreedesktopNetworkManagerInterface> nm, shared_ptr<OrgFreedesktopNetworkManagerSettingsInterface> settings, ActiveConnectionManager::SPtr connectionManager) :
        d(new Private(*this))
{
    d->m_dev = dev;
    d->m_nm = nm;
    d->m_settings = settings;
    d->m_connectionManager = connectionManager;
    d->m_devicePropertyCache = make_shared<util::DBusPropertyCache>(NM_DBUS_SERVICE, NM_DBUS_INTERFACE_DEVICE, dev->path(), dev->connection());
    connect(d->m_devicePropertyCache.get(), &util::DBusPropertyCache::propertyChanged, d.get(), &Private::devicePropertyChanged);

    d->m_wired = make_shared<OrgFreedesktopNetworkManagerDeviceWiredInterface>(NM_DBUS_SERVICE, dev->path(), dev->connection());

    // This regular expression extracts the number from the end of the DBus path
    static QRegularExpression re("^[^\\d]+(\\d+)$");
    auto match = re.match(d->m_dev->path());
    d->m_id = match.captured(1).toUInt();
    d->setName(d->m_dev->interface());

    connect(d->m_dev.get(), &OrgFreedesktopNetworkManagerDeviceInterface::StateChanged, d.get(), &Private::updateStatus);
    connect(d->m_connectionManager.get(), &ActiveConnectionManager::connectionsChanged, d.get(), &Private::updateActiveConnection);
    connect(d->m_settings.get(), &OrgFreedesktopNetworkManagerSettingsInterface::ConnectionRemoved, d.get(), &Private::updateAvailableConnections);
    connect(d->m_settings.get(), &OrgFreedesktopNetworkManagerSettingsInterface::NewConnection, d.get(), &Private::updateAvailableConnections);

    d->updateAvailableConnections();
    d->updateStatus(d->m_dev->state(), 0, 0);
    d->setAutoConnect(d->m_devicePropertyCache->get("Autoconnect").toBool());
}

EthernetLink::~EthernetLink()
{
}

Link::Type
EthernetLink::type() const
{
    return Type::ethernet;
}

uint32_t
EthernetLink::characteristics() const
{
    return Characteristics::empty;
}

Link::Status
EthernetLink::status() const
{
    return d->m_status;
}

Link::Id
EthernetLink::id() const
{
    return d->m_id;
}

QString
EthernetLink::name() const
{
    return d->m_name;
}

QList<AvailableConnection::SPtr>
EthernetLink::availableConnections() const
{
    return d->m_availableConnections.values();
}

AvailableConnection::SPtr
EthernetLink::preferredConnection() const
{
    return d->m_preferredConnection;
}

bool
EthernetLink::autoConnect() const
{
    return d->m_autoConnect;
}

void
EthernetLink::setAutoConnect(bool autoConnect)
{
    if (autoConnect)
    {
        if (d->m_preferredConnection)
        {
            auto reply = d->m_nm->ActivateConnection(d->m_preferredConnection->path(), QDBusObjectPath(d->m_dev->path()), QDBusObjectPath("/"));
            reply.waitForFinished();
            if (reply.isError())
            {
                qWarning() << reply.error().message();
            }
        }
        d->m_dev->setAutoconnect(true);
    }
    else
    {
        d->m_dev->setAutoconnect(false);
        auto reply = d->m_dev->Disconnect();
        reply.waitForFinished();
        if (reply.isError())
        {
            qWarning() << reply.error().message();
        }
    }

    Q_EMIT autoConnectChanged(autoConnect);
}

void
EthernetLink::setPreferredConnection(AvailableConnection::SPtr preferredConnection)
{
    if (d->m_preferredConnection == preferredConnection)
    {
        return;
    }

    d->m_preferredConnection = preferredConnection;

    if (d->m_autoConnect && preferredConnection)
    {
        auto reply = d->m_nm->ActivateConnection(d->m_preferredConnection->path(), QDBusObjectPath(d->m_dev->path()), QDBusObjectPath("/"));
        reply.waitForFinished();
        if (reply.isError())
        {
            qWarning() << reply.error().message();
        }
    }

    Q_EMIT preferredConnectionChanged(preferredConnection);
}

QDBusObjectPath
EthernetLink::devicePath() const
{
    return QDBusObjectPath(d->m_dev->path());
}

}
}

#include "ethernet-link.moc"
