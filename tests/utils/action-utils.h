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
 *     Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <cassert>
#include <menumodel-cpp/action-group.h>
#include <menumodel-cpp/menu-item.h>

namespace testutils
{

std::string
string_value (MenuItem::Ptr menuItem, const std::string &name);

bool
bool_value (MenuItem::Ptr menuItem, const std::string &name);

Action::Ptr
findAction (ActionGroup::Ptr actionGroup, const std::string &name);

} /* namespace testutils */
