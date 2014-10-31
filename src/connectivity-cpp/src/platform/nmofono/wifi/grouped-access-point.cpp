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

#include "grouped-access-point.h"
#include "access-point.h"
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <mutex>

namespace platform {
namespace nmofono {
namespace wifi {


struct GroupedAccessPoint::Private {
    ~Private();
    std::vector<std::shared_ptr<platform::nmofono::wifi::AccessPoint>> aplist;

    std::map<std::shared_ptr<platform::nmofono::wifi::AccessPoint>, core::Connection> conns1;
    std::map<std::shared_ptr<platform::nmofono::wifi::AccessPoint>, core::Connection> conns2;
    core::Property<double> strength;
    core::Property<std::chrono::system_clock::time_point> lastTime;
    std::mutex m;

    void add_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap);
    void remove_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap);
    void update_strength(double);
    void update_lasttime(std::chrono::system_clock::time_point newTime);
    void disconnect(const std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap);
    bool has_object(const core::dbus::types::ObjectPath &p) const;
};

GroupedAccessPoint::Private::~Private() {
    try
    {
        for(const auto &i : aplist)
        {
            disconnect(i);
        }
    } catch(...) {
        std::cerr << "Problem in destructor " << __PRETTY_FUNCTION__ << std::endl;
    }
}

void GroupedAccessPoint::Private::disconnect(const std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap) {
    assert(conns1.find(ap) != conns1.end());
    conns1.find(ap)->second.disconnect();
    conns1.erase(ap);
    assert(conns2.find(ap) != conns2.end());
    conns2.find(ap)->second.disconnect();
}

void GroupedAccessPoint::Private::add_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap)
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
    if(ap->strength().get() > strength.get())
    {
        strength.set(ap->strength().get());
    }
    update_lasttime(ap->lastConnected().get());
    conns1.insert(std::make_pair(ap, ap->strength().changed().connect([this](double newValue) { this->update_strength(newValue); })));
    conns2.insert(std::make_pair(ap, ap->lastConnected().changed().connect([this](std::chrono::system_clock::time_point newTime) { this->update_lasttime(newTime); })));
}

void GroupedAccessPoint::Private::remove_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap) {
    std::vector<std::shared_ptr<platform::nmofono::wifi::AccessPoint>> new_aps;
    for(const auto &i: aplist) {
        if(i->object_path() != ap->object_path()) {
            new_aps.push_back(i);
        }
    }
    assert(!new_aps.empty());
    if(new_aps.size() >= aplist.size()) {
        std::cerr << "Tried to remove an AP that has not been added." << std::endl;
        return;
    }
    disconnect(ap);
    conns2.erase(ap);
    aplist.clear();
    strength.set(0);
    // Do not reset lasttime because it does not change.
    for(auto &i : new_aps) {
        add_ap(i);
    }

}

void GroupedAccessPoint::Private::update_lasttime(std::chrono::system_clock::time_point newTime)
{
    if(newTime > lastTime.get())
    {
        lastTime.set(newTime);
    }
}

void GroupedAccessPoint::Private::update_strength(double)
{
    auto nselem = std::max_element(aplist.begin(), aplist.end(), [](
                  const std::shared_ptr<platform::nmofono::wifi::AccessPoint> &a,
                  std::shared_ptr<platform::nmofono::wifi::AccessPoint> &b) {
        return a->strength().get() < b->strength().get(); });
    double newstrength = (*nselem)->strength().get();
    if(std::abs(newstrength - strength.get()) > 0.01) {
        strength.set(newstrength);
    }
}

bool GroupedAccessPoint::Private::has_object(const core::dbus::types::ObjectPath &p) const {
    for(const auto &i : aplist) {
        if(i->object_path() == p) {
            return true;
        }
    }
    return false;
}

GroupedAccessPoint::GroupedAccessPoint(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap)
        : p(new Private())
{
    p->add_ap(ap);
}

GroupedAccessPoint::~GroupedAccessPoint() {

}

const core::dbus::types::ObjectPath GroupedAccessPoint::object_path() const {
    std::lock_guard<std::mutex> l(p->m);
    return p->aplist.at(0)->object_path();
}

const core::Property<double>& GroupedAccessPoint::strength() const
{
    std::lock_guard<std::mutex> l(p->m);
    return p->strength;
}

const core::Property<std::chrono::system_clock::time_point>& GroupedAccessPoint::lastConnected() const
{
    std::lock_guard<std::mutex> l(p->m);
    return p->lastTime;
}

const std::string& GroupedAccessPoint::ssid() const
{
    std::lock_guard<std::mutex> l(p->m);
    return p->aplist.at(0)->ssid();
}

const std::vector<std::int8_t>& GroupedAccessPoint::raw_ssid() const
{
    std::lock_guard<std::mutex> l(p->m);
    return p->aplist.at(0)->raw_ssid();
}

bool GroupedAccessPoint::secured() const
{
    std::lock_guard<std::mutex> l(p->m);
    return p->aplist.at(0)->secured();
}

bool GroupedAccessPoint::adhoc() const
{
    std::lock_guard<std::mutex> l(p->m);
    return p->aplist.at(0)->adhoc();
}

void GroupedAccessPoint::add_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap) {
    std::lock_guard<std::mutex> l(p->m);
    p->add_ap(ap);
}

void GroupedAccessPoint::remove_ap(std::shared_ptr<platform::nmofono::wifi::AccessPoint> &ap) {
    std::lock_guard<std::mutex> l(p->m);
    p->remove_ap(ap);
}

int GroupedAccessPoint::num_aps() const {
    std::lock_guard<std::mutex> l(p->m);
    return (int)p->aplist.size();
}

bool GroupedAccessPoint::has_object(const core::dbus::types::ObjectPath &path) const {
    std::lock_guard<std::mutex> l(p->m);
    return p->has_object(path);
}


}
}
}
