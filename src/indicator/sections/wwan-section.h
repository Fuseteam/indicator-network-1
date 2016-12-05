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

#include <menuitems/switch-item.h>
#include <menuitems/section.h>
#include <nmofono/manager.h>

class WwanSection : public Section
{
    class Private;
    std::shared_ptr<Private> d;

public:
    UNITY_DEFINES_PTRS(WwanSection);

    explicit WwanSection(nmofono::Manager::Ptr modemManager, SwitchItem::Ptr mobileDataSwitch, SwitchItem::Ptr hotspotSwitch);
    virtual ~WwanSection();

    virtual ActionGroup::Ptr actionGroup();
    virtual MenuModel::Ptr menuModel();
};
