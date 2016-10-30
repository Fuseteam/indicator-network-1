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

#include <connectivityqt/sims-list-model.h>

#include "internal/sims-list-model-parameters.h"

#include <QDebug>

using namespace std;

namespace connectivityqt
{

class SimsListModel::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(SimsListModel& parent) :
        p(parent)
    {
    }

    void updateSimDBusPaths(QList<QDBusObjectPath> values)
    {
        auto paths = values.toSet();

        QSet<QDBusObjectPath> current;
        for (const auto& m: m_sims)
        {
            current << m->path();
        }

        auto toRemove(current);
        toRemove.subtract(paths);

        auto toAdd(paths);
        toAdd.subtract(current);

        QMutableListIterator<Sim::SPtr> i(m_sims);
        int idx = 0;
        while (i.hasNext())
        {
            auto sim(i.next());
            if (toRemove.contains(sim->path()))
            {
                p.beginRemoveRows(QModelIndex(), idx, idx);
                i.remove();
                p.endRemoveRows();
            }
            else
            {
                ++idx;
            }
        }

        if (!toAdd.isEmpty())
        {
            p.beginInsertRows(QModelIndex(), m_sims.size(), m_sims.size() + toAdd.size() - 1);
            for (const auto& path: toAdd)
            {
                auto sim = std::make_shared<Sim>(path, m_propertyCache->connection(), nullptr);
                m_objectOwner(sim.get());
                m_sims << sim;
                connect(sim.get(), &Sim::lockedChanged, this, &Priv::lockedChanged);
                connect(sim.get(), &Sim::presentChanged, this, &Priv::presentChanged);
                connect(sim.get(), &Sim::dataRoamingEnabledChanged, this, &Priv::dataRoamingEnabledChanged);
                connect(sim.get(), &Sim::imsiChanged, this, &Priv::imsiChanged);
                connect(sim.get(), &Sim::primaryPhoneNumberChanged, this, &Priv::primaryPhoneNumberChanged);
                connect(sim.get(), &Sim::mccChanged, this, &Priv::mccChanged);
                connect(sim.get(), &Sim::mncChanged, this, &Priv::mncChanged);
                connect(sim.get(), &Sim::preferredLanguagesChanged, this, &Priv::preferredLanguagesChanged);
            }
            p.endInsertRows();
        }

        Q_EMIT p.simsUpdated();
    }

    QModelIndex findSim(QObject* o)
    {
        auto sim = qobject_cast<Sim*>(o);
        for (auto i = m_sims.constBegin(); i != m_sims.constEnd(); ++i)
        {
            if (i->get() == sim)
            {
                return p.index(std::distance(m_sims.constBegin(), i));
            }
        }
        return QModelIndex();
    }

public Q_SLOTS:
    void lockedChanged()
    {
        auto idx = findSim(sender());
        p.dataChanged(idx, idx, {SimsListModel::Roles::RoleLocked});
    }

    void presentChanged()
    {
        auto idx = findSim(sender());
        p.dataChanged(idx, idx, {SimsListModel::Roles::RolePresent});
    }
    void dataRoamingEnabledChanged()
    {
        auto idx = findSim(sender());
        p.dataChanged(idx, idx, {SimsListModel::Roles::RoleDataRoamingEnabled});
    }
    void imsiChanged()
    {
        auto idx = findSim(sender());
        p.dataChanged(idx, idx, {SimsListModel::Roles::RoleImsi});
    }
    void primaryPhoneNumberChanged()
    {
        auto idx = findSim(sender());
        p.dataChanged(idx, idx, {SimsListModel::Roles::RolePrimaryPhoneNumber});
    }
    void mccChanged()
    {
        auto idx = findSim(sender());
        p.dataChanged(idx, idx, {SimsListModel::Roles::RoleMcc});
    }
    void mncChanged()
    {
        auto idx = findSim(sender());
        p.dataChanged(idx, idx, {SimsListModel::Roles::RoleMnc});
    }

    void preferredLanguagesChanged()
    {
        auto idx = findSim(sender());
        p.dataChanged(idx, idx, {SimsListModel::Roles::RolePreferredLanguages});
    }

    void propertyChanged(const QString& name, const QVariant& value)
    {
        if (name == "Sims")
        {
            QList<QDBusObjectPath> tmp;
            qvariant_cast<QDBusArgument>(value) >> tmp;
            updateSimDBusPaths(tmp);
        }
    }

public:
    SimsListModel& p;
    function<void(QObject*)> m_objectOwner;
    QList<QDBusObjectPath> m_dbus_paths;
    QList<Sim::SPtr> m_sims;

    shared_ptr<ComUbuntuConnectivity1PrivateInterface> m_writeInterface;
    util::DBusPropertyCache::SPtr m_propertyCache;
};

SimsListModel::SimsListModel(const internal::SimsListModelParameters &parameters) :
    QAbstractListModel(0),
    d(new Priv(*this))
{
    d->m_objectOwner = parameters.objectOwner;
    d->m_writeInterface = parameters.writeInterface;
    d->m_propertyCache = parameters.propertyCache;

    connect(d->m_propertyCache.get(),
            &util::DBusPropertyCache::propertyChanged, d.get(),
            &Priv::propertyChanged);

    QList<QDBusObjectPath> tmp;
    qvariant_cast<QDBusArgument>(d->m_propertyCache->get("Sims")) >> tmp;
    d->updateSimDBusPaths(tmp);
}

SimsListModel::~SimsListModel()
{
}

int SimsListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int SimsListModel::rowCount(const QModelIndex &) const
{
    return d->m_sims.size();
}

QVariant SimsListModel::data(const QModelIndex &index, int role) const
{
    int row(index.row());
    if (row < 0 || row >= d->m_sims.size())
    {
        return QVariant();
    }

    auto sim = d->m_sims.value(row);

    switch (role)
    {
        case Roles::RoleIccid:
            return sim->iccid();
            break;
        case Roles::RoleImsi:
            return sim->imsi();
            break;
        case Roles::RolePrimaryPhoneNumber:
            return sim->primaryPhoneNumber();
            break;
        case RoleLocked:
            return sim->locked();
            break;
        case RolePresent:
            return sim->present();
            break;
        case Roles::RoleMcc:
            return sim->mcc();
            break;
        case Roles::RoleMnc:
            return sim->mnc();
        case Roles::RolePreferredLanguages:
            return QVariant::fromValue<QStringList>(sim->preferredLanguages());
            break;
        case Roles::RoleDataRoamingEnabled:
            return sim->dataRoamingEnabled();
            break;
        case Roles::RoleSim:
            return QVariant::fromValue<Sim*>(sim.get());
            break;
    }


    return QVariant();
}

Sim::SPtr SimsListModel::getSimByPath(const QDBusObjectPath &path) const
{
    for (auto sim : d->m_sims)
    {
        if (sim->path() == path) {
            return sim;
        }
    }
    return nullptr;
}


}

#include "sims-list-model.moc"
