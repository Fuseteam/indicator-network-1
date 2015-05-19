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

#include <memory>

namespace connectivity {
namespace networking {

#ifndef CONNECTIVITY_CPP_EXPORT
#define CONNECTIVITY_CPP_EXPORT __attribute ((visibility ("default")))
#endif

/// @private
class CONNECTIVITY_CPP_EXPORT
Service
{
public:

    typedef std::shared_ptr<Service> Ptr;

    enum class Type {
        vpn,
        tethering,
        tor
    };
    virtual Type type() const = 0;

    enum class Status {
        stopped,
        starting,
        running
    };

    virtual ~Service() = default;

    virtual const core::Property<Status>& status() const = 0;

    // which other Service this service requires to be active
    // before it can be activated
    virtual std::shared_ptr<Service> requires() = 0;

    // possible link this service provides.
    // check with:
    //    if (service->link()) {
    //        // we have a link coming from the service
    //        do_something_with(link);
    //    }
    virtual Link::Ptr link() = 0;

    virtual void start()  = 0;
    virtual void stop() = 0;

    typedef unsigned int Id;
    virtual Id id() const = 0;

    virtual core::Property<std::string>& name() const = 0;
};

}
}
