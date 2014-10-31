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

#include "quick-access-section.h"

#include "menuitems/switch-item.h"

#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu-merger.h"

namespace networking = connectivity::networking;

class QuickAccessSection::Private : public std::enable_shared_from_this<Private>
{
public:
    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    std::shared_ptr<networking::Manager> m_manager;

    SwitchItem::Ptr m_flightModeSwitch;

    Private() = delete;
    Private(std::shared_ptr<networking::Manager> manager);
    void ConstructL();
};

QuickAccessSection::Private::Private(std::shared_ptr<networking::Manager> manager)
    : m_manager{manager}
{}

void
QuickAccessSection::Private::ConstructL()
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    auto that = shared_from_this();

    m_flightModeSwitch = std::make_shared<SwitchItem>(_("Flight Mode"), "airplane", "enabled");
    switch (m_manager->flightMode().get()) {
    case networking::Manager::FlightModeStatus::off:
        m_flightModeSwitch->state().set(false);
        break;
    case networking::Manager::FlightModeStatus::on:
        m_flightModeSwitch->state().set(true);
        break;
    }
    m_manager->flightMode().changed().connect([that](networking::Manager::FlightModeStatus value){
        GMainLoopDispatch([that, value]
        {
            switch (value) {
            case networking::Manager::FlightModeStatus::off:
                that->m_flightModeSwitch->state().set(false);
                break;
            case networking::Manager::FlightModeStatus::on:
                that->m_flightModeSwitch->state().set(true);
                break;
            }
        });
    });
    m_flightModeSwitch->activated().connect([this](){
        if (m_flightModeSwitch->state().get()) {
            try {
                m_manager->enableFlightMode();
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        } else {
            try {
                m_manager->disableFlightMode();
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }
    });

    m_actionGroupMerger->add(*m_flightModeSwitch);
    m_menu->append(*m_flightModeSwitch);
}

QuickAccessSection::QuickAccessSection(std::shared_ptr<networking::Manager> manager)
    : d{new Private(manager)}
{
    d->ConstructL();
}

QuickAccessSection::~QuickAccessSection()
{

}

ActionGroup::Ptr
QuickAccessSection::actionGroup()
{
    return *d->m_actionGroupMerger;
}

MenuModel::Ptr
QuickAccessSection::menuModel()
{
    return d->m_menu;
}
