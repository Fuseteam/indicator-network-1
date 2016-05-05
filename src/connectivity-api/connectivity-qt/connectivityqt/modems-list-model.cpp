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

#include <connectivityqt/modems-list-model.h>
#include <connectivityqt/modem.h>

#include <QDebug>

using namespace std;

namespace connectivityqt
{

class ModemsListModel::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(ModemsListModel& parent, const QDBusConnection &connection) :
        p(parent),
        m_connection(connection)
    {
    }

    void updateModemDBusPaths(QList<QDBusObjectPath> values)
    {
        auto paths = values.toSet();

        QSet<QDBusObjectPath> current;
        for (const auto& m: m_modems)
        {
            current << m->path();
        }

        auto toRemove(current);
        toRemove.subtract(paths);

        auto toAdd(paths);
        toAdd.subtract(current);

        QMutableListIterator<Modem::SPtr> i(m_modems);
        int idx = 0;
        while (i.hasNext())
        {
            auto modem(i.next());
            if (toRemove.contains(modem->path()))
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
            p.beginInsertRows(QModelIndex(), m_modems.size(), m_modems.size() + toAdd.size() - 1);
            for (const auto& path: toAdd)
            {
                qDebug() << "ADDING " << path.path();
                auto modem = std::make_shared<Modem>(path, m_connection, m_sims, this);
                m_modems << modem;
                connect(modem.get(), &Modem::simChanged, this, &Priv::simChanged);
                qDebug() << "ADDED " << path.path();
            }
            p.endInsertRows();
        }
    }

    QModelIndex findModem(QObject* o)
    {
        auto modem = qobject_cast<Modem*>(o);
        for (auto i = m_modems.constBegin(); i != m_modems.constEnd(); ++i)
        {
            if (i->get() == modem)
            {
                return p.index(std::distance(m_modems.constBegin(), i));
            }
        }
        return QModelIndex();
    }

public Q_SLOTS:
    void simChanged(Sim *sim)
    {
        Q_UNUSED(sim)
        auto idx = findModem(sender());
        p.dataChanged(idx, idx, {ModemsListModel::Roles::RoleSim});
    }

public:
    ModemsListModel& p;
    SimsListModel *m_sims;
    QList<QDBusObjectPath> m_dbus_paths;
    QList<Modem::SPtr> m_modems;

    QDBusConnection m_connection;
};

ModemsListModel::ModemsListModel(const QDBusConnection& connection, SimsListModel *sims, QObject *parent) :
    QAbstractListModel(parent),
    d(new Priv(*this, connection))
{
    d->m_sims = sims;
}

ModemsListModel::~ModemsListModel()
{
}

int ModemsListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int ModemsListModel::rowCount(const QModelIndex &) const
{
    return d->m_modems.size();
}

QVariant ModemsListModel::data(const QModelIndex &index, int role) const
{
    int row(index.row());
    if (row < 0 || row >= d->m_modems.size())
    {
        return QVariant();
    }

    auto modem = d->m_modems.value(row);

    switch (role)
    {
        case Roles::RoleIndex:
            return modem->index();
            break;
        case Roles::RoleSerial:
            return modem->serial();
            break;
        case Roles::RoleSim:
            return QVariant::fromValue<Sim*>(modem->sim());
            break;
    }

    return QVariant();
}

void ModemsListModel::updateModemDBusPaths(QList<QDBusObjectPath> values)
{
    d->updateModemDBusPaths(values);
}


}

#include "modems-list-model.moc"
