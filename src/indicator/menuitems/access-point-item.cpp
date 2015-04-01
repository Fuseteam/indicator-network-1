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

#include "access-point-item.h"

#include "menumodel-cpp/action.h"
#include "menumodel-cpp/menu-item.h"
#include "menumodel-cpp/gio-helpers/variant.h"

namespace networking = connectivity::networking;

#include <vector>

class AccessPointItem::Private : public QObject
{
    Q_OBJECT

public:
    AccessPointItem& q;

    networking::wifi::AccessPoint::Ptr m_accessPoint;
    bool m_isActive;

    Action::Ptr m_actionActivate;
    Action::Ptr m_actionStrength;
    MenuItem::Ptr m_item;

    Private() = delete;
    explicit Private(AccessPointItem& parent, networking::wifi::AccessPoint::Ptr accessPoint, bool isActive = false)
        : q{parent},
          m_accessPoint{accessPoint},
          m_isActive{isActive}
    {
        static int id = 0;
        ++id; /// @todo guard me.

        std::string actionId = "accesspoint." + std::to_string(id);
        std::string strengthActionId = actionId + "::strength";

        m_item = std::make_shared<MenuItem>(m_accessPoint->ssid().toStdString(),
                                            "indicator." + actionId);



        m_item->setAttribute("x-canonical-type", TypedVariant<std::string>("unity.widgets.systemsettings.tablet.accesspoint"));
        m_item->setAttribute("x-canonical-wifi-ap-is-adhoc", TypedVariant<bool>(m_accessPoint->adhoc()));
        m_item->setAttribute("x-canonical-wifi-ap-is-secure", TypedVariant<bool>(m_accessPoint->secured()));
        m_item->setAttribute("x-canonical-wifi-ap-strength-action", TypedVariant<std::string>("indicator." + strengthActionId));

        m_actionStrength = std::make_shared<Action>(strengthActionId,
                                                    nullptr,
                                                    TypedVariant<std::uint8_t>(m_accessPoint->strength()));

        connect(m_accessPoint.get(), &networking::wifi::AccessPoint::strengthUpdated, this, &Private::setStrength);

        m_actionActivate = std::make_shared<Action>(actionId,
                                                    nullptr,
                                                    TypedVariant<bool>(m_isActive));
        connect(m_actionActivate.get(), &Action::activated, &q, &AccessPointItem::activated);

        q.actionGroup()->add(m_actionActivate);
        q.actionGroup()->add(m_actionStrength);
    }

    virtual ~Private()
    {
    }

public Q_SLOTS:
    void setStrength(double value)
    {
        /// @todo narrow_cast<>;
        m_actionStrength->setState(TypedVariant<std::uint8_t>(value));
    }
};

AccessPointItem::AccessPointItem(networking::wifi::AccessPoint::Ptr accessPoint, bool isActive)
    : d{new Private(*this, accessPoint, isActive)}
{
}

AccessPointItem::~AccessPointItem()
{}

void
AccessPointItem::setActive(bool value)
{
    d->m_isActive = value;
    d->m_actionActivate->setState(TypedVariant<bool>(d->m_isActive));
}

MenuItem::Ptr
AccessPointItem::menuItem()
{
    return d->m_item;
}

#include "access-point-item.moc"
