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

#include <menumodel-cpp/menu.h>

#include <QDebug>

class WwanLinkItem::Private : public QObject
{
    Q_OBJECT
public:

    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    Modem::Ptr m_modem;
    ModemManager::Ptr m_modemManager;

    ModemInfoItem::Ptr m_infoItem;

    bool m_showIdentifier = false;

    Private() = delete;
    Private(Modem::Ptr modem, ModemManager::Ptr modemManager);

    void unlockSim();

public Q_SLOTS:
    void update();

    void unlockModem()
    {
        m_modemManager->unlockModem(m_modem);
    }
};

WwanLinkItem::Private::Private(Modem::Ptr modem, ModemManager::Ptr modemManager)
    : m_modem{modem},
      m_modemManager{modemManager}
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    m_infoItem = std::make_shared<ModemInfoItem>();
    connect(m_infoItem.get(), &ModemInfoItem::unlock, this, &Private::unlockModem);

    m_actionGroupMerger->add(*m_infoItem);
    m_menu->append(*m_infoItem);

    connect(m_modem.get(), &Modem::updated, this, &Private::update);
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
    case Modem::SimStatus::missing:
        m_infoItem->setStatusIcon("no-simcard");
        m_infoItem->setStatusText(_("No SIM"));
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);
        break;
    case Modem::SimStatus::error:
        m_infoItem->setStatusIcon("simcard-error");
        m_infoItem->setStatusText(_("SIM Error"));
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);
        break;
    case Modem::SimStatus::locked:
    case Modem::SimStatus::permanentlyLocked:
        m_infoItem->setStatusIcon("simcard-locked");
        m_infoItem->setStatusText(_("SIM Locked"));
        m_infoItem->setLocked(true);
        m_infoItem->setRoaming(false);
        break;
    case Modem::SimStatus::ready:
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);

        if (m_modem->online()) {
            switch (m_modem->status()) {
            case Modem::Status::unregistered:
                m_infoItem->setStatusIcon("gsm-3g-disabled");
                m_infoItem->setStatusText(_("Unregistered"));
                break;
            case Modem::Status::unknown:
                m_infoItem->setStatusIcon("gsm-3g-disabled");
                m_infoItem->setStatusText(_("Unknown"));
                break;
            case Modem::Status::denied:
                m_infoItem->setStatusIcon("gsm-3g-disabled");
                m_infoItem->setStatusText(_("Denied"));
                break;
            case Modem::Status::searching:
                m_infoItem->setStatusIcon("gsm-3g-disabled");
                m_infoItem->setStatusText(_("Searching"));
                break;
            case Modem::Status::roaming:
                m_infoItem->setRoaming(true);
                /* fallthrough */
            case Modem::Status::registered:
                if (m_modem->strength() != 0) {
                    m_infoItem->setStatusIcon(Modem::strengthIcon(m_modem->strength()));
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
    case Modem::SimStatus::not_available:
        m_infoItem->setStatusIcon("gsm-3g-disabled");
        m_infoItem->setStatusText(_("Offline"));
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);
        break;
    }
}

WwanLinkItem::WwanLinkItem(Modem::Ptr modem, ModemManager::Ptr modemManager)
    : d{new Private(modem, modemManager)}

{
}

WwanLinkItem::~WwanLinkItem()
{}

ActionGroup::Ptr
WwanLinkItem::actionGroup()
{
    return *d->m_actionGroupMerger;
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
