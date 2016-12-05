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

    QString m_label;
    QString m_action;
    QString m_icon;

    std::map<QString, Variant> m_attributes;

public:
    typedef std::shared_ptr<MenuItem> Ptr;

    static MenuItem::Ptr newSubmenu(MenuModel::Ptr submenu,
                                    const QString &label = "");

    static MenuItem::Ptr newSection(MenuModel::Ptr submenu,
                                    const QString &label = "");

    explicit MenuItem(const QString &label  = "",
             const QString &action = "");

    ~MenuItem();

    QString label();

    QString icon();

    void clearAttribute(const QString &attribute);

    GMenuItem *gmenuitem();

    const QString& action () const;

public Q_SLOTS:
    void setLabel(const QString &value);

    void setIcon(const QString &icon);

    void setAction(const QString &value);

    void setActionAndTargetValue(const QString &value, const Variant& target);

    void setAttribute(const QString &attribute,
                      Variant value);

Q_SIGNALS:
    void changed();
};
