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

#include "item.h"
#include "menumodel-cpp/action.h"
#include "menumodel-cpp/menu-item.h"
#include "menumodel-cpp/gio-helpers/variant.h"

#include <functional>
#include <vector>

class EthernetItem : public Item
{
    Q_OBJECT

    class Private;
    std::unique_ptr<Private> d;

public:
    typedef std::shared_ptr<EthernetItem> Ptr;

    EthernetItem(unsigned int id);
    virtual ~EthernetItem();

    virtual MenuItem::Ptr menuItem();

public Q_SLOTS:
    void setStatusText(const QString &value);

    void setName(const QString &value);

    void setAutoConnect(bool autoConnect);

Q_SIGNALS:
    void autoConnectChanged(bool autoconnect);
};
