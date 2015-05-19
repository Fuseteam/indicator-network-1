/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include <config.h>
#include <libintl.h>
#include <config.h>
#include <factory.h>
#include <dbus-types.h>
#include <util/unix-signal-handler.h>

#include <QCoreApplication>
#include <iostream>

using namespace std;


int main(int argc, char *argv[]) {
    QCoreApplication application(argc, argv);
    DBusTypes::registerMetaTypes();

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
    textdomain(GETTEXT_PACKAGE);

    util::UnixSignalHandler handler([]
    {
        QCoreApplication::exit(0);
    });
    handler.setupUnixSignalHandlers();

    Factory factory;
    auto secretAgent = factory.newSecretAgent();

    if (argc == 2 && QString("--print-address") == argv[1]) {
        cout << QDBusConnection::systemBus().baseService().toStdString()
                << endl;
    }

    return application.exec();
}
