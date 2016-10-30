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
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <util/dbus-property-cache.h>
#include <connectivityqt/sim.h>
#include <dbus-types.h>

#include <SimInterface.h>

using namespace std;

namespace connectivityqt
{

class Sim::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(Sim& parent) :
        p(parent)
    {
    }

public Q_SLOTS:
    void propertyChanged(const QString& name, const QVariant& value)
    {
        if (name == "Locked")
        {
            Q_EMIT p.lockedChanged(value.toBool());
        } else if (name == "Present")
        {
            Q_EMIT p.presentChanged(value.toBool());
        } else if (name == "DataRoamingEnabled")
        {
            Q_EMIT p.dataRoamingEnabledChanged(value.toBool());
        } else if (name == "Imsi")
        {
            Q_EMIT p.imsiChanged(value.toString());
        } else if (name == "PrimaryPhoneNumber")
        {
            Q_EMIT p.primaryPhoneNumberChanged(value.toString());
        } else if (name == "PreferredLanguages")
        {
            Q_EMIT p.preferredLanguagesChanged();
        } else {
            qWarning() << "connectivityqt::Sim::Priv::propertyChanged(): "
                       << "Unexpected property: " << name;
        }
    }

public:
    Sim& p;

    unique_ptr<ComUbuntuConnectivity1SimInterface> m_simInterface;

    util::DBusPropertyCache::UPtr m_propertyCache;
};

Sim::Sim(const QDBusObjectPath& path, const QDBusConnection& connection, QObject* parent) :
        QObject(parent), d(new Priv(*this))
{
    d->m_simInterface = make_unique<
            ComUbuntuConnectivity1SimInterface>(
            DBusTypes::DBUS_NAME, path.path(), connection);

    d->m_propertyCache =
            make_unique<util::DBusPropertyCache>(
                    DBusTypes::DBUS_NAME,
                    ComUbuntuConnectivity1SimInterface::staticInterfaceName(),
                    path.path(), connection);

    connect(d->m_propertyCache.get(),
                &util::DBusPropertyCache::propertyChanged, d.get(),
                &Priv::propertyChanged);
}

Sim::~Sim()
{
}

QDBusObjectPath Sim::path() const
{
    return QDBusObjectPath(d->m_simInterface->path());
}

QString Sim::iccid() const
{
    return d->m_propertyCache->get("Iccid").toString();
}

QString Sim::imsi() const
{
    return d->m_propertyCache->get("Imsi").toString();
}

QString Sim::primaryPhoneNumber() const
{
    return d->m_propertyCache->get("PrimaryPhoneNumber").toString();
}

bool Sim::locked() const
{
    return d->m_propertyCache->get("Locked").toBool();
}

bool Sim::present() const
{
    return d->m_propertyCache->get("Present").toBool();
}

QString Sim::mcc() const
{
    return d->m_propertyCache->get("Mcc").toString();
}

QString Sim::mnc() const
{
    return d->m_propertyCache->get("Mnc").toString();
}

QList<QString> Sim::preferredLanguages() const
{
    return d->m_propertyCache->get("PreferredLanguages").toStringList();
}

bool Sim::dataRoamingEnabled() const
{
    return d->m_propertyCache->get("DataRoamingEnabled").toBool();
}

void Sim::setDataRoamingEnabled(bool value)
{
    d->m_propertyCache->set("DataRoamingEnabled", QVariant(value));
}

void Sim::unlock()
{
    d->m_simInterface->Unlock();
}

}

#include "sim.moc"

