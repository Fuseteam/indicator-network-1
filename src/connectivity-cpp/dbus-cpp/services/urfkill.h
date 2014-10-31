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
 * Authored by: Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */
#ifndef PLATFORM_MANAGER_NMOFONO_URFKILL_H
#define PLATFORM_MANAGER_NMOFONO_URFKILL_H

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/any.h>
#include <core/dbus/types/struct.h>
#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>
#include "util.h"

namespace org
{
namespace freedesktop
{
namespace URfkill
{

struct Interface
{
    struct Killswitch
    {
        static const std::string& name()
        {
            static const std::string s{"org.freedesktop.URfkill.Killswitch"};
            return s;
        }

        struct Property
        {
            struct State
            {
                static const std::string& name()
                {
                    static const std::string s{"state"};
                    return s;
                }

                typedef Killswitch Interface;
                typedef std::int32_t ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
        };

        enum class Type;
        Killswitch(const std::shared_ptr<core::dbus::Object>& object,
                   Type type)
            : object(object),
              type(type),
              _state(object->get_property<Property::State>())
        {
            _state->changed().connect(std::bind(&Killswitch::_stateChanged, this, std::placeholders::_1));
            _stateChanged(_state->get());
        }

        enum class State
        {
            not_available = -1,
            unblocked = 0,
            soft_blocked = 1,
            hard_blocked = 2
        };

        enum class Type {
            bluetooth,
            fm,
            gps,
            nfc,
            uwb,
            wimax,
            wlan,
            wwan
        };

        static const std::string path(Type type)
        {
            switch(type) {
            case Type::bluetooth:
                return "/org/freedesktop/URfkill/BLUETOOTH";
            case Type::fm:
                return "/org/freedesktop/URfkill/FM";
            case Type::gps:
                return "/org/freedesktop/URfkill/GPS";
            case Type::nfc:
                return "/org/freedesktop/URfkill/NFC";
            case Type::uwb:
                return "/org/freedesktop/URfkill/UWB";
            case Type::wimax:
                return "/org/freedesktop/URfkill/WIMAX";
            case Type::wlan:
                return "/org/freedesktop/URfkill/WLAN";
            case Type::wwan:
                return "/org/freedesktop/URfkill/WWAN";
            }

            return "";
        }

        void _stateChanged(Property::State::ValueType val)
        {
            if (val == -1)
                state.set(State::not_available);
            else if (val == 0)
                state.set(State::unblocked);
            else if (val == 1)
                state.set(State::soft_blocked);
            else if (val == 2)
                state.set(State::hard_blocked);
            else
                throw std::runtime_error("got unknown Killswitch state from urfkill: " + std::to_string(val));
        }

        std::shared_ptr<core::dbus::Object> object;
        const Type type;
        std::shared_ptr<core::dbus::Property<Property::State>> _state;
        core::Property<State> state;
    };

    struct URfkill
    {
        static const std::string& name()
        {
            static const std::string s{"org.freedesktop.URfkill"};
            return s;
        }

        struct Method
        {
            struct Block
            {
                static const std::string& name()
                {
                    static const std::string s{"Block"};
                    return s;
                }

                typedef URfkill Interface;
                typedef bool ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct FlightMode
            {
                static const std::string& name()
                {
                    static const std::string s{"FlightMode"};
                    return s;
                }

                typedef URfkill Interface;
                typedef bool ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct IsFlightMode
            {
                static const std::string& name()
                {
                    static const std::string s{"IsFlightMode"};
                    return s;
                }

                typedef URfkill Interface;
                typedef bool ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };
        };

        struct Signal
        {
            struct FlightModeChanged
            {
                static const std::string& name()
                {
                    static const std::string s{"FlightModeChanged"};
                    return s;
                }

                typedef URfkill Interface;
                typedef bool ArgumentType;
            };
        };

