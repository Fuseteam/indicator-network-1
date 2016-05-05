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

#include <nmofono/wwan/sim.h>

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QObject>
#include <QString>

#include <unity/util/DefinesPtrs.h>

class SimAdaptor;

namespace connectivity_service
{

class DBusSim: public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_DISABLE_COPY(DBusSim)

    friend SimAdaptor;

public:
    UNITY_DEFINES_PTRS(DBusSim);

    DBusSim(nmofono::wwan::Sim::Ptr sim, const QDBusConnection& connection);

    virtual ~DBusSim();

    Q_PROPERTY(QString Imsi READ imsi)
    QString imsi() const;

    Q_PROPERTY(QString PrimaryPhoneNumber READ primaryPhoneNumber)
    QString primaryPhoneNumber() const;

    Q_PROPERTY(bool Locked READ locked)
    bool locked() const;

    Q_PROPERTY(bool Present READ present)
    bool present() const;

    Q_PROPERTY(QString Mcc READ mcc)
    QString mcc() const;

    Q_PROPERTY(QString Mnc READ mnc)
    QString mnc() const;

    Q_PROPERTY(QStringList PreferredLanguages READ preferredLanguages)
    QStringList preferredLanguages() const;

    Q_PROPERTY(bool DataRoamingEnabled READ dataRoamingEnabled WRITE setDataRoamingEnabled)
    bool dataRoamingEnabled() const;
    void setDataRoamingEnabled(bool value) const;

    QDBusObjectPath path() const;

    nmofono::wwan::Sim::Ptr sim() const;

Q_SIGNALS:

protected Q_SLOTS:

    void Unlock();

    void lockedChanged();
    void presentChanged();
    void dataRoamingEnabledChanged();

private:
    void notifyProperties(const QStringList& propertyNames);

protected:
    void registerDBusObject();

    nmofono::wwan::Sim::Ptr m_sim;

    QDBusConnection m_connection;

    QDBusObjectPath m_path;
};

}
