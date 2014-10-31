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

#include "wifi-section.h"

#include "wifi-link-item.h"

#include "url-dispatcher-cpp/url-dispatcher.h"

#include "menuitems/switch-item.h"
#include "menuitems/text-item.h"

#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-merger.h"

using namespace connectivity;

class WifiSection::Private : public std::enable_shared_from_this<Private>
{
public:
    std::shared_ptr<connectivity::networking::Manager> m_manager;

    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;
    Menu::Ptr m_settingsMenu;

    SwitchItem::Ptr m_switch;

    WifiLinkItem::Ptr m_wifiLink;
    TextItem::Ptr m_openWifiSettings;

    Private() = delete;
    Private(std::shared_ptr<connectivity::networking::Manager> manager)
        : m_manager{manager}
    {}

    void
    ConstructL()
    {
        m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
        m_menu = std::make_shared<Menu>();
        m_settingsMenu = std::make_shared<Menu>();


        m_switch = std::make_shared<SwitchItem>(_("Wi-Fi"), "wifi", "enable");

        /// @todo don't now really care about actully being able to detach the whole
        ///       wifi chipset. on touch devices we always have wifi.
        if (m_manager->hasWifi().get()) {
            m_actionGroupMerger->add(*m_switch);
            m_menu->append(*m_switch);
            m_settingsMenu->append(*m_switch);
        }

        auto that = shared_from_this();

        m_switch->state().set(m_manager->wifiEnabled().get());
        m_manager->wifiEnabled().changed().connect([that](bool value) {
            GMainLoopDispatch([that, value]()
            {
                that->m_switch->state().set(value);
            });
        });
        m_switch->activated().connect([this](){
            if (m_switch->state().get()) {
                if (!m_manager->enableWifi()) {
                    /// try to work around the switch getting out of state on unity8 side
                    m_switch->state().set(false);
                }
            } else {
                if (!m_manager->disableWifi())
                    m_switch->state().set(true);
            }
        });


        updateLinks();
        m_manager->links().changed().connect([that](std::set<networking::Link::Ptr>)
        {
            GMainLoopDispatch([that]()
            {
                    that->updateLinks();
            });
        });

        m_openWifiSettings = std::make_shared<TextItem>(_("Wi-Fi settings…"), "wifi", "settings");
        m_openWifiSettings->activated().connect([](){
            UrlDispatcher::send("settings:///system/wifi", [](std::string url, bool success){
                if (!success)
                    std::cerr << "URL Dispatcher failed on " << url << std::endl;
            });
        });

        m_actionGroupMerger->add(*m_openWifiSettings);
        m_menu->append(*m_openWifiSettings);
    }

    void updateLinks()
    {
        // remove all and recreate. we have top 1 now anyway
        if (m_wifiLink) {
            m_actionGroupMerger->remove(*m_wifiLink);
            m_menu->removeAll(*m_wifiLink);
            m_settingsMenu->removeAll(*m_wifiLink);
            m_wifiLink.reset();
        }

        for (auto link : m_manager->links().get()) {
            auto wifi_link = std::dynamic_pointer_cast<networking::wifi::Link>(link);
            m_wifiLink = std::make_shared<WifiLinkItem>(wifi_link);

            m_actionGroupMerger->add(*m_wifiLink);
            m_menu->append(*m_wifiLink);
            m_settingsMenu->append(*m_wifiLink);

            // just take the first one
            break;
        }
    }

public:
};

WifiSection::WifiSection(std::shared_ptr<connectivity::networking::Manager> manager)
    : d{new Private(manager)}
{
    d->ConstructL();
}

WifiSection::~WifiSection()
{}

ActionGroup::Ptr
WifiSection::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuModel::Ptr
WifiSection::menuModel()
{
    return d->m_menu;
}

MenuModel::Ptr
WifiSection::settingsModel()
{
    return d->m_settingsMenu;
}
