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

#include <agent/PasswordMenu.h>
#include <gio/gio.h>

#include <QString>
#include <QDebug>

namespace agent
{
namespace
{
static const QString PASSWORD_ACTION_PATH("/action/%1");
static const QString PASSWORD_MENU_PATH("/menu/%1");
}

class PasswordMenuPriv {
public:
	PasswordMenuPriv() :
			m_connection(g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr,
			nullptr)), m_exportedActionGroupId(0), m_exportedMenuModelId(0) {
	}

	~PasswordMenuPriv() {
		g_object_unref(m_connection);
	}

	static void passwordChangedCallback(GAction *passwordAction,
			GVariant *variant, gpointer userData) {
		Q_UNUSED(passwordAction);
		PasswordMenuPriv *self(reinterpret_cast<PasswordMenuPriv*>(userData));
		self->passwordChanged(variant);
	}

	void passwordChanged(GVariant *variant) {
		m_password = QString::fromUtf8(g_variant_get_string(variant, 0));
		if (qEnvironmentVariableIsSet("SECRET_AGENT_DEBUG_PASSWORD")) {
			qDebug() << "Password received";
		}
	}

	GDBusConnection *m_connection;

	QString m_busName;

	QString m_actionPath;

	QString m_menuPath;

	unsigned int m_exportedActionGroupId;

	unsigned int m_exportedMenuModelId;

	QString m_password;

};

PasswordMenu::PasswordMenu() :
		p(new PasswordMenuPriv()) {
	int exportrev;

	p->m_busName = QString::fromUtf8(
			g_dbus_connection_get_unique_name(p->m_connection));

	// menu
	GMenu *menu(g_menu_new());

	GMenuItem *passwordItem(g_menu_item_new("", "notifications.password"));
	g_menu_item_set_attribute_value(passwordItem, "x-canonical-type",
			g_variant_new_string("com.canonical.snapdecision.textfield"));
	g_menu_item_set_attribute_value(passwordItem, "x-echo-mode-password",
			g_variant_new_boolean(true));

	g_menu_append_item(menu, passwordItem);

	// actions
	GActionGroup *actions(G_ACTION_GROUP(g_simple_action_group_new()));
	GAction *passwordAction(G_ACTION(
			g_simple_action_new_stateful("password", G_VARIANT_TYPE_STRING,
					g_variant_new_string(""))));

	g_signal_connect(G_OBJECT(passwordAction), "change-state",
			G_CALLBACK(PasswordMenuPriv::passwordChangedCallback),
			reinterpret_cast<gpointer>(p.data()));

	g_action_map_add_action(G_ACTION_MAP(actions), passwordAction);

	/* Export the actions group.  If we can't get a name, keep trying to
	   use increasing numbers.  There is possible races on fast import/exports.
	   They're rare, but worth protecting against. */
	exportrev = 0;
	do {
		exportrev++;
		p->m_actionPath = PASSWORD_ACTION_PATH.arg(exportrev);
		p->m_exportedActionGroupId = g_dbus_connection_export_action_group(
				p->m_connection, p->m_actionPath.toUtf8().data(), actions, nullptr);
	} while (p->m_exportedActionGroupId == 0 && exportrev < 128);

	/* Export the menu.  If we can't get a name, keep trying to
	   use increasing numbers.  There is possible races on fast import/exports.
	   They're rare, but worth protecting against. */
	exportrev = 0;
	do {
		exportrev++;
		p->m_menuPath = PASSWORD_MENU_PATH.arg(exportrev);
		p->m_exportedMenuModelId = g_dbus_connection_export_menu_model(
				p->m_connection, p->m_menuPath.toUtf8().data(),
				G_MENU_MODEL(menu), nullptr);
	} while (p->m_exportedMenuModelId == 0 && exportrev < 128);

	/* Unref the objects as a reference is maintained by the fact that they're
	   exported onto the bus. */
	g_object_unref(menu);
	g_object_unref(passwordItem);

	g_object_unref(actions);
	g_object_unref(passwordAction);
}

PasswordMenu::~PasswordMenu() {
	g_dbus_connection_unexport_action_group(p->m_connection,
			p->m_exportedActionGroupId);
	g_dbus_connection_unexport_menu_model(p->m_connection,
			p->m_exportedMenuModelId);
}

const QString & PasswordMenu::busName() const {
	return p->m_busName;
}

const QString & PasswordMenu::password() const {
	return p->m_password;
}

const QString & PasswordMenu::actionPath() const {
	return p->m_actionPath;
}

const QString & PasswordMenu::menuPath() const {
	return p->m_menuPath;
}

}
