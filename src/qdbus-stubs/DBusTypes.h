/*
 * Copyright (C) 2013 Canonical, Ltd.
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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#ifndef DBUSTYPES_H_
#define DBUSTYPES_H_

#include <QDBusMetaType>
#include <QtCore>
#include <QString>
#include <QVariantMap>

typedef QMap<QString, QVariantMap> QVariantDictMap;
Q_DECLARE_METATYPE(QVariantDictMap)

class DBusTypes {
public:
    static void
    registerMetaTypes()
    {
        qRegisterMetaType<QVariantDictMap>("QVariantDictMap");

        qDBusRegisterMetaType<QVariantDictMap>();
    }

    static constexpr char const* DBUS_NAME = "com.ubuntu.connectivity1";

    static constexpr char const* SERVICE_INTERFACE = "com.ubuntu.connectivity1.NetworkingStatus";

    static constexpr char const* PRIVATE_INTERFACE = "com.ubuntu.connectivity1.Private";

    static constexpr char const* SERVICE_PATH = "/com/ubuntu/connectivity1/NetworkingStatus";

    static constexpr char const* PRIVATE_PATH = "/com/ubuntu/connectivity1/Private";
};

#endif /* DBUSTYPES_H_ */
