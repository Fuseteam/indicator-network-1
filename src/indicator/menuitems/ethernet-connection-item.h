/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <menuitems/item.h>
#include <menumodel-cpp/menu-item.h>
#include <nmofono/connection/available-connection.h>

#include <unity/util/DefinesPtrs.h>

#include <QObject>
#include <QString>

class EthernetConnectionItem : public Item
{

public:
    UNITY_DEFINES_PTRS(EthernetConnectionItem);

    EthernetConnectionItem() = delete;
    ~EthernetConnectionItem() = default;
    EthernetConnectionItem(nmofono::connection::AvailableConnection::SPtr connection, Action::Ptr action);

    virtual MenuItem::Ptr menuItem();

private:
    class Private;
    std::shared_ptr<Private> d;
};
