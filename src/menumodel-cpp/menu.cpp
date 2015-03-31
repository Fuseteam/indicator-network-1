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

#include "menu.h"

Menu::Menu()
{
    m_gmenu = make_gmenu_ptr();
}

Menu::~Menu()
{
    clear();
//    GMainLoopSync([]{});
}

void Menu::append(MenuItem::Ptr item)
{
    m_items.push_back(item);
    // prevent this-> from being captured
    g_menu_append_item(m_gmenu.get(), item->gmenuitem());
    if (std::count(m_items.begin(), m_items.end(), item) == 1) {
        connect(item.get(), &MenuItem::changed, this, &Menu::itemChanged);
    }
}

void Menu::insert(MenuItem::Ptr item, iterator position)
{
    int index = 0;
    auto iter = m_items.begin();
    while (iter != m_items.end()) {
        if (iter == position)
            break;
        ++index;
        ++iter;
    }

    // prevent this-> from being captured
    auto menu = m_gmenu;
    g_menu_insert_item(m_gmenu.get(), index, item->gmenuitem());
    m_items.insert(position, item);
}

/* Binary function that accepts two elements in the range as arguments,
 * and returns a value convertible to bool. The value returned indicates
 * whether the element passed as first argument is considered to go before
 * the second in the specific strict weak ordering it defines.
 * The function shall not modify any of its arguments.
 * This can either be a function pointer or a function object.
 */
void Menu::insert(MenuItem::Ptr item, std::function<bool(MenuItem::Ptr a, MenuItem::Ptr b)> compare)
{
    for (auto iter = m_items.begin(); iter != m_items.end(); iter++) {
        if (compare(item, *iter)) {
            insert(item, iter);
            return;
        }
    }
    // end of list reached.
    append(item);
}

//iterator insertAbove(MenuItem::Ptr item, iterator position);
//iterator insertBelow(MenuItem::Ptr item, iterator position);

void Menu::remove(iterator item)
{
    /// @todo check that the item actually is part of the menu
    /// @todo disconnect the changed signal
    int index = 0;
    auto iter = m_items.begin();
    while (iter != item && iter != m_items.end()) {
        ++index;
        ++iter;
    }

    if (iter == m_items.end())
     return;

    g_menu_remove(m_gmenu.get(), index);
    m_items.erase(item);
}

void Menu::removeAll(MenuItem::Ptr item)
{
    /// @todo check that item has been added before.
    /// @todo disconnect the changed signal

    // work reversed so that GMenu positions match tbe m_items positions
    int index = m_items.size()-1;
    for (auto iter = m_items.rbegin(); iter != m_items.rend(); ++iter) {
        // prevent this-> from being captured
        auto menu = m_gmenu;
        if (*iter == item) {
            g_menu_remove(m_gmenu.get(), index);
        }
        --index;
    }
    // after the loop finishes we should be at index -1
    // which is one above the first g_menu index (=0)
    assert(index == -1);
    auto iter = std::remove(m_items.begin(), m_items.end(), item);
    m_items.erase(iter, m_items.end());
}

//void removeRange(iterator first, iterator last);

void Menu::moveTo(iterator item, iterator position)
{
    /// @todo assert that iter is part of the group
    /// @todo assert that position is valid

    if (item == position)
        return;

    remove(item);
    insert(*item, position);
}

//void moveRangeTo(iterator first, iterator last, iterator position);

/// finds the first occurence of item
Menu::iterator Menu::find(MenuItem::Ptr item)
{
    return std::find(m_items.begin(), m_items.end(), item);
}

Menu::iterator Menu::begin()
{
    return m_items.begin();
}

Menu::iterator Menu::end()
{
    return m_items.end();
}

// clear the whole menu
void Menu::clear()
{
    // prevent this-> from being captured
    g_menu_remove_all(m_gmenu.get());
    m_items.clear();
}

void Menu::itemChanged()
{
    auto item = qobject_cast<MenuItem*>(sender());

    int index = 0;
    for (auto iter = m_items.begin(); iter != m_items.end(); ++iter) {
        if (iter->get() == item) {
            g_menu_remove(m_gmenu.get(), index);
            g_menu_insert_item(m_gmenu.get(), index, item->gmenuitem());
        }
        ++index;
    }
}
