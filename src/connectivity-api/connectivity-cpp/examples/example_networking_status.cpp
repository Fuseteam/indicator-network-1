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

#include <signal.h>

//! [include]
#include <connectivity/networking/manager.h>
namespace networking = connectivity::networking;
//! [include]

int
main(int, char *[])
{
    //! [create manager]
    auto mgr = networking::Manager::createInstance();
    //! [create manager]

    // Subscribe to system networking changes
    //! [subscribe networking status changes]
    mgr->status().changed().connect(
                [](networking::Manager::NetworkingStatus status)
    {
        switch(status) {
        case networking::Manager::NetworkingStatus::offline:
        {
            std::cout << "System networking status changed to: offline" << std::endl;
            break;
        }
        case networking::Manager::NetworkingStatus::connecting:
        {
            std::cout << "System networking status changed to: connecting" << std::endl;
            break;
        }
        case networking::Manager::NetworkingStatus::online:
        {
            std::cout << "System networking status changed to: online" << std::endl;
            break;
        }
        }
    });
    //! [subscribe networking status changes]

    // Subscribe to characteristics changes
    //! [subscribe characteristics changes]
    mgr->characteristics().changed().connect(
                [](std::uint32_t characteristics)
    {
        std::cout << "System networking characteristics changed:" << std::endl;
        if ((characteristics & networking::Link::Characteristics::has_monetary_costs) != 0) {
            std::cout << "    - has monetary costs" << std::endl;
        }
        if ((characteristics & networking::Link::Characteristics::is_bandwidth_limited) != 0) {
            std::cout << "    - is bandwith limited" << std::endl;
        }

        if ((characteristics & networking::Link::Characteristics::is_volume_limited) != 0) {
            std::cout << "    - is volume_limited" << std::endl;
        }
    });
    //! [subscribe characteristics changes]

    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    int signal;
    sigwait(&signal_set, &signal);

    return 0;
}
