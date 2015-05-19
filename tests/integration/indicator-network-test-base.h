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

#pragma once

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbustest/QProcessDBusService.h>
#include <libqtdbusmock/DBusMock.h>

#include <menuharness/MatchUtils.h>
#include <menuharness/MenuMatcher.h>

#include <NetworkManager.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

inline void PrintTo(const QVariant& variant, std::ostream* os) {
        *os << "QVariant(" << variant.toString().toStdString() << ")";
}

#define WAIT_FOR_SIGNALS(signalSpy, signalsExpected)\
{\
    while (signalSpy.size() < signalsExpected)\
    {\
        ASSERT_TRUE(signalSpy.wait());\
    }\
    ASSERT_EQ(signalsExpected, signalSpy.size());\
}

#define CHECK_METHOD_CALL(signalSpy, signalIndex, methodName, ...)\
{\
    QVariantList const& call(signalSpy.at(signalIndex));\
    EXPECT_EQ(methodName, call.at(0));\
    auto arguments = vector<pair<int, QVariant>>{__VA_ARGS__};\
    if (!arguments.empty())\
    {\
        QVariantList const& args(call.at(1).toList());\
        ASSERT_LE(arguments.back().first + 1, args.size());\
        for (auto const& argument : arguments)\
        {\
            EXPECT_EQ(argument.second, args.at(argument.first));\
        }\
    }\
}

class IndicatorNetworkTestBase: public testing::Test
{
public:
    enum class Secure
    {
        secure,
        insecure
    };

    enum class ApMode
    {
        infra,
        adhoc
    };

    enum class ConnectionStatus
    {
        connected,
        disconnected
    };

    IndicatorNetworkTestBase();

    ~IndicatorNetworkTestBase();

protected:
    void SetUp() override;

    static menuharness::MenuMatcher::Parameters phoneParameters();

    static menuharness::MenuMatcher::Parameters unlockSimParameters(std::string const& busName, int exportId);

    void startIndicator();

    QString createWiFiDevice(int state, const QString& id = "0");

    static QString randomMac();

    void enableWiFi();

    void disableWiFi();

    QString createAccessPoint(const QString& id, const QString& ssid, const QString& device, uchar strength = 100,
                              Secure secure = Secure::secure, ApMode apMode = ApMode::infra);

    void removeAccessPoint(const QString& device, const QString& ap);

    QString createAccessPointConnection(const QString& id, const QString& ssid, const QString& device);

    void removeWifiConnection(const QString& device, const QString& connection);

    QString createActiveConnection(const QString& id, const QString& device, const QString& connection, const QString& ap);

    void removeActiveConnection(const QString& device, const QString& active_connection);

    void setGlobalConnectedState(int state);

    void setNmProperty(const QString& path, const QString& iface, const QString& name, const QVariant& value);

    QString createModem(const QString& id);

    void setModemProperty(const QString& path, const QString& propertyName, const QVariant& value);

    void setSimManagerProperty(const QString& path, const QString& propertyName, const QVariant& value);

    void setConnectionManagerProperty(const QString& path, const QString& propertyName, const QVariant& value);

    void setNetworkRegistrationProperty(const QString& path, const QString& propertyName, const QVariant& value);

    OrgFreedesktopDBusMockInterface& notificationsMockInterface();

    OrgFreedesktopDBusMockInterface& modemMockInterface(const QString& path);

    static bool qDBusArgumentToMap(QVariant const& variant, QVariantMap& map);

    static QString firstModem();

    static menuharness::MenuItemMatcher flightModeSwitch(bool toggled = false);

    static menuharness::MenuItemMatcher accessPoint(const std::string& ssid, Secure secure,
                ApMode apMode, ConnectionStatus connectionStatus, uchar strength = 100);

    static menuharness::MenuItemMatcher wifiEnableSwitch(bool toggled = true);

    static menuharness::MenuItemMatcher wifiSettings();

    static menuharness::MenuItemMatcher modemInfo(const std::string& simIdentifier, const std::string& label, const std::string& statusIcon, bool locked = false);

    static menuharness::MenuItemMatcher cellularSettings();

    QtDBusTest::DBusTestRunner dbusTestRunner;

    QtDBusMock::DBusMock dbusMock;

    QtDBusTest::DBusServicePtr indicator;
};
