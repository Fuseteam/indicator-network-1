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

#include <QDebug>

using namespace std;

namespace connectivityqt
{

class SimsListModel::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(SimsListModel& parent, const QDBusConnection &connection) :
        p(parent),
        m_connection(connection)
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
                auto sim = std::make_shared<Sim>(path, m_connection, this);
                m_sims << sim;
                connect(sim.get(), &Sim::lockedChanged, this, &Priv::lockedChanged);
                connect(sim.get(), &Sim::presentChanged, this, &Priv::presentChanged);
                connect(sim.get(), &Sim::dataRoamingEnabledChanged, this, &Priv::dataRoamingEnabledChanged);
            }
            p.endInsertRows();
        }
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

public:
    SimsListModel& p;
    QList<QDBusObjectPath> m_dbus_paths;
    QList<Sim::SPtr> m_sims;

    QDBusConnection m_connection;
};

SimsListModel::SimsListModel(const QDBusConnection& connection, QObject *parent) :
    QAbstractListModel(parent),
    d(new Priv(*this, connection))
{
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
        case Roles::RoleImsi:
            return sim->imsi();
            break;
        case Roles::RolePrimaryPhoneNumber:
            return sim->primaryPhoneNumber();
            break;
        case Roles::RolePhoneNumbers:
            return QVariant::fromValue<QStringList>(sim->phoneNumbers());
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
        case Roles::RoleSimObject:
            return QVariant::fromValue<Sim*>(sim.get());
            break;
    }


    return QVariant();
}

void SimsListModel::updateSimDBusPaths(QList<QDBusObjectPath> values)
{
    d->updateSimDBusPaths(values);
}

Sim *SimsListModel::getSimByPath(const QDBusObjectPath &path) const
{
    for (auto sim : d->m_sims)
    {
        if (sim->path() == path) {
            return sim.get();
        }
    }
    return nullptr;
}


}

#include "sims-list-model.moc"
