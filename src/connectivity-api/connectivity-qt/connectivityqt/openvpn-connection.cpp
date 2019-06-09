/*
 * Copyright © 2015 Canonical Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <connectivityqt/openvpn-connection.h>
#include <util/dbus-property-cache.h>
#include <dbus-types.h>

#include <OpenVpnConnectionInterface.h>

using namespace std;

#define DEFINE_PROPERTY_GETTER(name, strname, type, conversion)\
type OpenvpnConnection::name() const\
{\
    return d->m_propertyCache->get(strname).conversion();\
}\

#define DEFINE_PROPERTY_GETTER_ENUM(name, strname, type)\
OpenvpnConnection::type OpenvpnConnection::name() const\
{\
    return static_cast<type>(d->m_propertyCache->get(strname).toInt());\
}

#define DEFINE_PROPERTY_SETTER(uppername, strname, type)\
void OpenvpnConnection::set##uppername(type value)\
{\
    d->m_propertyCache->set(strname, value);\
}

#define DEFINE_PROPERTY_SETTER_ENUM(uppername, strname, type)\
void OpenvpnConnection::set##uppername(type value)\
{\
    d->m_propertyCache->set(strname, static_cast<int>(value));\
}

#define DEFINE_PROPERTY_UPDATE(varname, strname, conversion)\
else if (name == strname)\
{\
    Q_EMIT p.varname##Changed(value.conversion());\
}

#define DEFINE_PROPERTY_UPDATE_ENUM(varname, strname, type)\
else if (name == strname)\
{\
    Q_EMIT p.varname##Changed(static_cast<type>(value.toInt()));\
}

namespace connectivityqt
{

class OpenvpnConnection::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(OpenvpnConnection& parent) :
        p(parent)
    {
    }

public Q_SLOTS:
    void propertyChanged(const QString& name, const QVariant& value)
    {
        if (name == "") {}

        // Basic properties

        DEFINE_PROPERTY_UPDATE(ca, "ca", toString)
        DEFINE_PROPERTY_UPDATE_ENUM(connectionType, "connectionType", ConnectionType)
        DEFINE_PROPERTY_UPDATE(certPass, "certPass", toString)
        DEFINE_PROPERTY_UPDATE(cert, "cert", toString)
        DEFINE_PROPERTY_UPDATE(key, "key", toString)
        DEFINE_PROPERTY_UPDATE(localIp, "localIp", toString)
        DEFINE_PROPERTY_UPDATE(password, "password", toString)
        DEFINE_PROPERTY_UPDATE(remote, "remote", toString)
        DEFINE_PROPERTY_UPDATE(remoteIp, "remoteIp", toString)
        DEFINE_PROPERTY_UPDATE(staticKey, "staticKey", toString)
        DEFINE_PROPERTY_UPDATE_ENUM(staticKeyDirection, "staticKeyDirection", KeyDir)
        DEFINE_PROPERTY_UPDATE(username, "username", toString)

        // Advanced general properties
        DEFINE_PROPERTY_UPDATE(port, "port", toInt)
        DEFINE_PROPERTY_UPDATE(portSet, "portSet", toBool)
        DEFINE_PROPERTY_UPDATE(renegSeconds, "renegSeconds", toInt)
        DEFINE_PROPERTY_UPDATE(renegSecondsSet, "renegSecondsSet", toBool)
        DEFINE_PROPERTY_UPDATE(compLzo, "compLzo", toBool)
        DEFINE_PROPERTY_UPDATE(protoTcp, "protoTcp", toBool)
        DEFINE_PROPERTY_UPDATE(dev, "dev", toString)
        DEFINE_PROPERTY_UPDATE_ENUM(devType, "devType", DevType)
        DEFINE_PROPERTY_UPDATE(devTypeSet, "devTypeSet", toBool)
        DEFINE_PROPERTY_UPDATE(tunnelMtu, "tunnelMtu", toInt)
        DEFINE_PROPERTY_UPDATE(tunnelMtuSet, "tunnelMtuSet", toBool)
        DEFINE_PROPERTY_UPDATE(fragmentSize, "fragmentSize", toInt)
        DEFINE_PROPERTY_UPDATE(fragmentSizeSet, "fragmentSizeSet", toBool)
        DEFINE_PROPERTY_UPDATE(mssFix, "mssFix", toBool)
        DEFINE_PROPERTY_UPDATE(remoteRandom, "remoteRandom", toBool)

        // Advanced security properties

        DEFINE_PROPERTY_UPDATE_ENUM(cipher, "cipher", Cipher)
        DEFINE_PROPERTY_UPDATE(keysize, "keysize", toInt)
        DEFINE_PROPERTY_UPDATE(keysizeSet, "keysizeSet", toBool)
        DEFINE_PROPERTY_UPDATE_ENUM(auth, "auth", Auth)

        // Advanced TLS auth properties

        DEFINE_PROPERTY_UPDATE(tlsRemote, "tlsRemote", toString)
        DEFINE_PROPERTY_UPDATE_ENUM(remoteCertTls, "remoteCertTls", TlsType)
        DEFINE_PROPERTY_UPDATE(remoteCertTlsSet, "remoteCertTlsSet", toBool)
        DEFINE_PROPERTY_UPDATE(ta, "ta", toString)
        DEFINE_PROPERTY_UPDATE_ENUM(taDir, "taDir", KeyDir)
        DEFINE_PROPERTY_UPDATE(taSet, "taSet", toBool)

        // Advanced proxy settings

        DEFINE_PROPERTY_UPDATE_ENUM(proxyType, "proxyType", ProxyType)
        DEFINE_PROPERTY_UPDATE(proxyServer, "proxyServer", toString)
        DEFINE_PROPERTY_UPDATE(proxyPort, "proxyPort", toInt)
        DEFINE_PROPERTY_UPDATE(proxyRetry, "proxyRetry", toBool)
        DEFINE_PROPERTY_UPDATE(proxyUsername, "proxyUsername", toString)
        DEFINE_PROPERTY_UPDATE(proxyPassword, "proxyPassword", toString)
    }

public:
    OpenvpnConnection& p;

    unique_ptr<ComUbuntuConnectivity1VpnVpnConnectionOpenVpnInterface> m_openvpnInterface;

    util::DBusPropertyCache::UPtr m_propertyCache;
};

OpenvpnConnection::OpenvpnConnection(const QDBusObjectPath& path, const QDBusConnection& connection) :
        VpnConnection(path, connection),
        d(new Priv(*this))
{
    d->m_openvpnInterface = make_unique<
            ComUbuntuConnectivity1VpnVpnConnectionOpenVpnInterface>(
            DBusTypes::DBUS_NAME, path.path(), connection);

    d->m_propertyCache =
            make_unique<util::DBusPropertyCache>(
                    DBusTypes::DBUS_NAME,
                    ComUbuntuConnectivity1VpnVpnConnectionOpenVpnInterface::staticInterfaceName(),
                    path.path(), connection);

    connect(d->m_propertyCache.get(),
                    &util::DBusPropertyCache::propertyChanged, d.get(),
                    &Priv::propertyChanged);
}

OpenvpnConnection::~OpenvpnConnection()
{
}

VpnConnection::Type OpenvpnConnection::type() const
{
    return Type::OPENVPN;
}

// Basic properties

DEFINE_PROPERTY_GETTER(ca, "ca", QString, toString)
DEFINE_PROPERTY_GETTER(cert, "cert", QString, toString)
DEFINE_PROPERTY_GETTER(certPass, "certPass", QString, toString)
DEFINE_PROPERTY_GETTER_ENUM(connectionType, "connectionType", ConnectionType)
DEFINE_PROPERTY_GETTER(key, "key", QString, toString)
DEFINE_PROPERTY_GETTER(localIp, "localIp", QString, toString)
DEFINE_PROPERTY_GETTER(password, "password", QString, toString)
DEFINE_PROPERTY_GETTER(remote, "remote", QString, toString)
DEFINE_PROPERTY_GETTER(remoteIp, "remoteIp", QString, toString)
DEFINE_PROPERTY_GETTER(staticKey, "staticKey", QString, toString)
DEFINE_PROPERTY_GETTER_ENUM(staticKeyDirection, "staticKeyDirection", KeyDir)
DEFINE_PROPERTY_GETTER(username, "username", QString, toString)

// Advanced general properties

DEFINE_PROPERTY_GETTER(port, "port", int, toInt)
DEFINE_PROPERTY_GETTER(portSet, "portSet", bool, toBool)
DEFINE_PROPERTY_GETTER(renegSeconds, "renegSeconds", int, toInt)
DEFINE_PROPERTY_GETTER(renegSecondsSet, "renegSecondsSet", bool, toBool)
DEFINE_PROPERTY_GETTER(compLzo, "compLzo", bool, toBool)
DEFINE_PROPERTY_GETTER(protoTcp, "protoTcp", bool, toBool)
DEFINE_PROPERTY_GETTER(dev, "dev", QString, toString)
DEFINE_PROPERTY_GETTER_ENUM(devType, "devType", DevType)
DEFINE_PROPERTY_GETTER(devTypeSet, "devTypeSet", bool, toBool)
DEFINE_PROPERTY_GETTER(tunnelMtu, "tunnelMtu", int, toInt)
DEFINE_PROPERTY_GETTER(tunnelMtuSet, "tunnelMtuSet", bool, toBool)
DEFINE_PROPERTY_GETTER(fragmentSize, "fragmentSize", int, toInt)
DEFINE_PROPERTY_GETTER(fragmentSizeSet, "fragmentSizeSet", bool, toBool)
DEFINE_PROPERTY_GETTER(mssFix, "mssFix", bool, toBool)
DEFINE_PROPERTY_GETTER(remoteRandom, "remoteRandom", bool, toBool)

// Advanced security properties

DEFINE_PROPERTY_GETTER_ENUM(cipher, "cipher", Cipher)
DEFINE_PROPERTY_GETTER(keysize, "keysize", int, toInt)
DEFINE_PROPERTY_GETTER(keysizeSet, "keysizeSet", bool, toBool)
DEFINE_PROPERTY_GETTER_ENUM(auth, "auth", Auth)

// Advanced TLS auth properties

DEFINE_PROPERTY_GETTER(tlsRemote, "tlsRemote", QString, toString)
DEFINE_PROPERTY_GETTER_ENUM(remoteCertTls, "remoteCertTls", TlsType)
DEFINE_PROPERTY_GETTER(remoteCertTlsSet, "remoteCertTlsSet", bool, toBool)
DEFINE_PROPERTY_GETTER(ta, "ta", QString, toString)
DEFINE_PROPERTY_GETTER_ENUM(taDir, "taDir", KeyDir)
DEFINE_PROPERTY_GETTER(taSet, "taSet", bool, toBool)

// Advanced proxy settings

DEFINE_PROPERTY_GETTER_ENUM(proxyType, "proxyType", ProxyType)
DEFINE_PROPERTY_GETTER(proxyServer, "proxyServer", QString, toString)
DEFINE_PROPERTY_GETTER(proxyPort, "proxyPort", int, toInt)
DEFINE_PROPERTY_GETTER(proxyRetry, "proxyRetry", bool, toBool)
DEFINE_PROPERTY_GETTER(proxyUsername, "proxyUsername", QString, toString)
DEFINE_PROPERTY_GETTER(proxyPassword, "proxyPassword", QString, toString)



// Basic properties

DEFINE_PROPERTY_SETTER(Ca, "ca", const QString &)
DEFINE_PROPERTY_SETTER(Cert, "cert", const QString &)
DEFINE_PROPERTY_SETTER(CertPass, "certPass", const QString &)
DEFINE_PROPERTY_SETTER_ENUM(ConnectionType, "connectionType", ConnectionType)
DEFINE_PROPERTY_SETTER(Key, "key", const QString &)
DEFINE_PROPERTY_SETTER(LocalIp, "localIp", const QString &)
DEFINE_PROPERTY_SETTER(Password, "password", const QString &)
DEFINE_PROPERTY_SETTER(Remote, "remote", const QString &)
DEFINE_PROPERTY_SETTER(RemoteIp, "remoteIp", const QString &)
DEFINE_PROPERTY_SETTER(StaticKey, "staticKey", const QString &)
DEFINE_PROPERTY_SETTER_ENUM(StaticKeyDirection, "staticKeyDirection", KeyDir)
DEFINE_PROPERTY_SETTER(Username, "username", const QString &)

// Advanced general properties

DEFINE_PROPERTY_SETTER(Port, "port", int)
DEFINE_PROPERTY_SETTER(PortSet, "portSet", bool)
DEFINE_PROPERTY_SETTER(RenegSeconds, "renegSeconds", int)
DEFINE_PROPERTY_SETTER(RenegSecondsSet, "renegSecondsSet", bool)
DEFINE_PROPERTY_SETTER(CompLzo, "compLzo", bool)
DEFINE_PROPERTY_SETTER(ProtoTcp, "protoTcp", bool)
DEFINE_PROPERTY_SETTER(Dev, "dev", const QString &)
DEFINE_PROPERTY_SETTER_ENUM(DevType, "devType", DevType)
DEFINE_PROPERTY_SETTER(DevTypeSet, "devTypeSet", bool)
DEFINE_PROPERTY_SETTER(TunnelMtu, "tunnelMtu", int)
DEFINE_PROPERTY_SETTER(TunnelMtuSet, "tunnelMtuSet", bool)
DEFINE_PROPERTY_SETTER(FragmentSize, "fragmentSize", int)
DEFINE_PROPERTY_SETTER(FragmentSizeSet, "fragmentSizeSet", bool)
DEFINE_PROPERTY_SETTER(MssFix, "mssFix", bool)
DEFINE_PROPERTY_SETTER(RemoteRandom, "remoteRandom", bool)

// Advanced security properties

DEFINE_PROPERTY_SETTER_ENUM(Cipher, "cipher", Cipher)
DEFINE_PROPERTY_SETTER(Keysize, "keysize", int)
DEFINE_PROPERTY_SETTER(KeysizeSet, "keysizeSet", bool)
DEFINE_PROPERTY_SETTER_ENUM(Auth, "auth", Auth)

// Advanced TLS auth properties

DEFINE_PROPERTY_SETTER(TlsRemote, "tlsRemote", const QString &)
DEFINE_PROPERTY_SETTER_ENUM(RemoteCertTls, "remoteCertTls", TlsType)
DEFINE_PROPERTY_SETTER(RemoteCertTlsSet, "remoteCertTlsSet", bool)
DEFINE_PROPERTY_SETTER(Ta, "ta", const QString &)
DEFINE_PROPERTY_SETTER_ENUM(TaDir, "taDir", KeyDir)
DEFINE_PROPERTY_SETTER(TaSet, "taSet", bool)

// Advanced proxy settings

DEFINE_PROPERTY_SETTER_ENUM(ProxyType, "proxyType", ProxyType)
DEFINE_PROPERTY_SETTER(ProxyServer, "proxyServer", const QString &)
DEFINE_PROPERTY_SETTER(ProxyPort, "proxyPort", int)
DEFINE_PROPERTY_SETTER(ProxyRetry, "proxyRetry", bool)
DEFINE_PROPERTY_SETTER(ProxyUsername, "proxyUsername", const QString &)
DEFINE_PROPERTY_SETTER(ProxyPassword, "proxyPassword", const QString &)

}

#include "openvpn-connection.moc"
