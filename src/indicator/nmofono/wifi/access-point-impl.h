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

#pragma once

#include <nmofono/wifi/access-point.h>

#include <NetworkManagerAccessPointInterface.h>
#include <NetworkManagerActiveConnectionInterface.h>

#include <chrono>

namespace nmofono {
namespace wifi {

class AccessPointImpl : public AccessPoint
{
    Q_OBJECT

public:
    typedef std::shared_ptr<AccessPointImpl> Ptr;

    struct Key {
        QString ssid;
        uint32_t secflags;
        uint32_t mode;

        bool operator<(const Key &other) const
        {
            // Standard lexigraphic comparison.
            if(ssid < other.ssid)
                return true;
            if(ssid > other.ssid)
                return false;

            if(secflags  < other.secflags)
                return true;
            if(secflags > other.secflags)
                return false;

            if(mode < other.mode)
                return true;
            if(mode > other.mode)
                return false;
            return false;
        }

        Key() = delete;
        Key(const AccessPointImpl::Ptr &curap)
        {
            ssid = curap->ssid();
            secflags = curap->m_secflags;
            mode = curap->m_mode;
        }
    };
    friend struct Key;


    AccessPointImpl(std::shared_ptr<OrgFreedesktopNetworkManagerAccessPointInterface> ap);
    double strength() const override;
    virtual ~AccessPointImpl() = default;

    // time when last connected to this access point
    // for APs that have never been connected the
    // lastConnected->time_since_epoch().count() is 0
    Q_PROPERTY(std::chrono::system_clock::time_point lastConnected READ lastConnected NOTIFY lastConnectedUpdated)
    std::chrono::system_clock::time_point lastConnected() const;

    QString ssid() const override;
    QString bssid() const override;
    QByteArray raw_ssid() const override;

    std::uint32_t secflags() const;
    std::uint32_t mode() const;

    bool secured() const override;
    KeyManagementType keyManagementType() const override;
    bool adhoc() const override;

    QDBusObjectPath object_path() const override;

    bool operator==(const AccessPointImpl &other) const;
    bool operator!=(const AccessPointImpl &other) const { return !(*this == other); }

Q_SIGNALS:
    void lastConnectedUpdated(std::chrono::system_clock::time_point lastConnected);

private Q_SLOTS:
    void ap_properties_changed(const QVariantMap &properties);

private:
    double m_strength;
    std::chrono::system_clock::time_point m_lastConnected;
    std::shared_ptr<OrgFreedesktopNetworkManagerAccessPointInterface> m_ap;
    QString m_ssid;
    QString m_bssid;
    QByteArray m_raw_ssid;
    bool m_secured;
    bool m_adhoc;

    std::uint32_t m_secflags;
    std::uint32_t m_mode;
};

}
}
