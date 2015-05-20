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
 */

#ifndef CONNECTIVITY_NETWORKING_STATUS_H
#define CONNECTIVITY_NETWORKING_STATUS_H

#include <QObject>
#include <QScopedPointer>
#include <QVector>

namespace ubuntu {
namespace connectivity {

/**
 * @brief Overall system networking status.
 *
 * This is the top-level class for accessing networking information.
 *
 * * For system networking status, see NetworkingStatus::status.
 * * For connection limitations, see NetworkingStatus::limitations.
 *
 * Examples:
 * - @ref networking-status "Getting the networking status."
 */
class Q_DECL_EXPORT NetworkingStatus : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(NetworkingStatus)

    Q_ENUMS(Limitations)
    Q_ENUMS(Status)

    /**
     * limitations of the overall system networking
     *
     * @initvalue {}
     * @accessors limitations()
     * @notify limitationsChanged()
     *
     * \snippet example_networking_status.cpp limitations
     */
    Q_PROPERTY(QVector<Limitations> limitations
               READ limitations
               NOTIFY limitationsChanged)

    /**
     * status of the overall system networking
     *
     * @initvalue NetworkingStatus::Online
     * @accessors status()
     * @notify statusChanged()
     *
     * \snippet example_networking_status.cpp status
     */
    Q_PROPERTY(Status status
               READ status
               NOTIFY statusChanged)

public:
    explicit NetworkingStatus(QObject *parent = 0);
    virtual ~NetworkingStatus();

    /**
      * @brief enum for networking limitations
      *
      * Networking limitations may be accessed through the NetworkingStatus::limitations property.
      */
    enum Limitations {
        /**
         * indicates that the bandwith of the Internet connection has limitations.
         * Applications should minimize their bandwith usage if possible.
         */
        Bandwith
    };

    /**
      * @brief enum for networking status
      *
      * Networking status may be accessed through the NetworkingStatus::status property.
      */
    enum Status {
        Offline,     /**< No Internet connection available.            */
        Connecting,  /**< System is actively establising a connection. */
        Online       /**< System is connected to the Internet.         */
    };


    /** @see NetworkingStatus::limitations */
    QVector<Limitations> limitations() const;

    /** @see NetworkingStatus::status */
    Status status() const;

Q_SIGNALS:
    /** @see NetworkingStatus::limitations */
    void limitationsChanged();

    /** @see NetworkingStatus::status */
    void statusChanged(Status value);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(ubuntu::connectivity::NetworkingStatus::Limitations)
Q_DECLARE_METATYPE(QVector<ubuntu::connectivity::NetworkingStatus::Limitations>)
Q_DECLARE_METATYPE(ubuntu::connectivity::NetworkingStatus::Status)

#endif // CONNECTIVITY_NETWORKING_STATUS_H
