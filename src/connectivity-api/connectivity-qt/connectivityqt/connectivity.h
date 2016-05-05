/*
 * Copyright © 2014 Canonical Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <QAbstractListModel>
#include <QDBusConnection>
#include <QObject>
#include <QStringList>
#include <functional>
#include <memory>

#include <unity/util/DefinesPtrs.h>

#include <connectivityqt/vpn-connections-list-model.h>

#include <QDBusObjectPath>

namespace connectivityqt
{

class Sim;

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
      * Networking limitations may be accessed through the Connectivity::limitations property.
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
      * Networking status may be accessed through the Connectivity::status property.
      */
    enum class Status
    {
        Offline,     /**< No Internet connection available.            */
        Connecting,  /**< System is actively establising a connection. */
        Online       /**< System is connected to the Internet.         */
    };

    Connectivity(const QDBusConnection& sessionConnection = QDBusConnection::sessionBus(), QObject* parent = 0);

    Connectivity(const std::function<void(QObject*)>& objectOwner,
                 const QDBusConnection& sessionConnection = QDBusConnection::sessionBus(),
                 QObject* parent = 0);

    ~Connectivity();

    Q_PROPERTY(bool flightMode READ flightMode WRITE setFlightMode NOTIFY flightModeUpdated)
    bool flightMode() const;

    Q_PROPERTY(bool online READ online NOTIFY onlineUpdated)
    bool online() const;

    Q_PROPERTY(bool limitedBandwith READ limitedBandwith NOTIFY limitedBandwithUpdated)
    bool limitedBandwith() const;

    Q_PROPERTY(QVector<Limitations> Limitations READ limitations NOTIFY limitationsUpdated)
    QVector<Limitations> limitations() const;

    Q_PROPERTY(connectivityqt::Connectivity::Status status READ status NOTIFY statusUpdated)
    connectivityqt::Connectivity::Status status() const;

    Q_PROPERTY(bool wifiEnabled READ wifiEnabled WRITE setwifiEnabled NOTIFY wifiEnabledUpdated)
    bool wifiEnabled() const;

    Q_PROPERTY(bool unstoppableOperationHappening READ unstoppableOperationHappening NOTIFY unstoppableOperationHappeningUpdated)
    bool unstoppableOperationHappening() const;

    Q_PROPERTY(bool flightModeSwitchEnabled READ flightModeSwitchEnabled NOTIFY flightModeSwitchEnabledUpdated)
    bool flightModeSwitchEnabled() const;

    Q_PROPERTY(bool wifiSwitchEnabled READ wifiSwitchEnabled NOTIFY wifiSwitchEnabledUpdated)
    bool wifiSwitchEnabled() const;

    Q_PROPERTY(bool hotspotSwitchEnabled READ hotspotSwitchEnabled NOTIFY hotspotSwitchEnabledUpdated)
    bool hotspotSwitchEnabled() const;

    Q_PROPERTY(bool modemAvailable READ modemAvailable NOTIFY modemAvailableUpdated)
    bool modemAvailable() const;

    Q_PROPERTY(QByteArray hotspotSsid READ hotspotSsid WRITE setHotspotSsid NOTIFY hotspotSsidUpdated)
    QByteArray hotspotSsid() const;

    Q_PROPERTY(QString hotspotPassword READ hotspotPassword WRITE setHotspotPassword NOTIFY hotspotPasswordUpdated)
    QString hotspotPassword() const;

    Q_PROPERTY(bool hotspotEnabled READ hotspotEnabled WRITE setHotspotEnabled NOTIFY hotspotEnabledUpdated)
    bool hotspotEnabled() const;

    Q_PROPERTY(QString hotspotMode READ hotspotMode WRITE setHotspotMode NOTIFY hotspotModeUpdated)
    QString hotspotMode() const;

    Q_PROPERTY(QString hotspotAuth READ hotspotAuth WRITE setHotspotAuth NOTIFY hotspotAuthUpdated)
    QString hotspotAuth() const;

    Q_PROPERTY(bool hotspotStored READ hotspotStored NOTIFY hotspotStoredUpdated)
    bool hotspotStored() const;

    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initialized)
    bool isInitialized() const;

    Q_PROPERTY(QAbstractItemModel* vpnConnections READ vpnConnections NOTIFY vpnConnectionsUpdated)
    QAbstractItemModel* vpnConnections() const;

    Q_PROPERTY(bool mobileDataEnabled
               READ mobileDataEnabled
               WRITE setMobileDataEnabled
               NOTIFY mobileDataEnabledUpdated
               REVISION 1)
    bool mobileDataEnabled() const;

    Q_PROPERTY(Sim* simForMobileData
               READ simForMobileData
               WRITE setSimForMobileData
               NOTIFY simForMobileDataUpdated
               REVISION 1)
    Sim* simForMobileData() const;

    Q_PROPERTY(QAbstractItemModel* modems
               READ modems
               REVISION 1
               CONSTANT)
    QAbstractItemModel* modems() const;

    Q_PROPERTY(QAbstractItemModel* sims
               READ sims
               REVISION 1
               CONSTANT)
    QAbstractItemModel* sims() const;

public Q_SLOTS:
    void setFlightMode(bool enabled);

    void setwifiEnabled(bool enabled);

    void setHotspotEnabled(bool active);

    void setHotspotSsid(const QByteArray& ssid);

    void setHotspotPassword(const QString& password);

    void setHotspotMode(const QString& mode);

    void setHotspotAuth(const QString& auth);

    void setMobileDataEnabled(bool enabled);

    void setSimForMobileData(Sim *sim);

Q_SIGNALS:
    void flightModeUpdated(bool);

    void onlineUpdated(bool value);

    void limitedBandwithUpdated(bool value);

    void limitationsUpdated(const QVector<Limitations>&);

    void statusUpdated(connectivityqt::Connectivity::Status value);

    void wifiEnabledUpdated(bool);

    void unstoppableOperationHappeningUpdated(bool);

    void flightModeSwitchEnabledUpdated(bool);

    void wifiSwitchEnabledUpdated(bool);

    void hotspotSwitchEnabledUpdated(bool);

    void hotspotSsidUpdated(const QByteArray& name);

    void hotspotPasswordUpdated(const QString& password);

    void modemAvailableUpdated(bool);

    void hotspotEnabledUpdated(bool);

    void hotspotModeUpdated(const QString& mode);

    void hotspotAuthUpdated(const QString& auth);

    void hotspotStoredUpdated(bool);

    void reportError(int reason);

    void initialized();

    void vpnConnectionsUpdated(QAbstractItemModel*);

    void mobileDataEnabledUpdated(bool);

    void simForMobileDataUpdated(Sim* sim);

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}

Q_DECLARE_METATYPE(connectivityqt::Connectivity::Limitations)
Q_DECLARE_METATYPE(QVector<connectivityqt::Connectivity::Limitations>)
Q_DECLARE_METATYPE(connectivityqt::Connectivity::Status)

