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

#pragma once

#include <memory>
#include <map>

#include <gio/gio.h>

#include "gio-helpers/variant.h"
#include "gio-helpers/util.h"

#include "menu-model.h"

#include <QObject>

class MenuItem: public QObject
{
    Q_OBJECT

    GMenuItemPtr m_gmenuitem;

    std::string m_label;
    std::string m_action;

    std::map<std::string, Variant> m_attributes;

public:
    typedef std::shared_ptr<MenuItem> Ptr;

    static MenuItem::Ptr newSubmenu(MenuModel::Ptr submenu,
                                    const std::string &label = "");

    static MenuItem::Ptr newSection(MenuModel::Ptr submenu,
                                    const std::string &label = "");

    explicit MenuItem(const std::string &label  = "",
             const std::string &action = "");

    ~MenuItem();

    std::string label();

    void setLabel(const std::string &value);

    void setAction(const std::string &value);

    void setAttribute(const std::string &attribute,
                      Variant value);

    void clearAttribute(const std::string &attribute);

    GMenuItem *gmenuitem();

    const std::string &
    action () const;

Q_SIGNALS:
    void changed();
};
