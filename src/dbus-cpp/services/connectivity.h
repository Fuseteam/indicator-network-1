/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authors: Antti Kaijanmäki <antti.kaijanmaki@canonical.com
 */
#ifndef DBUS_CPP_SERVICES_UBUNTU_CONNECTIVITY_H
#define DBUS_CPP_SERVICES_UBUNTU_CONNECTIVITY_H

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/struct.h>
#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#define UBUNTU_CONNECTIVITY_DBUS_API_VERSION "1"
#define UBUNTU_CONNECTIVITY_NAME_BASE "com.ubuntu.connectivity" UBUNTU_CONNECTIVITY_DBUS_API_VERSION
#define UBUNTU_CONNECTIVITY_PATH_BASE "/com/ubuntu/connectivity" UBUNTU_CONNECTIVITY_DBUS_API_VERSION

#define UBUNTU_CONNECTIVITY_SERVICE_NAME UBUNTU_CONNECTIVITY_NAME_BASE
#define UBUNTU_CONNECTIVITY_SERVICE_PATH UBUNTU_CONNECTIVITY_PATH_BASE

namespace com
{
namespace ubuntu
{
namespace connectivity
{

struct Interface
{
    struct NetworkingStatus
    {
        static const std::string& name()
        {
            static const std::string s{UBUNTU_CONNECTIVITY_NAME_BASE ".NetworkingStatus"};
            return s;
        }
        static const std::string& path()
        {
            static const std::string s{UBUNTU_CONNECTIVITY_PATH_BASE "/NetworkingStatus"};
            return s;
        }

        struct Property
        {
            struct Limitations {
                static const std::string &name()
                {
                    static const std::string s{"Limitations"};
                    return s;
                }

                typedef NetworkingStatus Interface;
                typedef std::vector<std::string> ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Status {
                static const std::string &name()
                {
                    static const std::string s{"Status"};
                    return s;
                }

                typedef NetworkingStatus Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
        };

        NetworkingStatus(std::shared_ptr<core::dbus::Service> &service,
                         std::shared_ptr<core::dbus::Object> &object)
            : service(service),
              object(object)
        {

        }

        std::shared_ptr<core::dbus::Service> service;
        std::shared_ptr<core::dbus::Object>  object;
    };

    struct Private
    {
        static const std::string& name()
        {
            static const std::string s{UBUNTU_CONNECTIVITY_NAME_BASE ".Private"};
            return s;
        }
        static const std::string& path()
        {
            static const std::string s{UBUNTU_CONNECTIVITY_PATH_BASE "/Private"};
            return s;
        }

        struct Method
        {
            struct UnlockAllModems {
                static const std::string& name()
                {
                    static const std::string s{"UnlockAllModems"};
                    return s;
                }

                typedef Private Interface;
                typedef void ValueType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct UnlockModem {
                static const std::string& name()
                {
                    static const std::string s{"UnlockModem"};
                    return s;
                }

                typedef Private Interface;
                typedef void ValueType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };
        };

        Private(std::shared_ptr<core::dbus::Service> &service,
                std::shared_ptr<core::dbus::Object> &object)
            : service(service),
              object(object)
        {}

        std::shared_ptr<core::dbus::Service> service;
        std::shared_ptr<core::dbus::Object>  object;
    };


}; // Interface

struct Service
{
    std::shared_ptr<Interface::NetworkingStatus> networkingStatus;

    static const std::string& name()
    {
        static const std::string s{UBUNTU_CONNECTIVITY_SERVICE_NAME};
        return s;
    }

    Service(const core::dbus::Bus::Ptr& bus)
    {
        auto service = core::dbus::Service::use_service<Service>(bus);
        auto object = service->object_for_path(core::dbus::types::ObjectPath(Interface::NetworkingStatus::path()));
        try {
            networkingStatus = std::make_shared<Interface::NetworkingStatus>(service, object);
        } catch (const std::exception &e) {
            std::cerr << "Failed to access NetworkingStatus interface: " << e.what() << std::endl;
        }
    }

    struct Mock
    {
        std::shared_ptr<Interface::NetworkingStatus> networkingStatus;

        Mock(const core::dbus::Bus::Ptr& bus)
        {
            auto service = core::dbus::Service::add_service<Service>(bus);
            auto object = service->add_object_for_path(core::dbus::types::ObjectPath(Interface::NetworkingStatus::path()));
            networkingStatus = std::make_shared<Interface::NetworkingStatus>(service, object);
        }
    };
};

}
}
}

#endif // DBUS_CPP_SERVICES_UBUNTU_CONNECTIVITY_H

