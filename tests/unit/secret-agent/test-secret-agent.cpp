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

#include <agent/SecretAgent.h>
#include <SecretAgentInterface.h>
#include <NotificationsInterface.h>
#include <NetworkManager.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <qmenumodel/unitymenumodel.h>
#include <QSignalSpy>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;
using namespace agent;

namespace {

class TestSecretAgentCommon {
protected:
	TestSecretAgentCommon() :
			dbusMock(dbusTestRunner) {

		DBusTypes::registerMetaTypes();

		dbusMock.registerNotificationDaemon();
		dbusMock.registerNetworkManager();
		dbusTestRunner.startServices();

		QProcessEnvironment env(QProcessEnvironment::systemEnvironment());
		env.insert("SECRET_AGENT_DEBUG_PASSWORD", "1");
		secretAgent.setProcessEnvironment(env);
		secretAgent.setProcessChannelMode(QProcess::MergedChannels);
		secretAgent.start(SECRET_AGENT_BIN, QStringList() << "--print-address");
		secretAgent.waitForStarted();
		secretAgent.waitForReadyRead();
		agentBus = secretAgent.readAll().trimmed();

		agentInterface.reset(
				new OrgFreedesktopNetworkManagerSecretAgentInterface(agentBus,
				NM_DBUS_PATH_SECRET_AGENT, dbusTestRunner.systemConnection()));


		notificationsInterface.reset(
				new OrgFreedesktopDBusMockInterface(
						"org.freedesktop.Notifications",
						"/org/freedesktop/Notifications",
						dbusTestRunner.sessionConnection()));
	}

	virtual ~TestSecretAgentCommon() {
		secretAgent.terminate();
		secretAgent.waitForFinished();
	}

	QVariantDictMap connection(const QString &keyManagement) {
		QVariantMap wirelessSecurity;
		wirelessSecurity[SecretAgent::WIRELESS_SECURITY_KEY_MGMT] =
				keyManagement;

		QVariantMap conn;
		conn[SecretAgent::CONNECTION_ID] = "the ssid";

		QVariantDictMap connection;
		connection[SecretAgent::WIRELESS_SECURITY_SETTING_NAME] =
				wirelessSecurity;
		connection[SecretAgent::CONNECTION_SETTING_NAME] = conn;

		return connection;
	}

	QVariantDictMap expected(const QString &keyManagement,
			const QString &keyName, const QString &password) {

		QVariantMap wirelessSecurity;
		wirelessSecurity[SecretAgent::WIRELESS_SECURITY_KEY_MGMT] =
				keyManagement;
		wirelessSecurity[keyName] = password;

		QVariantMap conn;
		conn[SecretAgent::CONNECTION_ID] = "the ssid";

		QVariantDictMap connection;
		connection[SecretAgent::WIRELESS_SECURITY_SETTING_NAME] =
				wirelessSecurity;
		connection[SecretAgent::CONNECTION_SETTING_NAME] = conn;

		return connection;
	}

	DBusTestRunner dbusTestRunner;

	DBusMock dbusMock;

	QProcess secretAgent;

	QString agentBus;

	QScopedPointer<OrgFreedesktopNetworkManagerSecretAgentInterface> agentInterface;

	QScopedPointer<OrgFreedesktopDBusMockInterface> notificationsInterface;
};

struct TestSecretAgentParams {
	QString keyManagement;

	QString passwordKey;

