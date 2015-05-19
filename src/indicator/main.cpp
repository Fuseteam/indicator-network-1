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

#include <factory.h>
#include <util/unix-signal-handler.h>
#include <dbus-types.h>

#include <QCoreApplication>

#include <libintl.h>

#include <glib.h>
#include <libnotify/notify.h>

#include <config.h>

using namespace std;
using namespace connectivity_service;

int
main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    DBusTypes::registerMetaTypes();
    Variant::registerMetaTypes();

    util::UnixSignalHandler handler([]{
        QCoreApplication::exit(0);
    });
    handler.setupUnixSignalHandlers();

    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
    textdomain(GETTEXT_PACKAGE);

    notify_init(GETTEXT_PACKAGE);

    if (argc == 2 && QString("--print-address") == argv[1])
    {
        qDebug() << QDBusConnection::systemBus().baseService();
    }

    int result = 0;
    {
        Factory factory;
        auto menu = factory.newMenuBuilder();
        auto secretAgent = factory.newSecretAgent();
        auto connectivityService = factory.newConnectivityService();

        QObject::connect(connectivityService.get(), &ConnectivityService::unlockAllModems, menu.get(), &MenuBuilder::unlockAllModems);
        QObject::connect(connectivityService.get(), &ConnectivityService::unlockModem, menu.get(), &MenuBuilder::unlockModem);
        result = app.exec();
    }
    notify_uninit();

    return result;
}
