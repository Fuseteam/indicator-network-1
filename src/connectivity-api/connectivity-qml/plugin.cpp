/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <plugin.h>
#include <connectivityqt/connectivity.h>
#include <connectivityqt/openvpn-connection.h>

#include <connectivityqt/sim.h>

#include <connectivityqt/modems-list-model.h>
#include <connectivityqt/sims-list-model.h>


#include <QtQml>

namespace
{
static QObject *
connectivitySingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)
    return new connectivityqt::Connectivity(
            [](QObject *o)
            {
                QQmlEngine::setObjectOwnership(o, QQmlEngine::CppOwnership);
            },
            QDBusConnection::sessionBus(),
            engine);
}
}

void
QmlConnectivityNetworkingPlugin::registerTypes(const char *uri)
{
    connectivityqt::Connectivity::registerMetaTypes();
    qmlRegisterSingletonType<connectivityqt::Connectivity>(uri, 1, 0, "NetworkingStatus", connectivitySingletonProvider);
    qmlRegisterSingletonType<connectivityqt::Connectivity>(uri, 1, 1, "Connectivity", connectivitySingletonProvider);
    qmlRegisterUncreatableType<connectivityqt::VpnConnectionsListModel>(uri, 1, 0, "VpnConnectionsListModel", "Access VpnConnectionsListModel via Connectivity object");
    qmlRegisterUncreatableType<connectivityqt::VpnConnection>(uri, 1, 0, "VpnConnection", "Access VpnConnection via VpnConnectionsListModel object");
    qmlRegisterUncreatableType<connectivityqt::OpenvpnConnection>(uri, 1, 0, "OpenvpnConnection", "Access OpenvpnConnection via VpnConnectionsListModel object");

    qmlRegisterUncreatableType<connectivityqt::Sim>(uri, 1, 1, "Sim", "");

    qmlRegisterUncreatableType<connectivityqt::ModemsListModel>(uri, 1, 1, "ModemsListModel", "");
    qmlRegisterUncreatableType<connectivityqt::ModemsListModel>(uri, 1, 1, "SimsListModel", "");
}

void
QmlConnectivityNetworkingPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
    Q_UNUSED(engine);
}


