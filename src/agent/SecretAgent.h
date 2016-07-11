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

#pragma once

#include <dbus-types.h>
#include <agent/CredentialStore.h>

#include <memory>

#include <QDBusConnection>
#include <QDBusContext>
#include <QMap>

class SecretAgentAdaptor;

namespace notify
{
class NotificationManager;
}

namespace agent
{

class SecretRequest;

class SecretAgent: public QObject, protected QDBusContext {
	Q_OBJECT

	friend SecretAgentAdaptor;
	friend SecretRequest;

public:
	typedef std::shared_ptr<SecretAgent> Ptr;
	typedef std::unique_ptr<SecretAgent> UPtr;

	static constexpr char const* NM_CONNECTION_SETTING_NAME = "connection";

	static constexpr char const* NM_CONNECTION_ID = "id";
	static constexpr char const* NM_CONNECTION_UUID = "uuid";
	static constexpr char const* NM_CONNECTION_TYPE = "type";

	static constexpr char const* NM_ADSL_SETTING_NAME = "adsl";
	static constexpr char const* NM_ADSL_PASSWORD = "password";

	static constexpr char const* NM_GSM_SETTING_NAME = "gsm";
	static constexpr char const* NM_GSM_PASSWORD = "password";
	static constexpr char const* NM_GSM_PIN = "pin";

	static constexpr char const* NM_802_1X_SETTING_NAME = "802-1x";
	static constexpr char const* NM_802_1X_PASSWORD = "password";
	static constexpr char const* NM_802_1X_PRIVATE_KEY_PASSWORD = "private-key-password";
	static constexpr char const* NM_802_1X_PHASE2_PRIVATE_KEY_PASSWORD = "phase2-private-key-password";
	static constexpr char const* NM_802_1X_PIN = "pin";

	static constexpr char const* NM_CDMA_SETTING_NAME = "cdma";
	static constexpr char const* NM_CDMA_PASSWORD = "password";

	static constexpr char const* NM_PPPOE_SETTING_NAME = "pppoe";
	static constexpr char const* NM_PPPOE_PASSWORD = "password";

	static constexpr char const* NM_VPN_SETTING_NAME = "vpn";
	static constexpr char const* NM_VPN_SERVICE_TYPE = "service-type";
	static constexpr char const* NM_VPN_SECRETS = "secrets";

	static constexpr char const* NM_WIRELESS_SECURITY_SETTING_NAME = "802-11-wireless-security";
	static constexpr char const* NM_WIRELESS_SECURITY_KEY_MGMT = "key-mgmt";
	static constexpr char const* NM_WIRELESS_SECURITY_WEP_KEY0 = "wep-key0";
	static constexpr char const* NM_WIRELESS_SECURITY_WEP_KEY1 = "wep-key1";
	static constexpr char const* NM_WIRELESS_SECURITY_WEP_KEY2 = "wep-key2";
	static constexpr char const* NM_WIRELESS_SECURITY_WEP_KEY3 = "wep-key3";
	static constexpr char const* NM_WIRELESS_SECURITY_PSK = "psk";
	static constexpr char const* NM_WIRELESS_SECURITY_LEAP_PASSWORD = "leap-password";

	static constexpr char const* NM_KEY_MGMT_WPA_NONE = "wpa-none";
	static constexpr char const* NM_KEY_MGMT_WPA_PSK = "wpa-psk";
	static constexpr char const* NM_KEY_MGMT_NONE = "none";
	static constexpr char const* NM_KEY_MGMT_WAPI_PSK = "wapi-psk";

	explicit SecretAgent(std::shared_ptr<notify::NotificationManager> notificationManager,
			CredentialStore::SPtr credentialStore,
            const QDBusConnection &systemConnection,
            const QDBusConnection &sessionConnection, QObject *parent = 0);

	virtual ~SecretAgent();

protected Q_SLOTS:
	QVariantDictMap GetSecrets(const QVariantDictMap &connection,
			const QDBusObjectPath &connectionPath, const QString &settingName,
			const QStringList &hints, uint flags);

	void FinishGetSecrets(SecretRequest &request, bool error);

	void CancelGetSecrets(const QDBusObjectPath &connectionPath,
			const QString &settingName);

	void DeleteSecrets(const QVariantDictMap &connection,
			const QDBusObjectPath &connectionPath);

	void SaveSecrets(const QVariantDictMap &connection,
			const QDBusObjectPath &connectionPath);

	std::shared_ptr<notify::NotificationManager> notifications();

protected:
	class Priv;
	std::shared_ptr<Priv> d;
};

}
