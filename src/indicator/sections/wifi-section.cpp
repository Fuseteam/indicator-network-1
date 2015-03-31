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

#include <sections/wifi-section.h>
#include <menuitems/wifi-link-item.h>
#include <menuitems/switch-item.h>
#include <menuitems/text-item.h>

#include "url-dispatcher-cpp/url-dispatcher.h"

#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-merger.h"

using namespace connectivity;

class WifiSection::Private : public QObject
{
    Q_OBJECT
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
    {
        m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
        m_menu = std::make_shared<Menu>();
        m_settingsMenu = std::make_shared<Menu>();

        m_switch = std::make_shared<SwitchItem>(_("Wi-Fi"), "wifi", "enable");

        /// @todo don't now really care about actully being able to detach the whole
        ///       wifi chipset. on touch devices we always have wifi.
        if (m_manager->hasWifi()) {
            m_actionGroupMerger->add(*m_switch);
            m_menu->append(*m_switch);
            m_settingsMenu->append(*m_switch);
        }

        m_switch->setState(m_manager->wifiEnabled());
        connect(m_manager.get(), &networking::Manager::wifiEnabledUpdated, m_switch.get(), &SwitchItem::setState);
        connect(m_switch.get(), &SwitchItem::stateUpdated, this, &Private::switchActivated);

        updateLinks();
        connect(m_manager.get(), &networking::Manager::linksUpdated, this, &Private::updateLinks);

        m_openWifiSettings = std::make_shared<TextItem>(_("Wi-Fi settings…"), "wifi", "settings");
        connect(m_openWifiSettings.get(), &TextItem::activated, this, &Private::openWiFiSettings);

        m_actionGroupMerger->add(*m_openWifiSettings);
        m_menu->append(*m_openWifiSettings);
    }

public Q_SLOTS:
    void openWiFiSettings()
    {
        UrlDispatcher::send("settings:///system/wifi", [](std::string url, bool success){
            if (!success)
                std::cerr << "URL Dispatcher failed on " << url << std::endl;
        });
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

        for (auto link : m_manager->links()) {
            auto wifi_link = std::dynamic_pointer_cast<networking::wifi::Link>(link);
            m_wifiLink = std::make_shared<WifiLinkItem>(wifi_link);

            m_actionGroupMerger->add(*m_wifiLink);
            m_menu->append(*m_wifiLink);
            m_settingsMenu->append(*m_wifiLink);

            // just take the first one
            break;
        }
    }

    void switchActivated(bool state)
    {
        m_switch->setEnabled(false);
        runGMainloop();
        if (state) {
            m_manager->enableWifi();
        } else {
            m_manager->disableWifi();
        }
        m_switch->setEnabled(true);
    }
};

WifiSection::WifiSection(std::shared_ptr<connectivity::networking::Manager> manager)
    : d{new Private(manager)}
{
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

SwitchItem::Ptr
WifiSection::wifiSwitch()
{
    return d->m_switch;
}

#include "wifi-section.moc"
