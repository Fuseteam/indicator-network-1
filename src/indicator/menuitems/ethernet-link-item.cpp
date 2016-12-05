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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <menuitems/ethernet-item.h>
#include <util/localisation.h>
#include <icons.h>
#include <menuitems/ethernet-link-item.h>

#include <QDebug>

using namespace std;
using namespace nmofono;
using namespace nmofono::ethernet;

class EthernetLinkItem::Private: public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void
    statusUpdated(Link::Status status)
    {
        const static QMap<Link::Status, QString> statusMap{
            {Link::Status::connected, _("Connected")},
            {Link::Status::connecting, _("Connecting")},
            {Link::Status::disabled, _("Disabled")},
            {Link::Status::offline, _("Disconnected")},
            {Link::Status::online, _("Online")},
            {Link::Status::failed, _("Failed")},
        };

        m_item->setStatusText(statusMap[status]);
    }

    void
    updateIdentifierText()
    {
        if (m_showInterface)
        {
            m_item->setName(
                    QString(_("Ethernet (%1)")).arg(m_ethernetLink->name()));
        }
        else
        {
            m_item->setName(QString(_("Ethernet")));
        }
    }

public:
    EthernetLink::SPtr m_ethernetLink;

    EthernetItem::Ptr m_item;

    bool m_showInterface = true;
};

EthernetLinkItem::EthernetLinkItem(EthernetLink::SPtr ethernetLink) :
        d(new Private)
{
    d->m_ethernetLink = ethernetLink;

    d->m_item = make_shared<EthernetItem>(ethernetLink->id());

    m_actionGroupMerger->add(d->m_item->actionGroup());

    connect(d->m_ethernetLink.get(), &Link::statusUpdated, d.get(),
            &Private::statusUpdated);
    d->statusUpdated(d->m_ethernetLink->status());

    connect(d->m_ethernetLink.get(), &Link::nameUpdated, d.get(),
            &Private::updateIdentifierText);
    d->updateIdentifierText();

    connect(d->m_ethernetLink.get(), &EthernetLink::autoConnectChanged,
            d->m_item.get(), &EthernetItem::setAutoConnect);
    connect(d->m_item.get(), &EthernetItem::autoConnectChanged,
            d->m_ethernetLink.get(), &EthernetLink::setAutoConnect);
    d->m_item->setAutoConnect(d->m_ethernetLink->autoConnect());
}

MenuItem::Ptr
EthernetLinkItem::menuItem()
{
    return d->m_item->menuItem();
}

void
EthernetLinkItem::setShowInterface(bool showInterface)
{
    d->m_showInterface = showInterface;
    d->updateIdentifierText();
}

#include "ethernet-link-item.moc"
