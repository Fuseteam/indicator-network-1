
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
 * Author: Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <indicator-network-test-base.h>
#include <dbus-types.h>
#include <NetworkManagerSettingsInterface.h>

#include <QDebug>
#include <QTestEventLoop>

using namespace std;
using namespace testing;
using namespace connectivityqt;

namespace
{

class TestConnectivityApiSim: public IndicatorNetworkTestBase
{
protected:
    static void SetUpTestCase()
    {
        Connectivity::registerMetaTypes();
    }
};

TEST_F(TestConnectivityApiSim, Constants)
{

}

TEST_F(TestConnectivityApiSim, Present)
{

}

TEST_F(TestConnectivityApiSim, Locked)
{

}

TEST_F(TestConnectivityApiSim, DataRoamingEnabled)
{

}

TEST_F(TestConnectivityApiSim, Unlock)
{

}

}
