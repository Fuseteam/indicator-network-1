/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <sections/ethernet-link-section.h>
#include <sections/ethernet-section.h>
#include <menuitems/switch-item.h>
#include <menuitems/text-item.h>

#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-merger.h"

#include <util/localisation.h>
#include <util/qhash-sharedptr.h>

using namespace std;
using namespace nmofono;
using namespace nmofono::ethernet;

class EthernetSection::Private : public QObject
{
    Q_OBJECT
public:
    Manager::Ptr m_manager;

    Menu::Ptr m_settingsMenu;

    ActionGroupMerger::Ptr m_actionGroupMerger;

    MenuMerger::Ptr m_menuMerger;

    QMap<EthernetLink::SPtr, EthernetLinkSection::SPtr> m_items;

    Private()
    {
    }

public Q_SLOTS:
    void linksChanged()
    {
        auto links = m_manager->ethernetLinks();
        auto current(m_items.keys().toSet());

        auto removed(current);
        removed.subtract(links);

        auto added(links);
        added.subtract(current);

        if (added.isEmpty() && removed.isEmpty())
        {
            return;
        }

        for (auto connection : removed)
        {
            m_actionGroupMerger->remove(m_items[connection]->actionGroup());
            m_items.remove(connection);
        }

        for (auto link : added)
        {
            auto section = make_shared<EthernetLinkSection>(m_manager, link);
            m_items[link] = section;
            m_actionGroupMerger->add(section->actionGroup());
        }

        m_menuMerger->clear();

        multimap<Link::Id, EthernetLinkSection::SPtr> sorted;
        QMapIterator<EthernetLink::SPtr, EthernetLinkSection::SPtr> it(m_items);
        while (it.hasNext())
        {
            it.next();
            sorted.insert(make_pair(it.key()->id(), it.value()));
        }
        for (auto pair : sorted)
        {
            m_menuMerger->append(pair.second->menuModel());
        }

        if (!links.isEmpty())
        {
            m_menuMerger->append(m_settingsMenu);
        }
    }
};

EthernetSection::EthernetSection(Manager::Ptr manager)
    : d{new Private()}
{
    d->m_manager = manager;

    d->m_actionGroupMerger = make_shared<ActionGroupMerger>();
    d->m_menuMerger = make_shared<MenuMerger>();

    auto settingsItem = make_shared<TextItem>(_("Ethernet settings…"), "ethernet", "settings");
    d->m_actionGroupMerger->add(settingsItem->actionGroup());

    d->m_settingsMenu = make_shared<Menu>();
    d->m_settingsMenu->append(settingsItem->menuItem());

    QObject::connect(d->m_manager.get(), &Manager::linksUpdated, d.get(), &Private::linksChanged);
    d->linksChanged();
}

EthernetSection::~EthernetSection()
{
}

ActionGroup::Ptr
EthernetSection::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuModel::Ptr
EthernetSection::menuModel()
{
    return d->m_menuMerger;
}

#include "ethernet-section.moc"
