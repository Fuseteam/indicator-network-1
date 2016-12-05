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

#include <nmofono/connection/active-connection.h>
#include <NetworkManagerActiveConnectionInterface.h>

using namespace std;

namespace nmofono
{
namespace connection
{

class ActiveConnection::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(ActiveConnection& parent) :
        p(parent)
    {
    }

public Q_SLOTS:
    void propertiesChanged(const QVariantMap &properties)
    {
        QMapIterator<QString, QVariant> it(properties);
        while (it.hasNext())
        {
            it.next();
            QString property = it.key();
            QVariant value = it.value();

            if (property == "Id")
            {
                setId(value.toString());
            }
            else if (property == "Type")
            {
                setType(value.toString());
            }
            else if (property == "State")
            {
                setState(static_cast<State>(value.toUInt()));
            }
            else if (property == "Connection")
            {
                setConnectionPath(qvariant_cast<QDBusObjectPath>(value));
            }
            else if (property == "SpecificObject")
            {
                setSpecificObject(qvariant_cast<QDBusObjectPath>(value));
            }
        }
    }

    void setId(const QString& id)
    {
        if (id == m_id)
        {
            return;
        }

        m_id = id;
        Q_EMIT p.idChanged(m_id);
    }

    void setType(const QString& type)
    {
        if (type == m_type)
        {
            return;
        }

        m_type = type;
        Q_EMIT p.typeChanged(m_type);

        if (m_type == "vpn")
        {
            m_activeVpnConnection = make_shared<ActiveVpnConnection>(
                    QDBusObjectPath(m_activeConnection->path()),
                    m_activeConnection->QDBusAbstractInterface::connection(),
                    p);
        }
        else
        {
            m_activeVpnConnection.reset();
        }
    }

    void setState(State state)
    {
        if (state == m_state)
        {
            return;
        }

        m_state = state;
        Q_EMIT p.stateChanged(m_state);
    }

    void setConnectionPath(const QDBusObjectPath& connectionPath)
    {
        if (connectionPath == m_connectionPath)
        {
            return;
        }

        m_connectionPath = connectionPath;
        Q_EMIT p.connectionPathChanged(m_connectionPath);
    }

    void setSpecificObject(const QDBusObjectPath& specificObject)
    {
        if (specificObject == m_specificObject)
        {
            return;
        }

        m_specificObject = specificObject;
        Q_EMIT p.specificObjectChanged(m_specificObject);
    }

public:
    ActiveConnection& p;

    shared_ptr<OrgFreedesktopNetworkManagerConnectionActiveInterface> m_activeConnection;

    ActiveVpnConnection::SPtr m_activeVpnConnection;

    QString m_uuid;

    QString m_id;

    QString m_type;

    State m_state = State::unknown;

    QDBusObjectPath m_connectionPath;

    QDBusObjectPath m_specificObject;
};

ActiveConnection::ActiveConnection(const QDBusObjectPath& path, const QDBusConnection& systemConnection) :
        d(new Priv(*this))
{
    d->m_activeConnection = make_shared<OrgFreedesktopNetworkManagerConnectionActiveInterface>(NM_DBUS_SERVICE, path.path(), systemConnection);

    d->m_uuid = d->m_activeConnection->uuid();
    d->setId(d->m_activeConnection->id());
    d->setType(d->m_activeConnection->type());
    d->setState(static_cast<State>(d->m_activeConnection->state()));
    d->setConnectionPath(d->m_activeConnection->connection());
    d->setSpecificObject(d->m_activeConnection->specificObject());

    connect(d->m_activeConnection.get(), &OrgFreedesktopNetworkManagerConnectionActiveInterface::PropertiesChanged, d.get(), &Priv::propertiesChanged);
}

QString ActiveConnection::id() const
{
    return d->m_id;
}

QString ActiveConnection::uuid() const
{
    return d->m_uuid;
}

QString ActiveConnection::type() const
{
    return d->m_type;
}

ActiveConnection::State ActiveConnection::state() const
{
    return d->m_state;
}

QDBusObjectPath ActiveConnection::connectionPath() const
{
    return d->m_connectionPath;
}

QDBusObjectPath ActiveConnection::specificObject() const
{
    return d->m_specificObject;
}

QDBusObjectPath ActiveConnection::path() const
{
    return QDBusObjectPath(d->m_activeConnection->path());
}

ActiveVpnConnection::SPtr ActiveConnection::vpnConnection() const
{
    return d->m_activeVpnConnection;
}

}
}

#include "active-connection.moc"
