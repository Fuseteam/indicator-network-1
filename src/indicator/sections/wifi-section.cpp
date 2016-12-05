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

#include <util/localisation.h>
#include <util/qhash-sharedptr.h>

using namespace nmofono;
using namespace std;

class WifiSection::Private : public QObject
{
    Q_OBJECT
public:
    Manager::Ptr m_manager;

    ActionGroupMerger::Ptr m_actionGroupMerger;
    MenuMerger::Ptr m_menuMerger;

    Menu::Ptr m_switchMenu;
    Menu::Ptr m_menu;
    Menu::Ptr m_settingsMenu;

    SwitchItem::Ptr m_switch;

    QMap<wifi::WifiLink::SPtr, WifiLinkItem::Ptr> m_items;
    TextItem::Ptr m_openWifiSettings;

    Private(Manager::Ptr manager, SwitchItem::Ptr wifiSwitch)
        : m_manager{manager}, m_switch{wifiSwitch}
    {
        m_actionGroupMerger = make_shared<ActionGroupMerger>();
        m_menuMerger = make_shared<MenuMerger>();
        m_switchMenu = make_shared<Menu>();
        m_menu = make_shared<Menu>();
        m_settingsMenu = make_shared<Menu>();

        m_openWifiSettings = make_shared<TextItem>(_("Wi-Fi settings…"), "wifi", "settings");
        connect(m_openWifiSettings.get(), &TextItem::activated, this, &Private::openWiFiSettings);

        m_actionGroupMerger->add(m_switch->actionGroup());
        m_actionGroupMerger->add(m_openWifiSettings->actionGroup());
        m_menuMerger->append(m_switchMenu);
        m_menuMerger->append(m_menu);
        m_menuMerger->append(m_settingsMenu);

        updateLinks();
        connect(m_manager.get(), &Manager::linksUpdated, this, &Private::updateLinks);
    }

public Q_SLOTS:
    void openWiFiSettings()
    {
        UrlDispatcher::send("settings:///system/wifi", [](string url, bool success){
            if (!success)
                cerr << "URL Dispatcher failed on " << url << endl;
        });
    }

    void updateLinks()
    {
        auto links = m_manager->wifiLinks();
        auto current(m_items.keys().toSet());

        auto removed(current);
        removed.subtract(links);

        auto added(links);
        added.subtract(current);

        if (removed.isEmpty() && added.isEmpty())
        {
            return;
        }

        for (auto link : removed)
        {
            m_actionGroupMerger->remove(m_items[link]->actionGroup());
            m_items.remove(link);
        }

        for (auto link : added)
        {
            auto item = make_shared<WifiLinkItem>(link);
            m_items[link] = item;
            m_actionGroupMerger->add(item->actionGroup());
        }

        m_menu->clear();

        multimap<Link::Id, WifiLinkItem::Ptr> sorted;
        QMapIterator<wifi::WifiLink::SPtr, WifiLinkItem::Ptr> it(m_items);
        while (it.hasNext())
        {
            it.next();
            sorted.insert(make_pair(it.key()->id(), it.value()));
        }
        for (auto pair : sorted)
        {
            m_menu->append(pair.second->menuItem());
        }

        if (links.isEmpty())
        {
            m_switchMenu->removeAll(m_switch->menuItem());
            m_settingsMenu->removeAll(m_openWifiSettings->menuItem());
        }
        else
        {
            if (m_settingsMenu->find(m_openWifiSettings->menuItem()) == m_settingsMenu->end())
            {
                m_settingsMenu->append(m_openWifiSettings->menuItem());
            }
            if (m_switchMenu->find(m_switch->menuItem()) == m_switchMenu->end())
            {
                m_switchMenu->append(m_switch->menuItem());
            }
        }
    }
};

WifiSection::WifiSection(Manager::Ptr manager, SwitchItem::Ptr wifiSwitch)
    : d{new Private(manager, wifiSwitch)}
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
    return d->m_menuMerger;
}

MenuModel::Ptr
WifiSection::settingsModel()
{
    return d->m_settingsMenu;
}

#include "wifi-section.moc"
