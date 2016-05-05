/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#pragma once

#include <nmofono/wwan/modem.h>

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QObject>
#include <QString>

#include <unity/util/DefinesPtrs.h>

class ModemAdaptor;

namespace connectivity_service
{

class DBusModem: public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_DISABLE_COPY(DBusModem)

    friend ModemAdaptor;

public:
    UNITY_DEFINES_PTRS(DBusModem);

    DBusModem(nmofono::wwan::Modem::Ptr modem, const QDBusConnection& connection);

    virtual ~DBusModem();

    Q_PROPERTY(int Index READ index)
    int index() const;

    Q_PROPERTY(QString Serial READ serial)
    QString serial() const;

    Q_PROPERTY(QDBusObjectPath Sim READ sim)
    QDBusObjectPath sim() const;

    void setSim(QDBusObjectPath path);

    QDBusObjectPath path() const;

Q_SIGNALS:

protected Q_SLOTS:

private:
    void notifyProperties(const QStringList& propertyNames);

protected:
    void registerDBusObject();

    nmofono::wwan::Modem::Ptr m_modem;

    QDBusConnection m_connection;

    QDBusObjectPath m_path;
    QDBusObjectPath m_simpath;

};

}
