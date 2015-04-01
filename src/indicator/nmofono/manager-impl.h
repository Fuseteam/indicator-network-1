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

#include <nmofono/manager.h>

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QVariantMap>

namespace platform {
namespace nmofono {
    class Manager;
    class Network;
}
}

class platform::nmofono::Manager : public connectivity::networking::Manager
{
Q_OBJECT
    class Private;
    struct State;
    std::shared_ptr<Private> d;

    void updateNetworkingStatus(uint state);

public:

    Manager(const QDBusConnection& systemBus);

    // Public API
    void enableFlightMode() override;
    void disableFlightMode() override;
    connectivity::networking::Manager::FlightModeStatus flightMode() const override;

    bool hasWifi() const override;
    bool wifiEnabled() const override;

    bool enableWifi() override;
    bool disableWifi() override;

    const std::set<std::shared_ptr<connectivity::networking::Link>>& links() const;
    connectivity::networking::Manager::NetworkingStatus status() const override;
    std::uint32_t characteristics() const override;

private Q_SLOTS:
    void device_added(const QDBusObjectPath &path);
    void device_removed(const QDBusObjectPath &path);
    void nm_properties_changed(const QVariantMap &properties);
};

#endif
