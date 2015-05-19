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

namespace nmofono {
namespace wifi {

AccessPointImpl::AccessPointImpl(std::shared_ptr<OrgFreedesktopNetworkManagerAccessPointInterface> ap)
        : m_ap(ap)
{
    uint mode = m_ap->mode();


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

    connect(m_ap.get(), &OrgFreedesktopNetworkManagerAccessPointInterface::PropertiesChanged, this, &AccessPointImpl::ap_properties_changed);

    /* NetworkManager seems to set the wpa and rns flags
     * for AccessPoints on the same network in a total random manner.
     * Sometimes only wpa_flags or rns_flags is set and sometimes
     * they both are set but always to the same value
     */
    m_secflags = m_ap->wpaFlags() | m_ap->rsnFlags();
    m_mode = mode;

    m_secured = (m_secflags != NM_802_11_AP_SEC_NONE);
}

void AccessPointImpl::ap_properties_changed(const QVariantMap &properties)
{
    auto strengthIt = properties.find("Strength");
    if (strengthIt != properties.cend())
    {
        m_strength = qvariant_cast<int8_t>(*strengthIt);
        Q_EMIT strengthUpdated(m_strength);
    }
}

QDBusObjectPath AccessPointImpl::object_path() const {
    return QDBusObjectPath(m_ap->path());
}

double AccessPointImpl::strength() const
{
    return m_strength;
}

std::chrono::system_clock::time_point AccessPointImpl::lastConnected() const
{
    return m_lastConnected;
}

QString AccessPointImpl::ssid() const
{
    return m_ssid;
}

QByteArray AccessPointImpl::raw_ssid() const
{
    return m_raw_ssid;
}

bool AccessPointImpl::secured() const
{
    return m_secured;
}

bool AccessPointImpl::adhoc() const
{
    return m_adhoc;
}

uint32_t AccessPointImpl::secflags() const
{
    return m_secflags;
}

uint32_t AccessPointImpl::mode() const
{
    return m_mode;
}

bool AccessPointImpl::operator==(const AccessPointImpl &other) const {
    if(this == &other)
        return true;
    return m_ssid == other.m_ssid &&
            m_secflags == other.m_secflags &&
            m_mode == other.m_mode;
}

}
}
