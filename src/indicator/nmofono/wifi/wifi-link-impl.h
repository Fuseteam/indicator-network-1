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

#ifndef PLATFORM_TEST_WIFI_LINK
#define PLATFORM_TEST_WIFI_LINK

#include <nmofono/kill-switch-impl.h>
#include <nmofono/wifi/wifi-link.h>

#include <NetworkManagerInterface.h>
#include <NetworkManagerDeviceInterface.h>

namespace platform {
namespace nmofono {
namespace wifi {
    class Link;
}
}
}

class platform::nmofono::wifi::Link : public connectivity::networking::wifi::Link
{
    Q_OBJECT

public:

    Link(std::shared_ptr<OrgFreedesktopNetworkManagerDeviceInterface> dev,
         std::shared_ptr<OrgFreedesktopNetworkManagerInterface> nm,
         KillSwitch::Ptr killSwitch);
    ~Link();

    // public API
    void enable() override;
    void disable() override;

    Type type() const override;
    Id id() const override;
    QString name() const override;

    std::uint32_t characteristics() const override;
    Status status() const override;

    const std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>>& accessPoints() const override;
    Q_PROPERTY(std::set<connectivity::networking::wifi::AccessPoint::Ptr> rawAccessPoints READ rawAccessPoints NOTIFY rawAccessPointsUpdated)
    const std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>>& rawAccessPoints() const;
    void connect_to(std::shared_ptr<connectivity::networking::wifi::AccessPoint> accessPoint) override;
    std::shared_ptr<connectivity::networking::wifi::AccessPoint> activeAccessPoint() override;

    // private API

//    void updateDeviceState(uint new_state);
//    void updateActiveConnection(const QDBusObjectPath &path);

    QDBusObjectPath device_path() const;

Q_SIGNALS:
    void rawAccessPointsUpdated(const std::set<connectivity::networking::wifi::AccessPoint::Ptr>&);

private:
    struct Private;
    std::unique_ptr<Private> d;
};

#endif
