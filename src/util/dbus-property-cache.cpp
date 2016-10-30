/*
 * Copyright © 2014 Canonical Ltd.
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

#include "dbus-property-cache.h"

#include <PropertiesInterface.h>

using namespace std;

namespace util
{

class DBusPropertyCache::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(DBusPropertyCache& parent, const QDBusConnection& connection) :
        p(parent), m_connection(connection)
    {
    }

    DBusPropertyCache& p;

    QDBusConnection m_connection;

    QString m_service;

    QString m_interface;

    QString m_path;

    shared_ptr<QDBusServiceWatcher> m_serviceWatcher;

    shared_ptr<OrgFreedesktopDBusPropertiesInterface> m_propertiesInterface;

    QVariantMap m_propertyCache;

    void refreshProperties(const QStringList& names)
    {
        for(const QString& name: names)
        {
            QDBusVariant v = m_propertiesInterface->Get(m_interface, name);
            QVariant variant = v.variant();
            m_propertyCache[name] = variant;
            Q_EMIT p.propertyChanged(name, variant);
        }
    }

public Q_SLOTS:
    void serviceOwnerChanged(const QString &, const QString &,
                        const QString & newOwner)
    {
        m_propertiesInterface.reset();
        m_propertyCache.clear();

        if (newOwner.isEmpty())
        {
            return;
        }

        m_propertiesInterface = make_shared<
                OrgFreedesktopDBusPropertiesInterface>(m_service, m_path,
                                                       m_connection);

        connect(m_propertiesInterface.get(),
                &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this,
                &Priv::propertiesChanged);

        m_propertyCache = m_propertiesInterface->GetAll(m_interface);
        QMapIterator<QString, QVariant> it(m_propertyCache);
        while (it.hasNext())
        {
            it.next();
            Q_EMIT p.propertyChanged(it.key(), it.value());
        }

        Q_EMIT p.initialized();
    }

    void propertiesChanged(const QString &,
                      const QVariantMap &changedProperties,
                      const QStringList &invalidatedProperties)
    {
        QMapIterator<QString, QVariant> it(changedProperties);
        while (it.hasNext())
        {
            it.next();
            if (m_propertyCache[it.key()] != it.value())
            {
                m_propertyCache[it.key()] = it.value();
                Q_EMIT p.propertyChanged(it.key(), it.value());
            }
        }

        refreshProperties(invalidatedProperties);
    }

    void dbusCallFinished(QDBusPendingCallWatcher *call)
    {
        QDBusPendingReply<> reply = *call;
        if (reply.isError())
        {
            qCritical() << __PRETTY_FUNCTION__ << reply.error().message();
        }
        call->deleteLater();
    }
};

DBusPropertyCache::DBusPropertyCache(const QString &service,
                                     const QString &interface,
                                     const QString &path,
                                     const QDBusConnection &connection) :
        d(new Priv(*this, connection))
{
    d->m_service = service;
    d->m_interface = interface;
    d->m_path = path;

    d->m_serviceWatcher = make_shared<QDBusServiceWatcher>(service,
                                                           connection);

    connect(d->m_serviceWatcher.get(), &QDBusServiceWatcher::serviceOwnerChanged,
            d.get(), &Priv::serviceOwnerChanged);

    // If the service is already registered
    QString serviceOwner = connection.interface()->serviceOwner(service);
    if (!serviceOwner.isEmpty())
    {
        d->serviceOwnerChanged(service, "", serviceOwner);
    }
}

DBusPropertyCache::~DBusPropertyCache()
{
}

void DBusPropertyCache::set(const QString& name, const QVariant& value)
{
    if (d->m_propertyCache.value(name) != value)
    {
        auto reply = d->m_propertiesInterface->Set(d->m_interface, name, QDBusVariant(value));
        auto watcher(new QDBusPendingCallWatcher(reply, this));
        connect(watcher, &QDBusPendingCallWatcher::finished, d.get(), &Priv::dbusCallFinished);
    }
}

QVariant DBusPropertyCache::get(const QString& name) const
{
    return d->m_propertyCache[name];
}

bool DBusPropertyCache::isInitialized() const
{
    return !d->m_propertyCache.empty();
}

QDBusConnection DBusPropertyCache::connection() const
{
    return d->m_connection;
}

}

#include "dbus-property-cache.moc"

