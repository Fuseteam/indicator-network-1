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

#include <connectivityqt/internal/dbus-property-cache.h>
#include <connectivityqt/modem.h>
#include <dbus-types.h>

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
        if (name == "Sim")
        {
            qDebug() << "MODEM SIM: " << value.value<QDBusObjectPath>().path();
            m_sim = m_sims->getSimByPath(value.value<QDBusObjectPath>());
            qDebug() << "MODEM SIM: " << m_sim;
            Q_EMIT p.simChanged(m_sim);
        }
    }

public:
    Modem& p;

    Sim *m_sim;
    SimsListModel *m_sims;

    unique_ptr<ComUbuntuConnectivity1ModemInterface> m_modemInterface;

    internal::DBusPropertyCache::UPtr m_propertyCache;
};

Modem::Modem(const QDBusObjectPath& path,
             const QDBusConnection& connection,
             SimsListModel *sims,
             QObject* parent) :
        QObject(parent), d(new Priv(*this))
{
    d->m_sims = sims;
    d->m_modemInterface = make_unique<
            ComUbuntuConnectivity1ModemInterface>(
            DBusTypes::DBUS_NAME, path.path(), connection);

    d->m_propertyCache =
            make_unique<internal::DBusPropertyCache>(
                    DBusTypes::DBUS_NAME,
                    ComUbuntuConnectivity1ModemInterface::staticInterfaceName(),
                    path.path(), connection);

    connect(d->m_propertyCache.get(),
                &internal::DBusPropertyCache::propertyChanged, d.get(),
                &Priv::propertyChanged);

    qDebug() << "MODEM SIM: " << d->m_propertyCache->get("Sim").value<QDBusObjectPath>().path();
    d->m_sim = d->m_sims->getSimByPath(d->m_propertyCache->get("Sim").value<QDBusObjectPath>());
    qDebug() << "MODEM SIM: " << d->m_sim;

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
    return d->m_sim;
}

QString Modem::serial() const
{
    return d->m_propertyCache->get("Serial").toString();
}

}

#include "modem.moc"
