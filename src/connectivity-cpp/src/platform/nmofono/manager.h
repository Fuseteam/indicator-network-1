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
#ifndef PLATFORM_NMOFONO_MANAGER
#define PLATFORM_NMOFONO_MANAGER

#include <connectivity/networking/manager.h>
#include <core/dbus/types/object_path.h>

namespace core {
namespace dbus {
    class Bus;
}
}

namespace platform {
namespace nmofono {
    class Manager;
    class Network;
}
}

class platform::nmofono::Manager : public connectivity::networking::Manager
{
    struct Private;
    struct State;
    std::unique_ptr<Private> p;

    void updateNetworkingStatus(std::uint32_t);

public:

    Manager();

    // Public API
    void enableFlightMode() override;
    void disableFlightMode() override;
    const core::Property<connectivity::networking::Manager::FlightModeStatus>& flightMode() const override;

    const core::Property<bool>& hasWifi() const override;
    const core::Property<bool>& wifiEnabled() const override;

    bool enableWifi() override;
    bool disableWifi() override;

    const core::Property<std::set<std::shared_ptr<connectivity::networking::Link>>>& links() const;
    const core::Property<std::set<std::shared_ptr<connectivity::networking::Service>>>&services() const override;
    const core::Property<connectivity::networking::Manager::NetworkingStatus> & status() const override;
    const core::Property<std::uint32_t>& characteristics() const override;

private:
    void device_added(const core::dbus::types::ObjectPath &path);
};

#endif
