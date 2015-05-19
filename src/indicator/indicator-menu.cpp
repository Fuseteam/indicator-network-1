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

#include <indicator-menu.h>

#include <menumodel-cpp/menu-merger.h>
#include <menumodel-cpp/action-group-merger.h>

using namespace std;

struct IndicatorMenu::Private: public QObject
{
    Q_OBJECT

public:
    Private() = default;

    RootState::Ptr m_rootState;
    string m_prefix;

    Action::Ptr m_rootAction;
    MenuItem::Ptr m_rootItem;

    Menu::Ptr m_rootMenu;
    MenuMerger::Ptr m_subMenuMerger;

    ActionGroupMerger::Ptr m_actionGroupMerger;
    ActionGroup::Ptr m_actionGroup;

    vector<Section::Ptr> m_sections;

public Q_SLOTS:
    void setState(const Variant &state)
    {
        m_rootAction->setState(state);
    }
};

IndicatorMenu::IndicatorMenu(RootState::Ptr rootState, const string &prefix)
    : d(new Private)
{
    d->m_rootState = rootState;
    d->m_prefix = prefix;
    d->m_actionGroupMerger = make_shared<ActionGroupMerger>();
    d->m_actionGroup = make_shared<ActionGroup>();

    d->m_actionGroupMerger->add(d->m_actionGroup);

    d->m_rootAction = make_shared<Action>(prefix + ".network-status",
                                            nullptr,
                                            rootState->state());
    QObject::connect(d->m_rootState.get(), &RootState::stateUpdated, d.get(), &Private::setState);
    d->m_actionGroup->add(d->m_rootAction);

    d->m_rootMenu = make_shared<Menu>();
    d->m_subMenuMerger = make_shared<MenuMerger>();

    d->m_rootItem = MenuItem::newSubmenu(d->m_subMenuMerger);

    d->m_rootItem->setAction("indicator." + prefix + ".network-status");
    d->m_rootItem->setAttribute("x-canonical-type", TypedVariant<string>("com.canonical.indicator.root"));
    d->m_rootMenu->append(d->m_rootItem);
}

void
IndicatorMenu::addSection(Section::Ptr section)
{
    d->m_sections.push_back(section);
    d->m_actionGroupMerger->add(section->actionGroup());
    d->m_subMenuMerger->append(section->menuModel());
}

Menu::Ptr
IndicatorMenu::menu() const
{
    return d->m_rootMenu;
}

ActionGroup::Ptr
IndicatorMenu::actionGroup() const
{
    return d->m_actionGroupMerger->actionGroup();
}

#include "indicator-menu.moc"
