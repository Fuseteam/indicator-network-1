/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include <nmofono/manager.h>

#include <QDBusContext>
#include <QDBusConnection>
#include <QObject>

class NetworkingStatusAdaptor;
class PrivateAdaptor;

namespace connectivity_service
{
class PrivateService;

class ConnectivityService: public QObject, protected QDBusContext
{
    Q_OBJECT

    friend NetworkingStatusAdaptor;
    friend PrivateService;

public:
    ConnectivityService(nmofono::Manager::Ptr manager, const QDBusConnection& connection);
    virtual ~ConnectivityService();

public:
    Q_PROPERTY(QStringList Limitations READ limitations)
    QStringList limitations() const;

    Q_PROPERTY(QString Status READ status)
    QString status() const;

    Q_PROPERTY(bool WifiEnabled READ wifiEnabled)
    bool wifiEnabled() const;

    Q_PROPERTY(bool FlightMode READ flightMode)
    bool flightMode() const;

    Q_PROPERTY(bool UnstoppableOperationHappening READ unstoppableOperationHappening)
    bool unstoppableOperationHappening() const;

Q_SIGNALS:
    void unlockAllModems();

    void unlockModem(const QString& modem);

private:
    class Private;
    std::shared_ptr<Private> d;
};

class PrivateService : public QObject, protected QDBusContext
{
    Q_OBJECT

    friend PrivateAdaptor;

public:
    PrivateService(ConnectivityService& parent);

    ~PrivateService() = default;

protected Q_SLOTS:
    void UnlockAllModems();

    void UnlockModem(const QString &modem);

    void SetFlightMode(bool enabled);

    void SetWifiEnabled(bool enabled);

protected:
    ConnectivityService& p;
};

}
