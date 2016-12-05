/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>

#include <unity/util/DefinesPtrs.h>
#include <NetworkManager.h>

namespace nmofono
{
namespace ethernet
{
class EthernetLink;
}

namespace connection
{

class AvailableConnection: public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(AvailableConnection);

    AvailableConnection(const QDBusObjectPath& path, const QDBusConnection& systemConnection);

    ~AvailableConnection() = default;

    QDBusObjectPath path() const;

    QString connectionId() const;

    QString connectionUuid() const;

Q_SIGNALS:
    void connectionIdChanged(const QString &);

    void connectionUuidChanged(const QString &);

protected:
    class Priv;
    std::shared_ptr<Priv> d;

    friend ethernet::EthernetLink;
};

}
}
