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

#include <connectivity-service/dbus-sim.h>
#include <SimAdaptor.h>
#include <dbus-types.h>
#include <util/dbus-utils.h>

#include <QDebug>

using namespace std;
using namespace nmofono::wwan;

namespace connectivity_service
{

DBusSim::DBusSim(Sim::Ptr sim,
                 const QDBusConnection& connection) :
    m_sim(sim),
    m_connection(connection)
{
    m_path.setPath(DBusTypes::simPath(m_sim->imsi()));

    new SimAdaptor(this);

    registerDBusObject();

    connect(sim.get(), &Sim::lockedChanged, this, &DBusSim::lockedChanged);
    connect(sim.get(), &Sim::presentChanged, this, &DBusSim::presentChanged);
    connect(sim.get(), &Sim::dataRoamingEnabledChanged, this, &DBusSim::dataRoamingEnabledChanged);
}

DBusSim::~DBusSim()
{
}

QDBusObjectPath DBusSim::path() const
{
    return m_path;
}

void DBusSim::registerDBusObject()
{
    if (!m_connection.registerObject(m_path.path(), this))
    {
        qWarning() << "Unable to register SIM object" << m_path.path();
    }
}

void DBusSim::notifyProperties(const QStringList& propertyNames)
{
    DBusUtils::notifyPropertyChanged(
        m_connection,
        *this,
        m_path.path(),
        SimAdaptor::staticMetaObject.classInfo(SimAdaptor::staticMetaObject.indexOfClassInfo("D-Bus Interface")).value(),
        propertyNames
    );
}

QString DBusSim::imsi() const
{
    return m_sim->imsi();
}

QString DBusSim::primaryPhoneNumber() const
{
    return m_sim->primaryPhoneNumber();
}

bool DBusSim::locked() const
{
    return m_sim->locked();
}

bool DBusSim::present() const
{
    return m_sim->present();
}

QString DBusSim::mcc() const
{
    return m_sim->mcc();
}

QString DBusSim::mnc() const
{
    return m_sim->mnc();
}

QStringList DBusSim::preferredLanguages() const
{
    return m_sim->preferredLanguages();
}

bool DBusSim::dataRoamingEnabled() const
{
    return m_sim->dataRoamingEnabled();
}

void DBusSim::setDataRoamingEnabled(bool value) const
{
    m_sim->setDataRoamingEnabled(value);
}

void DBusSim::Unlock()
{
    m_sim->unlock();
}

void DBusSim::lockedChanged()
{
    notifyProperties({"Locked"});
}

void DBusSim::presentChanged()
{
    notifyProperties({"Present"});
}

void DBusSim::dataRoamingEnabledChanged()
{
    notifyProperties({"DataRoamingEnabled"});
}

nmofono::wwan::Sim::Ptr DBusSim::sim() const
{
    return m_sim;
}


}
