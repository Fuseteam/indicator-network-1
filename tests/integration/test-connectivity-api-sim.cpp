/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 *   Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 *   Pete Woods <pete.woods@canonical.com>
 */

#include <connectivityqt/sim.h>
#include <connectivityqt/sims-list-model.h>
#include <indicator-network-test-base.h>
#include <dbus-types.h>
#include <NetworkManagerSettingsInterface.h>

#include <QDebug>
#include <QTestEventLoop>

#define DEFINE_MODEL_LISTENERS \
    QSignalSpy rowsAboutToBeRemovedSpy(sims.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));\
    QSignalSpy rowsRemovedSpy(sims.get(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)));\
    QSignalSpy rowsAboutToBeInsertedSpy(sims.get(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));\
    QSignalSpy rowsInsertedSpy(sims.get(), SIGNAL(rowsInserted(const QModelIndex &, int, int)));\
    QSignalSpy dataChangedSpy(sims.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));\

#define CLEAR_MODEL_LISTENERS \
    rowsAboutToBeRemovedSpy.clear();\
    rowsRemovedSpy.clear();\
    rowsAboutToBeInsertedSpy.clear();\
    rowsInsertedSpy.clear();\
    dataChangedSpy.clear();

struct SS {
    QString iccid;
    QString imsi;
    QString primaryPhoneNumber;
    bool locked;
    bool present;
    QString mcc;
    QString mnc;
    QList<QString> preferredLanguages;
    bool dataRoamingEnabled;

    bool operator==(const SS& other) const
    {
        return iccid == other.iccid
                && imsi == other.imsi
                && primaryPhoneNumber == other.primaryPhoneNumber
                && locked == other.locked && present == other.present
                && mcc == other.mcc && mnc == other.mnc
                && preferredLanguages == other.preferredLanguages
                && dataRoamingEnabled == other.dataRoamingEnabled;
    }
};
typedef QList<SS> SSL;

inline void PrintTo (const SS& simState, std::ostream* os)
{
    *os << "SS("
            << "ICCID: " << simState.iccid.toStdString () << ", "
            << "IMSI: " << simState.imsi.toStdString () << ", "
            << "Phone #: " << simState.primaryPhoneNumber.toStdString () << ", "
            << "Locked: " << (simState.locked ? "y" : "n") << ", "
            << "Present: " << (simState.present ? "y" : "n") << ", "
            << "MCC: " << simState.mcc.toStdString () << ", "
            << "MNC: " << simState.mnc.toStdString () << ", "
            << "Langs: [" << QStringList(simState.preferredLanguages).join (",").toStdString () << "], "
            << "Roaming: " << (simState.dataRoamingEnabled ? "y" : "n")
            << ")";
}

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

    unique_ptr<QSortFilterProxyModel> getSortedSims(Connectivity& connectivity)
    {
        auto sims = connectivity.sims();

        auto sortedSims = make_unique<QSortFilterProxyModel>();
        sortedSims->setSortRole(SimsListModel::RoleIccid);
        sortedSims->sort(0);

        sortedSims->setSourceModel(sims);

        return sortedSims;
    }

    connectivityqt::Sim* getSim(const QAbstractItemModel& model, int idx)
    {
        auto sim = model.data(model.index(idx,0),
                              connectivityqt::SimsListModel::RoleSim)
                .value<connectivityqt::Sim*>();
        EXPECT_TRUE(sim);
        return sim;
    }

    SSL simList(QAbstractItemModel& model)
    {
        SSL simStates;
        int rowCount(model.rowCount());
        for (int i = 0; i < rowCount; ++i)
        {
            auto idx = model.index(i, 0);

            SS simState;
            simState.iccid = model.data(idx, SimsListModel::RoleIccid).toString();
            simState.imsi = model.data(idx, SimsListModel::RoleImsi).toString();
            simState.primaryPhoneNumber = model.data(idx, SimsListModel::RolePrimaryPhoneNumber).toString();
            simState.locked = model.data(idx, SimsListModel::RoleLocked).toBool();
            simState.present = model.data(idx, SimsListModel::RolePresent).toBool();
            simState.mcc = model.data(idx, SimsListModel::RoleMcc).toString();
            simState.mnc = model.data(idx, SimsListModel::RoleMnc).toString();
            simState.preferredLanguages = model.data(idx, SimsListModel::RolePreferredLanguages).toStringList();
            simState.dataRoamingEnabled = model.data(idx, SimsListModel::RoleDataRoamingEnabled).toBool();

            simStates << simState;
        }
        return simStates;
    }
};

