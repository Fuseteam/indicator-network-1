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
#include <QDebug>

using namespace std;

MenuItem::Ptr MenuItem::newSubmenu(MenuModel::Ptr submenu,
                                const QString &label)
{
    auto menu = std::make_shared<MenuItem>(label);
    g_menu_item_set_submenu(menu->m_gmenuitem.get(), *submenu);
    return menu;
}

MenuItem::Ptr MenuItem::newSection(MenuModel::Ptr submenu,
                                const QString &label)
{
    auto menu = std::make_shared<MenuItem>(label);
    g_menu_item_set_section(menu->m_gmenuitem.get(), *submenu);
    return menu;
}

MenuItem::MenuItem(const QString &label,
         const QString &action)
    : m_label{label},
      m_action{action}
{
    /// @todo validate that action is valid
    m_gmenuitem = make_gmenuitem_ptr(g_menu_item_new(nullptr, nullptr));
    if (!label.isEmpty()) {
        g_menu_item_set_label(m_gmenuitem.get(), label.toUtf8().constData());
    }
    if (!action.isEmpty()) {
        g_menu_item_set_detailed_action(m_gmenuitem.get(), action.toUtf8().constData());
    }
}

MenuItem::~MenuItem()
{
}

QString MenuItem::label()
{
    return m_label;
}

void MenuItem::setLabel(const QString &value)
{
    if (m_label == value)
        return;
    m_label = value;
    g_menu_item_set_label(m_gmenuitem.get(), m_label.toUtf8().constData());
    Q_EMIT changed();
}

void MenuItem::setIcon(const QString &icon)
{
    if (m_icon == icon)
    {
        return;
    }
    m_icon = icon;

    GError *error = nullptr;
    auto gicon = shared_ptr<GIcon>(g_icon_new_for_string(m_icon.toUtf8().constData(), &error), GObjectDeleter());
    if (error)
    {
        qWarning() << error->message;
        g_error_free(error);
        return;
    }

    g_menu_item_set_icon(m_gmenuitem.get(), gicon.get());
    Q_EMIT changed();
}

void MenuItem::setAction(const QString &value)
{
    /// @todo validate that action is valid
    if (m_action == value)
        return;
    m_action = value;
    g_menu_item_set_detailed_action(m_gmenuitem.get(), m_action.toUtf8().constData());
    Q_EMIT changed();
}

void MenuItem::setActionAndTargetValue(const QString &action, const Variant& target)
{
    if (m_action == action)
    {
        return;
    }

    m_action = action;
    g_menu_item_set_action_and_target_value(m_gmenuitem.get(), m_action.toUtf8().constData(), target);
    Q_EMIT changed();
}

void MenuItem::setAttribute(const QString &attribute,
                  Variant value)
{
    /// @todo validate that attribute is valid.
    assert(value);
    assert(!attribute.isEmpty());

    auto iter = m_attributes.find(attribute);
    if (iter != m_attributes.end()) {
        if (iter->second == value)
            return;
        iter->second = value;
    } else {
        m_attributes[attribute] = value;
    }
    g_menu_item_set_attribute_value(m_gmenuitem.get(), attribute.toUtf8().constData(), value);
    Q_EMIT changed();
}

void MenuItem::clearAttribute(const QString &attribute)
{
    assert(!attribute.isEmpty());
    m_attributes.erase(attribute);
    g_menu_item_set_attribute(m_gmenuitem.get(), attribute.toUtf8().constData(), nullptr);
    Q_EMIT changed();
}

GMenuItem *
MenuItem::gmenuitem()
{
    return m_gmenuitem.get();
}

const QString &
MenuItem::action () const
{
    return m_action;
}
