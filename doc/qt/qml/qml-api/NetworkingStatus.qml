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

import QtQuick 2.0

/*!
 \qmltype NetworkingStatus
 \ingroup connectivity
 \brief Overall system networking status.

 This is the top-level class for accessing networking information.

 This class inherits the Qt C++ ubuntu::connectivity::NetworkingStatus
 and provides two utility properties online and limitedBandwith for easier
 QML usage.

 This object is exposed as a singleton.

 \b{note:}
 Using this component in confined application requires \e{connectivity} policy group.

 \quotefile example_networking_status.qml
 */

QtObject {

    /*!
       \qmlproperty bool NetworkingStatus::online
       \b{true} if system has Internet connection.

       shorthand for C++:
       \code
           networkingStatus->status() == NetworkingStatus::Online
       \endcode
     */
    property bool online

    /*!
       \qmlproperty bool NetworkingStatus::limitedBandwith
       \b{true} if Internet connection is bandwith limited.

       shorthand for C++:
       \code
           networkingStatus->limitations().contains(NetworkingStatus::Limitations::Bandwith)
       \endcode
     */
    property bool limitedBandwith

    /*!
       \qmlproperty list<Limitations> NetworkingStatus::limitations
     */
    property list<Limitations> limitations

    /*!
       \qmlproperty NetworkingStatus::Status status
       status property of the base C++ class.

       \code
       onStatusChanged: {
           if (status === NetworkingStatus::Offline)
               ;
           else if (status === NetworkingStatus::Connecting)
               ;
           else if (status === NetworkingStatus::Online)
               ;
       }
       \endcode
     */
    property Status status
}
