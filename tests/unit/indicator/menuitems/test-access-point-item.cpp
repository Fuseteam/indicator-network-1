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

#include <cassert>

#include <menuitems/access-point-item.h>
#include <utils/action-utils.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace testutils;

using namespace nmofono;

namespace
{

class MockAccessPoint : public wifi::AccessPoint
{
public:
    MOCK_CONST_METHOD0(ssid, QString());

    MOCK_CONST_METHOD0(raw_ssid, QByteArray());

    MOCK_CONST_METHOD0(object_path, QDBusObjectPath());

    MOCK_CONST_METHOD0(secured, bool());

    MOCK_CONST_METHOD0(adhoc, bool());

    MOCK_CONST_METHOD0(strength, double());
};

class TestAccessPointItem : public Test
{
protected:
    DBusTestRunner dbus;
};

TEST_F(TestAccessPointItem, ExportBasicActionsAndMenu)
{
    shared_ptr<MockAccessPoint> accessPoint = make_shared<
            NiceMock<MockAccessPoint>>();
    ON_CALL(*accessPoint, ssid()).WillByDefault(Return(QString("the ssid")));
    ON_CALL(*accessPoint, secured()).WillByDefault(Return(true));
    ON_CALL(*accessPoint, adhoc()).WillByDefault(Return(false));
    ON_CALL(*accessPoint, strength()).WillByDefault(Return(70.0));

    auto accessPointItem = make_shared<AccessPointItem>(accessPoint);

    auto menuItem = accessPointItem->menuItem();

    EXPECT_EQ("the ssid", menuItem->label());
    EXPECT_FALSE(bool_value(menuItem, "x-canonical-wifi-ap-is-adhoc"));
    EXPECT_TRUE(bool_value(menuItem, "x-canonical-wifi-ap-is-secure"));

    string strengthActionName = string_value(
            menuItem, "x-canonical-wifi-ap-strength-action");

    auto strengthAction = findAction(accessPointItem->actionGroup(),
                                     strengthActionName);

    ASSERT_FALSE(strengthAction.get() == nullptr);
    EXPECT_EQ(70, strengthAction->state().as<uint8_t>());

    ON_CALL(*accessPoint, strength()).WillByDefault(Return(20.0));
    Q_EMIT accessPoint->strengthUpdated(20.0);
    EXPECT_EQ(20, strengthAction->state().as<uint8_t>());
}

} // namespace
