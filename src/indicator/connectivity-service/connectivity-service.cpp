/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include "connectivity-service.h"
#include <dbus-cpp/services/connectivity.h>

#include <menumodel-cpp/gio-helpers/util.h>

#include <core/dbus/asio/executor.h>
#include <core/dbus/helper/type_mapper.h>

#include <sstream>

namespace networking = connectivity::networking;

template<typename T>
std::string generatePropertyIntrospectionXml()
{
    std::ostringstream output;

    output << "        <property ";
    output << "name=\"" << T::name() << "\" ";
    output << "type=\"" << core::dbus::helper::TypeMapper<typename T::ValueType>::signature() << "\" ";

    output << "access=\"";
    if (T::readable && T::writable)
        output << "readwrite";
    else if (T::readable && !T::writable)
        output << "read";
    else if (!T::readable && T::writable)
        output << "write";
    else
        throw(std::logic_error("Property defined as neither readable nor writable."));
    output << "\"";

    output << "/>";
    return output.str();
}

namespace {
struct Interface {
struct Introspectable
{
    inline static const std::string& name()
    {
        static const std::string s{"org.freedesktop.DBus.Introspectable"};
        return s;
    }

    struct Method
    {
        struct Introspect
        {
            typedef Introspectable Interface;
            inline static std::string name()
            {
                return "Introspect";
            }
            static const bool call_synchronously = true;
            inline static const std::chrono::milliseconds default_timeout()
            {
                return std::chrono::seconds{1};
            }
        };
    };
};
};
}

class ConnectivityService::Private : public std::enable_shared_from_this<Private>
{
public:
    std::thread m_connectivityServiceWorker;

    core::dbus::Bus::Ptr m_bus;
    core::dbus::Service::Ptr m_service;

    core::dbus::Object::Ptr m_networkingStatusObject;
    std::shared_ptr<com::ubuntu::connectivity::Interface::NetworkingStatus> m_networkingStatus;
    std::shared_ptr<core::dbus::Property<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Limitations>> m_limitations;
    std::shared_ptr<core::dbus::Property<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status>> m_status;
    core::dbus::Signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged,
                       core::dbus::interfaces::Properties::Signals::PropertiesChanged::ArgumentType>::Ptr  m_propertiesChanged;

    core::dbus::Object::Ptr m_privateObject;
    std::shared_ptr<com::ubuntu::connectivity::Interface::Private> m_private;

    core::Signal<>            m_unlockAllModems;
    core::Signal<std::string> m_unlockModem;

    std::shared_ptr<networking::Manager> m_manager;

    Private() = delete;
    Private(std::shared_ptr<networking::Manager> manager);
    void ConstructL();
    ~Private();

    void updateNetworkingStatus();

    std::string introspectXml();
};

ConnectivityService::Private::Private(std::shared_ptr<networking::Manager> manager)
    : m_manager{manager}
{}

