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

#include <iostream>
#include <memory>

#include <connectivity/networking/manager.h>
#include <connectivity/networking/wifi/link.h>

namespace cuc = connectivity;

int main(int, char *[])
{
    auto mgr = cuc::networking::Manager::createInstance();

    // Store the first network over a wireless link with snr > 80%
    std::shared_ptr<cuc::networking::wifi::AccessPoint> wireless_network;

    for (auto link : mgr->links().get())
    {
        if (link->type() != cuc::networking::Link::Type::wifi)
            continue;

        auto wifilink = std::dynamic_pointer_cast<cuc::networking::wifi::Link>(link);

        for (auto ap : wifilink->accessPoints().get()) {
            if (ap->strength().get() > 80.0) {
                wireless_network = ap;
                break;
            }
        }

        if (wireless_network) {
            std::cout << "Connecting to " << wireless_network->ssid() << std::endl;
            wifilink->connect_to(wireless_network);
            break;
        }
    }

    if (!wireless_network) {
        std::cout << "Could not find suitable network." << std::endl;
    }

    return 0;
}
