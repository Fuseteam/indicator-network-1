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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <menuitems/ethernet-connection-item.h>
#include <QDebug>

using namespace std;

using namespace nmofono::connection;

class EthernetConnectionItem::Private : public QObject
{
Q_OBJECT

public:
    Private(EthernetConnectionItem& parent) :
            p(parent)
    {
    }

public Q_SLOTS:
    void
    connectionIdChanged(const QString& connectionId)
    {
        m_item->setLabel(connectionId);
    }

public:
    EthernetConnectionItem& p;

    AvailableConnection::SPtr m_connection;

    MenuItem::Ptr m_item;
};

EthernetConnectionItem::EthernetConnectionItem(
        AvailableConnection::SPtr connection, Action::Ptr action) :
        d(new Private(*this))
{
    d->m_connection = connection;
    connect(d->m_connection.get(), &AvailableConnection::connectionIdChanged,
            d.get(), &Private::connectionIdChanged);

    d->m_item = make_shared<MenuItem>(d->m_connection->connectionId());
    d->m_item->setActionAndTargetValue(
            "indicator." + action->name(),
            TypedVariant<string>(connection->connectionUuid().toStdString()));
}

MenuItem::Ptr
EthernetConnectionItem::menuItem()
{
    return d->m_item;
}

#include "ethernet-connection-item.moc"

