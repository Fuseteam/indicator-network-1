/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include "ethernet-item.h"
#include <QDebug>

using namespace std;

class EthernetItem::Private : public QObject
{
Q_OBJECT

public Q_SLOTS:
    void
    actionActivated(const Variant&)
    {
        // Remember that the GAction was only activated, and hasn't had its state changed.
        bool newAutoConnectState = !(m_actionConnected->state().as<bool>());
        Q_EMIT p.autoConnectChanged(newAutoConnectState);
    }

public:
    Private(EthernetItem& parent) :
            p(parent)
    {
    }

    EthernetItem& p;

    Action::Ptr m_actionConnected;
    Action::Ptr m_actionStatusLabel;

    MenuItem::Ptr m_item;
};

EthernetItem::EthernetItem(unsigned int id)
{
    d.reset(new Private(*this));

    QString actionIdBase = "ethernet." + QString::number(id);

    QString statusConnectedActionId = actionIdBase;
    QString statusLabelActionId = actionIdBase + "::status-label";

    d->m_actionConnected = make_shared<Action>(statusConnectedActionId, nullptr, TypedVariant<bool>(false));
    d->m_actionStatusLabel = make_shared<Action>(statusLabelActionId, nullptr, TypedVariant<string>());
    m_actionGroup->add(d->m_actionConnected);
    m_actionGroup->add(d->m_actionStatusLabel);

    connect(d->m_actionConnected.get(), &Action::activated, d.get(), &Private::actionActivated);

    d->m_item = make_shared<MenuItem>();
    d->m_item->setAction("indicator." + statusConnectedActionId);

    d->m_item->setAttribute("x-canonical-type", TypedVariant<string>("com.canonical.indicator.switch"));
    d->m_item->setAttribute("x-canonical-subtitle-action", TypedVariant<string>("indicator." + statusLabelActionId.toStdString()));
}

EthernetItem::~EthernetItem()
{
}

void
EthernetItem::setAutoConnect(bool autoConnect)
{
    d->m_actionConnected->setState(TypedVariant<bool>(autoConnect));
}

void
EthernetItem::setStatusText(const QString &value)
{
    d->m_actionStatusLabel->setState(TypedVariant<string>(value.toStdString()));
}

void
EthernetItem::setName(const QString &value)
{
    d->m_item->setLabel(value);
}

MenuItem::Ptr
EthernetItem::menuItem()
{
    return d->m_item;
}

#include "ethernet-item.moc"
