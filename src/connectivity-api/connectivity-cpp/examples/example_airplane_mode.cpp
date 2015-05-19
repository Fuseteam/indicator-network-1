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

#include <signal.h>

namespace cuc = connectivity;

int
main(int, char *[])
{
    auto mgr = cuc::networking::Manager::createInstance();

    if (mgr->flightMode().get()
            == cuc::networking::Manager::FlightModeStatus::off)
        std::cout << "Initial Flight Mode: Off" << std::endl;
    else
        std::cout << "Initial Flight Mode: On" << std::endl;

    mgr->flightMode().changed().connect(
        [](cuc::networking::Manager::FlightModeStatus status) {
        switch (status) {
        case cuc::networking::Manager::FlightModeStatus::on:
            std::cout << "Flight Mode is On" << std::endl;
            break;
        case cuc::networking::Manager::FlightModeStatus::off:
            std::cout << "Flight Mode Off" << std::endl;
            break;
        }
    });

    mgr->enableFlightMode();

    // enable wifi devices while in flightmode
    for (auto link : mgr->links().get()) {
        if (link->type() == cuc::networking::Link::Type::wifi) {
            link->enable();
        }
    }

    mgr->disableFlightMode();

    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    int signal;
    sigwait(&signal_set, &signal);


    return 0;
}
