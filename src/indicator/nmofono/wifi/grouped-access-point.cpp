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
 *     Jussi Pakkanen <jussi.pakkanen@canonical.com>
 */

#include <nmofono/wifi/grouped-access-point.h>
#include <nmofono/wifi/access-point-impl.h>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cassert>

using namespace std;

namespace nmofono {
namespace wifi {

class GroupedAccessPoint::Private : public QObject
{
    Q_OBJECT

public:
    Private(GroupedAccessPoint& parent) :
            m_parent(parent), m_strength(.0)
    {
    }

    ~Private() = default;
    GroupedAccessPoint& m_parent;
    vector<AccessPointImpl::Ptr> aplist;

    double m_strength;
    chrono::system_clock::time_point m_lastTime;

    void add_ap(AccessPointImpl::Ptr ap)
    {
        if(!aplist.empty())
        {
            // We should check for all attributes but deduplicating logic
            // is elsewhere so it is enough to just guard against simple mistakes.
            if(aplist[0]->ssid() != ap->ssid())
            {
                throw runtime_error("Tried to merge two access points from different networks.");
            }
        }
        aplist.push_back(ap);
        if (ap->strength() > m_strength)
        {
            setStrength(ap->strength());
        }
        update_lasttime(ap->lastConnected());
        connect(ap.get(), &AccessPoint::strengthUpdated, this, &Private::update_strength);
        connect(ap.get(), &AccessPointImpl::lastConnectedUpdated, this, &Private::update_lasttime);
    }

    void remove_ap(AccessPointImpl::Ptr ap) {
        vector<AccessPointImpl::Ptr> new_aps;
        for(const auto &i: aplist) {
            if(i->object_path() != ap->object_path()) {
                new_aps.push_back(i);
            }
        }
        if(new_aps.size() >= aplist.size()) {
            qWarning() << "Tried to remove an AP that has not been added.";
            return;
        }
        aplist.clear();
        setStrength(.0);
        // Do not reset lasttime because it does not change.
        for(auto &i : new_aps) {
            add_ap(i);
        }

    }

    bool has_object(const QDBusObjectPath &p) const {
        for(const auto &i : aplist) {
            if(i->object_path() == p) {
                return true;
            }
        }
        return false;
    }

    void setStrength(double s)
    {
        m_strength = s;
        Q_EMIT m_parent.strengthUpdated(m_strength);
    }

    void setLastTime(chrono::system_clock::time_point newTime)
    {
        m_lastTime = newTime;
        Q_EMIT m_parent.lastConnectedUpdated(m_lastTime);
    }

public Q_SLOTS:
    void update_lasttime(chrono::system_clock::time_point newTime)
    {
        if(newTime > m_lastTime)
        {
            setLastTime(newTime);
        }
    }

    void update_strength(double)
    {
        auto nselem = max_element(aplist.begin(), aplist.end(), [](
                      const AccessPointImpl::Ptr &a,
                      const AccessPointImpl::Ptr &b) {
            return a->strength() < b->strength(); });
        double newstrength = (*nselem)->strength();
        if(abs(newstrength - m_strength) > 0.01) {
            setStrength(newstrength);
        }
    }
};

GroupedAccessPoint::GroupedAccessPoint(const AccessPointImpl::Ptr &ap)
        : p(new Private(*this))
{
    p->add_ap(ap);
}

GroupedAccessPoint::~GroupedAccessPoint() {
}

QDBusObjectPath GroupedAccessPoint::object_path() const {
    if (p->aplist.empty())
    {
        return QDBusObjectPath("/");
    }

    return p->aplist.at(0)->object_path();
}

double GroupedAccessPoint::strength() const
{
    return p->m_strength;
}

chrono::system_clock::time_point GroupedAccessPoint::lastConnected() const
{
    return p->m_lastTime;
}

QString GroupedAccessPoint::ssid() const
{
    if (p->aplist.empty())
    {
        return QString();
    }

    return p->aplist.at(0)->ssid();
}

QString GroupedAccessPoint::bssid() const
{
    if (p->aplist.empty())
    {
        return QString();
    }

    return p->aplist.at(0)->bssid();
}


QByteArray GroupedAccessPoint::raw_ssid() const
{
    if (p->aplist.empty())
    {
        return QByteArray();
    }

    return p->aplist.at(0)->raw_ssid();
}

bool GroupedAccessPoint::secured() const
{
    if (p->aplist.empty())
    {
        return false;
    }

    return p->aplist.at(0)->secured();
}

AccessPoint::KeyManagementType GroupedAccessPoint::keyManagementType() const
{
    if (p->aplist.empty())
    {
        return KeyManagementType::psk;
    }

    return p->aplist.at(0)->keyManagementType();
}

bool GroupedAccessPoint::adhoc() const
{
    if (p->aplist.empty())
    {
        return false;
    }

    return p->aplist.at(0)->adhoc();
}

void GroupedAccessPoint::add_ap(AccessPointImpl::Ptr &ap) {
    p->add_ap(ap);
}

void GroupedAccessPoint::remove_ap(AccessPointImpl::Ptr &ap) {
    p->remove_ap(ap);
}

int GroupedAccessPoint::num_aps() const {
    return (int)p->aplist.size();
}

bool GroupedAccessPoint::has_object(const QDBusObjectPath &path) const {
    return p->has_object(path);
}

}
}

#include "grouped-access-point.moc"
