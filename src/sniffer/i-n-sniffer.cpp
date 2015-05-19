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
 *     Jussi Pakkanen <jussi.pakkanen@canonical.com>
 */


#include <QCoreApplication>

#include "dbusdata.h"
#include "eventprinter.h"
#include <NetworkManager.h>
#include <NetworkManagerAccessPointInterface.h>
#include <NetworkManagerActiveConnectionInterface.h>
#include <NetworkManagerSettingsConnectionInterface.h>
#include <NetworkManagerInterface.h>
#include <NetworkManagerDeviceWirelessInterface.h>
#include <URfkillInterface.h>
#include <URfkillKillswitchInterface.h>

#define slots
#include <qofono-qt5/qofonomanager.h>
#include <qofono-qt5/qofonomodem.h>
#include <qofono-qt5/qofonosimmanager.h>
#include <qofono-qt5/qofononetworkregistration.h>
#undef slots

static std::map<const int, const char*> nm_active_connection_strings = {{0, "Unknown"}, {1, "Activating"},
        {2, "Activated"}, {3, "Deactivating"}, {4, "Deactivated"}};
static std::map<const int, const char*> nm_connectivity_strings = {{0, "Unknown"}, {1, "None"}, {2, "Portal"},
        {3, "Limited"}, {4, "Full"}};
static std::map<const int, const char*> nm_state_strings = {{0, "Unknown"}, {10, "Asleep"}, {20, "Disconnected"},
    {30, "Disconnecting"}, {40, "Connecting"}, {50, "Connected local"}, {60, "Connected site"}, {70, "Connected global"}};
static std::map<const int, const char*> nm_ap_mode_strings = {{0, "Unknown"}, {1, "Adhoc"}, {2, "Infra"}, {3, "Access point"}};

// Lo-fi way of waiting for properties to populate
static void wait_for_a_bit()
{
    QEventLoop q;
    QTimer tT;
    tT.setSingleShot(true);
    QObject::connect(&tT, &QTimer::timeout, &q, &QEventLoop::quit);
    tT.start(50);
    q.exec();
}

static void print_nm_wlans(OrgFreedesktopNetworkManagerInterface *nmroot) {
    auto paths = nmroot->activeConnections();
    for(const auto &path : paths) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface ac(NM_DBUS_SERVICE, path.path(), QDBusConnection::systemBus(), nullptr);
        auto devices = ac.devices();
        // FIXME, should check all but most connections only have one.
        OrgFreedesktopNetworkManagerDeviceWirelessInterface dev(NM_DBUS_SERVICE, devices[0].path(), QDBusConnection::systemBus(), nullptr);
        auto reply = dev.GetAccessPoints();
        reply.waitForFinished();
        if(reply.isValid()) {
            // FIXME and the same here.
            // FIXME also that active access point might not exist
            OrgFreedesktopNetworkManagerAccessPointInterface ap(NM_DBUS_SERVICE, dev.activeAccessPoint().path(), QDBusConnection::systemBus(), nullptr);
            printf("Connection %s:\n", path.path().toUtf8().data());
            auto raw_ssid = ap.ssid();
            std::string ssid(raw_ssid.cbegin(), raw_ssid.cend());
            printf("  ssid: %s\n", ssid.c_str());
            printf("  mode: %d (%s)\n", ap.mode(), nm_ap_mode_strings[ap.mode()]);
            printf("  visible networks:\n");
            for(const auto &c : reply.value()) {
                OrgFreedesktopNetworkManagerAccessPointInterface i(NM_DBUS_SERVICE, c.path(), QDBusConnection::systemBus(), nullptr);
                auto i_raw_ssid = i.ssid();
                std::string i_ssid(i_raw_ssid.cbegin(), i_raw_ssid.cend());
                printf("    %s %d\n", i_ssid.c_str(), (int)i.strength());
            }
        } else {
            printf("Connection %s is not wireless.\n", path.path().toUtf8().data());
        }
    }
}

