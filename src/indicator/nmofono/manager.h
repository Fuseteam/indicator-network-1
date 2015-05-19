/*
 * Copyright © 2013 Canonical Ltd.
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

#include <nmofono/link.h>
#include <nmofono/wifi/wifi-link.h>
#include <nmofono/wwan/modem.h>

#include <memory>
#include <QSet>
#include <QObject>

namespace nmofono {

#ifndef CONNECTIVITY_CPP_EXPORT
#define CONNECTIVITY_CPP_EXPORT __attribute ((visibility ("default")))
#endif

/**
 * @brief networking manager
 *
 * This is the top-level manager class for accessing networking information.
 *
 * A new instance of this class can only be created using the static Manager::createInstance().
 *
 * For system networking status, see Manager::status.
 * For connection characteristics, see Manager::characteristics.
 *
 * Examples:
 * - \ref networking-status "Getting the networking status."
 */
class CONNECTIVITY_CPP_EXPORT
Manager: public QObject
{
    Q_OBJECT
public:
    typedef std::shared_ptr<Manager> Ptr;

    Manager &operator=(const Manager&) = delete;
    virtual ~Manager()                 = default;
    Manager(const Manager&)            = delete;

    /// @private
    enum class FlightModeStatus {
        on,
        off
    };

    /**
     * @brief enum class for networking status
     *
     * Networking status may be accessed through the Manager::status property.
     */
    enum class NetworkingStatus {
        /** No Internet connection available. */
        offline,

        /** System is actively establising a connection. */
        connecting,

        /** System is connected to the Internet. */
        online
    };

    /// @private
    Q_PROPERTY(Manager::FlightModeStatus flightMode READ flightMode NOTIFY flightModeUpdated)
    virtual FlightModeStatus flightMode() const = 0;

    Q_PROPERTY(bool unstoppableOperationHappening READ unstoppableOperationHappening NOTIFY unstoppableOperationHappeningUpdated)
    virtual bool unstoppableOperationHappening() const = 0;

    /// @private
    Q_PROPERTY(QSet<Link::Ptr> links READ links NOTIFY linksUpdated)
    virtual QSet<Link::Ptr> links() const = 0;

    virtual QSet<wifi::WifiLink::Ptr> wifiLinks() const = 0;

    virtual QSet<wwan::Modem::Ptr> modemLinks() const = 0;

    /**
     * status of the overall system networking
     *
     * \snippet example_networking_status.cpp subscribe networking status changes
     */
    Q_PROPERTY(Manager::NetworkingStatus status READ status NOTIFY statusUpdated)
    virtual NetworkingStatus status() const = 0;

    /**
     * characteristics of the overall system networking
     *
     * the value is a bitfield and the individial bits are defined
     * in Link::Characteristics.
     *
     * \snippet example_networking_status.cpp subscribe characteristics changes
     */
    Q_PROPERTY(std::uint32_t characteristics READ characteristics NOTIFY characteristicsUpdated)
    virtual std::uint32_t characteristics() const = 0;

    Q_PROPERTY(bool hasWifi READ hasWifi NOTIFY hasWifiUpdated)
    virtual bool hasWifi() const = 0;

    Q_PROPERTY(bool wifiEnabled READ wifiEnabled NOTIFY wifiEnabledUpdated)
    virtual bool wifiEnabled() const = 0;

    virtual bool roaming() const = 0;

    virtual void unlockModem(wwan::Modem::Ptr modem) = 0;

    virtual void unlockAllModems() = 0;

    virtual void unlockModemByName(const QString &name) = 0;

Q_SIGNALS:
    void flightModeUpdated(FlightModeStatus);

    void linksUpdated();

    void statusUpdated(NetworkingStatus);

    void characteristicsUpdated(std::uint32_t);

    void hasWifiUpdated(bool);

    void wifiEnabledUpdated(bool);

    void unstoppableOperationHappeningUpdated(bool);

public Q_SLOTS:
    virtual bool setWifiEnabled(bool) = 0;

    virtual void setFlightMode(bool)  = 0;

protected:
    /**
     * @brief The default constructor is protected.
     *
     * To create an instance of the networking manager call Manager::createInstance().
     *
     */
    Manager();
};

}