void
ConnectivityService::Private::ConstructL()
{
    auto that = shared_from_this();
    m_manager->characteristics().changed().connect(
                [that](int){ GMainLoopDispatch([that](){ that->updateNetworkingStatus(); }); });

    typedef connectivity::networking::Manager::NetworkingStatus NetworkingStatus;
    m_manager->status().changed().connect(
                [that](NetworkingStatus){ GMainLoopDispatch([that](){ that->updateNetworkingStatus(); }); });

    m_bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);

    auto executor = core::dbus::asio::make_executor(m_bus);
    m_bus->install_executor(executor);
    m_connectivityServiceWorker = std::move(std::thread([this](){ try {
        m_bus->run();
    } catch(const std::exception &e) {
        std::cerr << __PRETTY_FUNCTION__ << "dbus-cpp crashed: " << e.what() << "\n";
        std::quick_exit(0);
    }
    }));

    m_service = core::dbus::Service::add_service<com::ubuntu::connectivity::Service>(m_bus);

    m_networkingStatusObject = m_service->add_object_for_path(core::dbus::types::ObjectPath(com::ubuntu::connectivity::Interface::NetworkingStatus::path()));
    m_networkingStatus = std::make_shared<com::ubuntu::connectivity::Interface::NetworkingStatus>(m_service, m_networkingStatusObject);
    m_propertiesChanged = m_networkingStatusObject->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>();


    m_networkingStatusObject->install_method_handler<Interface::Introspectable::Method::Introspect>([this](const core::dbus::Message::Ptr& msg)
    {
        auto reply = core::dbus::Message::make_method_return(msg);
        reply->writer() << introspectXml();
        m_bus->send(reply);
    });

    m_limitations = m_networkingStatusObject->get_property<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Limitations>();
    m_status = m_networkingStatusObject->get_property<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status>();

    updateNetworkingStatus();

    m_networkingStatusObject->install_method_handler<core::dbus::interfaces::Properties::GetAll>([this](const core::dbus::Message::Ptr& msg)
    {

        core::dbus::Message::Reader reader;
        try {
            reader = msg->reader();
        } catch(...) {
            m_bus->send(core::dbus::Message::make_error(msg, "need.to.add.full.properties.support.to.dbus.cpp", "invalid argument."));
            return;
        }
        if (reader.type() != core::dbus::ArgumentType::string) {
            m_bus->send(core::dbus::Message::make_error(msg, "need.to.add.full.properties.support.to.dbus.cpp", "invalid argument."));
            return;
        }
        std::string interface = reader.pop_string();

        if (interface != com::ubuntu::connectivity::Interface::NetworkingStatus::name()) {
            m_bus->send(core::dbus::Message::make_error(msg, "need.to.add.full.properties.support.to.dbus.cpp", "no such interface."));
            return;
        }

        auto reply = core::dbus::Message::make_method_return(msg);
        std::map<std::string, core::dbus::types::Variant> props;
        props[com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Limitations::name()]
                = core::dbus::types::TypedVariant<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Limitations::ValueType>(m_limitations->get());
        props[com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status::name()]
                = core::dbus::types::TypedVariant<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status::ValueType>(m_status->get());
        reply->writer() << props;
        m_bus->send(reply);
    });

    m_privateObject = m_service->add_object_for_path(core::dbus::types::ObjectPath(com::ubuntu::connectivity::Interface::Private::path()));
    m_privateObject->install_method_handler<com::ubuntu::connectivity::Interface::Private::Method::UnlockAllModems>([this](const core::dbus::Message::Ptr& msg)
    {
        auto reply = core::dbus::Message::make_method_return(msg);
        m_bus->send(reply);

        auto that = shared_from_this();
        GMainLoopDispatch([that](){ that->m_unlockAllModems(); });
    });

    m_privateObject->install_method_handler<com::ubuntu::connectivity::Interface::Private::Method::UnlockModem>([this](const core::dbus::Message::Ptr& msg)
    {
        core::dbus::Message::Reader reader;
        try {
            reader = msg->reader();
        } catch(...) {
            m_bus->send(core::dbus::Message::make_error(msg, "org.freedesktop.DBus.Error.InvalidArgs", "no modem name specified"));
            return;
        }
        if (reader.type() != core::dbus::ArgumentType::string) {
            m_bus->send(core::dbus::Message::make_error(msg, "org.freedesktop.DBus.Error.InvalidArgs", "modem name must be a string"));
            return;
        }
        std::string name = reader.pop_string();

        auto reply = core::dbus::Message::make_method_return(msg);
        m_bus->send(reply);

        auto that = shared_from_this();
        GMainLoopDispatch([that, name](){ that->m_unlockModem(name); });
    });
}

ConnectivityService::Private::~Private()
{
    m_bus->stop();
    try {
        if (m_connectivityServiceWorker.joinable())
            m_connectivityServiceWorker.join();
    } catch(const std::system_error &e) {
        // This is the only exception type that may be thrown.
        // http://en.cppreference.com/w/cpp/thread/thread/join
        std::cerr << "Error when destroying worker thread, error code " << e.code()
                  << ", error message: " << e.what() << std::endl;
    }
}

