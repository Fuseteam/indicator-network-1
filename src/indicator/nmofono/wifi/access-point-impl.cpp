/*
 * Copyright © 2013 Canonical Ltd.
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

#include <nmofono/wifi/access-point-impl.h>

#include <QTextCodec>
#include <NetworkManager.h>

namespace platform {
namespace nmofono {
namespace wifi {


AccessPoint::AccessPoint(std::shared_ptr<OrgFreedesktopNetworkManagerAccessPointInterface> ap)
        : m_ap(ap)
{
    uint flags = m_ap->flags();
    uint mode = m_ap->mode();

    m_secured = (flags == NM_802_11_AP_FLAGS_PRIVACY);
    /// @todo check for the other modes also..
    m_adhoc = (mode != NM_802_11_MODE_INFRA);

    QString ssid;
    // Note: raw_ssid is _not_ guaranteed to be null terminated.
    m_raw_ssid = m_ap->ssid();

    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    const QString text = codec->toUnicode(m_raw_ssid.constData(), m_raw_ssid.size(), &state);
    if (state.invalidChars > 0)
    {
        for (auto c : m_raw_ssid) {
            if (isprint(c)) {
                ssid += (char)c;
            } else {
                // contains unprintable characters
                /// @todo do something more elegant
                ssid += u8"�";
            }
        }
    }
    else
    {
        ssid = QString::fromUtf8(m_raw_ssid);
    }

    m_ssid = ssid;

    m_strength = m_ap->strength();

    connect(m_ap.get(), &OrgFreedesktopNetworkManagerAccessPointInterface::PropertiesChanged, this, &AccessPoint::ap_properties_changed);

    m_flags = flags;
    /* NetworkManager seems to set the wpa and rns flags
     * for AccessPoints on the same network in a total random manner.
     * Sometimes only wpa_flags or rns_flags is set and sometimes
     * they both are set but always to the same value
     */
    m_secflags = m_ap->wpaFlags() | m_ap->rsnFlags();
    m_mode = mode;
}

void AccessPoint::ap_properties_changed(const QVariantMap &properties)
{
    auto strengthIt = properties.find("Strength");
    if (strengthIt != properties.cend())
    {
        m_strength = qvariant_cast<uchar>(*strengthIt);
        Q_EMIT strengthUpdated(m_strength);
    }
}

QDBusObjectPath AccessPoint::object_path() const {
    return QDBusObjectPath(m_ap->path());
}

double AccessPoint::strength() const
{
    return m_strength;
}

std::chrono::system_clock::time_point AccessPoint::lastConnected() const
{
    return m_lastConnected;
}

QString AccessPoint::ssid() const
{
    return m_ssid;
}

QByteArray AccessPoint::raw_ssid() const
{
    return m_raw_ssid;
}

bool AccessPoint::secured() const
{
    return m_secured;
}

bool AccessPoint::adhoc() const
{
    return m_adhoc;
}

bool AccessPoint::operator==(const platform::nmofono::wifi::AccessPoint &other) const {
    if(this == &other)
        return true;
    return m_ssid == other.m_ssid &&
            m_flags == other.m_flags &&
            m_secflags == other.m_secflags &&
            m_mode == other.m_mode;
}

}
}
}
