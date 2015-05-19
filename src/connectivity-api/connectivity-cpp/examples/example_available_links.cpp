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

int
main(int, char *[])
{
    auto mgr = cuc::networking::Manager::createInstance();

    for (auto link : mgr->links().get()) {
        switch (link->type()) {
        case(cuc::networking::Link::Type::wifi): {
            std::cout << "Wi-Fi link: " << std::endl;

            auto wifilink = std::dynamic_pointer_cast<cuc::networking::wifi::Link>(link);

            std::cout << "  Available Wi-Fi access points:" << std::endl;
            for (auto ap : wifilink->accessPoints().get()) {
                std::cout << "    " << ap->ssid()
                          << ", secured: " << (ap->secured() ? "yes" : "no")
                          << ", strength: " << ap->strength().get()
                          << ", adhoc: " << (ap->adhoc() ? "yes" : "no")
                          << std::endl;
            }
            break;
        }
        case(cuc::networking::Link::Type::wired): {
            std::cout << "Wired link" << std::endl;
            break;
        }
        case(cuc::networking::Link::Type::wwan): {
            std::cout << "WWan link" << std::endl;
            break;
        }
        case(cuc::networking::Link::Type::service): {
            std::cout << "Service link" << std::endl;
            break;

        }
        }
    }

    return 0;
}
