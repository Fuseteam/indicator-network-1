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

#include <connectivity/networking/service.h>

namespace connectivity {
namespace networking {
namespace service {
namespace tor {

/// @private
class
Service : public connectivity::networking::Service
{
public:
    typedef std::shared_ptr<Service> Ptr;
    virtual ~Service() = default;

    virtual const core::Property<std::shared_ptr<Link>> &establishedOver() const = 0;
};

}
}
}
}
