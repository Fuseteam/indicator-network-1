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

#include <memory>
#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/menu-model.h"

class Section
{
public:
    typedef std::shared_ptr<Section> Ptr;

    virtual ~Section() {}

    virtual ActionGroup::Ptr actionGroup() = 0;
    virtual MenuModel::Ptr   menuModel()   = 0;
};
