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

#pragma once

#include <nmofono/link.h>
#include <nmofono/connection/active-connection-manager.h>
#include <nmofono/connection/available-connection.h>
#include <NetworkManagerInterface.h>
#include <NetworkManagerSettingsInterface.h>
#include <NetworkManagerDeviceInterface.h>

#include <unity/util/DefinesPtrs.h>

#include <QSet>

namespace nmofono {
namespace ethernet {

#ifndef CONNECTIVITY_CPP_EXPORT
#define CONNECTIVITY_CPP_EXPORT __attribute ((visibility ("default")))
#endif

class CONNECTIVITY_CPP_EXPORT
EthernetLink : public Link
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(EthernetLink);

    EthernetLink(std::shared_ptr<OrgFreedesktopNetworkManagerDeviceInterface> dev,
                 std::shared_ptr<OrgFreedesktopNetworkManagerInterface> nm,
                 std::shared_ptr<OrgFreedesktopNetworkManagerSettingsInterface> settings,
                 nmofono::connection::ActiveConnectionManager::SPtr connectionManager);

    EthernetLink() = delete;
    EthernetLink(const EthernetLink&) = delete;
    ~EthernetLink();

    Type type() const;

    std::uint32_t characteristics() const;

    Status status() const;

    Id id() const;

    QString name() const;

    QList<connection::AvailableConnection::SPtr> availableConnections() const;

    connection::AvailableConnection::SPtr preferredConnection() const;

    bool autoConnect() const;

    QDBusObjectPath devicePath() const;

public Q_SLOTS:
    void setPreferredConnection(connection::AvailableConnection::SPtr availableConnection);

    void setAutoConnect(bool autoConnect);

Q_SIGNALS:
    void availableConnectionsChanged();

    void preferredConnectionChanged(connection::AvailableConnection::SPtr availableConnection);

    void autoConnectChanged(bool autoconnect);

protected:
    class Private;
    std::shared_ptr<Private> d;
};

}
}
