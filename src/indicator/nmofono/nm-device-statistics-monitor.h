/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#pragma once

#include <QObject>

#include "link.h"

#include <memory>

namespace nmofono
{

class NMDeviceStatisticsMonitor : public QObject
{
    Q_OBJECT

    class Private;
    std::shared_ptr<Private> d;

public:

    typedef std::shared_ptr<NMDeviceStatisticsMonitor> Ptr;
    typedef std::weak_ptr<NMDeviceStatisticsMonitor> WeakPtr;

    Q_PROPERTY(bool tx READ tx NOTIFY txChanged)
    virtual bool tx() const;

    Q_PROPERTY(bool rx READ rx NOTIFY rxChanged)
    virtual bool rx() const;

    NMDeviceStatisticsMonitor();
    ~NMDeviceStatisticsMonitor();

    void addLink(Link::SPtr link);

    void remove(const QString &nmPath);

Q_SIGNALS:

    void txChanged();
    void rxChanged();
};

}
