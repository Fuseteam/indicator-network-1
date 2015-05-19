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

#pragma once

#include <menumodel-cpp/action-group-merger.h>
#include <menumodel-cpp/menu-item.h>

#include <memory>
#include <QObject>

class Item: public QObject
{
public:
    typedef std::shared_ptr<Item> Ptr;

    Item()
    {
        m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
        m_actionGroup = std::make_shared<ActionGroup>();
        m_actionGroupMerger->add(m_actionGroup);
    }

    virtual ~Item() = default;

    virtual ActionGroup::Ptr actionGroup()
    {
        return m_actionGroupMerger->actionGroup();
    }

    virtual MenuItem::Ptr menuItem() = 0;

protected:

    ActionGroup::Ptr m_actionGroup;
    ActionGroupMerger::Ptr m_actionGroupMerger;

private:
};
