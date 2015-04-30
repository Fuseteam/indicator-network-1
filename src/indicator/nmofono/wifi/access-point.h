/*
 * Copyright © 2013 Canonical Ltd.
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
 */

#ifndef CONNECTIVITY_NETWORKING_WIFI_ACCESS_POINT
#define CONNECTIVITY_NETWORKING_WIFI_ACCESS_POINT

#include <QObject>
#include <QString>

#include <memory>

namespace connectivity {
namespace networking {
namespace wifi {

#ifndef CONNECTIVITY_CPP_EXPORT
#define CONNECTIVITY_CPP_EXPORT __attribute ((visibility ("default")))
#endif

/// @private
class CONNECTIVITY_CPP_EXPORT
AccessPoint: public QObject
{
    Q_OBJECT

public:
    typedef std::shared_ptr<AccessPoint> Ptr;
    AccessPoint();
    virtual ~AccessPoint();

    /* from 0.00 to 100.00,
     *  -1 not available
     */
    Q_PROPERTY(double strength READ strength NOTIFY strengthUpdated)
    virtual double strength() const = 0;

    virtual QString ssid()     const = 0;
    virtual bool secured()            const = 0;
    virtual bool adhoc()              const = 0;

Q_SIGNALS:
    void strengthUpdated(double strength);
};

}
}
}

#endif

