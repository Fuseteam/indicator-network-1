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
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <util/dbus-property-cache.h>
#include <connectivityqt/modem.h>
#include <qdbus-stubs/dbus-types.h>

#include <ModemInterface.h>

using namespace std;

namespace connectivityqt
{

class Modem::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(Modem& parent) :
        p(parent)
    {
    }

public Q_SLOTS:
    void propertyChanged(const QString& name, const QVariant& value)
    {
        Q_UNUSED(value);
        if (name == "Sim")
        {
            simsUpdated();
        }
    }

    void simsUpdated()
    {
        auto path = m_propertyCache->get("Sim").value<QDBusObjectPath>();
        auto sim = m_sims->getSimByPath(path);
        setSim(sim);
    }

    void setSim(Sim::SPtr sim)
    {
        if (m_sim == sim)
        {
            return;
        }
        m_sim = sim;
        Q_EMIT p.simChanged(m_sim.get());
    }

public:
    Modem& p;

    Sim::SPtr m_sim;
    SimsListModel::SPtr m_sims;

    unique_ptr<ComUbuntuConnectivity1ModemInterface> m_modemInterface;

    util::DBusPropertyCache::UPtr m_propertyCache;
};

Modem::Modem(const QDBusObjectPath& path,
             const QDBusConnection& connection,
             SimsListModel::SPtr sims,
             QObject* parent) :
        QObject(parent), d(new Priv(*this))
{
    d->m_sims = sims;
    d->m_modemInterface = make_unique<
            ComUbuntuConnectivity1ModemInterface>(
            DBusTypes::DBUS_NAME, path.path(), connection);

    d->m_propertyCache =
            make_unique<util::DBusPropertyCache>(
                    DBusTypes::DBUS_NAME,
                    ComUbuntuConnectivity1ModemInterface::staticInterfaceName(),
                    path.path(), connection);

    connect(d->m_propertyCache.get(),
                &util::DBusPropertyCache::propertyChanged, d.get(),
                &Priv::propertyChanged);

    d->simsUpdated();

    connect(d->m_sims.get(), &SimsListModel::simsUpdated, d.get(), &Priv::simsUpdated);
}

Modem::~Modem()
{
}

QDBusObjectPath Modem::path() const
{
    return QDBusObjectPath(d->m_modemInterface->path());
}

int Modem::index() const
{
    return d->m_propertyCache->get("Index").toInt();
}

Sim *Modem::sim() const
{
    return d->m_sim.get();
}

QString Modem::serial() const
{
    return d->m_propertyCache->get("Serial").toString();
}

}

#include "modem.moc"
