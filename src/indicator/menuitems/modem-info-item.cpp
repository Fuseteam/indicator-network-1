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

#include "modem-info-item.h"

class ModemInfoItem::Private
{
public:
    Action::Ptr m_actionStatusLabel;
    Action::Ptr m_actionStatusIcon;
    Action::Ptr m_actionConnectivityIcon;
    Action::Ptr m_actionSimIdentifier;
    Action::Ptr m_actionRoaming;
    Action::Ptr m_actionLocked;

    MenuItem::Ptr m_item;
};

ModemInfoItem::ModemInfoItem()
{
    d.reset(new Private);

    static int id = 0;
    ++id; /// @todo guard me.

    std::string actionIdBase = "modem." + std::to_string(id);

    std::string statusLabelActionId = actionIdBase + "::status-label";
    std::string statusIconActionId = actionIdBase + "::status-icon";
    std::string connectivityIconActionId = actionIdBase + "::connectivity-icon";
    std::string simIdentifierActionId = actionIdBase + "::sim-identifier-label";
    std::string roamingActionId = actionIdBase + "::roaming";
    std::string lockedActionId = actionIdBase + "::locked";

    d->m_item = std::make_shared<MenuItem>();

    d->m_item->setAttribute("x-canonical-type", TypedVariant<std::string>("com.canonical.indicator.network.modeminfoitem"));
    d->m_item->setAttribute("x-canonical-modem-status-label-action", TypedVariant<std::string>("indicator." + statusLabelActionId));
    d->m_item->setAttribute("x-canonical-modem-status-icon-action", TypedVariant<std::string>("indicator." + statusIconActionId));
    d->m_item->setAttribute("x-canonical-modem-connectivity-icon-action", TypedVariant<std::string>("indicator." +  connectivityIconActionId));
    d->m_item->setAttribute("x-canonical-modem-sim-identifier-label-action", TypedVariant<std::string>("indicator." +  simIdentifierActionId));
    d->m_item->setAttribute("x-canonical-modem-roaming-action", TypedVariant<std::string>("indicator." +  roamingActionId));
    d->m_item->setAttribute("x-canonical-modem-locked-action", TypedVariant<std::string>("indicator." +  lockedActionId));



    d->m_actionStatusLabel = std::make_shared<Action>(statusLabelActionId,
                                                      nullptr,
                                                      TypedVariant<std::string>());
    d->m_actionStatusIcon = std::make_shared<Action>(statusIconActionId,
                                                     nullptr,
                                                     TypedVariant<std::string>());
    d->m_actionConnectivityIcon = std::make_shared<Action>(connectivityIconActionId,
                                                           nullptr,
                                                           TypedVariant<std::string>());
    d->m_actionSimIdentifier = std::make_shared<Action>(simIdentifierActionId,
                                                        nullptr,
                                                        TypedVariant<std::string>());
    d->m_actionRoaming = std::make_shared<Action>(roamingActionId,
                                                  nullptr,
                                                  TypedVariant<bool>(false));
    d->m_actionLocked = std::make_shared<Action>(lockedActionId,
                                                 nullptr,
                                                 TypedVariant<bool>(false));
    m_actionGroup->add(d->m_actionStatusLabel);
    m_actionGroup->add(d->m_actionStatusIcon);
    m_actionGroup->add(d->m_actionConnectivityIcon);
    m_actionGroup->add(d->m_actionSimIdentifier);
    m_actionGroup->add(d->m_actionRoaming);
    m_actionGroup->add(d->m_actionLocked);

    connect(d->m_actionLocked.get(), &Action::activated, this, &ModemInfoItem::unlock);
}

ModemInfoItem::~ModemInfoItem()
{

}

void
ModemInfoItem::setStatusIcon(const QString &name)
{
    d->m_actionStatusIcon->setState(TypedVariant<std::string>(name.toStdString()));
}

void
ModemInfoItem::setStatusText(const QString &value)
{
    d->m_actionStatusLabel->setState(TypedVariant<std::string>(value.toStdString()));
}

void
ModemInfoItem::setConnectivityIcon(const QString &name)
{
    d->m_actionConnectivityIcon->setState(TypedVariant<std::string>(name.toStdString()));
}

void
ModemInfoItem::setSimIdentifierText(const QString &value)

{
    d->m_actionSimIdentifier->setState(TypedVariant<std::string>(value.toStdString()));
}

void
ModemInfoItem::setLocked(bool value)
{
    d->m_actionLocked->setState(TypedVariant<bool>(value));
}

void
ModemInfoItem::setRoaming(bool value)
{
    d->m_actionRoaming->setState(TypedVariant<bool>(value));
}

MenuItem::Ptr
ModemInfoItem::menuItem()
{
    return d->m_item;
}

