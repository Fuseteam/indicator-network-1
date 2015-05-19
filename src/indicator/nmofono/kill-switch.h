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

#pragma once

#include <exception>
#include <memory>

#include <QDBusConnection>

namespace nmofono {

class KillSwitch : public QObject
{
    Q_OBJECT

    class Private;
    std::unique_ptr<Private> d;

public:
    typedef std::shared_ptr<KillSwitch> Ptr;

    struct exception
    {
        struct HardBlocked : public std::runtime_error
        {
            HardBlocked()
                : std::runtime_error("Killswitch is hard blocked.")
            {}
        };

        struct Failed : public std::runtime_error
        {
            Failed() = delete;
            Failed(std::string what)
                : std::runtime_error(what)
            {}
        };
    };

    enum class State
    {
        not_available = -1,
        first_ = not_available,
        unblocked = 0,
        soft_blocked = 1,
        hard_blocked = 2,
        last_ = hard_blocked
    };

    KillSwitch(const QDBusConnection& systemBus);
    ~KillSwitch();

    /// @throws exception::Failed if the switch fails to block
    void block();

    /// @throws exception::HardBlocked if trying to unblock when switch is hard blocked
    /// @throws exception::Failed if the switch fails to unblock
    void unblock();

    State state() const;
    bool flightMode(bool enable);
    bool isFlightMode();

Q_SIGNALS:
    void stateChanged();
    void flightModeChanged(bool);

private Q_SLOTS:
    void setFlightMode(bool);
};

}
