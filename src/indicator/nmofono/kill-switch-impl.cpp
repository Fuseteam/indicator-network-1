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

#include <nmofono/kill-switch-impl.h>
#include <backend-utils.h>

#include <URfkillInterface.h>
#include <URfkillKillswitchInterface.h>

using namespace backend;
using namespace platform::nmofono;

static QString const cBusName = "org.freedesktop.URfkill";
static QString const cURfkillPath = "/org/freedesktop/URfkill";
static QString const cURfkillKillswitchPath = "/org/freedesktop/URfkill/WLAN";

class KillSwitch::Private
{
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

    std::shared_ptr<OrgFreedesktopURfkillInterface> urfkill;
    std::shared_ptr<OrgFreedesktopURfkillKillswitchInterface> killSwitch;

    bool m_flightMode = false;

    Private(std::shared_ptr<OrgFreedesktopURfkillInterface> urfkill,
            std::shared_ptr<OrgFreedesktopURfkillKillswitchInterface> killSwitch)
        : urfkill(urfkill),
          killSwitch(killSwitch) {}
};


KillSwitch::KillSwitch(const QDBusConnection& systemBus)
{
    auto urfkill = std::make_shared<OrgFreedesktopURfkillInterface>(cBusName,
                                                                    cURfkillPath,
                                                                    systemBus);

    auto killSwitch = std::make_shared<OrgFreedesktopURfkillKillswitchInterface>(cBusName,
                                                                                 cURfkillKillswitchPath,
                                                                                 systemBus);

    connect(urfkill.get(), SIGNAL(FlightModeChanged(bool)), this, SLOT(setFlightMode(bool)));
    connect(killSwitch.get(), SIGNAL(StateChanged()), this, SIGNAL(stateChanged()));

    d.reset(new Private(urfkill, killSwitch));

    setFlightMode(urfkill->IsFlightMode());
}

KillSwitch::~KillSwitch()
{}

void KillSwitch::setFlightMode(bool flightMode)
{
    if (flightMode == d->m_flightMode)
    {
        return;
    }

    d->m_flightMode = flightMode;
    Q_EMIT flightModeChanged(flightMode);
}

void
KillSwitch::block()
{
    if (state() == KillSwitch::State::hard_blocked)
        throw KillSwitch::exception::HardBlocked();

    try {
        if (!utils::getOrThrow(d->urfkill->Block(static_cast<uint>(Private::DeviceType::wlan), true)))
            throw KillSwitch::exception::Failed("Failed to block killswitch");
    } catch (std::exception &e) {
        throw KillSwitch::exception::Failed(e.what());
    }
}

void
KillSwitch::unblock()
{
    try {
        if (!utils::getOrThrow(d->urfkill->Block(static_cast<uint>(Private::DeviceType::wlan), false)))
            throw KillSwitch::exception::Failed("Failed to unblock killswitch");
    } catch (std::exception &e) {
        throw KillSwitch::exception::Failed(e.what());
    }
}

KillSwitch::State KillSwitch::state() const
{
    int stateIndex = d->killSwitch->state();
    if (stateIndex >= static_cast<int>(State::first_) &&
        stateIndex <= static_cast<int>(State::last_))
    {
        return static_cast<KillSwitch::State>(stateIndex);
    }
    return KillSwitch::State::not_available;
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
        qWarning() << __PRETTY_FUNCTION__ << ": " << QString::fromStdString(e.what());
        return false;
    }
}

bool KillSwitch::isFlightMode()
{
    return d->m_flightMode;
}