        URfkill(std::shared_ptr<core::dbus::Service> &service,
                std::shared_ptr<core::dbus::Object> &object)
            : service{service},
              object{object},
              flightModeChanged{object->get_signal<Signal::FlightModeChanged>()}
        {
            switches[Killswitch::Type::bluetooth] = std::make_shared<Killswitch>(service->object_for_path(Killswitch::path(Killswitch::Type::bluetooth)), Killswitch::Type::bluetooth);
            switches[Killswitch::Type::fm]        = std::make_shared<Killswitch>(service->object_for_path(Killswitch::path(Killswitch::Type::fm)),        Killswitch::Type::fm);
            switches[Killswitch::Type::gps]       = std::make_shared<Killswitch>(service->object_for_path(Killswitch::path(Killswitch::Type::gps)),       Killswitch::Type::gps);
            switches[Killswitch::Type::nfc]       = std::make_shared<Killswitch>(service->object_for_path(Killswitch::path(Killswitch::Type::nfc)),       Killswitch::Type::nfc);
            switches[Killswitch::Type::uwb]       = std::make_shared<Killswitch>(service->object_for_path(Killswitch::path(Killswitch::Type::uwb)),       Killswitch::Type::uwb);
            switches[Killswitch::Type::wimax]     = std::make_shared<Killswitch>(service->object_for_path(Killswitch::path(Killswitch::Type::wimax)),     Killswitch::Type::wimax);
            switches[Killswitch::Type::wlan]      = std::make_shared<Killswitch>(service->object_for_path(Killswitch::path(Killswitch::Type::wlan)),      Killswitch::Type::wlan);
            switches[Killswitch::Type::wwan]      = std::make_shared<Killswitch>(service->object_for_path(Killswitch::path(Killswitch::Type::wwan)),      Killswitch::Type::wwan);
        }

        bool block(Killswitch::Type type, bool block)
        {
            std::uint32_t utype;
            switch (type) {
            case Killswitch::Type::bluetooth:
                utype = 2;
                break;
            case Killswitch::Type::fm:
                utype = 7;
                break;
            case Killswitch::Type::gps:
                utype = 6;
                break;
            case Killswitch::Type::nfc:
                utype = 8; /// @todo verify me
                break;
            case Killswitch::Type::uwb:
                utype = 3;
                break;
            case Killswitch::Type::wimax:
                utype = 4;
                break;
            case Killswitch::Type::wlan:
                utype = 1;
                break;
            case Killswitch::Type::wwan:
                utype = 5;
                break;
            }
            // we can't handle "all" (0) with this, but that's deprecated by FlightMode anyway

            auto result =
                    object->invoke_method_synchronously<
                       Method::Block, Method::Block::ResultType>
                    (utype, block);

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            return result.value();
        }

        bool flightMode(bool block)
        {
            auto result =
                    object->invoke_method_synchronously<
                       Method::FlightMode, Method::FlightMode::ResultType>
                    (block);

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            return result.value();
        }

        bool isFlightMode()
        {
            auto result =
                    object->invoke_method_synchronously<
                       Method::IsFlightMode, Method::IsFlightMode::ResultType>
                    ();

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            return result.value();
        }

        std::shared_ptr<core::dbus::Service> service;
        std::shared_ptr<core::dbus::Object> object;
        std::shared_ptr<core::dbus::Signal<Signal::FlightModeChanged, Signal::FlightModeChanged::ArgumentType>> flightModeChanged;

        std::map<Killswitch::Type, std::shared_ptr<Killswitch>> switches;
    };
};

struct Service
{
    std::shared_ptr<Interface::URfkill> urfkill;

    Service(const core::dbus::Bus::Ptr& bus)
    {
        auto service = core::dbus::Service::use_service<Interface::URfkill>(bus);
        auto object = service->object_for_path(core::dbus::types::ObjectPath("/org/freedesktop/URfkill"));
        urfkill = std::make_shared<Interface::URfkill>(service, object);
    }

    struct Mock
    {
        std::shared_ptr<Interface::URfkill> urfkill;

        Mock(const core::dbus::Bus::Ptr& bus)
        {
            auto service = core::dbus::Service::add_service<Interface::URfkill>(bus);
            auto object = service->add_object_for_path(core::dbus::types::ObjectPath("/org/freedesktop/URfkill"));
            urfkill = std::make_shared<Interface::URfkill>(service, object);
        }
    };
};
}
}
}

#endif // PLATFORM_MANAGER_NMOFONO_URFKILL_H
