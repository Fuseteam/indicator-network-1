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

#include "action-group.h"
#include <QDebug>

ActionGroup::ActionGroup()
{
}

std::set<Action::Ptr> ActionGroup::actions()
{
    return m_actions;
}

void ActionGroup::add(Action::Ptr action)
{
    auto iter = m_actions.find(action);
    if (iter != m_actions.end()) {
        /// @todo throw something.
        std::cerr << "Trying to add action which was already added before." << std::endl;
        return;
    }
    m_actions.insert(action);
    Q_EMIT actionAdded(action);
}

void ActionGroup::remove(Action::Ptr action)
{
    auto iter = m_actions.find(action);
    if (iter == m_actions.end()) {
        /// @todo throw something.
        std::cerr << "Trying to remove action which was not added before." << std::endl;
        return;
    }
    Q_EMIT actionRemoved(action);
    m_actions.erase(action);
}

bool ActionGroup::contains(Action::Ptr action)
{
    return m_actions.find(action) != m_actions.end();
}
