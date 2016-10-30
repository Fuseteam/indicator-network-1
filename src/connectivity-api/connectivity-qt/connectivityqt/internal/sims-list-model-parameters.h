/*
 * Copyright © 2016 Canonical Ltd.
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

#include <util/dbus-property-cache.h>

#include <NetworkingStatusPrivateInterface.h>

#include <functional>
#include <memory>

namespace connectivityqt
{
namespace internal
{

struct SimsListModelParameters
{
    std::function<void(QObject*)> objectOwner;
    std::shared_ptr<ComUbuntuConnectivity1PrivateInterface> writeInterface;
    std::shared_ptr<util::DBusPropertyCache> propertyCache;
};

}
}