	QString password;
};

class TestSecretAgentGetSecrets: public TestSecretAgentCommon,
		public TestWithParam<TestSecretAgentParams> {
};

static void transform(QVariantMap &map);

static void transform(QVariant &variant) {
	if (variant.canConvert<QDBusArgument>()) {
		QDBusArgument value(variant.value<QDBusArgument>());
		if (value.currentType() == QDBusArgument::MapType) {
			QVariantMap map;
			value >> map;
			transform(map);
			variant = map;
		}
	}
}

static void transform(QVariantMap &map) {
	for (auto it(map.begin()); it != map.end(); ++it) {
		transform(*it);
	}
}

static void transform(QVariantList &list) {
	for (auto it(list.begin()); it != list.end(); ++it) {
		transform(*it);
	}
}

TEST_P(TestSecretAgentGetSecrets, ProvidesPasswordForWpaPsk) {
	QDBusPendingReply<QVariantDictMap> reply(
			agentInterface->GetSecrets(connection(GetParam().keyManagement),
					QDBusObjectPath("/connection/foo"),
					SecretAgent::WIRELESS_SECURITY_SETTING_NAME, QStringList(),
					5));

	QSignalSpy notificationSpy(notificationsInterface.data(),
	SIGNAL(MethodCalled(const QString &, const QVariantList &)));
	notificationSpy.wait();

	ASSERT_EQ(1, notificationSpy.size());
	const QVariantList &call(notificationSpy.at(0));
	ASSERT_EQ("Notify", call.at(0));

	QVariantList args(call.at(1).toList());
	transform(args);

	ASSERT_EQ(8, args.size());
	EXPECT_EQ("indicator-network", args.at(0));
	EXPECT_EQ("Connect to “the ssid”", args.at(3).toString().toStdString());

	QVariantMap hints(args.at(6).toMap());
	QVariantMap menuInfo(hints["x-canonical-private-menu-model"].toMap());

	QString busName(menuInfo["busName"].toString());
	QString menuPath(menuInfo["menuPath"].toString());
	QVariantMap actions(menuInfo["actions"].toMap());

	{
		UnityMenuModel menuModel;

		QSignalSpy menuSpy(&menuModel,
		SIGNAL(rowsInserted(const QModelIndex&, int, int)));

		menuModel.setBusName(busName.toUtf8());
		menuModel.setMenuObjectPath(menuPath.toUtf8());
		menuModel.setActions(actions);

		menuSpy.wait();

		menuModel.changeState(0, GetParam().password);

		// It seems like UnityMenuModel or the GLib
		// DBus connection needs some grace time to
		// finish dispatching.
		secretAgent.waitForReadyRead();
		ASSERT_EQ("Password received", secretAgent.readAll().trimmed());
	}

	notificationsInterface->EmitSignal(
			OrgFreedesktopNotificationsInterface::staticInterfaceName(),
			"ActionInvoked", "us", QVariantList() << 1 << "connect_id");

	QVariantDictMap result(reply);

	EXPECT_EQ(
			expected(GetParam().keyManagement, GetParam().passwordKey,
					GetParam().password), result);
}

INSTANTIATE_TEST_CASE_P(WpaPsk, TestSecretAgentGetSecrets,
		Values(TestSecretAgentParams( { SecretAgent::KEY_MGMT_WPA_PSK,
				SecretAgent::WIRELESS_SECURITY_PSK, "hard-coded-password" })));

INSTANTIATE_TEST_CASE_P(WpaPskLongPassword, TestSecretAgentGetSecrets,
		Values(TestSecretAgentParams( { SecretAgent::KEY_MGMT_WPA_PSK,
				SecretAgent::WIRELESS_SECURITY_PSK, "123456789012345678901234567890123456789012345678901234" })));

INSTANTIATE_TEST_CASE_P(WpaNone, TestSecretAgentGetSecrets,
		Values(TestSecretAgentParams( { SecretAgent::KEY_MGMT_WPA_NONE,
				SecretAgent::WIRELESS_SECURITY_PSK, "hard-coded-password" })));

INSTANTIATE_TEST_CASE_P(None, TestSecretAgentGetSecrets,
		Values(
				TestSecretAgentParams( { SecretAgent::KEY_MGMT_NONE,
						SecretAgent::WIRELESS_SECURITY_WEP_KEY0,
						"hard-coded-password" })));

class TestSecretAgent: public TestSecretAgentCommon, public Test {
};

TEST_F(TestSecretAgent, GetSecretsWithNone) {

	QDBusPendingReply<QVariantDictMap> reply(
			agentInterface->GetSecrets(
					connection(SecretAgent::KEY_MGMT_WPA_PSK),
					QDBusObjectPath("/connection/foo"),
					SecretAgent::WIRELESS_SECURITY_SETTING_NAME, QStringList(),
					0));
	reply.waitForFinished();

	ASSERT_TRUE(reply.isError());
	EXPECT_EQ(QDBusError::InternalError, reply.error().type());
	EXPECT_EQ("No password found for this connection.",
			reply.error().message());
}

/* Tests that if we request secrets and then cancel the request
   that we close the notification */
TEST_F(TestSecretAgent, CancelGetSecrets) {
	agentInterface->GetSecrets(
			connection(SecretAgent::KEY_MGMT_WPA_PSK),
			QDBusObjectPath("/connection/foo"),
			SecretAgent::WIRELESS_SECURITY_SETTING_NAME, QStringList(),
			5);

	QSignalSpy notificationSpy(notificationsInterface.data(), SIGNAL(MethodCalled(const QString &, const QVariantList &)));
	notificationSpy.wait();

	ASSERT_EQ(1, notificationSpy.size());
	const QVariantList &call(notificationSpy.at(0));
	ASSERT_EQ("Notify", call.at(0));

	notificationSpy.clear();

	agentInterface->CancelGetSecrets(QDBusObjectPath("/connection/foo"),
			SecretAgent::WIRELESS_SECURITY_SETTING_NAME);

	notificationSpy.wait();

	ASSERT_EQ(1, notificationSpy.size());
	const QVariantList &closecall(notificationSpy.at(0));
	ASSERT_EQ("CloseNotification", closecall.at(0));
}

/* Ensures that if we request secrets twice we close the notification
   for the first request */
TEST_F(TestSecretAgent, MultiSecrets) {
	QSignalSpy notificationSpy(notificationsInterface.data(), SIGNAL(MethodCalled(const QString &, const QVariantList &)));

	agentInterface->GetSecrets(
			connection(SecretAgent::KEY_MGMT_WPA_PSK),
			QDBusObjectPath("/connection/foo"),
			SecretAgent::WIRELESS_SECURITY_SETTING_NAME, QStringList(),
			5);

	notificationSpy.wait();

	ASSERT_EQ(1, notificationSpy.size());
	const QVariantList &call(notificationSpy.at(0));
	ASSERT_EQ("Notify", call.at(0));

	notificationSpy.clear();

	agentInterface->GetSecrets(
			connection(SecretAgent::KEY_MGMT_WPA_PSK),
			QDBusObjectPath("/connection/foo2"),
			SecretAgent::WIRELESS_SECURITY_SETTING_NAME, QStringList(),
			5);

	notificationSpy.wait();
	notificationSpy.wait();

	ASSERT_EQ(2, notificationSpy.size());
	const QVariantList &closecall(notificationSpy.at(1));
	ASSERT_EQ("CloseNotification", closecall.at(0));

	const QVariantList &newnotify(notificationSpy.at(0));
	ASSERT_EQ("Notify", newnotify.at(0));
}

TEST_F(TestSecretAgent, SaveSecrets) {
	agentInterface->SaveSecrets(QVariantDictMap(),
			QDBusObjectPath("/connection/foo")).waitForFinished();
}

TEST_F(TestSecretAgent, DeleteSecrets) {
	agentInterface->DeleteSecrets(QVariantDictMap(),
			QDBusObjectPath("/connection/foo")).waitForFinished();
}

} // namespace
