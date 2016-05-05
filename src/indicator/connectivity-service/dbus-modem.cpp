/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include <connectivity-service/dbus-modem.h>
#include <ModemAdaptor.h>
#include <dbus-types.h>
#include <util/dbus-utils.h>

using namespace std;
using namespace nmofono::wwan;

namespace connectivity_service
{

DBusModem::DBusModem(Modem::Ptr modem,
                     const QDBusConnection& connection) :
    m_modem(modem),
    m_connection(connection)
{
    m_path.setPath(DBusTypes::modemPath(m_modem->serial()));

    new ModemAdaptor(this);

    registerDBusObject();
}

DBusModem::~DBusModem()
{
}

void DBusModem::registerDBusObject()
{
    if (!m_connection.registerObject(m_path.path(), this))
    {
        qWarning() << "Unable to register Modem object" << m_path.path();
    }
}

void DBusModem::notifyProperties(const QStringList& propertyNames)
{
    DBusUtils::notifyPropertyChanged(
        m_connection,
        *this,
        m_path.path(),
        ModemAdaptor::staticMetaObject.classInfo(ModemAdaptor::staticMetaObject.indexOfClassInfo("D-Bus Interface")).value(),
        propertyNames
    );
}

QDBusObjectPath DBusModem::sim() const
{
    return m_simpath;
}

void DBusModem::setSim(QDBusObjectPath path)
{
    if (m_simpath == path)
    {
        return;
    }
    m_simpath = path;
    notifyProperties({"Sim"});
}

int DBusModem::index() const
{
    return m_modem->index();
}


QString DBusModem::serial() const
{
    return m_modem->serial();
}

QDBusObjectPath
DBusModem::path() const
{
    return m_path;
}

}
