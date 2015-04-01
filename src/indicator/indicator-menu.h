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

#ifndef INDICATOR_MENU_H
#define INDICATOR_MENU_H

#include <menumodel-cpp/action-group.h>
#include <menumodel-cpp/menu.h>
#include <menuitems/section.h>
#include <root-state.h>

#include <vector>

class IndicatorMenu
{
public:
    typedef std::shared_ptr<IndicatorMenu> Ptr;

    IndicatorMenu() = delete;

    virtual ~IndicatorMenu() = default;

    IndicatorMenu(RootState::Ptr rootState, const std::string &prefix);

    virtual void addSection(Section::Ptr section);

    Menu::Ptr menu() const;

    ActionGroup::Ptr actionGroup() const;

private:
    struct Private;
    std::shared_ptr<Private> d;
};

#endif // INDICATOR_MENU_H
