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
#include <agent/PasswordMenu.h>

#include <QDBusMessage>
#include <QDBusObjectPath>

namespace agent
{

class SecretRequest;
class SecretAgent;

class SecretRequest: public QObject {
Q_OBJECT
public:
	explicit SecretRequest(SecretAgent &secretAgent,
			const QVariantDictMap &connection,
			const QDBusObjectPath &connectionPath, const QString &settingName,
			const QStringList &hints, uint flags, const QDBusMessage &reply,
			QObject *parent = 0);

	virtual ~SecretRequest();

public Q_SLOTS:
	void actionInvoked(uint id, const QString &actionKey);
	void notificationClosed(uint id, uint reason);

public:
	const QVariantDictMap & connection() const;

	const QDBusMessage & message() const;

protected:
	unsigned int m_notificationId;

	SecretAgent &m_secretAgent;

	QVariantDictMap m_connection;

	QDBusObjectPath m_connectionPath;

	QString m_settingName;

	QStringList m_hints;

	uint m_flags;

	QDBusMessage m_message;

	PasswordMenu m_menu;
};

}
