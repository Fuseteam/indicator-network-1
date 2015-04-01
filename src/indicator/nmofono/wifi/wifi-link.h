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

#ifndef CONNECTIVITY_NETWORKING_WIFI_LINK
#define CONNECTIVITY_NETWORKING_WIFI_LINK

#include <nmofono/link.h>
#include <nmofono/wifi/access-point.h>

#include <set>

namespace connectivity {
namespace networking {
namespace wifi {

#ifndef CONNECTIVITY_CPP_EXPORT
#define CONNECTIVITY_CPP_EXPORT __attribute ((visibility ("default")))
#endif

/// @private
class CONNECTIVITY_CPP_EXPORT
Link : public connectivity::networking::Link
{
    Q_OBJECT

public:
    typedef std::shared_ptr<Link> Ptr;
    typedef unsigned int Id;

    Link() = default;
    Link(const Link&) = delete;
    virtual ~Link() = default;

    Q_PROPERTY(std::set<connectivity::networking::wifi::AccessPoint::Ptr> accessPoints READ accessPoints NOTIFY accessPointsUpdated)
    virtual const std::set<AccessPoint::Ptr>& accessPoints() const = 0;

    virtual void connect_to(AccessPoint::Ptr accessPoint) = 0;

    Q_PROPERTY(connectivity::networking::wifi::AccessPoint::Ptr activeAccessPoint READ activeAccessPoint NOTIFY activeAccessPointUpdated)
    virtual AccessPoint::Ptr activeAccessPoint() = 0;

Q_SIGNALS:
    void accessPointsUpdated(const std::set<AccessPoint::Ptr>&);

    void activeAccessPointUpdated(AccessPoint::Ptr);

};

}
}
}

#endif
