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

#include "menu-item.h"


MenuItem::Ptr MenuItem::newSubmenu(MenuModel::Ptr submenu,
                                const std::string &label)
{
    auto menu = std::make_shared<MenuItem>(label);
    g_menu_item_set_submenu(menu->m_gmenuitem.get(), *submenu);
    return menu;
}

MenuItem::Ptr MenuItem::newSection(MenuModel::Ptr submenu,
                                const std::string &label)
{
    auto menu = std::make_shared<MenuItem>(label);
    g_menu_item_set_section(menu->m_gmenuitem.get(), *submenu);
    return menu;
}

MenuItem::MenuItem(const std::string &label,
         const std::string &action)
    : m_label{label},
      m_action{action}
{
    /// @todo validate that action is valid
    m_gmenuitem = make_gmenuitem_ptr(g_menu_item_new(nullptr, nullptr));
    if (!label.empty()) {
        g_menu_item_set_label(m_gmenuitem.get(), label.c_str());
    }
    if (!action.empty()) {
        g_menu_item_set_detailed_action(m_gmenuitem.get(), action.c_str());
    }
}

MenuItem::~MenuItem()
{
}

std::string MenuItem::label()
{
    return m_label;
}

void MenuItem::setLabel(const std::string &value)
{
    if (m_label == value)
        return;
    m_label = value;
    g_menu_item_set_label(m_gmenuitem.get(), m_label.c_str());
    Q_EMIT changed();
}

void MenuItem::setAction(const std::string &value)
{
    /// @todo validate that action is valid
    if (m_action == value)
        return;
    m_action = value;
    g_menu_item_set_detailed_action(m_gmenuitem.get(), m_action.c_str());
    Q_EMIT changed();
}

void MenuItem::setAttribute(const std::string &attribute,
                  Variant value)
{
    /// @todo validate that attribute is valid.
    assert(value);
    assert(!attribute.empty());

    auto iter = m_attributes.find(attribute);
    if (iter != m_attributes.end()) {
        if (iter->second == value)
            return;
        iter->second = value;
    } else {
        m_attributes[attribute] = value;
    }
    g_menu_item_set_attribute_value(m_gmenuitem.get(), attribute.c_str(), value);
    Q_EMIT changed();
}

void MenuItem::clearAttribute(const std::string &attribute)
{
    assert(!attribute.empty());
    m_attributes.erase(attribute);
    g_menu_item_set_attribute(m_gmenuitem.get(), attribute.c_str(), nullptr);
    Q_EMIT changed();
}

GMenuItem *
MenuItem::gmenuitem()
{
    return m_gmenuitem.get();
}

const std::string &
MenuItem::action () const
{
    return m_action;
}