static void print_info(QCoreApplication &app) {
    OrgFreedesktopURfkillInterface *urfkill = new OrgFreedesktopURfkillInterface(URFKILL_SERVICE, URFKILL_OBJECT,
            QDBusConnection::systemBus(), &app);
    OrgFreedesktopURfkillKillswitchInterface *urfkillwlan = new OrgFreedesktopURfkillKillswitchInterface(URFKILL_SERVICE, URFKILL_WLAN_OBJECT,
            QDBusConnection::systemBus(), &app);
    OrgFreedesktopURfkillKillswitchInterface *urfkillbt = new OrgFreedesktopURfkillKillswitchInterface(URFKILL_SERVICE, URFKILL_BLUETOOTH_OBJECT,
            QDBusConnection::systemBus(), &app);
    OrgFreedesktopURfkillKillswitchInterface *urfkillgps = new OrgFreedesktopURfkillKillswitchInterface(URFKILL_SERVICE, URFKILL_GPS_OBJECT,
            QDBusConnection::systemBus(), &app);
    OrgFreedesktopNetworkManagerInterface *nmroot = new OrgFreedesktopNetworkManagerInterface(NM_DBUS_SERVICE, NM_DBUS_PATH,
            QDBusConnection::systemBus(), &app);
    QOfonoManager *ofonoroot = new QOfonoManager(&app);
    wait_for_a_bit();

    printf("Urfkill flightmode: %d\n", urfkill->IsFlightMode().value());
    printf("Urfkill WLAN killswitch: %d\n", urfkillwlan->state());
    printf("Urfkill BLUETOOTH killswitch: %d\n", urfkillbt->state());
    printf("Urfkill GPS killswitch: %d\n", urfkillgps->state());
    printf("\n");
    auto nm_connectivity = nmroot->connectivity();
    printf("NetworkManager connectivity: %d (%s)\n", nm_connectivity, nm_connectivity_strings[nm_connectivity]);
    auto nm_state = nmroot->state().value();
    printf("NetworkManager state: %d (%s).\n", nm_state, nm_state_strings[nm_state]);
    printf("NetworkManager networking enabled: %d\n", nmroot->networkingEnabled());
    printf("NetworkManager wireless enabled: %d\n", nmroot->wirelessEnabled());
    printf("NetworkManager wireless hardware enabled: %d\n", nmroot->wirelessHardwareEnabled());
    print_nm_wlans(nmroot);
    printf("\n");
    auto modems = ofonoroot->modems();
    printf("Ofono modem count: %d\n", modems.size());
    for(const auto &m : modems) {
        QOfonoModem modem;
        modem.setModemPath(m);

        QOfonoSimManager man;
        man.setModemPath(m);

        QOfonoNetworkRegistration netreg;
        netreg.setModemPath(m);

        wait_for_a_bit();

        printf("Modem %s:\n", m.toUtf8().constData());
        // For proper usage should check for existance before indexing.
        printf(" Powered: %d\n", modem.powered());
        printf(" Online: %d\n", modem.online());
        printf(" Model: %s\n", modem.model().toUtf8().constData());
        printf(" Manufacturer: %s\n", modem.manufacturer().toUtf8().constData());
        printf(" Pin required: %d\n", man.pinRequired());
        printf(" Status: %s\n", netreg.status().toUtf8().constData());
        printf(" Strength: %d\n", netreg.strength());
        printf(" Operator: %s\n", netreg.name().toUtf8().constData());
    }
}

static int run_daemon(QCoreApplication &app) {
    new EventPrinter(&app);
    return app.exec();
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    DBusTypes::registerMetaTypes();

    if(argc == 1) {
        print_info(app);
    } else if(argc == 2) {
        static const std::string arg("-d"); // For daemon mode. Feel free to change to something saner.
        if(argv[1] == arg)
            return run_daemon(app);
        printf("Unknown argument.\n");
        return 1;
    } else {
        printf("Too many arguments.\n");
        return 1;
    }
    return 0;
}
