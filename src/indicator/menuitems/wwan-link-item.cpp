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

#include <menuitems/wwan-link-item.h>
#include <menuitems/modem-info-item.h>
#include <icons.h>
#include <util/localisation.h>

#include <menumodel-cpp/menu.h>

#include <QDebug>

using namespace std;
using namespace nmofono;

class WwanLinkItem::Private : public QObject
{
    Q_OBJECT
public:

    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    wwan::Modem::Ptr m_modem;
    Manager::Ptr m_modemManager;

    ModemInfoItem::Ptr m_infoItem;

    bool m_showIdentifier = false;

    Private() = delete;
    Private(wwan::Modem::Ptr modem, Manager::Ptr manager);

    void unlockSim();

public Q_SLOTS:
    void update();

    void unlockModem()
    {
        m_modemManager->unlockModem(m_modem);
    }
};

WwanLinkItem::Private::Private(wwan::Modem::Ptr modem, Manager::Ptr manager)
    : m_modem{modem},
      m_modemManager{manager}
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    m_infoItem = std::make_shared<ModemInfoItem>();
    connect(m_infoItem.get(), &ModemInfoItem::unlock, this, &Private::unlockModem);

    m_actionGroupMerger->add(m_infoItem->actionGroup());
    m_menu->append(m_infoItem->menuItem());

    connect(m_modem.get(), &wwan::Modem::updated, this, &Private::update);
    update();
}

void
WwanLinkItem::Private::update()
{
    if (m_showIdentifier) {
        m_infoItem->setSimIdentifierText(m_modem->simIdentifier());
    } else {
        m_infoItem->setSimIdentifierText("");
    }
    /// @todo implement me.
    m_infoItem->setConnectivityIcon("");

    switch(m_modem->simStatus()) {
    case wwan::Modem::SimStatus::missing:
        m_infoItem->setStatusIcon("no-simcard");
        m_infoItem->setStatusText(_("No SIM"));
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);
        break;
    case wwan::Modem::SimStatus::error:
        m_infoItem->setStatusIcon("simcard-error");
        m_infoItem->setStatusText(_("SIM Error"));
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);
        break;
    case wwan::Modem::SimStatus::locked:
    case wwan::Modem::SimStatus::permanentlyLocked:
        m_infoItem->setStatusIcon("simcard-locked");
        m_infoItem->setStatusText(_("SIM Locked"));
        m_infoItem->setLocked(true);
        m_infoItem->setRoaming(false);
        break;
    case wwan::Modem::SimStatus::ready:
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);

        if (m_modem->online()) {
            switch (m_modem->modemStatus()) {
            case wwan::Modem::ModemStatus::unregistered:
                m_infoItem->setStatusIcon("gsm-3g-disabled");
                m_infoItem->setStatusText(_("Unregistered"));
                break;
            case wwan::Modem::ModemStatus::unknown:
                m_infoItem->setStatusIcon("gsm-3g-disabled");
                m_infoItem->setStatusText(_("Unknown"));
                break;
            case wwan::Modem::ModemStatus::denied:
                m_infoItem->setStatusIcon("gsm-3g-disabled");
                m_infoItem->setStatusText(_("Denied"));
                break;
            case wwan::Modem::ModemStatus::searching:
                m_infoItem->setStatusIcon("gsm-3g-disabled");
                m_infoItem->setStatusText(_("Searching"));
                break;
            case wwan::Modem::ModemStatus::roaming:
                m_infoItem->setRoaming(true);
                /* fallthrough */
            case wwan::Modem::ModemStatus::registered:
                if (m_modem->strength() != 0) {
                    m_infoItem->setStatusIcon(Icons::strengthIcon(m_modem->strength()));
                    m_infoItem->setStatusText(m_modem->operatorName());
                } else {
                    m_infoItem->setStatusIcon("gsm-3g-no-service");
                    m_infoItem->setStatusText(_("No Signal"));
                }
                break;
            }
        } else {
            m_infoItem->setStatusIcon("gsm-3g-disabled");
            m_infoItem->setStatusText(_("Offline"));
        }

        break;
    case wwan::Modem::SimStatus::not_available:
        m_infoItem->setStatusIcon("gsm-3g-disabled");
        m_infoItem->setStatusText(_("Offline"));
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);
        break;
    }
}

WwanLinkItem::WwanLinkItem(wwan::Modem::Ptr modem, Manager::Ptr manager)
    : d{new Private(modem, manager)}

{
}

WwanLinkItem::~WwanLinkItem()
{}

ActionGroup::Ptr
WwanLinkItem::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuModel::Ptr
WwanLinkItem::menuModel()
{
    return d->m_menu;
}

void
WwanLinkItem::showSimIdentifier(bool value)
{
    d->m_showIdentifier = value;
    d->update();
}

#include "wwan-link-item.moc"
