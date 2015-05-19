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
 * Authors:
 *     Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <QDBusConnection>
#include <QObject>
#include <QStringList>
#include <memory>

#include <unity/util/DefinesPtrs.h>

namespace connectivityqt
{

class Q_DECL_EXPORT Connectivity: public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(Connectivity);

    Q_DISABLE_COPY(Connectivity)

    Q_ENUMS(Limitations)
    Q_ENUMS(Status)

    static void registerMetaTypes();

    /**
      * @brief enum for networking limitations
      *
      * Networking limitations may be accessed through the NetworkingStatus::limitations property.
      */
    enum class Limitations
    {
        /**
         * indicates that the bandwith of the Internet connection has limitations.
         * Applications should minimize their bandwith usage if possible.
         */
        Bandwith
    };

    /**
      * @brief enum for networking status
      *
      * Networking status may be accessed through the NetworkingStatus::status property.
      */
    enum class Status
    {
        Offline,     /**< No Internet connection available.            */
        Connecting,  /**< System is actively establising a connection. */
        Online       /**< System is connected to the Internet.         */
    };

    Connectivity(const QDBusConnection& sessionConnection = QDBusConnection::sessionBus(), QObject* parent = 0);

    ~Connectivity();

    Q_PROPERTY(bool FlightMode READ flightMode WRITE setFlightMode NOTIFY flightModeUpdated)
    bool flightMode() const;

    Q_PROPERTY(bool online READ online NOTIFY onlineUpdated)
    bool online() const;

    Q_PROPERTY(bool limitedBandwith READ limitedBandwith NOTIFY limitedBandwithUpdated)
    bool limitedBandwith() const;

    Q_PROPERTY(QVector<Limitations> Limitations READ limitations NOTIFY limitationsUpdated)
    QVector<Limitations> limitations() const;

    Q_PROPERTY(connectivityqt::Connectivity::Status Status READ status NOTIFY statusUpdated)
    Status status() const;

    Q_PROPERTY(bool WifiEnabled READ wifiEnabled WRITE setwifiEnabled NOTIFY wifiEnabledUpdated)
    bool wifiEnabled() const;

    Q_PROPERTY(bool UnstoppableOperationHappening READ unstoppableOperationHappening NOTIFY unstoppableOperationHappeningUpdated)
    bool unstoppableOperationHappening() const;

    Q_PROPERTY(bool Initialized READ isInitialized NOTIFY initialized)
    bool isInitialized() const;

public Q_SLOTS:
    void setFlightMode(bool enabled);

    void setwifiEnabled(bool enabled);

Q_SIGNALS:
    void flightModeUpdated(bool);

    void onlineUpdated(bool value);

    void limitedBandwithUpdated(bool value);

    void limitationsUpdated(const QVector<Limitations>&);

    void statusUpdated(connectivityqt::Connectivity::Status value);

    void wifiEnabledUpdated(bool);

    void unstoppableOperationHappeningUpdated(bool);

    void initialized();

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}

Q_DECLARE_METATYPE(connectivityqt::Connectivity::Limitations)
Q_DECLARE_METATYPE(QVector<connectivityqt::Connectivity::Limitations>)
Q_DECLARE_METATYPE(connectivityqt::Connectivity::Status)

