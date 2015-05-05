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

#include <nmofono/wifi/grouped-access-point-impl.h>
#include <nmofono/wifi/access-point-impl.h>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cassert>

namespace platform {
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
    std::vector<std::shared_ptr<platform::nmofono::wifi::AccessPoint>> aplist;

    double m_strength;
    std::chrono::system_clock::time_point m_lastTime;

    void add_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> ap);
    void remove_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> ap);
    bool has_object(const QDBusObjectPath &p) const;

    void setStrength(double s)
    {
        m_strength = s;
        Q_EMIT m_parent.strengthUpdated(m_strength);
    }

    void setLastTime(std::chrono::system_clock::time_point newTime)
    {
        m_lastTime = newTime;
        Q_EMIT m_parent.lastConnectedUpdated(m_lastTime);
    }

public Q_SLOTS:
    void update_strength(double);
    void update_lasttime(std::chrono::system_clock::time_point newTime);
};

void GroupedAccessPoint::Private::add_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> ap)
{
    if(!aplist.empty())
    {
        // We should check for all attributes but deduplicating logic
        // is elsewhere so it is enough to just guard against simple mistakes.
        if(aplist[0]->ssid() != ap->ssid())
        {
            throw std::runtime_error("Tried to merge two access points from different networks.");
        }
    }
    aplist.push_back(ap);
    if (ap->strength() > m_strength)
    {
        setStrength(ap->strength());
    }
    update_lasttime(ap->lastConnected());
    connect(ap.get(), &connectivity::networking::wifi::AccessPoint::strengthUpdated, this, &Private::update_strength);
    connect(ap.get(), &platform::nmofono::wifi::AccessPoint::lastConnectedUpdated, this, &Private::update_lasttime);
}

void GroupedAccessPoint::Private::remove_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> ap) {
    std::vector<std::shared_ptr<platform::nmofono::wifi::AccessPoint>> new_aps;
    for(const auto &i: aplist) {
        if(i->object_path() != ap->object_path()) {
            new_aps.push_back(i);
        }
    }
    assert(!new_aps.empty());
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

void GroupedAccessPoint::Private::update_lasttime(std::chrono::system_clock::time_point newTime)
{
    if(newTime > m_lastTime)
    {
        setLastTime(newTime);
    }
}

void GroupedAccessPoint::Private::update_strength(double)
{
    auto nselem = std::max_element(aplist.begin(), aplist.end(), [](
                  const std::shared_ptr<platform::nmofono::wifi::AccessPoint> &a,
                  std::shared_ptr<platform::nmofono::wifi::AccessPoint> &b) {
        return a->strength() < b->strength(); });
    double newstrength = (*nselem)->strength();
    if(std::abs(newstrength - m_strength) > 0.01) {
        setStrength(newstrength);
    }
}

bool GroupedAccessPoint::Private::has_object(const QDBusObjectPath &p) const {
    for(const auto &i : aplist) {
        if(i->object_path() == p) {
            return true;
        }
    }
    return false;
}

GroupedAccessPoint::GroupedAccessPoint(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap)
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

std::chrono::system_clock::time_point GroupedAccessPoint::lastConnected() const
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

bool GroupedAccessPoint::adhoc() const
{
    if (p->aplist.empty())
    {
        return false;
    }

    return p->aplist.at(0)->adhoc();
}

void GroupedAccessPoint::add_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap) {
    p->add_ap(ap);
}

void GroupedAccessPoint::remove_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap) {
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
}

#include "grouped-access-point-impl.moc"
