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
 *     Jussi Pakkanen <jussi.pakkanen@canonical.com>
 */

#ifndef PLATFORM_NMOFONO_WIFI_GROUPED_ACCESS_POINT
#define PLATFORM_NMOFONO_WIFI_GROUPED_ACCESS_POINT

#include <nmofono/wifi/access-point.h>

#include <chrono>

#include <QDBusObjectPath>
#include <QString>

namespace platform {
namespace nmofono {
namespace wifi {

class AccessPoint;

// A class that joins multiple access points of the same type into one.
// Signal strength is the maximum of all subaccesspoints.

class GroupedAccessPoint : public connectivity::networking::wifi::AccessPoint
{
    Q_OBJECT

public:
    GroupedAccessPoint(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap);
    double strength() const;
    virtual ~GroupedAccessPoint();

    // time when last connected to this access point
    // for APs that have never been connected the
    // lastConnected->time_since_epoch().count() is 0
    Q_PROPERTY(std::chrono::system_clock::time_point lastConnected READ lastConnected NOTIFY lastConnectedUpdated)
    std::chrono::system_clock::time_point lastConnected() const;

    const QString& ssid() const override;
    const QByteArray& raw_ssid() const;

    bool secured() const override;

    bool adhoc() const override;

    QDBusObjectPath object_path() const;

    void add_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap);
    void remove_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap);
    int num_aps() const;

    bool has_object(const QDBusObjectPath &path) const;

Q_SIGNALS:
    void lastConnectedUpdated(std::chrono::system_clock::time_point lastConnected);

private:
    struct Private;
    std::unique_ptr<Private> p;
};

}
}
}

#endif
