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

#include <nmofono/connection/available-connection.h>
#include <NetworkManagerSettingsConnectionInterface.h>

using namespace std;

namespace nmofono
{
namespace connection
{

class AvailableConnection::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(AvailableConnection& parent) :
        p(parent)
    {
    }

public Q_SLOTS:
    void updated()
    {
        QVariantDictMap settings = m_connectionInterface->GetSettings();

        auto connection = settings.value("connection");
        m_connectionUuid = connection.value("uuid").toString();
        setConnectionId(connection.value("id").toString());
    }

    void setConnectionId(const QString& connectionId)
    {
        if (connectionId == m_connectionId)
        {
            return;
        }

        m_connectionId = connectionId;
        Q_EMIT p.connectionIdChanged(m_connectionId);
    }

public:
    AvailableConnection& p;

    shared_ptr<OrgFreedesktopNetworkManagerSettingsConnectionInterface> m_connectionInterface;

    QString m_connectionUuid;

    QString m_connectionId;
};

AvailableConnection::AvailableConnection(const QDBusObjectPath& path, const QDBusConnection& systemConnection) :
        d(new Priv(*this))
{
    d->m_connectionInterface = make_shared<OrgFreedesktopNetworkManagerSettingsConnectionInterface>(NM_DBUS_SERVICE, path.path(), systemConnection);
    connect(d->m_connectionInterface.get(), &OrgFreedesktopNetworkManagerSettingsConnectionInterface::Updated, d.get(), &Priv::updated);
    d->updated();
}

QDBusObjectPath AvailableConnection::path() const
{
    return QDBusObjectPath(d->m_connectionInterface->path());
}

QString AvailableConnection::connectionUuid() const
{
    return d->m_connectionUuid;
}

QString AvailableConnection::connectionId() const
{
    return d->m_connectionId;
}

}
}

#include "available-connection.moc"
