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

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>

#include <unity/util/DefinesPtrs.h>
#include <connectivityqt/sims-list-model.h>

namespace connectivityqt
{

class Q_DECL_EXPORT Modem : public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(Modem);

    Modem(const QDBusObjectPath& path,
          const QDBusConnection& connection,
          SimsListModel *sims,
          QObject* parent = 0);

    virtual ~Modem();

    Q_PROPERTY(QDBusObjectPath path READ path)
    QDBusObjectPath path() const;

    Q_PROPERTY(QString serial READ serial)
    QString serial() const;

    Q_PROPERTY(int index READ index CONSTANT)
    int index() const;

    Q_PROPERTY(connectivityqt::Sim* sim READ sim NOTIFY simChanged)
    Sim* sim() const;

public Q_SLOTS:

Q_SIGNALS:
    void simChanged(Sim *sim);

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
