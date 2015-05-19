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

#include <util/localisation.h>

#include <QDebug>

using namespace std;
using namespace nmofono;

class QuickAccessSection::Private : public QObject
{
    Q_OBJECT

public:
    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    Manager::Ptr m_manager;

    SwitchItem::Ptr m_flightModeSwitch;
    SwitchItem::Ptr m_wifiSwitch;

    Private() = delete;
    Private(Manager::Ptr manager, SwitchItem::Ptr wifiSwitch);

public Q_SLOTS:
    void unstoppableOperationHappeningUpdated(bool happening)
    {
        m_flightModeSwitch->setEnabled(!happening);
        m_wifiSwitch->setEnabled(!happening);

        if (happening)
        {
            // Give the GActionGroup a chance to emit its Changed signal
            runGMainloop();
        }
    }

    void flightModeUpdated(Manager::FlightModeStatus value)
    {
        switch (value) {
        case Manager::FlightModeStatus::off:
            m_flightModeSwitch->setState(false);
            break;
        case Manager::FlightModeStatus::on:
            m_flightModeSwitch->setState(true);
            break;
        }
    }
};

QuickAccessSection::Private::Private(Manager::Ptr manager,
                                     SwitchItem::Ptr wifiSwitch)
    : m_manager{manager}, m_wifiSwitch(wifiSwitch)
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    m_flightModeSwitch = std::make_shared<SwitchItem>(_("Flight Mode"), "airplane", "enabled");
    flightModeUpdated(m_manager->flightMode());
    connect(m_manager.get(), &Manager::flightModeUpdated, this, &Private::flightModeUpdated);
    connect(m_flightModeSwitch.get(), &SwitchItem::stateUpdated, m_manager.get(), &Manager::setFlightMode);

    m_actionGroupMerger->add(m_flightModeSwitch->actionGroup());
    m_menu->append(m_flightModeSwitch->menuItem());

    // Connect the unstoppable operation property to the toggle enabled properties
    // Make sure we don't also connect this signal in the wifi-section.cpp
    connect(m_manager.get(), &Manager::unstoppableOperationHappeningUpdated,
            this, &Private::unstoppableOperationHappeningUpdated);
}

QuickAccessSection::QuickAccessSection(Manager::Ptr manager, SwitchItem::Ptr wifiSwitch)
    : d{new Private(manager, wifiSwitch)}
{
}

QuickAccessSection::~QuickAccessSection()
{
}

ActionGroup::Ptr
QuickAccessSection::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuModel::Ptr
QuickAccessSection::menuModel()
{
    return d->m_menu;
}

#include "quick-access-section.moc"
