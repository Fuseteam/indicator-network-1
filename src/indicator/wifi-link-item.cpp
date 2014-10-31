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

#include "wifi-link-item.h"

#include "menuitems/text-item.h"
#include "menuitems/access-point-item.h"

#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-merger.h"

namespace networking = connectivity::networking;

#include <algorithm>
#include <locale>

class WifiLinkItem::Private : public std::enable_shared_from_this<Private>
{
public:
    ActionGroupMerger::Ptr m_actionGroupMerger;

    networking::wifi::Link::Ptr m_link;

    /// @todo do something with me...
    Action::Ptr m_actionBusy;

    networking::wifi::AccessPoint::Ptr m_activeAccessPoint;
    std::map<networking::wifi::AccessPoint::Ptr, AccessPointItem::Ptr> m_accessPoints;

    Menu::Ptr m_topMenu;

    Menu::Ptr m_connectedBeforeApsMenu;
    Menu::Ptr m_neverConnectedApsMenu;
    MenuMerger::Ptr m_apsMerger;

    Menu::Ptr m_bottomMenu;

    MenuMerger::Ptr m_rootMerger;
    MenuItem::Ptr m_item;

    std::function<bool(MenuItem::Ptr a, MenuItem::Ptr b)> m_accessPointCompare;

    std::string m_icon;
    std::string m_a11ydesc;

    TextItem::Ptr m_otherNetwork;

    std::mutex m_updateActiveAccessPointMutex;

public:


    Private() = delete;
    ~Private() {}
    Private(networking::wifi::Link::Ptr link)
        : m_link {link}
    {}

    void ConstructL()
    {
        m_actionGroupMerger = std::make_shared<ActionGroupMerger>();

        m_topMenu = std::make_shared<Menu>();

        m_connectedBeforeApsMenu = std::make_shared<Menu>();
        m_neverConnectedApsMenu = std::make_shared<Menu>();
        m_apsMerger = std::make_shared<MenuMerger>();

        m_bottomMenu = std::make_shared<Menu>();

        m_rootMerger = std::make_shared<MenuMerger>();

        m_accessPointCompare = [](MenuItem::Ptr a, MenuItem::Ptr b){
            // order alphabetically by SSID

            std::locale loc;

            std::string a_label = a->label();
            std::string b_label = b->label();

            std::string a_upper;
            std::string b_upper;

            for (std::string::size_type i=0; i<a_label.length(); ++i)
                a_upper += std::toupper(a_label[i], loc);

            for (std::string::size_type i=0; i<b_label.length(); ++i)
                b_upper += std::toupper(b_label[i], loc);

            return a_upper < b_upper;
        };

        auto that = shared_from_this();

        updateAccessPoints(m_link->accessPoints().get());
        m_link->accessPoints().changed().connect([that](std::set<networking::wifi::AccessPoint::Ptr> accessPoints)
        {
            GMainLoopDispatch([that, accessPoints]()
            {
                that->updateAccessPoints(accessPoints);
            });
        });

        updateActiveAccessPoint(m_link->activeAccessPoint().get());
        m_link->activeAccessPoint().changed().connect([that](networking::wifi::AccessPoint::Ptr ap)
        {
            GMainLoopDispatch([that, ap]()
            {
                that->updateActiveAccessPoint(ap);
            });
        });

        m_otherNetwork = std::make_shared<TextItem>(_("Other network…"), "wifi", "othernetwork");
        //m_actionGroupMerger->add(*m_otherNetwork);

        m_rootMerger->append(m_topMenu);

        m_apsMerger->append(m_connectedBeforeApsMenu);
        m_apsMerger->append(m_neverConnectedApsMenu);
        m_rootMerger->append(m_apsMerger);

        //m_bottomMenu->append(*m_otherNetwork);
        m_rootMerger->append(m_bottomMenu);

        m_item = MenuItem::newSection(m_rootMerger);
    }

    void updateAccessPoints(std::set<networking::wifi::AccessPoint::Ptr> accessPoints)
    {
        /// @todo previously connected
        /// @todo apply visibility policy.

        std::set<networking::wifi::AccessPoint::Ptr> old;
        for (auto pair: m_accessPoints) {
            old.insert(pair.first);
        }

        std::set<networking::wifi::AccessPoint::Ptr> added;
        std::set_difference(accessPoints.begin(), accessPoints.end(),
                            old.begin(), old.end(),
                            std::inserter(added, added.begin()));

        std::set<networking::wifi::AccessPoint::Ptr> removed;
        std::set_difference(old.begin(), old.end(),
                            accessPoints.begin(), accessPoints.end(),
                            std::inserter(removed, removed.begin()));

        for (auto ap: removed) {
            bool isActive = (ap == m_activeAccessPoint);
            if (isActive)
                m_connectedBeforeApsMenu->removeAll(m_accessPoints[ap]->menuItem());
            else
                m_neverConnectedApsMenu->removeAll(m_accessPoints[ap]->menuItem());
            /// @todo disconnect activated...
            m_actionGroupMerger->remove(m_accessPoints[ap]->actionGroup());
            m_accessPoints.erase(ap);
        }

        for (auto ap : added) {

            /// @todo handle hidden APs all the way
            if (ap->ssid().empty())
                continue;

            bool isActive = (ap == m_activeAccessPoint);
            auto item = std::make_shared<AccessPointItem>(ap, isActive);
            item->activated().connect([this, ap](){
                updateActiveAccessPoint(ap);
                m_link->connect_to(ap);
            });
            m_accessPoints[ap] = item;
            m_actionGroupMerger->add(item->actionGroup());
            if (isActive) {
                updateActiveAccessPoint(m_activeAccessPoint);
            } else {
                m_neverConnectedApsMenu->insert(item->menuItem(), m_accessPointCompare);
            }
        }
    }

    void updateActiveAccessPoint(networking::wifi::AccessPoint::Ptr ap)
    {
        std::lock_guard<std::mutex> lock(m_updateActiveAccessPointMutex);
        m_activeAccessPoint = ap;

        auto current = m_connectedBeforeApsMenu->begin();
        if (current != m_connectedBeforeApsMenu->end()) {
            // move to other menu
            m_neverConnectedApsMenu->insert(*current, m_accessPointCompare);
            m_connectedBeforeApsMenu->clear();
        }

        for (auto pair : m_accessPoints) {
            if (ap && ap == pair.first) {
                m_connectedBeforeApsMenu->insert(pair.second->menuItem(), m_connectedBeforeApsMenu->begin());
                pair.second->setActive(true);
                m_neverConnectedApsMenu->removeAll(pair.second->menuItem());
                continue;
            }
            pair.second->setActive(false);
        }
    }

};


WifiLinkItem::WifiLinkItem(connectivity::networking::wifi::Link::Ptr link)
    : d{new Private(link)}
{
    d->ConstructL();
}

WifiLinkItem::~WifiLinkItem()
{}

ActionGroup::Ptr
WifiLinkItem::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuItem::Ptr
WifiLinkItem::menuItem()
{
    return d->m_item;
}
