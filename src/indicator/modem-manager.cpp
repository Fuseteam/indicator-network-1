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

#include "modem-manager.h"

#include <menumodel-cpp/gio-helpers/util.h>

#include <notify-cpp/snapdecision/sim-unlock.h>

#include "dbus-cpp/services/ofono.h"
#include <core/dbus/asio/executor.h>
#include <core/dbus/dbus.h>
#include <core/dbus/service_watcher.h>

#include <algorithm>

#include "sim-unlock-dialog.h"

class ModemManager::Private : public std::enable_shared_from_this<Private>
{
public:

    core::Property<std::set<Modem::Ptr>> m_modems;

    std::thread m_ofonoWorker;
    std::shared_ptr<core::dbus::Bus> m_bus;
    std::shared_ptr<org::ofono::Service> m_ofono;

    std::unique_ptr<core::dbus::ServiceWatcher> m_watcher;
    std::unique_ptr<core::dbus::DBus> m_dbus;

    SimUnlockDialog::Ptr m_unlockDialog;
    std::list<Modem::Ptr> m_pendingUnlocks;

    Private()
    {}

    void
    ConstructL()
    {
        m_bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::system);

        auto executor = core::dbus::asio::make_executor(m_bus);
        m_bus->install_executor(executor);
        m_ofonoWorker = std::move(std::thread([this](){
            try {
                m_bus->run();
            } catch(std::exception &e) {
                /// @bug dbus-cpp internal logic exploded
                // If this happens, indicator-network is in an unknown state with no clear way of
                // recovering. The only reasonable way out is a graceful exit.
                std::cerr << __PRETTY_FUNCTION__ << " Failed to run dbus service: " << e.what() << std::endl;
                exit(0);
            }
        }));

        m_dbus.reset(new core::dbus::DBus(m_bus));

        m_unlockDialog = std::make_shared<SimUnlockDialog>();
        m_unlockDialog->ready().connect([this](){
            if (!m_pendingUnlocks.empty()) {
                auto modem = m_pendingUnlocks.front();
                m_pendingUnlocks.pop_front();
                if (modem->requiredPin().get() != Modem::PinType::none)
                    m_unlockDialog->unlock(modem);
            }
        });

        auto names = m_dbus->list_names();
        if (std::find(names.begin(), names.end(), org::ofono::Service::name()) != names.end()) {
            ofono_appeared();
        } else {
            ofono_disappeared();
        }
        m_watcher = m_dbus->make_service_watcher(org::ofono::Service::name());

        auto that = shared_from_this();
        m_watcher->service_registered().connect([that](){ GMainLoopDispatch([that](){ that->ofono_appeared(); }); });
        m_watcher->service_unregistered().connect([that](){ GMainLoopDispatch([that](){ that->ofono_disappeared(); }); });
    }

    ~Private()
    {
        m_bus->stop();
        try {
            if (m_ofonoWorker.joinable())
                m_ofonoWorker.join();
        } catch(const std::system_error &e) {
            // This is the only exception type that may be thrown.
            // http://en.cppreference.com/w/cpp/thread/thread/join
            std::cerr << "Error when destroying worker thread, error code " << e.code()
                      << ", error message: " << e.what() << std::endl;
        }
    }

    void ofono_appeared()
    {
        try {
            m_ofono = std::make_shared<org::ofono::Service>(m_bus);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

        auto that = shared_from_this();
        m_ofono->manager->modems.changed().connect(
                    [that](std::map<core::dbus::types::ObjectPath, org::ofono::Interface::Modem::Ptr> modems)
        { GMainLoopDispatch([that, modems]() { that->modems_changed(modems); }); });
        modems_changed(m_ofono->manager->modems.get());
    }

    void modems_changed(std::map<core::dbus::types::ObjectPath, org::ofono::Interface::Modem::Ptr> ofonoModemsMap)
    {
        std::set<org::ofono::Interface::Modem::Ptr> ofonoModems;
        for (auto element : ofonoModemsMap)
            ofonoModems.insert(element.second);

        auto currentModems = m_modems.get();

        std::set<org::ofono::Interface::Modem::Ptr> current;
        for (auto modem : currentModems)
            current.insert(modem->ofonoModem());

        std::set<org::ofono::Interface::Modem::Ptr> removed;
        std::set_difference(current.begin(), current.end(),
                            ofonoModems.begin(),ofonoModems.end(),
                            std::inserter(removed, removed.begin()));

        std::set<org::ofono::Interface::Modem::Ptr> added;
        std::set_difference(ofonoModems.begin(), ofonoModems.end(),
                            current.begin(), current.end(),
                            std::inserter(added, added.begin()));

        auto iter = currentModems.begin();
        while (iter != currentModems.end()) {
            if (removed.find((*iter)->ofonoModem()) != removed.end()) {

                m_pendingUnlocks.remove(*iter);
                if (m_unlockDialog->modem() == *iter)
                    m_unlockDialog->cancel();

                iter = currentModems.erase(iter);
                continue;
            }
            ++iter;
        }

        for (auto ofonoModem : added) {
            currentModems.insert(std::make_shared<Modem>(ofonoModem));
        }


        m_unlockDialog->showSimIdentifiers().set(currentModems.size() > 1);
        m_modems.set(currentModems);
    }

    void ofono_disappeared()
    {
        m_ofono.reset();
        m_modems.set(std::set<Modem::Ptr>());

        m_pendingUnlocks.clear();
        if (m_unlockDialog->state() == SimUnlockDialog::State::unlocking)
            m_unlockDialog->cancel();
    }

};

ModemManager::ModemManager()
    : d{new Private}
{
    d->ConstructL();
}

ModemManager::~ModemManager()
{}

void
ModemManager::unlockModem(Modem::Ptr modem)
{
    try {
        auto modems = d->m_modems.get();

        if (std::count(modems.begin(), modems.end(), modem) == 0
                || d->m_unlockDialog->modem() == modem
                || std::count(d->m_pendingUnlocks.begin(), d->m_pendingUnlocks.end(), modem) != 0)
            return;

        if (d->m_unlockDialog->state() == SimUnlockDialog::State::ready
                && d->m_pendingUnlocks.size() == 0)
            d->m_unlockDialog->unlock(modem);
        else
            d->m_pendingUnlocks.push_back(modem);
    } catch(const std::exception &e) {
        // Something unexpected has happened. As an example, unity8 might have
        // crashed taking the notification server with it. There is no graceful
        // and reliable way to recover so die and get restarted.
        // See also https://bugs.launchpad.net/unity-notifications/+bug/1238990
        std::cerr << __PRETTY_FUNCTION__ << " sim unlocking failed: " << e.what() << "\n";
        exit(0);
    }
}

void
ModemManager::unlockAllModems()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
    std::multimap<int, Modem::Ptr, Modem::Compare> sorted;
    for (auto m : d->m_modems.get()) {
        sorted.insert(std::make_pair(m->index(), m));
    }
    for (auto pair : sorted) {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        std::cout << "Unlocking " << pair.second->simIdentifier().get() << std::endl;
#endif
        unlockModem(pair.second);
    }
}


const core::Property<std::set<Modem::Ptr>> &
ModemManager::modems()
{
    return d->m_modems;
}
