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

#include "access-point.h"
#include <glib.h>

namespace platform {
namespace nmofono {
namespace wifi {


AccessPoint::AccessPoint(const org::freedesktop::NetworkManager::Interface::AccessPoint &ap)
        : m_ap(ap)
{
    m_secured = m_ap.flags->get() == NM_802_11_AP_FLAGS_PRIVACY;
    /// @todo check for the other modes also..
    m_adhoc = m_ap.mode->get() != NM_802_11_MODE_INFRA;

    std::string ssid;
    // Note: raw_ssid is _not_ guaranteed to be null terminated.
    m_raw_ssid = m_ap.ssid->get();
    if(g_utf8_validate((const char*)(&m_raw_ssid[0]), m_raw_ssid.size(), nullptr)) {
        ssid = std::string(m_raw_ssid.begin(), m_raw_ssid.end());
    } else {
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
    m_ssid = ssid;

    m_strength.set(m_ap.strength->get());
    m_ap.properties_changed->connect([this](org::freedesktop::NetworkManager::Interface::AccessPoint::Signal::PropertiesChanged::ArgumentType map){
        for (const auto &entry : map) {
            if (entry.first == "Strength") {
                m_strength.set(entry.second.as<std::int8_t>());
            }
        }
    });

    m_flags = m_ap.flags->get();
    /* NetworkManager seems to set the wpa and rns flags
     * for AccessPoints on the same network in a total random manner.
     * Sometimes only wpa_flags or rns_flags is set and sometimes
     * they both are set but always to the same value
     */
    m_secflags = m_ap.wpa_flags->get()|m_ap.rsn_flags->get();
    m_mode = m_ap.mode->get();
}

const core::dbus::types::ObjectPath AccessPoint::object_path() const {
    return m_ap.object->path();
}

const core::Property<double>& AccessPoint::strength() const
{
    return m_strength;
}

const core::Property<std::chrono::system_clock::time_point>& AccessPoint::lastConnected() const
{
    return m_lastConnected;
}

const std::string& AccessPoint::ssid() const
{
    return m_ssid;
}

const std::vector<std::int8_t>& AccessPoint::raw_ssid() const
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
