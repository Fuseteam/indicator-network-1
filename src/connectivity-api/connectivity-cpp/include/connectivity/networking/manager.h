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

#include <connectivity/networking/link.h>
#include <connectivity/networking/service.h>

#include <core/property.h>
#include <core/signal.h>

#include <memory>

namespace connectivity {
namespace networking {

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
Manager
{
public:
    Manager &operator=(const Manager&) = delete;
    virtual ~Manager()                 = default;
    Manager(const Manager&)            = delete;

    /**
     * @brief Creates a new instance of a networking Manager.
     *
     * \snippet example_networking_status.cpp create manager
     *
     * Applications should call this function just once.
     * If application needs to share the instance internally the std::unique_ptr
     * can be transformed into a std::shared_ptr:
     * @code
     *     std::shared_ptr<Manager> mgr{std::move(Manager::createInstance())};
     * @endcode
     *
     * @note This call may block.
     *
     * @return std::unique_ptr to new instance of a networking manager.
     */
    static std::unique_ptr<Manager> createInstance();

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
    virtual void enableFlightMode()  = 0;

    /// @private
    virtual void disableFlightMode() = 0;

    /// @private
    virtual const core::Property<FlightModeStatus>& flightMode() const = 0;

    /// @private
    virtual const core::Property<std::set<Link::Ptr>>&    links() const = 0;

    /// @private
    virtual const core::Property<std::set<Service::Ptr>>& services() const = 0;

    /**
     * status of the overall system networking
     *
     * \snippet example_networking_status.cpp subscribe networking status changes
     */
    virtual const core::Property<NetworkingStatus> &status() const = 0;

    /**
     * characteristics of the overall system networking
     *
     * the value is a bitfield and the individial bits are defined
     * in Link::Characteristics.
     *
     * \snippet example_networking_status.cpp subscribe characteristics changes
     */
    virtual const core::Property<std::uint32_t>& characteristics() const = 0;

    virtual const core::Property<bool>& hasWifi() const = 0;
    virtual const core::Property<bool>& wifiEnabled() const = 0;
    virtual bool enableWifi() = 0;
    virtual bool disableWifi() = 0;

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
}
