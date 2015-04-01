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

#ifndef MENU_H
#define MENU_H

#include <memory>
#include <list>
#include <vector>
#include <map>
#include <algorithm>

#include <gio/gio.h>

#include "gio-helpers/util.h"
#include "menu-model.h"
#include "menu-item.h"

class Menu : public MenuModel
{
    Q_OBJECT

    GMenuPtr m_gmenu;
    std::list<MenuItem::Ptr> m_items;

public:
    typedef std::shared_ptr<Menu> Ptr;
    typedef std::list<MenuItem::Ptr>::iterator iterator;

    Menu();

    virtual ~Menu();

    void append(MenuItem::Ptr item);

    void insert(MenuItem::Ptr item, iterator position);

    /* Binary function that accepts two elements in the range as arguments,
     * and returns a value convertible to bool. The value returned indicates
     * whether the element passed as first argument is considered to go before
     * the second in the specific strict weak ordering it defines.
     * The function shall not modify any of its arguments.
     * This can either be a function pointer or a function object.
     */
    void insert(MenuItem::Ptr item, std::function<bool(MenuItem::Ptr a, MenuItem::Ptr b)> compare);

    //iterator insertAbove(MenuItem::Ptr item, iterator position);
    //iterator insertBelow(MenuItem::Ptr item, iterator position);

    void remove(iterator item);

    void removeAll(MenuItem::Ptr item);

    //void removeRange(iterator first, iterator last);

    void moveTo(iterator item, iterator position);

    //void moveRangeTo(iterator first, iterator last, iterator position);

    /// finds the first occurence of item
    iterator find(MenuItem::Ptr item);

    iterator begin();

    iterator end();

    // clear the whole menu
    void clear();

    operator GMenuModel*() { return G_MENU_MODEL(m_gmenu.get()); }

private Q_SLOTS:
    void itemChanged();
};

#endif
