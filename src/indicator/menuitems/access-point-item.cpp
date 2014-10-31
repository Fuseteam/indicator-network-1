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

#include <functional>
#include <vector>

class AccessPointItem::Private : public std::enable_shared_from_this<Private>
{
public:
    AccessPointItem *q;

    networking::wifi::AccessPoint::Ptr m_accessPoint;
    bool m_isActive;

    core::Signal<void> m_activated;

    std::vector<core::Connection> m_connections;

    Action::Ptr m_actionActivate;
    Action::Ptr m_actionStrength;
    MenuItem::Ptr m_item;


    Private() = delete;
    explicit Private(AccessPointItem *parent, networking::wifi::AccessPoint::Ptr accessPoint, bool isActive = false)
        : q{parent},
          m_accessPoint{accessPoint},
          m_isActive{isActive}
    {}

    void ConstructL()
    {
        static int id = 0;
        ++id; /// @todo guard me.

        std::string actionId = "accesspoint." + std::to_string(id);
        std::string strengthActionId = actionId + "::strength";

        m_item = std::make_shared<MenuItem>(m_accessPoint->ssid(),
                                            "indicator." + actionId);



        m_item->setAttribute("x-canonical-type", TypedVariant<std::string>("unity.widgets.systemsettings.tablet.accesspoint"));
        m_item->setAttribute("x-canonical-wifi-ap-is-adhoc", TypedVariant<bool>(m_accessPoint->adhoc()));
        m_item->setAttribute("x-canonical-wifi-ap-is-secure", TypedVariant<bool>(m_accessPoint->secured()));
        m_item->setAttribute("x-canonical-wifi-ap-strength-action", TypedVariant<std::string>("indicator." + strengthActionId));

        m_actionStrength = std::make_shared<Action>(strengthActionId,
                                                    nullptr,
                                                    TypedVariant<std::uint8_t>(m_accessPoint->strength().get()));

        auto weak = std::weak_ptr<Private>(shared_from_this());
        auto con = m_accessPoint->strength().changed().connect([weak](double value)
        {
            auto that = weak.lock();
            if (!that)
                return;
            GMainLoopDispatch([that, value]()
            {
               that->setStrength(value);
            });
        });
        m_connections.push_back(con);

        m_actionActivate = std::make_shared<Action>(actionId,
                                                    nullptr,
                                                    TypedVariant<bool>(m_isActive),
                                                    [this](Variant){
                ///@ todo something weird is happening as the unity8 side is not changing the state..
        });
        m_actionActivate->activated().connect([this](Variant){
            m_activated();
        });

        q->actionGroup()->add(m_actionActivate);
        q->actionGroup()->add(m_actionStrength);
    }

    virtual ~Private()
    {
        for (auto con : m_connections)
            con.disconnect();
    }

    void setStrength(double value)
    {
        /// @todo narrow_cast<>;
        m_actionStrength->setState(TypedVariant<std::uint8_t>(value));
    }
};

AccessPointItem::AccessPointItem(networking::wifi::AccessPoint::Ptr accessPoint, bool isActive)
    : d{new Private(this, accessPoint, isActive)}
{
    d->ConstructL();
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

core::Signal<void> &
AccessPointItem::activated()
{
    return d->m_activated;
}
