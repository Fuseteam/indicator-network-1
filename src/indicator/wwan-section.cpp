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

#include "wwan-section.h"

#include "wwan-link-item.h"

#include "menuitems/text-item.h"

#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu-merger.h"

#include "modem-manager.h"

#include "url-dispatcher-cpp/url-dispatcher.h"

class WwanSection::Private
{
public:

    ActionGroupMerger::Ptr m_actionGroupMerger;

    MenuMerger::Ptr m_menuMerger;

    Menu::Ptr m_upperMenu;
    MenuMerger::Ptr m_linkMenuMerger;
    Menu::Ptr m_bottomMenu;

    Menu::Ptr m_topMenu;
    MenuItem::Ptr m_topItem;

    ModemManager::Ptr m_modemManager;

    TextItem::Ptr m_openCellularSettings;

    std::list<std::pair<Modem::Ptr, WwanLinkItem::Ptr>> m_items;

    Private() = delete;
    Private(ModemManager::Ptr modemManager);
    void ConstructL();

    void modemsChanged(const std::set<Modem::Ptr> &modems);
};

WwanSection::Private::Private(ModemManager::Ptr modemManager)
    : m_modemManager{modemManager}
{}

void
WwanSection::Private::ConstructL()
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menuMerger = std::make_shared<MenuMerger>();

    m_upperMenu  = std::make_shared<Menu>();
    m_linkMenuMerger = std::make_shared<MenuMerger>();
    m_bottomMenu = std::make_shared<Menu>();

    m_menuMerger->append(m_upperMenu);
    m_menuMerger->append(m_linkMenuMerger);
    m_menuMerger->append(m_bottomMenu);

    // have the modem list in their own section.
    m_topItem = MenuItem::newSection(m_menuMerger);
    m_topMenu = std::make_shared<Menu>();
    m_topMenu->append(m_topItem);

    m_openCellularSettings = std::make_shared<TextItem>(_("Cellular settings…"), "cellular", "settings");
    m_openCellularSettings->activated().connect([](){
        UrlDispatcher::send("settings:///system/cellular", [](std::string url, bool success){
            if (!success)
                std::cerr << "URL Dispatcher failed on " << url << std::endl;
        });
    });
    m_actionGroupMerger->add(*m_openCellularSettings);

    // already synced with GMainLoop
    m_modemManager->modems().changed().connect(std::bind(&Private::modemsChanged, this, std::placeholders::_1));
    modemsChanged(m_modemManager->modems());
}

void
WwanSection::Private::modemsChanged(const std::set<Modem::Ptr> &modems)
{
    std::set<Modem::Ptr> current;
    for (auto element : m_items)
        current.insert(element.first);

    std::set<Modem::Ptr> removed;
    std::set_difference(current.begin(), current.end(),
                        modems.begin(), modems.end(),
                        std::inserter(removed, removed.begin()));

    std::set<Modem::Ptr> added;
    std::set_difference(modems.begin(), modems.end(),
                        current.begin(), current.end(),
                        std::inserter(added, added.begin()));
    for (auto modem : removed) {
        for (auto iter = m_items.begin(); iter != m_items.end(); ++iter) {
            m_linkMenuMerger->remove(*(*iter).second);
            m_actionGroupMerger->remove(*(*iter).second);
            iter = m_items.erase(iter);
            --iter;
        }
    }

    for (auto modem : added) {
        auto item = std::make_shared<WwanLinkItem>(modem, m_modemManager);

        m_items.push_back(std::make_pair(modem, item));
        m_actionGroupMerger->add(*item);

        // for now just throw everything away and rebuild
        /// @todo add MenuMerger::insert() and ::find()
        m_linkMenuMerger->clear();

        std::multimap<int, WwanLinkItem::Ptr, Modem::Compare> sorted;
        for (auto pair : m_items) {
            sorted.insert(std::make_pair(pair.first->index(), pair.second));
        }
        for (auto pair : sorted)
            m_linkMenuMerger->append(*(pair.second));
    }

    if (modems.size() == 0) {
        m_bottomMenu->clear();
    } else {
        if (m_bottomMenu->find(*m_openCellularSettings) == m_bottomMenu->end())
            m_bottomMenu->append(*m_openCellularSettings);
    }

    if (m_items.size() > 1) {
        for(auto i : m_items)
            i.second->showSimIdentifier(true);
    } else {
        for(auto i : m_items)
            i.second->showSimIdentifier(false);
    }
}

WwanSection::WwanSection(ModemManager::Ptr modemManager)
    : d{new Private(modemManager)}
{
    d->ConstructL();
}

WwanSection::~WwanSection()
{

}

ActionGroup::Ptr
WwanSection::actionGroup()
{
    return *d->m_actionGroupMerger;
}

MenuModel::Ptr
WwanSection::menuModel()
{
    return d->m_topMenu;
}

void
WwanSection::unlockAllModems()
{
    d->m_modemManager->unlockAllModems();
}