TEST_F(TestConnectivityApiSim, SingleSimAtStartup)
{
    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    // Get the SIMs model
    auto sims = getSortedSims(*connectivity);

    DEFINE_MODEL_LISTENERS

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, sims, 1)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());

    auto sim = getSim(*sims, 0);
    while (sim->imsi().isEmpty() || sim->primaryPhoneNumber().isEmpty())
    {
        EXPECT_TRUE(dataChangedSpy.wait());
    }

    EXPECT_EQ(SSL({
        SS{
            "893581234000000000000",
            "310150000000000",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"},
            false
        }
    }), simList(*sims));
}

TEST_F(TestConnectivityApiSim, TwoSimsAtStartup)
{
    createModem("ril_1");

    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    // Get the SIMs model
    auto sims = getSortedSims(*connectivity);

    DEFINE_MODEL_LISTENERS

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, sims, 2)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());
    EXPECT_TRUE(dataChangedSpy.isEmpty ());

    auto sim = getSim(*sims, 0);
    while (sim->imsi().isEmpty() || sim->primaryPhoneNumber().isEmpty())
    {
        EXPECT_TRUE(dataChangedSpy.wait());
    }
    auto sim2 = getSim(*sims, 1);
    while (sim2->imsi().isEmpty() || sim2->primaryPhoneNumber().isEmpty())
    {
        EXPECT_TRUE(dataChangedSpy.wait());
    }

    EXPECT_EQ(SSL({
        SS{
            "893581234000000000000",
            "310150000000000",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"},
            false
        },
        SS{
            "893581234000000000001",
            "310150000000001",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"},
            false
        }
    }), simList(*sims));
}

TEST_F(TestConnectivityApiSim, AddASim)
{
    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    // Get the SIMs model
    auto sims = getSortedSims(*connectivity);

    DEFINE_MODEL_LISTENERS

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, sims, 1)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());
    EXPECT_TRUE(dataChangedSpy.isEmpty ());

    auto sim = getSim(*sims, 0);
    while (sim->imsi().isEmpty() || sim->primaryPhoneNumber().isEmpty())
    {
        EXPECT_TRUE(dataChangedSpy.wait());
    }

    EXPECT_EQ(SSL({
        SS{
            "893581234000000000000",
            "310150000000000",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"},
            false
        }
    }), simList(*sims));

    CLEAR_MODEL_LISTENERS

    createModem("ril_1");

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, sims, 2)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());

    auto sim2 = getSim(*sims, 1);
    while (sim2->imsi().isEmpty() || sim2->primaryPhoneNumber().isEmpty())
    {
        EXPECT_TRUE(dataChangedSpy.wait());
    }

    EXPECT_EQ(SSL({
        SS{
            "893581234000000000000",
            "310150000000000",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"},
            false
        },
        SS{
            "893581234000000000001",
            "310150000000001",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"},
            false
        }
    }), simList(*sims));
}

TEST_F(TestConnectivityApiSim, SimProperties)
{
    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    // Get the SIMs model
    auto sims = getSortedSims(*connectivity);

    DEFINE_MODEL_LISTENERS

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, sims, 1)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());
    EXPECT_TRUE(dataChangedSpy.isEmpty ());

    auto sim = qvariant_cast<Sim*>(sims->data(sims->index(0, 0), SimsListModel::Roles::RoleSim));
    ASSERT_TRUE(sim);
    while (sim->imsi().isEmpty() || sim->primaryPhoneNumber().isEmpty())
    {
        EXPECT_TRUE(dataChangedSpy.wait());
    }

    EXPECT_EQ("893581234000000000000", sim->iccid().toStdString());
    EXPECT_EQ("310150000000000", sim->imsi().toStdString());
    EXPECT_EQ("123456789", sim->primaryPhoneNumber().toStdString());
    EXPECT_FALSE(sim->locked());
    EXPECT_TRUE(sim->present());
    EXPECT_EQ("310", sim->mcc().toStdString());
    EXPECT_EQ("150", sim->mnc().toStdString());
    EXPECT_EQ(QStringList{"en"}, sim->preferredLanguages());
}

TEST_F(TestConnectivityApiSim, RoamingAllowed)
{
//   test that roaming allowed has an effect.
}


}
