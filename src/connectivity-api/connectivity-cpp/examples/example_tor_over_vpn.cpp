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
#include <connectivity/networking/service/tor/service.h>
#include <connectivity/networking/service/vpn/service.h>

#include <exception>

namespace cuc = connectivity;

int
main(int, char *[])
{
    auto mgr = cuc::networking::Manager::createInstance();

    // this example assumes that there is only one VPN link configured
    // and the Tor service is configured to require that VPN link

    std::shared_ptr<cuc::networking::service::vpn::Service> vpn;
    std::shared_ptr<cuc::networking::service::tor::Service> tor;

    for (auto service : mgr->services().get())
    {
        if (service->type() == cuc::networking::Service::Type::tor) {
            tor = std::dynamic_pointer_cast<cuc::networking::service::tor::Service>(service);
            continue;
        }
        if (service->type() == cuc::networking::Service::Type::vpn) {
            vpn = std::dynamic_pointer_cast<cuc::networking::service::vpn::Service>(service);
            continue;
        }
    }
    try {
        if (!vpn || !tor)
            throw std::logic_error("not configured properly");

        // just do some verifying.
        // VPN and Tor are configured using private APIs.

        if (tor->requires() == vpn)
            std::cout << "tor requires vpn: check." << std::endl;
        else
            throw std::logic_error("not configured properly");

        if (tor->establishedOver() == vpn->link())
            std::cout << "tor uses the vpn link: check." << std::endl;

        tor->status().changed().connect(
                    [](cuc::networking::Service::Status status)
        {
            switch(status) {
            case cuc::networking::Service::Status::stopped: {
                std::cout << "Tor stopped" << std::endl;
            }
            case cuc::networking::Service::Status::starting: {
                std::cout << "Tor starting" << std::endl;
            }
            case cuc::networking::Service::Status::running: {
                std::cout << "Tor running" << std::endl;
            }
            }
        });
        // now, just start tor.
        // this will also bring up VPN service and the specific VPN link
        tor->start();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
