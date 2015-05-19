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

#include "item.h"
#include "menumodel-cpp/action.h"
#include "menumodel-cpp/menu-item.h"
#include "menumodel-cpp/gio-helpers/variant.h"

#include <functional>
#include <vector>

class ModemInfoItem : public Item
{
    Q_OBJECT

    class Private;
    std::unique_ptr<Private> d;

public:
    typedef std::shared_ptr<ModemInfoItem> Ptr;

    ModemInfoItem();
    virtual ~ModemInfoItem();

    virtual MenuItem::Ptr menuItem();

public Q_SLOTS:
    void setStatusIcon(const QString &name);
    void setStatusText(const QString &value);
    void setConnectivityIcon(const QString &name);
    void setSimIdentifierText(const QString &value);
    void setLocked(bool value);
    void setRoaming(bool value);

Q_SIGNALS:
    void unlock();
};
