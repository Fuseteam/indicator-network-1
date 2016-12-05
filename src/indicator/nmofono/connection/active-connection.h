/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>

#include <unity/util/DefinesPtrs.h>
#include <NetworkManager.h>

#include <nmofono/connection/active-vpn-connection.h>

namespace nmofono
{
namespace connection
{

class ActiveConnection: public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(ActiveConnection);

    enum class State
    {
        unknown = NM_ACTIVE_CONNECTION_STATE_UNKNOWN,
        activating = NM_ACTIVE_CONNECTION_STATE_ACTIVATING,
        activated = NM_ACTIVE_CONNECTION_STATE_ACTIVATED,
        deactivating = NM_ACTIVE_CONNECTION_STATE_DEACTIVATING,
        deactivated = NM_ACTIVE_CONNECTION_STATE_DEACTIVATED
    };

    ActiveConnection(const QDBusObjectPath& path, const QDBusConnection& systemConnection);

    ~ActiveConnection() = default;

    QString id() const;

    QString uuid() const;

    QString type() const;

    State state() const;

    QDBusObjectPath connectionPath() const;

    QDBusObjectPath specificObject() const;

    QDBusObjectPath path() const;

    ActiveVpnConnection::SPtr vpnConnection() const;

Q_SIGNALS:
    void idChanged(const QString& id);

    void typeChanged(const QString& type);

    void stateChanged(State state);

    void connectionPathChanged(const QDBusObjectPath& connectionPath);

    void specificObjectChanged(const QDBusObjectPath& specificObject);

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
}
