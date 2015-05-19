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

#include <memory>

#include <QDBusConnection>
#include <QDBusContext>
#include <QMap>

class OrgFreedesktopNotificationsInterface;
class SecretAgentAdaptor;

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

	static constexpr char const* CONNECTION_SETTING_NAME = "connection";
	static constexpr char const* WIRELESS_SECURITY_SETTING_NAME = "802-11-wireless-security";

	static constexpr char const* CONNECTION_ID = "id";

	static constexpr char const* WIRELESS_SECURITY_PSK = "psk";
	static constexpr char const* WIRELESS_SECURITY_WEP_KEY0 = "wep-key0";

	static constexpr char const* WIRELESS_SECURITY_KEY_MGMT = "key-mgmt";

	static constexpr char const* KEY_MGMT_WPA_NONE = "wpa-none";
	static constexpr char const* KEY_MGMT_WPA_PSK = "wpa-psk";
	static constexpr char const* KEY_MGMT_NONE = "none";

	explicit SecretAgent(const QDBusConnection &systemConnection,
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

	OrgFreedesktopNotificationsInterface & notifications();

protected:
	class Priv;
	std::shared_ptr<Priv> d;
};

}
