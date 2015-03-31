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

#ifndef WIFI_LINK_ITEM_H
#define WIFI_LINK_ITEM_H

#include <nmofono/wifi/wifi-link.h>

#include "menuitems/item.h"

class WifiLinkItem : public Item
{
    class Private;
    std::shared_ptr<Private> d;

public:
    typedef std::shared_ptr<WifiLinkItem> Ptr;

    WifiLinkItem() = delete;
    WifiLinkItem(connectivity::networking::wifi::Link::Ptr link);
    virtual ~WifiLinkItem();

    virtual MenuItem::Ptr menuItem();
    virtual ActionGroup::Ptr actionGroup();
};

#endif // WIFI_LINK_ITEM_H
