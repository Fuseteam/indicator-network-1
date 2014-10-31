/*
 * Copyright © 2012-2013 Canonical Ltd.
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
#ifndef PLATFORM_MANAGER_NMOFONO_NM_H
#define PLATFORM_MANAGER_NMOFONO_NM_H

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
#include <NetworkManager/NetworkManager.h>
#include "util.h"

namespace org
{
namespace freedesktop
{
namespace NetworkManager
{
    struct Interface {

        struct AccessPoint
        {
            static const std::string& name()
            {
                static const std::string s{NM_DBUS_INTERFACE_ACCESS_POINT};
                return s;
            }

            struct Property {
                struct Frequency
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Frequency"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct Flags
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Flags"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct WpaFlags
                {
                    static const std::string& name()
                    {
                        static const std::string s{"WpaFlags"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct RsnFlags
                {
                    static const std::string& name()
                    {
                        static const std::string s{"RsnFlags"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct Mode
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Mode"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct Ssid
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Ssid"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    /// @todo bug tvoss.. D-Bus BYTE(y) should be unsigned
                    typedef std::vector<std::int8_t> ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct HwAddress
                {
                    static const std::string& name()
                    {
                        static const std::string s{"HwAddress"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    typedef std::string ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct Strength
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Strength"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    typedef std::int8_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
            };

            struct Signal {
                struct PropertiesChanged
                {
                    static const std::string& name()
                    {
                        static const std::string s{"PropertiesChanged"};
                        return s;
                    }

                    typedef AccessPoint Interface;
                    typedef std::map<std::string, core::dbus::types::Variant> ArgumentType;
                };
            };

            AccessPoint(const std::shared_ptr<core::dbus::Object>& object)
                : object(object),
                  frequency(object->get_property<Property::Frequency>()),
                  hw_address(object->get_property<Property::HwAddress>()),
                  strength(object->get_property<Property::Strength>()),
                  flags(object->get_property<Property::Flags>()),
                  wpa_flags(object->get_property<Property::WpaFlags>()),
                  rsn_flags(object->get_property<Property::RsnFlags>()),
                  mode(object->get_property<Property::Mode>()),
                  ssid(object->get_property<Property::Ssid>()),
                  properties_changed(object->get_signal<Signal::PropertiesChanged>())

            {}

            std::shared_ptr<core::dbus::Object> object;
            std::shared_ptr<core::dbus::Property<Property::Frequency>> frequency;
            std::shared_ptr<core::dbus::Property<Property::HwAddress>> hw_address;
            std::shared_ptr<core::dbus::Property<Property::Strength>> strength;
            std::shared_ptr<core::dbus::Property<Property::Flags>> flags;
            std::shared_ptr<core::dbus::Property<Property::WpaFlags>> wpa_flags;
            std::shared_ptr<core::dbus::Property<Property::RsnFlags>> rsn_flags;
            std::shared_ptr<core::dbus::Property<Property::Mode>> mode;
            std::shared_ptr<core::dbus::Property<Property::Ssid>> ssid;
            std::shared_ptr<core::dbus::Signal<Signal::PropertiesChanged, Signal::PropertiesChanged::ArgumentType>> properties_changed;
        }; // Interface::AccessPoint


        struct ActiveConnection
        {
            static const std::string& name()
            {
                static const std::string s{NM_DBUS_INTERFACE_ACTIVE_CONNECTION};
                return s;
            }

            struct Property {
                struct Connection
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Connection"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef core::dbus::types::ObjectPath ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
                struct SpecificObject
                {
                    static const std::string& name()
                    {
                        static const std::string s{"SpecificObject"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef core::dbus::types::ObjectPath ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
                struct Uuid
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Uuid"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef std::string ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
                struct Devices
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Devices"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef std::vector<core::dbus::types::ObjectPath> ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
                struct State
                {
                    static const std::string& name()
                    {
                        static const std::string s{"State"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
                struct Default
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Default"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef bool ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
                struct Default6
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Default6"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef bool ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
                struct Vpn
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Vpn"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef bool ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
                struct Master
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Master"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef bool ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
            }; // struct Property

            struct Signal {
                struct PropertiesChanged
                {
                    static const std::string& name()
                    {
                        static const std::string s{"PropertiesChanged"};
                        return s;
                    }

                    typedef ActiveConnection Interface;
                    typedef std::map<std::string, core::dbus::types::Variant> ArgumentType;
                };
            };

            enum class State
            {
                unknown      = NM_ACTIVE_CONNECTION_STATE_UNKNOWN,
                activating   = NM_ACTIVE_CONNECTION_STATE_ACTIVATING,
                activated    = NM_ACTIVE_CONNECTION_STATE_ACTIVATED,
                deactivating = NM_ACTIVE_CONNECTION_STATE_DEACTIVATING,
                deactivated  = NM_ACTIVE_CONNECTION_STATE_DEACTIVATED
            };

            ActiveConnection(const std::shared_ptr<core::dbus::Object>& object)
                : object(object),
                  connection(object->get_property<Property::Connection>()),
                  specific_object(object->get_property<Property::SpecificObject>()),
                  uuid(object->get_property<Property::Uuid>()),
                  devices(object->get_property<Property::Devices>()),
                  state(object->get_property<Property::State>()),
                  default_ipv4(object->get_property<Property::Default>()),
                  default_ipv6(object->get_property<Property::Default6>()),
                  vpn(object->get_property<Property::Vpn>()),
                  master(object->get_property<Property::Master>()),
                  properties_changed(object->get_signal<Signal::PropertiesChanged>())
            {
                token = properties_changed->connect([this](const Signal::PropertiesChanged::ArgumentType &map){
                    for (auto entry: map) {
                        if (entry.first == "State") {
#if 0
                            std::uint32_t new_value = entry.second.as<std::uint32_t>();
                            state->update([&new_value](std::uint32_t &current_value){
                                if (new_value == current_value)
                                    return false;
                                current_value = new_value;
                                return true;
                            });
#endif
                            continue;
                        }
                        else {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
                            std::cout << "Unexpected Connection.Active update: " << entry.first << std::endl;
#endif
                        }
                    }
                });
            }
            ~ActiveConnection()
            {
                /// @todo fix dbus-cpp...
                //properties_changed->disconnect(token);
            }

            State get_state() { return static_cast<State>(state->get()); }

            std::shared_ptr<core::dbus::Object> object;
            std::shared_ptr<core::dbus::Property<Property::Connection>> connection;
            std::shared_ptr<core::dbus::Property<Property::SpecificObject>> specific_object;
            std::shared_ptr<core::dbus::Property<Property::Uuid>> uuid;
            std::shared_ptr<core::dbus::Property<Property::Devices>> devices;
            std::shared_ptr<core::dbus::Property<Property::State>> state;
            std::shared_ptr<core::dbus::Property<Property::Default>> default_ipv4;
            std::shared_ptr<core::dbus::Property<Property::Default6>> default_ipv6;
            std::shared_ptr<core::dbus::Property<Property::Vpn>> vpn;
            std::shared_ptr<core::dbus::Property<Property::Master>> master;

        private:
            std::shared_ptr<core::dbus::Signal<Signal::PropertiesChanged, Signal::PropertiesChanged::ArgumentType>> properties_changed;
            core::dbus::Signal<Signal::PropertiesChanged, Signal::PropertiesChanged::ArgumentType>::SubscriptionToken token;
        };  // Interface::ActiveConnection

        struct Connection
        {
            std::shared_ptr<core::dbus::Object> object;

            Connection(const std::shared_ptr<core::dbus::Object>& object)
                : object(object)
            {}

            static const std::string& name()
            {
                static const std::string s{NM_DBUS_IFACE_SETTINGS_CONNECTION};
                return s;
            }

            struct Method {
                struct GetSettings
                {
                    static const std::string& name()
                    {
                        static const std::string s{"GetSettings"};
                        return s;
                    }

                    typedef Connection Interface;
                    typedef std::map<std::string, std::map<std::string, core::dbus::types::Variant>> ResultType;

                    static std::chrono::milliseconds default_timeout()
                    {
                        return std::chrono::seconds{30};
                    }
                };
            };

            Method::GetSettings::ResultType
            get_settings()
            {

                auto result = object->invoke_method_synchronously<Method::GetSettings, Method::GetSettings::ResultType>();

                if (result.is_error())
                    connectivity::throw_dbus_exception(result.error());

                return result.value();
            }
        }; // Interface::Connection

        struct Device
        {
            static const std::string& name()
            {
                static const std::string s{NM_DBUS_INTERFACE_DEVICE};
                return s;
            }

            enum class Type
            {
                unknown     = NM_DEVICE_TYPE_UNKNOWN,
                ethernet    = NM_DEVICE_TYPE_ETHERNET,
                wifi        = NM_DEVICE_TYPE_WIFI,
                unused_1    = NM_DEVICE_TYPE_UNUSED1,
                unused_2    = NM_DEVICE_TYPE_UNUSED2,
                bluetooth   = NM_DEVICE_TYPE_BT,
                olpc_mesh   = NM_DEVICE_TYPE_OLPC_MESH,
                wimax       = NM_DEVICE_TYPE_WIMAX,
                modem       = NM_DEVICE_TYPE_MODEM,
                infiniband  = NM_DEVICE_TYPE_INFINIBAND,
                bond        = NM_DEVICE_TYPE_BOND,
                vlan        = NM_DEVICE_TYPE_VLAN,
                adsl        = NM_DEVICE_TYPE_ADSL,
                bridge      = NM_DEVICE_TYPE_BRIDGE
            };

            struct Wireless
            {
                static const std::string& name()
                {
                    static const std::string s{NM_DBUS_INTERFACE_DEVICE_WIRELESS};
                    return s;
                }

                struct Method {
                    struct GetAccessPoints
                    {
                        static const std::string& name()
                        {
                            static const std::string s{"GetAccessPoints"};
                            return s;
                        }

                        typedef Wireless Interface;
                        typedef std::vector<core::dbus::types::ObjectPath> ResultType;

                        static std::chrono::milliseconds default_timeout()
                        {
                            return std::chrono::seconds{30};
                        }
                    };
                };

                struct Signal {
                    struct AccessPointAdded
                    {
                        static const std::string& name()
                        {
                            static const std::string s{"AccessPointAdded"};
                            return s;
                        }

                        typedef Wireless Interface;
                        typedef core::dbus::types::ObjectPath ArgumentType;
                    };

                    struct AccessPointRemoved
                    {
                        static const std::string& name()
                        {
                            static const std::string s{"AccessPointRemoved"};
                            return s;
                        }

                        typedef Wireless Interface;
                        typedef core::dbus::types::ObjectPath ArgumentType;
                    };
                };

                Wireless(std::shared_ptr<core::dbus::Object> &base)
                    : access_point_added(base->get_signal<Signal::AccessPointAdded>()),
                      access_point_removed(base->get_signal<Signal::AccessPointRemoved>())
                {}

                std::shared_ptr<core::dbus::Signal<Signal::AccessPointAdded, Signal::AccessPointAdded::ArgumentType>> access_point_added;
                std::shared_ptr<core::dbus::Signal<Signal::AccessPointRemoved, Signal::AccessPointRemoved::ArgumentType>> access_point_removed;

            }; // Interface::Device::Wireless

            struct Property {
                struct DeviceType
                {
                    static const std::string& name()
                    {
                        static const std::string s{"DeviceType"};
                        return s;
                    }

                    typedef Device Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct DeviceInterface {
                    static const std::string& name()
                    {
                        static const std::string s{"Interface"};
                        return s;
                    }

                    typedef Device Interface;
                    typedef std::string ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct Autoconnect
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Autoconnect"};
                        return s;
                    }

                    typedef Device Interface;
                    typedef bool ValueType;
                    static const bool readable = true;
                    static const bool writable = true;
                };

                struct State {
                    static const std::string &name()
                    {
                        static const std::string s{"State"};
                        return s;
                    }

                    typedef Device Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct AvailableConnections {
                    static const std::string &name()
                    {
                        static const std::string s{"AvailableConnections"};
                        return s;
                    }

                    typedef Device Interface;
                    typedef std::vector<core::dbus::types::ObjectPath> ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct ActiveConnection {
                    static const std::string &name()
                    {
                        static const std::string s{"ActiveConnection"};
                        return s;
                    }

                    typedef Device Interface;
                    typedef core::dbus::types::ObjectPath ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };
            };

            std::vector<Connection>
            get_available_connections() {
                std::vector<Connection> list;
                for (auto c: available_connections->get()) {
                    list.push_back(Connection(service->object_for_path(c)));
                }
                return list;
            }

            struct Method {

                struct Disconnect
                {
                    static const std::string& name()
                    {
                        static const std::string s{"Disconnect"};
                        return s;
                    }

                    typedef Device Interface;
                    typedef void ResultType;

                    static std::chrono::milliseconds default_timeout()
                    {
                        return std::chrono::seconds{30};
                    }
                };
            };

            void disconnect()
            {
                auto result =
                        object->invoke_method_synchronously<
                            Device::Method::Disconnect, Device::Method::Disconnect::ResultType>();

                if (result.is_error())
                    connectivity::throw_dbus_exception(result.error());
            }

            struct Signal {
                struct StateChanged
                {
                    static const std::string& name()
                    {
                        static const std::string s{"StateChanged"};
                        return s;
                    }

                    typedef Device Interface;
                    typedef std::tuple<std::uint32_t, std::uint32_t, std::uint32_t> ArgumentType;
                };
            };


            Type type() const
            {
                return static_cast<Type>(device_type->get());
            }

            std::string interface() const
            {
                return device_interface->get();
            }

            Wireless::Method::GetAccessPoints::ResultType get_access_points() const
            {
                auto result = object->invoke_method_synchronously<Wireless::Method::GetAccessPoints,
                                                                  Wireless::Method::GetAccessPoints::ResultType>();

                if (result.is_error())
                    connectivity::throw_dbus_exception(result.error());

                return result.value();
            }

            Device(const std::shared_ptr<core::dbus::Service>& service,
                   const std::shared_ptr<core::dbus::Object>& object)
                : service(service),
                  object(object),
                  device_type(object->get_property<Property::DeviceType>()),
                  autoconnect(object->get_property<Property::Autoconnect>()),
                  device_interface(object->get_property<Property::DeviceInterface>()),
                  state(object->get_property<Property::State>()),
                  state_changed(object->get_signal<Signal::StateChanged>()),
                  available_connections(object->get_property<Property::AvailableConnections>()),
                  active_connection(object->get_property<Property::ActiveConnection>())
            {}

            std::shared_ptr<core::dbus::Service> service;
            std::shared_ptr<core::dbus::Object> object;
            std::shared_ptr<core::dbus::Property<Property::DeviceType>> device_type;
            std::shared_ptr<core::dbus::Property<Property::Autoconnect>> autoconnect;
            std::shared_ptr<core::dbus::Property<Property::DeviceInterface>> device_interface;
            std::shared_ptr<core::dbus::Property<Property::State>> state;
            std::shared_ptr<core::dbus::Signal<Signal::StateChanged, Signal::StateChanged::ArgumentType>> state_changed;
            std::shared_ptr<core::dbus::Property<Property::AvailableConnections>> available_connections;
            std::shared_ptr<core::dbus::Property<Property::ActiveConnection>> active_connection;
        }; // Interface::Device

        struct NetworkManager
        {
            static const std::string& name()
            {
                static const std::string s{NM_DBUS_INTERFACE};
                return s;
            }

            struct Method {
                struct ActivateConnection
                {
                    static const std::string& name()
                    {
                        static const std::string s{"ActivateConnection"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef core::dbus::types::ObjectPath ResultType;

                    static std::chrono::milliseconds default_timeout()
                    {
                        return std::chrono::seconds{30};
                    }
                };

                struct AddAndActivateConnection
                {
                    static const std::string& name()
                    {
                        static const std::string s{"AddAndActivateConnection"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef std::tuple<core::dbus::types::ObjectPath, core::dbus::types::ObjectPath> ResultType;

                    static std::chrono::milliseconds default_timeout()
                    {
                        return std::chrono::seconds{30};
                    }
                };

                struct GetDevices
                {
                    static const std::string& name()
                    {
                        static const std::string s{"GetDevices"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef std::vector<core::dbus::types::ObjectPath> ResultType;

                    static std::chrono::milliseconds default_timeout()
                    {
                        return std::chrono::seconds{30};
                    }
                };

            };


            struct Property
            {
                struct WirelessEnabled {
                    static const std::string &name()
                    {
                        static const std::string s{"WirelessEnabled"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef bool ValueType;
                    static const bool readable = true;
                    static const bool writable = true;
                };

                struct State {
                    static const std::string &name()
                    {
                        static const std::string s{"State"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef std::uint32_t ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

                struct PrimaryConnection {
                    static const std::string &name()
                    {
                        static const std::string s{"PrimaryConnection"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef core::dbus::types::ObjectPath ValueType;
                    static const bool readable = true;
                    static const bool writable = false;
                };

            };

            struct Signal {

                struct PropertiesChanged
                {
                    static const std::string& name()
                    {
                        static const std::string s{"PropertiesChanged"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef std::map<std::string, core::dbus::types::Variant> ArgumentType;
                };

                struct DeviceAdded
                {
                    static const std::string& name()
                    {
                        static const std::string s{"DeviceAdded"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef core::dbus::types::ObjectPath ArgumentType;
                };

                struct DeviceRemoved
                {
                    static const std::string& name()
                    {
                        static const std::string s{"DeviceRemoved"};
                        return s;
                    }

                    typedef NetworkManager Interface;
                    typedef core::dbus::types::ObjectPath ArgumentType;
                };
            };

            Method::ActivateConnection::ResultType
            activate_connection(const core::dbus::types::ObjectPath &connection,
                                const core::dbus::types::ObjectPath &device,
                                const core::dbus::types::ObjectPath &specific_object)
            {
                auto result =
                        object->invoke_method_synchronously<
                            Method::ActivateConnection,
                            Method::ActivateConnection::ResultType>
                        (connection, device, specific_object);

                if (result.is_error())
                    connectivity::throw_dbus_exception(result.error());

                return result.value();
            }

            Method::AddAndActivateConnection::ResultType
            add_and_activate_connection(std::map<std::string, std::map<std::string, core::dbus::types::Variant>> &connection,
                                        const core::dbus::types::ObjectPath &device,
                                        const core::dbus::types::ObjectPath &specific_object)
            {
                auto result =
                        object->invoke_method_synchronously<
                            Method::AddAndActivateConnection,
                            Method::AddAndActivateConnection::ResultType>
                        (connection, device, specific_object);

                if (result.is_error())
                    connectivity::throw_dbus_exception(result.error());


                /// @todo return the settings (std::get<0>) object at some point
                return result.value();
            }

            std::vector<Device> get_devices()
            {
                auto result =
                        object->invoke_method_synchronously<
                            Method::GetDevices,
                            Method::GetDevices::ResultType>();

                if (result.is_error())
                    connectivity::throw_dbus_exception(result.error());

                std::vector<Device> devices;
                for (const auto& path : result.value())
                {
                    devices.emplace_back(
                                Device(
                                    service,
                                    service->object_for_path(path)));
                }

                return devices;
            }

            NetworkManager(std::shared_ptr<core::dbus::Service> &service,
                           std::shared_ptr<core::dbus::Object> &object)
                : service(service),
                  object(object),
                  wireless_enabled(object->get_property<Property::WirelessEnabled>()),
                  state(object->get_property<Property::State>()),
                  properties_changed(object->get_signal<Signal::PropertiesChanged>()),
                  primary_connection(object->get_property<Property::PrimaryConnection>()),
                  device_added(object->get_signal<Signal::DeviceAdded>()),
                  device_removed(object->get_signal<Signal::DeviceRemoved>())
            {
            }

            std::shared_ptr<core::dbus::Service> service;
            std::shared_ptr<core::dbus::Object> object;

            std::shared_ptr<core::dbus::Property<Property::WirelessEnabled>> wireless_enabled;
            std::shared_ptr<core::dbus::Property<Property::State>> state;
            std::shared_ptr<core::dbus::Signal<Signal::PropertiesChanged, Signal::PropertiesChanged::ArgumentType>> properties_changed;
            std::shared_ptr<core::dbus::Property<Property::PrimaryConnection>> primary_connection;
            std::shared_ptr<core::dbus::Signal<Signal::DeviceAdded, Signal::DeviceAdded::ArgumentType>> device_added;
            std::shared_ptr<core::dbus::Signal<Signal::DeviceRemoved, Signal::DeviceRemoved::ArgumentType>> device_removed;
        }; // Interface::NetworkManager
    };

    struct Service
    {
        std::shared_ptr<Interface::NetworkManager> nm;

        Service(const core::dbus::Bus::Ptr& bus)
        {
            auto service = core::dbus::Service::use_service<Interface::NetworkManager>(bus);
            auto object = service->object_for_path(core::dbus::types::ObjectPath(NM_DBUS_PATH));
            nm = std::make_shared<Interface::NetworkManager>(service, object);
        }

        struct Mock
        {
            std::shared_ptr<Interface::NetworkManager> nm;

            Mock(const core::dbus::Bus::Ptr& bus)
            {
                auto service = core::dbus::Service::add_service<Interface::NetworkManager>(bus);
                auto object = service->add_object_for_path(core::dbus::types::ObjectPath(NM_DBUS_PATH));
                nm = std::make_shared<Interface::NetworkManager>(service, object);
            }
        };
    };
}
}
}

#endif // PLATFORM_MANAGER_NMOFONO_NM_H
