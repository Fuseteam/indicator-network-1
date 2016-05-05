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

#pragma once

#include <QAbstractItemModel>
#include <QDBusConnection>
#include <QDBusObjectPath>

#include <memory>

namespace connectivityqt
{

class SimsListModel;

class Q_DECL_EXPORT ModemsListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

public:

    enum Roles
    {
        RoleIndex = Qt::UserRole + 1,
        RoleSerial,
        RoleSim
    };

    ModemsListModel(const QDBusConnection &connection, SimsListModel *sims, QObject *parent);

    ~ModemsListModel();

    int columnCount(const QModelIndex &parent) const override;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles;
        roles[RoleIndex] = "Index";
        roles[RoleSerial] = "Serial";
        roles[RoleSim] = "Sim";
        return roles;
    }

    void updateModemDBusPaths(QList<QDBusObjectPath> values);

public Q_SLOTS:

Q_SIGNALS:

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
