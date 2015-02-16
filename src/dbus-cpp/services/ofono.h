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
#ifndef DBUS_CPP_SERVICES_OFONO_H
#define DBUS_CPP_SERVICES_OFONO_H

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

#include <ofono/dbus.h>

namespace org
{
namespace ofono
{

struct Interface
{
    struct NetworkRegistration
    {
        static const std::string& name()
        {
            static const std::string s{"org.ofono.NetworkRegistration"};
            return s;
        }

        struct Method
        {
            struct GetProperties
            {
                static const std::string& name()
                {
                    static const std::string s{"GetProperties"};
                    return s;
                }

                typedef NetworkRegistration Interface;
                typedef std::map<std::string, core::dbus::types::Variant> ValueType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };
        };

        struct Property
        {
            struct Status {
                static const std::string &name()
                {
                    static const std::string s{"Status"};
                    return s;
                }

                typedef NetworkRegistration Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Technology
            {
                static const std::string &name()
                {
                    static const std::string s{"Technology"};
                    return s;
                }

                typedef NetworkRegistration Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Strength
            {
                static const std::string &name()
                {
                    static const std::string s{"Strength"};
                    return s;
                }

                typedef NetworkRegistration Interface;
                typedef std::int8_t ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Name
            {
                static const std::string &name()
                {
                    static const std::string s{"Name"};
                    return s;
                }

                typedef NetworkRegistration Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
        };
    
        struct Signal
        {
            struct PropertyChanged
            {
                static const std::string& name()
                {
                    static const std::string s{"PropertyChanged"};
                    return s;
                }

                typedef NetworkRegistration Interface;
                typedef std::tuple<std::string, core::dbus::types::Variant> ArgumentType;
            };
        };

        enum class Status
        {
            unregistered,
            registered,
            searching,
            denied,
            unknown,
            roaming
        };

        Status
        str2status(std::string str)
        {
            if (str == "unregistered")
                return Status::unregistered;
            if (str == "registered")
                return Status::registered;
            if (str == "searching")
                return Status::searching;
            if (str == "denied")
                return Status::denied;
            if (str == "unknown")
                return Status::unknown;
            if (str == "roaming")
                return Status::roaming;

            throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Unknown status '" + str + "'");
        }

        enum class Technology
        {
            notAvailable,
            gsm,
            edge,
            umts,
            hspa,
            lte
        };

        Technology
        str2technology(std::string str)
        {
            if (str == "")
                return Technology::notAvailable;
            if (str == "gsm")
                return Technology::gsm;
            if (str == "edge")
                return Technology::edge;
            if (str == "umts")
                return Technology::umts;
            if (str == "hspa")
                return Technology::hspa;
            if (str == "lte")
                return Technology::lte;

            throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Unknown techonology '" + str + "'");
        }

        typedef std::shared_ptr<NetworkRegistration> Ptr;
        explicit NetworkRegistration(const std::shared_ptr<core::dbus::Object>& object)
            : object(object),
              propertyChanged(object->get_signal<Signal::PropertyChanged>())
        {
            status.set(Status::unknown);
            strength.set(-1);
            technology.set(Technology::notAvailable);

            propertyChanged->connect([this](Signal::PropertyChanged::ArgumentType args){
                _updateProperty(std::get<0>(args), std::get<1>(args));
            });
            auto result = object->invoke_method_synchronously<Method::GetProperties, Method::GetProperties::ValueType>();
            if (result.is_error())
                throw std::runtime_error(result.error().print());
            for (auto element : result.value())
                _updateProperty(element.first, element.second);
        }

        ~NetworkRegistration()
        {}

        void _updateProperty(std::string property, core::dbus::types::Variant value)
        {
            if (property == Property::Name::name()) {
                operatorName.set(value.as<Property::Name::ValueType>());
            } else if (property == Property::Status::name()) {
                status.set(str2status(value.as<Property::Status::ValueType>()));
            } else if (property == Property::Strength::name()) {
                strength.set(value.as<Property::Strength::ValueType>());
            } else if (property == Property::Technology::name()) {
                technology.set(str2technology(value.as<Property::Technology::ValueType>()));
            } else {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
                std::cout << std::string(__PRETTY_FUNCTION__) + ": unhandled property change: " + property << std::endl;
#endif
            }
        }

        std::shared_ptr<core::dbus::Object> object;
        std::shared_ptr<core::dbus::Signal<Signal::PropertyChanged, Signal::PropertyChanged::ArgumentType>> propertyChanged;


        core::Property<std::string> operatorName;
        core::Property<Status> status;

        /**
         * Contains the current signal strength as a percentage
         * between 0-100 percent.
         *
         * -1, not avalable
         */
        core::Property<std::int8_t> strength;

        core::Property<Technology> technology;
    };

    struct SimManager
    {
        static const std::string& name()
        {
            static const std::string s{OFONO_SIM_MANAGER_INTERFACE};
            return s;
        }

        struct Method
        {
            struct GetProperties
            {
                static const std::string& name()
                {
                    static const std::string s{"GetProperties"};
                    return s;
                }

                typedef SimManager Interface;
                typedef std::map<std::string, core::dbus::types::Variant> ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };
            struct SetProperty
            {
                static const std::string& name()
                {
                    static const std::string s{"SetProperty"};
                    return s;
                }

                typedef SimManager Interface;
                typedef void ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct ChangePin
            {
                static const std::string& name()
                {
                    static const std::string s{"ChangePin"};
                    return s;
                }

                typedef SimManager Interface;
                typedef void ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct EnterPin
            {
                static const std::string& name()
                {
                    static const std::string s{"EnterPin"};
                    return s;
                }

                typedef SimManager Interface;
                typedef void ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct ResetPin
            {
                static const std::string& name()
                {
                    static const std::string s{"ResetPin"};
                    return s;
                }

                typedef SimManager Interface;
                typedef void ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct LockPin
            {
                static const std::string& name()
                {
                    static const std::string s{"LockPin"};
                    return s;
                }

                typedef SimManager Interface;
                typedef void ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct UnlockPin
            {
                static const std::string& name()
                {
                    static const std::string s{"UnlockPin"};
                    return s;
                }

                typedef SimManager Interface;
                typedef void ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };
        };

        struct Property
        {
            struct Present
            {
                static const std::string &name()
                {
                    static const std::string s{"Present"};
                    return s;
                }

                typedef SimManager Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct SubscriberIdentity
            {
                static const std::string &name()
                {
                    static const std::string s{"SubscriberIdentity"};
                    return s;
                }

                typedef SimManager Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct PinRequired
            {
                static const std::string &name()
                {
                    static const std::string s{"PinRequired"};
                    return s;
                }

                typedef SimManager Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct LockedPins
            {
                static const std::string &name()
                {
                    static const std::string s{"LockedPins"};
                    return s;
                }

                typedef SimManager Interface;
                typedef std::vector<std::string> ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Retries
            {
                static const std::string &name()
                {
                    static const std::string s{"Retries"};
                    return s;
                }

                typedef SimManager Interface;
                typedef std::map<std::string, std::int8_t> ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
        };

        struct Signal
        {
            struct PropertyChanged
            {
                static const std::string& name()
                {
                    static const std::string s{"PropertyChanged"};
                    return s;
                }

                typedef SimManager Interface;
                typedef std::tuple<std::string, core::dbus::types::Variant> ArgumentType;
            };
        };

        enum class PinType
        {
            none,
            pin,
            phone,
            firstphone,
            pin2,
            network,
            netsub,
            service,
            corp,
            puk,
            firstphonepuk,
            puk2,
            networkpuk,
            netsubpuk,
            servicepuk,
            corppuk
        };

        static std::string
        pin2str(PinType type)
        {
            switch(type){
            case PinType::none:
                return "none";
            case PinType::pin:
                return "pin";
            case PinType::phone:
                return "phone";
            case PinType::firstphone:
                return "firstphone";
            case PinType::pin2:
                return "pin2";
            case PinType::network:
                return "network";
            case PinType::netsub:
                return "netsub";
            case PinType::service:
                return "service";
            case PinType::corp:
                return "corp";
            case PinType::puk:
                return "puk";
            case PinType::firstphonepuk:
                return "firstphonepuk";
            case PinType::puk2:
                return "puk2";
            case PinType::networkpuk:
                return "networkpuk";
            case PinType::netsubpuk:
                return "netsubpuk";
            case PinType::servicepuk:
                return "servicepuk";
            case PinType::corppuk:
                return "corppuk";
            }
            throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": unknown pinType: " + std::to_string(static_cast<int>(type)));
        }

        static PinType
        str2pin(std::string str)
        {
            if (str == "none")
                return PinType::none;
            else if (str == "pin")
                return PinType::pin;
            else if (str == "phone")
                return PinType::phone;
            else if (str == "firstphone")
                return PinType::firstphone;
            else if (str == "pin2")
                return PinType::pin2;
            else if (str == "network")
                return PinType::network;
            else if (str == "netsub")
                return PinType::netsub;
            else if (str == "service")
                return PinType::service;
            else if (str == "corp")
                return PinType::corp;
            else if (str == "puk")
                return PinType::puk;
            else if (str == "firstphonepuk")
                return PinType::firstphonepuk;
            else if (str == "puk2")
                return PinType::puk2;
            else if (str == "networkpuk")
                return PinType::networkpuk;
            else if (str == "netsubpuk")
                return PinType::netsubpuk;
            else if (str == "servicepuk")
                return PinType::servicepuk;
            else if (str == "corppuk")
                return PinType::corppuk;

            /// @todo throw something.
            std::cerr << "Unknown pin type: " << str << std::endl;
            return PinType::none;
        }

        std::map<std::string, core::dbus::types::Variant>
        getProperties()
        {
            auto result =
                    object->invoke_method_synchronously<
                        SimManager::Method::GetProperties, SimManager::Method::GetProperties::ResultType>();

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            return result.value();
        }

        bool
        changePin(PinType type, std::string oldPin, std::string newPin)
        {
            auto result =
                    object->invoke_method_synchronously<
                        SimManager::Method::ChangePin, SimManager::Method::ChangePin::ResultType>
                    (pin2str(type), oldPin, newPin);

            if (result.is_error()) {
                auto &error = result.error();
                if (error.name() == "org.ofono.Error.NotImplemented") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InProgress") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidArguments") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidFormat") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.Failed") {
                    return false;
                }
            }
            return true;
        }

        bool
        enterPin(PinType type, std::string pin)
        {
            try {
                auto result =
                        object->invoke_method_synchronously<
                        SimManager::Method::EnterPin, SimManager::Method::EnterPin::ResultType>
                        (pin2str(type), pin);
            } catch(std::runtime_error &e) {
                /// @todo dbus-cpp does not provide proper errors yet :/
                return false;
            }
#if 0
            if (result.is_error()) {
                auto &error = result.error();
                if (error.name() == "org.ofono.Error.NotImplemented") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InProgress") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidArguments") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidFormat") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.Failed") {
                    return false;
                }
            }
#endif
            return true;
        }

        bool
        resetPin(PinType type, std::string puk, std::string newPin)
        {
            try {
                auto result =
                        object->invoke_method_synchronously<
                        SimManager::Method::ResetPin, SimManager::Method::ResetPin::ResultType>
                        (pin2str(type), puk, newPin);
            } catch(std::runtime_error &e) {
                /// @todo dbus-cpp does not provide proper errors yet :/
                return false;
            }
#if 0
            if (result.is_error()) {
                auto &error = result.error();
                if (error.name() == "org.ofono.Error.NotImplemented") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InProgress") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidArguments") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidFormat") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.Failed") {
                    return false;
                }
            }
#endif
            return true;
        }

        bool
        lockPin(PinType type, std::string pin)
        {
            auto result =
                    object->invoke_method_synchronously<
                    SimManager::Method::LockPin, SimManager::Method::LockPin::ResultType>
                    (pin2str(type), pin);

            if (result.is_error()) {
                auto &error = result.error();
                if (error.name() == "org.ofono.Error.NotImplemented") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InProgress") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidArguments") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidFormat") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.Failed") {
                    return false;
                }
            }
            return true;
        }

        bool
        unlockPin(PinType type, std::string pin)
        {
            auto result =
                    object->invoke_method_synchronously<
                    SimManager::Method::UnlockPin, SimManager::Method::UnlockPin::ResultType>
                    (pin2str(type), pin);

            if (result.is_error()) {
                auto &error = result.error();
                if (error.name() == "org.ofono.Error.NotImplemented") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InProgress") {
                    throw std::runtime_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidArguments") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.InvalidFormat") {
                    throw std::logic_error(result.error().print());
                } else if (error.name() == "org.ofono.Error.Failed") {
                    return false;
                }
            }
            return true;
        }

        typedef std::shared_ptr<SimManager> Ptr;
        explicit SimManager(const std::shared_ptr<core::dbus::Object>& object)
            : object(object),
              propertyChanged(object->get_signal<Signal::PropertyChanged>())
        {
            pinRequired.set(PinType::none);
            present.set(false);
            present.changed().connect([this](bool value){
                // when sim goes away all the other properties are invalidated.
                if (!value) {
                    lockedPins.set(std::set<PinType>());
                    pinRequired.set(PinType::none);
                    retries.set(std::map<PinType, std::uint8_t>());
                    subscriberIdentity.set("");
                }
            });

            propertyChanged->connect([this](Signal::PropertyChanged::ArgumentType args){
                _updateProperty(std::get<0>(args), std::get<1>(args));
            });
            auto result =
                    object->invoke_method_synchronously<
                        SimManager::Method::GetProperties, SimManager::Method::GetProperties::ResultType>();
            if (result.is_error())
                throw std::runtime_error(result.error().print());
            for (auto element : result.value())
                _updateProperty(element.first, element.second);
        }

        ~SimManager()
        {}

        void _updateProperty(std::string property, core::dbus::types::Variant value)
        {
            if (property == Property::LockedPins::name()) {
                std::set<PinType> tmp;
                for (auto str : value.as<Property::LockedPins::ValueType>()) {
                    tmp.insert(str2pin(str));
                }
                lockedPins.set(tmp);
            } else if (property == Property::PinRequired::name()) {
                pinRequired.set(str2pin(value.as<Property::PinRequired::ValueType>()));
            } else if (property == Property::Present::name()) {
                present.set(value.as<Property::Present::ValueType>());
            } else if (property == Property::Retries::name()) {
                std::map<PinType, std::uint8_t> tmp;
                for (auto element : value.as<Property::Retries::ValueType>()) {
                    tmp[str2pin(element.first)] = element.second;
                }
                retries.set(tmp);
            } else if (property == Property::SubscriberIdentity::name()) {
                subscriberIdentity.set(value.as<Property::SubscriberIdentity::ValueType>());
            } else {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
                std::cout << std::string(__PRETTY_FUNCTION__) + ": unhandled property change: " + property << std::endl;
#endif
            }
        }

        std::shared_ptr<core::dbus::Object> object;
        std::shared_ptr<core::dbus::Signal<Signal::PropertyChanged, Signal::PropertyChanged::ArgumentType>> propertyChanged;

        core::Property<std::set<PinType>> lockedPins;
        core::Property<PinType> pinRequired;
        core::Property<bool> present;
        core::Property<std::map<PinType, std::uint8_t>> retries;
        core::Property<std::string> subscriberIdentity;
    }; // Interface::SimManager

    struct ConnectionManager
    {
        static const std::string& name()
        {
            static const std::string s{OFONO_CONNECTION_MANAGER_INTERFACE};
            return s;
        }

        struct Method
        {
            struct GetProperties
            {
                static const std::string& name()
                {
                    static const std::string s{"GetProperties"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef std::map<std::string, core::dbus::types::Variant> ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct SetProperty
            {
                static const std::string& name()
                {
                    static const std::string s{"SetProperty"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef void ResultType;
                typedef std::tuple<std::string, core::dbus::types::Variant> ArgumentType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct AddContext
            {
                static const std::string& name()
                {
                    static const std::string s{"AddContext"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef core::dbus::types::ObjectPath ResultType;
                typedef std::string ArgumentType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct RemoveContext
            {
                static const std::string& name()
                {
                    static const std::string s{"RemoveContext"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef void ResultType;
                typedef core::dbus::types::ObjectPath ArgumentType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct DeactivateAll
            {
                static const std::string& name()
                {
                    static const std::string s{"DeactivateAll"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef void ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

            struct GetContexts
            {
                static const std::string& name()
                {
                    static const std::string s{"GetContexts"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef std::tuple<core::dbus::types::ObjectPath, std::map<std::string, core::dbus::types::Variant>> ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };

        };

        struct Property
        {
            struct Powered
            {
                static const std::string &name()
                {
                    static const std::string s{"Powered"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
        };

        struct Signal
        {
            struct PropertyChanged
            {
                static const std::string& name()
                {
                    static const std::string s{"PropertyChanged"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef std::tuple<std::string, core::dbus::types::Variant> ArgumentType;
            };

            struct ContextAdded
            {
                static const std::string& name()
                {
                    static const std::string s{"ContextAdded"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef std::tuple<core::dbus::types::ObjectPath, core::dbus::types::Variant> ArgumentType;
            };

            struct ContextRemoved
            {
                static const std::string& name()
                {
                    static const std::string s{"ContextRemoved"};
                    return s;
                }

                typedef ConnectionManager Interface;
                typedef core::dbus::types::ObjectPath ArgumentType;
            };
        };


        std::map<std::string, core::dbus::types::Variant>
        getProperties()
        {
            auto result =
                    object->invoke_method_synchronously<
                    ConnectionManager::Method::GetProperties, ConnectionManager::Method::GetProperties::ResultType>();

            if (result.is_error()) {
                throw std::runtime_error(result.error().print());
            }

            return result.value();
        }

        void _updateProperty(std::string property, core::dbus::types::Variant value)
        {
            if (property == Property::Powered::name()) {
                powered.set(value.as<Property::Powered::ValueType>());
            }
        }

        typedef std::shared_ptr<ConnectionManager> Ptr;
        explicit ConnectionManager(const std::shared_ptr<core::dbus::Object>& object)
            : object(object),
              propertyChanged(object->get_signal<Signal::PropertyChanged>())
        {
            propertyChanged->connect([this](Signal::PropertyChanged::ArgumentType args){
                _updateProperty(std::get<0>(args), std::get<1>(args));
            });
            auto result = getProperties();
            for (auto element : result)
            {
                _updateProperty(element.first, element.second);
            }
        }

        ~ConnectionManager()
        {}

        std::shared_ptr<core::dbus::Object> object;
        std::shared_ptr<core::dbus::Signal<Signal::PropertyChanged, Signal::PropertyChanged::ArgumentType>> propertyChanged;

        core::Property<bool> powered;

    }; // Interface::ConnectionManager

    struct Modem
    {
        static const std::string& name()
        {
            static const std::string s{OFONO_MODEM_INTERFACE};
            return s;
        }

        struct Method
        {
            struct GetProperties
            {
                static const std::string& name()
                {
                    static const std::string s{"GetProperties"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::map<std::string, core::dbus::types::Variant> ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };
            struct SetProperty
            {
                static const std::string& name()
                {
                    static const std::string s{"SetProperty"};
                    return s;
                }

                typedef Modem Interface;
                typedef void ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };
        };

        struct Property
        {
            struct Interfaces
            {
                static const std::string &name()
                {
                    static const std::string s{"Interfaces"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::vector<std::string> ValueType;
                static const bool readable = true;
                static const bool writable = false;

                enum class Type
                {
                    AssistedSatelliteNavigation,
                    AudioSettings,
                    CallBarring,
                    CallForwarding,
                    CallMeter,
                    CallSettings,
                    CallVolume,
                    CellBroadcast,
                    ConnectionManager,
                    Handsfree,
                    LocationReporting,
                    MessageManager,
                    MessageWaiting,
                    NetworkRegistration,
                    Phonebook,
                    PushNotification,
                    RadioSettings,
                    SimManager,
                    SmartMessaging,
                    SimToolkit,
                    SupplementaryServices,
                    TextTelephony,
                    VoiceCallManager,
                };
            };

            struct Online
            {
                static const std::string &name()
                {
                    static const std::string s{"Online"};
                    return s;
                }

                typedef Modem Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = true;
            };

            struct Serial
            {
                static const std::string &name()
                {
                    static const std::string s{"Serial"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

            struct Type
            {
                static const std::string &name()
                {
                    static const std::string s{"Type"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };

        };

        struct Signal
        {
            struct PropertyChanged
            {
                static const std::string& name()
                {
                    static const std::string s{"PropertyChanged"};
                    return s;
                }

                typedef Modem Interface;
                typedef std::tuple<std::string, core::dbus::types::Variant> ArgumentType;
            };
        };

        enum class Type
        {
            test,
            hfp,
            sap,
            hardware
        };

        Type str2type(std::string str)
        {
            if (str == "test")
                return Type::test;
            if (str == "hfp")
                return Type::hfp;
            if (str == "sap")
                return Type::sap;
            if (str == "hardware")
                return Type::hardware;

            /// @todo throw something.
            std::cerr << "Unknown modem type: " << str << std::endl;
            return Type::test;
        }


        typedef std::shared_ptr<Modem> Ptr;
        Modem(const std::shared_ptr<core::dbus::Service>& service,
              const std::shared_ptr<core::dbus::Object>& object)
            : service(service),
              object(object),
              propertyChanged(object->get_signal<Signal::PropertyChanged>())
        {
            knownInterfaces = {std::make_pair(OFONO_GNSS_INTERFACE, Property::Interfaces::Type::AssistedSatelliteNavigation),
                               std::make_pair(OFONO_AUDIO_SETTINGS_INTERFACE,         Property::Interfaces::Type::AudioSettings              ),
                               std::make_pair(OFONO_CALL_BARRING_INTERFACE,           Property::Interfaces::Type::CallBarring                ),
                               std::make_pair(OFONO_CALL_FORWARDING_INTERFACE,        Property::Interfaces::Type::CallForwarding             ),
                               std::make_pair(OFONO_CALL_METER_INTERFACE,             Property::Interfaces::Type::CallMeter                  ),
                               std::make_pair(OFONO_CALL_SETTINGS_INTERFACE,          Property::Interfaces::Type::CallSettings               ),
                               std::make_pair(OFONO_CALL_VOLUME_INTERFACE,            Property::Interfaces::Type::CallVolume                 ),
                               std::make_pair(OFONO_CELL_BROADCAST_INTERFACE,         Property::Interfaces::Type::CellBroadcast              ),
                               std::make_pair(OFONO_CONNECTION_MANAGER_INTERFACE,     Property::Interfaces::Type::ConnectionManager          ),
                               std::make_pair(OFONO_HANDSFREE_INTERFACE,              Property::Interfaces::Type::Handsfree                  ),
                               std::make_pair(OFONO_LOCATION_REPORTING_INTERFACE,     Property::Interfaces::Type::LocationReporting          ),
                               std::make_pair(OFONO_MESSAGE_MANAGER_INTERFACE,        Property::Interfaces::Type::MessageManager             ),
                               std::make_pair(OFONO_MESSAGE_WAITING_INTERFACE,        Property::Interfaces::Type::MessageWaiting             ),
                               std::make_pair(OFONO_NETWORK_REGISTRATION_INTERFACE,   Property::Interfaces::Type::NetworkRegistration        ),
                               std::make_pair(OFONO_PHONEBOOK_INTERFACE,              Property::Interfaces::Type::Phonebook                  ),
                               std::make_pair("org.ofono.PushNotification",           Property::Interfaces::Type::PushNotification           ),
                               std::make_pair(OFONO_RADIO_SETTINGS_INTERFACE,         Property::Interfaces::Type::RadioSettings              ),
                               std::make_pair(OFONO_SIM_MANAGER_INTERFACE,            Property::Interfaces::Type::SimManager                 ),
                               std::make_pair("org.ofono.SmartMessaging",             Property::Interfaces::Type::SmartMessaging             ),
                               std::make_pair(OFONO_STK_INTERFACE,                    Property::Interfaces::Type::SimToolkit                 ),
                               std::make_pair(OFONO_SUPPLEMENTARY_SERVICES_INTERFACE, Property::Interfaces::Type::SupplementaryServices      ),
                               std::make_pair(OFONO_TEXT_TELEPHONY_INTERFACE,         Property::Interfaces::Type::TextTelephony              ),
                               std::make_pair(OFONO_VOICECALL_MANAGER_INTERFACE,      Property::Interfaces::Type::VoiceCallManager           )
                              };

            interfaces.changed().connect([this](Property::Interfaces::ValueType values)
            {
                std::vector<Property::Interfaces::Type> newInterfaces;
                for (auto interface : values) {
                    auto iter = knownInterfaces.find(interface);
                    if (iter != knownInterfaces.end()) {
                        newInterfaces.push_back(iter->second);
                    } else {
                        // custom interface, we don't care
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
                        std::cout << "Unknown Interface: " << interface << std::endl;
#endif
                    }
                }

                // construct a list of known interface types
                std::vector<Property::Interfaces::Type> knownInterfaceTypes;
                for (auto element : knownInterfaces)
                    knownInterfaceTypes.push_back(element.second);

                // check which interfaces have been removed
                for (auto known : knownInterfaceTypes)
                {
                    switch (known) {
                    case Property::Interfaces::Type::AssistedSatelliteNavigation:
                    case Property::Interfaces::Type::AudioSettings:
                    case Property::Interfaces::Type::CallBarring:
                    case Property::Interfaces::Type::CallForwarding:
                    case Property::Interfaces::Type::CallMeter:
                    case Property::Interfaces::Type::CallSettings:
                    case Property::Interfaces::Type::CallVolume:
                    case Property::Interfaces::Type::CellBroadcast:
                        break;
                    case Property::Interfaces::Type::ConnectionManager:
                    {
                        std::unique_lock<std::mutex> lock(_lock);
                        if (std::find(newInterfaces.begin(), newInterfaces.end(), known) == newInterfaces.end() &&
                            connectionManager.get()) {
                            connectionManager.set(std::shared_ptr<ConnectionManager>());
                        }
                        break;
                    }
                    case Property::Interfaces::Type::Handsfree:
                    case Property::Interfaces::Type::LocationReporting:
                    case Property::Interfaces::Type::MessageManager:
                    case Property::Interfaces::Type::MessageWaiting:
                        break;
                    case Property::Interfaces::Type::NetworkRegistration:
                    {
                        std::unique_lock<std::mutex> lock(_lock);
                        if (std::find(newInterfaces.begin(), newInterfaces.end(), known) == newInterfaces.end() &&
                            networkRegistration.get()) {
                            networkRegistration.set(std::shared_ptr<NetworkRegistration>());
                        }
                        break;
                    }
                    case Property::Interfaces::Type::Phonebook:
                    case Property::Interfaces::Type::PushNotification:
                    case Property::Interfaces::Type::RadioSettings:
                        break;
                    case Property::Interfaces::Type::SimManager:
                    {
                        std::unique_lock<std::mutex> lock(_lock);
                        if (std::find(newInterfaces.begin(), newInterfaces.end(), known) == newInterfaces.end() &&
                            simManager.get()) {
                            simManager.set(std::shared_ptr<SimManager>());
                        }
                        break;
                    }
                    case Property::Interfaces::Type::SmartMessaging:
                    case Property::Interfaces::Type::SimToolkit:
                    case Property::Interfaces::Type::SupplementaryServices:
                    case Property::Interfaces::Type::TextTelephony:
                    case Property::Interfaces::Type::VoiceCallManager:
                        break;
                    }
                }

                // add new interfaces
                for (auto type : newInterfaces) {
                    switch (type) {
                    case Property::Interfaces::Type::AssistedSatelliteNavigation:
                    case Property::Interfaces::Type::AudioSettings:
                    case Property::Interfaces::Type::CallBarring:
                    case Property::Interfaces::Type::CallForwarding:
                    case Property::Interfaces::Type::CallMeter:
                    case Property::Interfaces::Type::CallSettings:
                    case Property::Interfaces::Type::CallVolume:
                    case Property::Interfaces::Type::CellBroadcast:
                        break;
                    case Property::Interfaces::Type::ConnectionManager:
                    {
                        std::unique_lock<std::mutex> lock(_lock);
                        if (!connectionManager.get()) {
                            connectionManager.set(std::make_shared<ConnectionManager>(this->object));
                        }
                        break;
                    }
                    case Property::Interfaces::Type::Handsfree:
                    case Property::Interfaces::Type::LocationReporting:
                    case Property::Interfaces::Type::MessageManager:
                    case Property::Interfaces::Type::MessageWaiting:
                        break;
                    case Property::Interfaces::Type::NetworkRegistration:
                    {
                        std::unique_lock<std::mutex> lock(_lock);
                        if (!networkRegistration.get()) {
                            networkRegistration.set(std::make_shared<NetworkRegistration>(this->object));
                        }
                        break;
                    }
                    case Property::Interfaces::Type::Phonebook:
                    case Property::Interfaces::Type::PushNotification:
                    case Property::Interfaces::Type::RadioSettings:
                        break;
                    case Property::Interfaces::Type::SimManager:
                    {
                        std::unique_lock<std::mutex> lock(_lock);
                        if (!simManager.get()) {
                            simManager.set(std::make_shared<SimManager>(this->object));
                        }
                        break;
                    }
                    case Property::Interfaces::Type::SmartMessaging:
                    case Property::Interfaces::Type::SimToolkit:
                    case Property::Interfaces::Type::SupplementaryServices:
                    case Property::Interfaces::Type::TextTelephony:
                     case Property::Interfaces::Type::VoiceCallManager:
                        break;
                    }
                }
            });

            propertyChanged->connect([this](Signal::PropertyChanged::ArgumentType arg) {
                _updateProperty(std::get<0>(arg), std::get<1>(arg));
            });
            for (auto element : getProperties()) {
                _updateProperty(element.first, element.second);
            }
        }

        std::map<std::string, core::dbus::types::Variant>
        getProperties()
        {
            auto result =
                    object->invoke_method_synchronously<
                        Modem::Method::GetProperties, Modem::Method::GetProperties::ResultType>();

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            return result.value();
        }

        void setProperty(const std::string &property, core::dbus::types::Variant value)
        {
//            Possible Errors: [service].Error.InProgress
//                              [service].Error.NotImplemented
//                              [service].Error.InvalidArguments
//                              [service].Error.NotAvailable
//                              [service].Error.AccessDenied
//                              [service].Error.Failed
            auto result =
                    object->invoke_method_synchronously<
                        Modem::Method::SetProperty, Modem::Method::SetProperty::ResultType>(property, value);

            if (result.is_error())
                throw std::runtime_error(result.error().print());
        }

        ~Modem()
        {}

        void _updateProperty(const std::string &property, core::dbus::types::Variant value)
        {
            if (property == Property::Interfaces::name()) {
                auto newValue = value.as<Property::Interfaces::ValueType>();
                interfaces.set(newValue);
            } else if (property == Property::Online::name()) {
                online.set(value.as<Property::Online::ValueType>());
            } else if (property == Property::Serial::name()) {
                serial.set(value.as<Property::Serial::ValueType>());
            } else if (property == Property::Type::name()) {
                type.set(str2type(value.as<Property::Type::ValueType>()));
            } else {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
                std::cout << std::string(__PRETTY_FUNCTION__) + ": unhandled property change: " + property << std::endl;
#endif
            }
        }

        std::shared_ptr<core::dbus::Service> service;
        std::shared_ptr<core::dbus::Object> object;

        core::Property<Property::Interfaces::ValueType> interfaces;
        core::Property<bool> online;
        core::Property<std::string> serial;
        core::Property<Type> type;

        std::shared_ptr<core::dbus::Signal<Signal::PropertyChanged, Signal::PropertyChanged::ArgumentType>> propertyChanged;

        // this lock must be acquired for any access to networkRegistration or simManager
        std::mutex _lock;
        core::Property<ConnectionManager::Ptr> connectionManager;
        core::Property<NetworkRegistration::Ptr> networkRegistration;
        core::Property<SimManager::Ptr>          simManager;

        std::map<std::string, Property::Interfaces::Type> knownInterfaces;

    }; // Interface::Modem

    struct Manager
    {
        static const std::string& name()
        {
            static const std::string s{OFONO_MANAGER_INTERFACE};
            return s;
        }

        struct Method
        {
            struct GetModems
            {
                static const std::string& name()
                {
                    static const std::string s{"GetModems"};
                    return s;
                }

                typedef Manager Interface;
                typedef std::vector<core::dbus::types::Struct<core::dbus::types::ObjectPath>> ResultType;

                static std::chrono::milliseconds default_timeout()
                {
                    return std::chrono::seconds{30};
                }
            };
        };

        struct Signal
        {
            struct ModemAdded
            {
                static const std::string& name()
                {
                    static const std::string s{"ModemAdded"};
                    return s;
                }

                typedef Manager Interface;
                typedef std::tuple<core::dbus::types::ObjectPath, std::map<std::string, core::dbus::types::Variant>> ArgumentType;
            };

            struct ModemRemoved
            {
                static const std::string& name()
                {
                    static const std::string s{"ModemRemoved"};
                    return s;
                }

                typedef Manager Interface;
                typedef core::dbus::types::ObjectPath ArgumentType;
            };
        };

        Manager(std::shared_ptr<core::dbus::Service> &service,
                std::shared_ptr<core::dbus::Object> &object)
            : service(service),
              object(object),
              modem_added  (object->get_signal<Signal::ModemAdded>()),
              modem_removed(object->get_signal<Signal::ModemRemoved>())
        {
            auto result = object->invoke_method_synchronously<Method::GetModems, Method::GetModems::ResultType>();

            if (result.is_error())
                throw std::runtime_error(result.error().print());

            std::map<core::dbus::types::ObjectPath, Modem::Ptr> tmp;
            for (const auto& element : result.value())
            {
                tmp.insert(std::make_pair(element.value, std::make_shared<Modem>(service, service->object_for_path(element.value))));
            }
            modems.set(tmp);

            modem_added->connect([this](const Signal::ModemAdded::ArgumentType& arg)
            {
                auto current = modems.get();
                current.insert(std::make_pair(std::get<0>(arg),std::make_shared<Modem>(this->service, this->service->object_for_path(std::get<0>(arg)))));
                modems.set(current);
            });

            modem_removed->connect([this](const Signal::ModemRemoved::ArgumentType& arg)
            {
                auto current = modems.get();
                current.erase(arg);
                modems.set(current);
            });
        }

        ~Manager()
        {}

        std::shared_ptr<core::dbus::Service> service;
        std::shared_ptr<core::dbus::Object>  object;

        std::shared_ptr<
            core::dbus::Signal<Signal::ModemAdded,
                               Signal::ModemAdded::ArgumentType>
            > modem_added;

        std::shared_ptr<
            core::dbus::Signal<Signal::ModemRemoved,
                               Signal::ModemRemoved::ArgumentType>
            > modem_removed;

        core::Property<std::map<core::dbus::types::ObjectPath, Modem::Ptr>> modems;
    }; // Interface::Manager


}; // Interface

struct Service
{
    std::shared_ptr<Interface::Manager> manager;

    static const std::string& name()
    {
        static const std::string s{OFONO_SERVICE};
        return s;
    }


    explicit Service(const core::dbus::Bus::Ptr& bus)
    {
        auto service = core::dbus::Service::use_service<Service>(bus);
        auto object = service->object_for_path(core::dbus::types::ObjectPath(OFONO_MANAGER_PATH));
        manager = std::make_shared<Interface::Manager>(service, object);
    }

    struct Mock
    {
        std::shared_ptr<Interface::Manager> manager;

        explicit Mock(const core::dbus::Bus::Ptr& bus)
        {
            auto service = core::dbus::Service::add_service<Service>(bus);
            auto object = service->add_object_for_path(core::dbus::types::ObjectPath(OFONO_MANAGER_PATH));
            manager = std::make_shared<Interface::Manager>(service, object);
        }
    };
};

}
}

#endif // DBUS_CPP_SERVICES_OFONO_H

