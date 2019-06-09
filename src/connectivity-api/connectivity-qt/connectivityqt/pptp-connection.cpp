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

#include <connectivityqt/pptp-connection.h>
#include <util/dbus-property-cache.h>
#include <dbus-types.h>

#include <PptpConnectionInterface.h>

using namespace std;

#define DEFINE_PROPERTY_GETTER(name, strname, type, conversion)\
type PptpConnection::name() const\
{\
    return d->m_propertyCache->get(strname).conversion();\
}\

#define DEFINE_PROPERTY_GETTER_ENUM(name, strname, type)\
PptpConnection::type PptpConnection::name() const\
{\
    return static_cast<type>(d->m_propertyCache->get(strname).toInt());\
}

#define DEFINE_PROPERTY_SETTER(uppername, strname, type)\
void PptpConnection::set##uppername(type value)\
{\
    d->m_propertyCache->set(strname, value);\
}

#define DEFINE_PROPERTY_SETTER_ENUM(uppername, strname, type)\
void PptpConnection::set##uppername(type value)\
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

class PptpConnection::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(PptpConnection& parent) :
        p(parent)
    {
    }

public Q_SLOTS:
    void propertyChanged(const QString& name, const QVariant& value)
    {
        if (name == "") {}

        // Basic properties

        DEFINE_PROPERTY_UPDATE(gateway, "gateway", toString)
        DEFINE_PROPERTY_UPDATE(user, "user", toString)
        DEFINE_PROPERTY_UPDATE(password, "password", toString)
        DEFINE_PROPERTY_UPDATE(domain, "domain", toString)

        // Advanced properties

        DEFINE_PROPERTY_UPDATE(allowPap, "allowPap", toBool)
        DEFINE_PROPERTY_UPDATE(allowChap, "allowChap", toBool)
        DEFINE_PROPERTY_UPDATE(allowMschap, "allowMschap", toBool)
        DEFINE_PROPERTY_UPDATE(allowMschapv2, "allowMschapv2", toBool)
        DEFINE_PROPERTY_UPDATE(allowEap, "allowEap", toBool)
        DEFINE_PROPERTY_UPDATE(requireMppe, "requireMppe", toBool)
        DEFINE_PROPERTY_UPDATE_ENUM(mppeType, "mppeType", MppeType)
        DEFINE_PROPERTY_UPDATE(mppeStateful, "mppeStateful", toBool)
        DEFINE_PROPERTY_UPDATE(bsdCompression, "bsdCompression", toBool)
        DEFINE_PROPERTY_UPDATE(deflateCompression, "deflateCompression", toBool)
        DEFINE_PROPERTY_UPDATE(tcpHeaderCompression, "tcpHeaderCompression", toBool)
        DEFINE_PROPERTY_UPDATE(sendPppEchoPackets, "sendPppEchoPackets", toBool)
    }

public:
    PptpConnection& p;

    unique_ptr<ComUbuntuConnectivity1VpnVpnConnectionPptpInterface> m_pptpInterface;

    util::DBusPropertyCache::UPtr m_propertyCache;
};

PptpConnection::PptpConnection(const QDBusObjectPath& path, const QDBusConnection& connection) :
        VpnConnection(path, connection),
        d(new Priv(*this))
{
    d->m_pptpInterface = make_unique<
            ComUbuntuConnectivity1VpnVpnConnectionPptpInterface>(
            DBusTypes::DBUS_NAME, path.path(), connection);

    d->m_propertyCache =
            make_unique<util::DBusPropertyCache>(
                    DBusTypes::DBUS_NAME,
                    ComUbuntuConnectivity1VpnVpnConnectionPptpInterface::staticInterfaceName(),
                    path.path(), connection);

    connect(d->m_propertyCache.get(),
                    &util::DBusPropertyCache::propertyChanged, d.get(),
                    &Priv::propertyChanged);
}

PptpConnection::~PptpConnection()
{
}

VpnConnection::Type PptpConnection::type() const
{
    return Type::PPTP;
}

// Basic properties

DEFINE_PROPERTY_GETTER(gateway, "gateway", QString, toString)
DEFINE_PROPERTY_GETTER(user, "user", QString, toString)
DEFINE_PROPERTY_GETTER(password, "password", QString, toString)
DEFINE_PROPERTY_GETTER(domain, "domain", QString, toString)

// Advanced properties

DEFINE_PROPERTY_GETTER(allowPap, "allowPap", bool, toBool)
DEFINE_PROPERTY_GETTER(allowChap, "allowChap", bool, toBool)
DEFINE_PROPERTY_GETTER(allowMschap, "allowMschap", bool, toBool)
DEFINE_PROPERTY_GETTER(allowMschapv2, "allowMschapv2", bool, toBool)
DEFINE_PROPERTY_GETTER(allowEap, "allowEap", bool, toBool)
DEFINE_PROPERTY_GETTER(requireMppe, "requireMppe", bool, toBool)
DEFINE_PROPERTY_GETTER_ENUM(mppeType, "mppeType", MppeType)
DEFINE_PROPERTY_GETTER(mppeStateful, "mppeStateful", bool, toBool)
DEFINE_PROPERTY_GETTER(bsdCompression, "bsdCompression", bool, toBool)
DEFINE_PROPERTY_GETTER(deflateCompression, "deflateCompression", bool, toBool)
DEFINE_PROPERTY_GETTER(tcpHeaderCompression, "tcpHeaderCompression", bool, toBool)
DEFINE_PROPERTY_GETTER(sendPppEchoPackets, "sendPppEchoPackets", bool, toBool)



// Basic properties

DEFINE_PROPERTY_SETTER(Gateway, "gateway", const QString &)
DEFINE_PROPERTY_SETTER(User, "user", const QString &)
DEFINE_PROPERTY_SETTER(Password, "password", const QString &)
DEFINE_PROPERTY_SETTER(Domain, "domain", const QString &)

// Advanced properties

DEFINE_PROPERTY_SETTER(AllowPap, "allowPap", bool)
DEFINE_PROPERTY_SETTER(AllowChap, "allowChap", bool)
DEFINE_PROPERTY_SETTER(AllowMschap, "allowMschap", bool)
DEFINE_PROPERTY_SETTER(AllowMschapv2, "allowMschapv2", bool)
DEFINE_PROPERTY_SETTER(AllowEap, "allowEap", bool)
DEFINE_PROPERTY_SETTER(RequireMppe, "requireMppe", bool)
DEFINE_PROPERTY_SETTER_ENUM(MppeType, "mppeType", MppeType)
DEFINE_PROPERTY_SETTER(MppeStateful, "mppeStateful", bool)
DEFINE_PROPERTY_SETTER(BsdCompression, "bsdCompression", bool)
DEFINE_PROPERTY_SETTER(DeflateCompression, "deflateCompression", bool)
DEFINE_PROPERTY_SETTER(TcpHeaderCompression, "tcpHeaderCompression", bool)
DEFINE_PROPERTY_SETTER(SendPppEchoPackets, "sendPppEchoPackets", bool)

}

#include "pptp-connection.moc"
