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
#ifndef CONNECTIVITY_NETWORKING_WWAN_DEVICE
#define CONNECTIVITY_NETWORKING_WWAN_DEVICE

#include <memory>
#include <string>

namespace connectivity {
namespace networking {
namespace wwan {

#ifndef CONNECTIVITY_CPP_EXPORT
#define CONNECTIVITY_CPP_EXPORT __attribute ((visibility ("default")))
#endif

class CONNECTIVITY_CPP_EXPORT
Link : public connectivity::networking::Link
{
public:
    typedef std::shared_ptr<Link> Ptr;
    enum class Type {
        GSM,
        CDMA,
        BLUETOOTH_DUN,
        BLUETOOTH_PAN
    };

    virtual ~Link() {}

    virtual State state()   const = 0;
    virtual Type wwanType() const = 0;
};

}
}

#endif
