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
 *     Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <nmofono/kill-switch.h>
#include <backend-utils.h>
#include <dbus-types.h>

#include <URfkillInterface.h>
#include <URfkillKillswitchInterface.h>

using namespace std;

namespace nmofono
{

class KillSwitch::Private: public QObject
{
    Q_OBJECT

public:
    enum class DeviceType
    {
        wlan = 1,
        bluetooth = 2,
        uwb = 3,
        wimax = 4,
        wwan = 5,
        gps = 6,
        fm = 7,
        nfc = 8
    };

    KillSwitch& p;

    std::shared_ptr<OrgFreedesktopURfkillInterface> urfkill;
    std::shared_ptr<OrgFreedesktopURfkillKillswitchInterface> killSwitch;

    bool m_flightMode = false;
    State m_state = State::not_available;

    Private(KillSwitch& parent,
            std::shared_ptr<OrgFreedesktopURfkillInterface> urfkill,
            std::shared_ptr<OrgFreedesktopURfkillKillswitchInterface> killSwitch)
        : p(parent),
          urfkill(urfkill),
          killSwitch(killSwitch) {}

public Q_SLOTS:
    void setFlightMode(bool flightMode)
    {
        if (flightMode == m_flightMode)
        {
            return;
        }

        m_flightMode = flightMode;
        Q_EMIT p.flightModeChanged(flightMode);
    }

    void stateChanged()
    {
        int stateIndex = killSwitch->state();
        if (stateIndex >= static_cast<int>(State::first_) &&
            stateIndex <= static_cast<int>(State::last_))
        {
            m_state = static_cast<KillSwitch::State>(stateIndex);
        }
        else
        {
            m_state = KillSwitch::State::not_available;
        }

        Q_EMIT p.stateChanged(m_state);
    }
};


KillSwitch::KillSwitch(const QDBusConnection& systemBus)
{
    auto urfkill = std::make_shared<OrgFreedesktopURfkillInterface>(DBusTypes::URFKILL_BUS_NAME,
                                                                    DBusTypes::URFKILL_OBJ_PATH,
                                                                    systemBus);

    auto killSwitch = std::make_shared<OrgFreedesktopURfkillKillswitchInterface>(DBusTypes::URFKILL_BUS_NAME,
                                                                                 DBusTypes::URFKILL_WIFI_OBJ_PATH,
                                                                                 systemBus);

    d = make_unique<Private>(*this, urfkill, killSwitch);
    auto reply = urfkill->IsFlightMode();
    reply.waitForFinished();
    d->setFlightMode(reply.isValid() ? reply.value() : false);
    d->stateChanged();

    connect(urfkill.get(), &OrgFreedesktopURfkillInterface::FlightModeChanged, d.get(), &Private::setFlightMode);
    connect(killSwitch.get(), &OrgFreedesktopURfkillKillswitchInterface::StateChanged, d.get(), &Private::stateChanged);
}

KillSwitch::~KillSwitch()
{}

void
KillSwitch::setBlock(bool block)
{
    if (!block && state() == State::hard_blocked)
    {
        qCritical() << "Killswitch is hard blocked.";
        return;
    }

    if (!block && state() != State::soft_blocked)
    {
        return;
    }

    if (block && state() != State::unblocked)
    {
        return;
    }

    try
    {
        if (!utils::getOrThrow(d->urfkill->Block(static_cast<uint>(Private::DeviceType::wlan), block)))
        {
            throw std::runtime_error("Failed to block killswitch");
        }
    }
    catch (std::exception &e)
    {
        qCritical() << e.what();
    }
}

KillSwitch::State KillSwitch::state() const
{
    return d->m_state;
}

bool KillSwitch::flightMode(bool enable)
{
    if (enable == d->m_flightMode)
    {
        return true;
    }

    try
    {
        return utils::getOrThrow(d->urfkill->FlightMode(enable));
    }
    catch (std::runtime_error& e)
    {
        qWarning() << e.what();
        return false;
    }
}

bool KillSwitch::isFlightMode()
{
    return d->m_flightMode;
}

}

#include "kill-switch.moc"
