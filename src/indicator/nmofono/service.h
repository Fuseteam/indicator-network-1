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

#ifndef PLATFORM_NMOFONO_SERVICE
#define PLATFORM_NMOFONO_SERVICE

#include <com/ubuntu/connectivity/networking/service.h>

namespace platform {
namespace nmofono {
    class Service;
}
}

class platform::nmofono::Service : public connectivity::networking::Service
{
public:
    Type type() const;
    const Property<Status>& status();
    std::shared_ptr<Service> requires();
    std::shared_ptr<Link> link();

    void start();
    void stop();

    Id id() const;

    Property<std::string>& name();
};

#endif
