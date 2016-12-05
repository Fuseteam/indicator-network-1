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

#include <menuitems/ethernet-connection-item.h>
#include <menuitems/ethernet-link-item.h>
#include <sections/ethernet-link-section.h>

#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-merger.h"

#include <util/localisation.h>
#include <util/qhash-sharedptr.h>

using namespace std;
using namespace nmofono;
using namespace nmofono::connection;
using namespace nmofono::ethernet;

class EthernetLinkSection::Private : public QObject
{
Q_OBJECT
public:
    Manager::Ptr m_manager;

    EthernetLink::SPtr m_link;

    ActionGroupMerger::Ptr m_actionGroupMerger;

    ActionGroup::Ptr m_actionGroup;

    MenuMerger::Ptr m_menuMerger;

    Menu::Ptr m_menu;

    MenuItem::Ptr m_item;

    Menu::Ptr m_linkMenu;

    EthernetLinkItem::SPtr m_linkItem;

    Menu::Ptr m_connectionsMenu;

    QMap<AvailableConnection::SPtr, EthernetConnectionItem::SPtr> m_items;

    Action::Ptr m_connectionAction;

    Private()
    {
    }

public Q_SLOTS:
    void
    linksChanged()
    {
        m_linkItem->setShowInterface(m_manager->ethernetLinks().size() > 1);
    }

    void
    updateConnections()
    {
        auto connections = m_link->availableConnections().toSet();
        auto current(m_items.keys().toSet());

        auto removed(current);
        removed.subtract(connections);

        auto added(connections);
        added.subtract(current);

        for (auto connection : removed)
        {
            m_actionGroupMerger->remove(m_items[connection]->actionGroup());
            m_items.remove(connection);
        }

        for (auto connection : added)
        {
            auto item = make_shared<EthernetConnectionItem>(connection, m_connectionAction);
            m_items[connection] = item;
            m_actionGroupMerger->add(item->actionGroup());

            // Make sure we re-order if the ID changes
            connect(connection.get(), &AvailableConnection::connectionIdChanged, this, &Private::updateConnections, Qt::UniqueConnection);
        }

        m_connectionsMenu->clear();

        if (connections.size() <= 1)
        {
            return;
        }

        QMap<QString, EthernetConnectionItem::SPtr> sorted;
        for (QMapIterator<AvailableConnection::SPtr, EthernetConnectionItem::SPtr> it(m_items); it.hasNext();)
        {
            it.next();
            sorted[it.key()->connectionId()] = it.value();
        }
        for (auto item : sorted)
        {
            m_connectionsMenu->append(item->menuItem());
        }
    }

    void
    actionActivated(const Variant& value)
    {
        auto uuid = QString::fromStdString(value.as<string>());
        QMapIterator<AvailableConnection::SPtr, EthernetConnectionItem::SPtr> it(m_items);
        while (it.hasNext())
        {
            it.next();

            auto connection = it.key();
            if (connection->connectionUuid() == uuid)
            {
                m_link->setPreferredConnection(connection);
                break;
            }
        }
    }

    void
    preferredConnectionChanged(AvailableConnection::SPtr preferredConnection)
    {
        if (preferredConnection)
        {
            m_connectionAction->setState(TypedVariant<string>(preferredConnection->connectionUuid().toStdString()));
        }
        else
        {
            m_connectionAction->setState(TypedVariant<string>(""));
        }
    }
};

EthernetLinkSection::EthernetLinkSection(Manager::Ptr manager, EthernetLink::SPtr link) :
        d(new Private())
{
    d->m_manager = manager;
    d->m_link = link;

    d->m_actionGroupMerger = make_shared<ActionGroupMerger>();
    d->m_actionGroup = make_shared<ActionGroup>();
    d->m_actionGroupMerger->add(d->m_actionGroup);

    d->m_menuMerger = make_shared<MenuMerger>();

    d->m_linkMenu = make_shared<Menu>();
    d->m_linkItem = make_shared<EthernetLinkItem>(d->m_link);
    d->m_linkMenu->append(d->m_linkItem->menuItem());
    d->m_actionGroupMerger->add(d->m_linkItem->actionGroup());

    d->m_connectionsMenu = make_shared<Menu>();

    // have the ethernet link in its own section
    d->m_item = MenuItem::newSection(d->m_menuMerger);
    d->m_menu = make_shared<Menu>();
    d->m_menu->append(d->m_item);

    d->m_menuMerger->append(d->m_linkMenu);
    d->m_menuMerger->append(d->m_connectionsMenu);

    QString actionName = "ethernet." + QString::number(link->id()) + "::connection";
    d->m_connectionAction = make_shared<Action>(actionName, G_VARIANT_TYPE_STRING, TypedVariant<string>(""));
    d->m_actionGroup->add(d->m_connectionAction);

    QObject::connect(d->m_manager.get(), &Manager::linksUpdated, d.get(), &Private::linksChanged);
    d->linksChanged();
    QObject::connect(d->m_link.get(), &EthernetLink::availableConnectionsChanged, d.get(), &Private::updateConnections);
    d->updateConnections();
    QObject::connect(d->m_link.get(), &EthernetLink::preferredConnectionChanged, d.get(), &Private::preferredConnectionChanged);
    d->preferredConnectionChanged(d->m_link->preferredConnection());

    QObject::connect(d->m_connectionAction.get(), &Action::activated, d.get(), &Private::actionActivated);
}

EthernetLinkSection::~EthernetLinkSection()
{
}

ActionGroup::Ptr
EthernetLinkSection::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuModel::Ptr
EthernetLinkSection::menuModel()
{
    return d->m_menu;
}

#include "ethernet-link-section.moc"