void
ConnectivityService::Private::updateNetworkingStatus()
{
    std::vector<std::string> old_limitations = m_limitations->get();
    std::string old_status = m_status->get();

    switch(m_manager->status().get()) {
    case networking::Manager::NetworkingStatus::offline:
        m_status->set("offline");
        break;
    case networking::Manager::NetworkingStatus::connecting:
        m_status->set("connecting");
        break;
    case networking::Manager::NetworkingStatus::online:
        m_status->set("online");
    }
    if (old_status.empty()) {
        // initially not set
        old_status = m_status->get();
    }


    std::vector<std::string> limitations;
    auto characteristics = m_manager->characteristics().get();
    if ((characteristics & networking::Link::Characteristics::is_bandwidth_limited) != 0) {
        limitations.push_back("bandwith");
    }
    m_limitations->set(limitations);

    std::map<std::string, core::dbus::types::Variant> changed;
    if (old_limitations != m_limitations->get()) {
        changed[com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Limitations::name()]
                = core::dbus::types::TypedVariant<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Limitations::ValueType>(m_limitations->get());
    }
    if (old_status != m_status->get()) {
        changed[com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status::name()]
                = core::dbus::types::TypedVariant<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status::ValueType>(m_status->get());
    }

    if (changed.size() != 0) {
        core::dbus::interfaces::Properties::Signals::PropertiesChanged::ArgumentType args
                (com::ubuntu::connectivity::Interface::NetworkingStatus::name(),
                 changed,
                 {}
                 );
        m_propertiesChanged->emit(args);
    }
}

std::string
ConnectivityService::Private::introspectXml()
{
    std::ostringstream output;

    static const std::string header(
                "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
                "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n");

    static const std::string introspectable_interface(
                "    <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
                "        <method name=\"Introspect\">\n"
                "            <arg name=\"xml_data\" direction=\"out\" type=\"s\"/>\n"
                "        </method>\n"
                "    </interface>\n");

    static const std::string dbus_properties_interface(
                "    <interface name=\"org.freedesktop.DBus.Properties\">\n"
                "        <method name=\"Get\">\n"
                "            <arg name=\"interface_name\" direction=\"in\" type=\"s\"/>\n"
                "            <arg name=\"property_name\" direction=\"in\" type=\"s\"/>\n"
                "            <arg name=\"value\" direction=\"out\" type=\"v\"/>\n"
                "        </method>\n"
                "        <method name=\"Set\">\n"
                "             <arg name=\"interface_name\" direction=\"in\" type=\"s\"/>\n"
                "             <arg name=\"property_name\" direction=\"in\" type=\"s\"/>\n"
                "             <arg name=\"value\" direction=\"in\" type=\"v\"/>\n"
                "        </method>\n"
                "        <method name=\"GetAll\">\n"
                "            <arg name=\"interface_name\" direction=\"in\" type=\"s\"/>\n"
                "            <arg name=\"props\" direction=\"out\" type=\"a{sv}\"/>\n"
                "        </method>\n"
                "        <signal name=\"PropertiesChanged\">\n"
                "            <arg type=\"s\" name=\"interface_name\"/>\n"
                "            <arg type=\"a{sv}\" name=\"changed_properties\"/>\n"
                "            <arg type=\"as\" name=\"invalidated_properties\"/>\n"
                "        </signal>\n"
                "    </interface>\n");

    output << header;
    output << "<node name=\"" << com::ubuntu::connectivity::Interface::NetworkingStatus::path() << "\">\n";
    output << introspectable_interface;
    output << dbus_properties_interface;
    output << "    <interface name=\"" << com::ubuntu::connectivity::Interface::NetworkingStatus::name() << "\">\n";
    output << generatePropertyIntrospectionXml<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Limitations>() << std::endl;
    output << generatePropertyIntrospectionXml<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status>() << std::endl;
    output << "    </interface>\n";
    output << "</node>\n";

    return output.str();
}

ConnectivityService::ConnectivityService(std::shared_ptr<networking::Manager> manager)
    : d{new Private(manager)}
{
    d->ConstructL();
}

ConnectivityService::~ConnectivityService()
{}

core::Signal<> &
ConnectivityService::unlockAllModems()
{
    return d->m_unlockAllModems;
}

core::Signal<std::string> &
ConnectivityService::unlockModem()
{
    return d->m_unlockModem;
}

