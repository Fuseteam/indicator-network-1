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

#include <menuitems/item.h>
#include <menumodel-cpp/menu-item.h>

#include <QObject>

class SwitchItem : public Item
{
    Q_OBJECT

public:
    typedef std::shared_ptr<SwitchItem> Ptr;

    SwitchItem() = delete;
    virtual  ~SwitchItem() = default;
    SwitchItem(const std::string &label, const std::string &prefix, const std::string &name);

    virtual MenuItem::Ptr menuItem();

    // state of the switch
    bool state();

public Q_SLOTS:
    void setState(bool state);

    void setEnabled(bool enabled);

Q_SIGNALS:
    void stateUpdated(bool);

private Q_SLOTS:
    void actionStateChanged(const Variant&);

    void actionActivated(const Variant&);

private:
    Action::Ptr m_action;
    MenuItem::Ptr m_item;
};
