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

#pragma once

#include <QObject>
#include <QDBusVariant>

class OrgFreedesktopURfkillInterface;
class OrgFreedesktopURfkillKillswitchInterface;
class OrgFreedesktopNetworkManagerInterface;
class QOfonoModem;
class QOfonoSimManager;
class QOfonoNetworkRegistration;

class EventPrinter : QObject {
    Q_OBJECT

public:
    explicit EventPrinter(QObject *parent = nullptr);
    ~EventPrinter();

public Q_SLOTS:
    void flightModeChanged(bool new_mode);
    void btKillswitchChanged();
    void wlanKillswitchChanged();
    void nmStateChanged(uint new_state);
    void nmPropertiesChanged(const QVariantMap &props);
    void modem1PropertyChanged(const QString &name, const QVariant &value);
    void modem2PropertyChanged(const QString &name, const QVariant &value);
    void simman1PropertyChanged(const QString &name, const QVariant &value);
    void simman2PropertyChanged(const QString &name, const QVariant &value);
    void netreg1PropertyChanged(const QString &name, const QVariant &value);
    void netreg2PropertyChanged(const QString &name, const QVariant &value);

private:

    void modemPropertyChanged(const int modem, const QString &name, const QVariant &value);
    void simmanPropertyChanged(const int simman, const QString &name, const QVariant &value);
    void netregPropertyChanged(const int modem, const QString &name, const QVariant &value);
    void activeConnectionsChanged(const QVariant &list);
    void primaryConnectionTypeChanged(const QString &type);

    OrgFreedesktopURfkillInterface *urfkill;
    OrgFreedesktopURfkillKillswitchInterface *btkill;
    OrgFreedesktopURfkillKillswitchInterface *wlankill;
    OrgFreedesktopNetworkManagerInterface *nmroot;
    QOfonoModem *modem1;
    QOfonoModem *modem2;
    QOfonoSimManager *simman1;
    QOfonoSimManager *simman2;
    QOfonoNetworkRegistration *netreg1;
    QOfonoNetworkRegistration *netreg2;
};
