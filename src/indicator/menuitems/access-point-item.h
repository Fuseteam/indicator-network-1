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

#include <nmofono/wifi/access-point.h>
#include "item.h"

class AccessPointItem : public Item
{
    Q_OBJECT

    class Private;
    std::shared_ptr<Private> d;

public:
    typedef std::shared_ptr<AccessPointItem> Ptr;

    explicit AccessPointItem(nmofono::wifi::AccessPoint::Ptr accessPoint, bool isActive = false);
    virtual ~AccessPointItem();

    void setActive(bool value);

    virtual MenuItem::Ptr menuItem();

Q_SIGNALS:
    void activated();
};
