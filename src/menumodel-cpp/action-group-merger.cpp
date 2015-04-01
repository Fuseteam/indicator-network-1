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

#include "action-group-merger.h"

void ActionGroupMerger::addAction(Action::Ptr action)
{
    auto name_iter = m_names.find(action->name());
    if (name_iter != m_names.end()) {
        // we have two actions with the same name.
        // If they are from the same shared pointer, everything is OK and
        // count is incremented below, but if they have different pointer
        // then they will override each other in GActionGroup so let's catch that
        // early on.
        if (name_iter->second != action) {
            std::cerr << __PRETTY_FUNCTION__ << ": Conflicting action names. \"" << action->name() << "\"" << std::endl;
            /// @todo thow something.
            return;
        }
    } else {
        m_names[action->name()] = action;
    }

    auto count_iter = m_count.find(action);
    if (count_iter != m_count.end()) {
        count_iter->second += 1;
    } else {
        m_count[action] = 1;
        m_actionGroup->add(action);
    }
}

void ActionGroupMerger::removeAction(Action::Ptr action)
{
    auto count_iter = m_count.find(action);
    // it should not be possible for this function to be called for an action that
    // was not added before
    assert(count_iter != m_count.end());
    count_iter->second -= 1;
    if (count_iter->second == 0) {
        m_actionGroup->remove(action);
        m_names.erase(action->name());
        m_count.erase(count_iter);
    }
}


ActionGroupMerger::ActionGroupMerger(const std::string &prefix)
    : m_prefix {prefix}
{
    m_actionGroup = std::make_shared<ActionGroup>();
}

void ActionGroupMerger::add(ActionGroup::Ptr group)
{
    auto iter = m_groups.find(group);
    if (iter != m_groups.end()) {
        /// @todo throw something.
        std::cerr << __PRETTY_FUNCTION__ << ": Trying to add action group which was already added before." << std::endl;
        return;
    }

    for (auto action : group->actions()) {
        addAction(action);
    }

    auto added = connect(group.get(), &ActionGroup::actionAdded, this, &ActionGroupMerger::addAction);
    auto removed = connect(group.get(), &ActionGroup::actionRemoved, this, &ActionGroupMerger::removeAction);
    m_groups[group] = std::make_pair(added, removed);
}

void ActionGroupMerger::remove(ActionGroup::Ptr group)
{
    auto iter = m_groups.find(group);
    if (iter == m_groups.end()) {
        /// @todo throw something.
        std::cerr << __PRETTY_FUNCTION__ << ": Trying to remove action group which was not added before." << std::endl;
        return;
    }
    auto connections = m_groups[group];
    disconnect(connections.first);
    disconnect(connections.second);
    m_groups.erase(group);
    for (auto action : group->actions()) {
        removeAction(action);
    }
}

ActionGroup::Ptr ActionGroupMerger::actionGroup()
{
    return m_actionGroup;
}
