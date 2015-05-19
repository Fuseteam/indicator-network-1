/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#pragma once

#include <memory>
#include <gio/gio.h>

#include "gio-helpers/util.h"
#include "action-group.h"
#include <set>
#include <map>
#include <QObject>

class ActionGroupMerger: public QObject
{
    Q_OBJECT

    std::string m_prefix;
    ActionGroup::Ptr m_actionGroup;

    std::map<ActionGroup::Ptr, std::pair<QMetaObject::Connection, QMetaObject::Connection>> m_groups;

    std::map<Action::Ptr, int> m_count;
    std::map<std::string, Action::Ptr> m_names;

private Q_SLOTS:
    void addAction(Action::Ptr action);

    void removeAction(Action::Ptr action);

public:
    typedef std::shared_ptr<ActionGroupMerger> Ptr;
    typedef std::unique_ptr<ActionGroupMerger> UPtr;

    explicit ActionGroupMerger(const std::string &prefix = "");

    void add(ActionGroup::Ptr group);

    void remove(ActionGroup::Ptr group);

    ActionGroup::Ptr actionGroup();
};
