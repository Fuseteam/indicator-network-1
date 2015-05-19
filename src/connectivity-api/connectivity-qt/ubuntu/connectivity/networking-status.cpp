/*
 * Copyright © 2014 Canonical Ltd.
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

#include <ubuntu/connectivity/networking-status.h>
#include <connectivityqt/connectivity.h>

#include <QDebug>

using namespace std;

namespace ubuntu
{
namespace connectivity
{

class Q_DECL_HIDDEN NetworkingStatus::Private: public QObject
{
    Q_OBJECT

public:
    NetworkingStatus& p;

    connectivityqt::Connectivity::SPtr m_connectivity;

    Private(NetworkingStatus& parent) :
        p(parent)
    {
        m_connectivity = make_shared<connectivityqt::Connectivity>();

        connect(m_connectivity.get(),
                &connectivityqt::Connectivity::limitationsUpdated, this,
                &Private::limitationsUpdated);

        connect(m_connectivity.get(),
                &connectivityqt::Connectivity::statusUpdated, this,
                &Private::statusUpdated);
    }

public Q_SLOTS:
    void limitationsUpdated(const QVector<connectivityqt::Connectivity::Limitations>&)
    {
        Q_EMIT p.limitationsChanged();
    }

    void statusUpdated(connectivityqt::Connectivity::Status value)
    {
        Q_EMIT p.statusChanged(static_cast<NetworkingStatus::Status>(value));
    }
};

NetworkingStatus::NetworkingStatus(QObject *parent)
        : QObject(parent),
          d{new Private{*this}}
{
    qRegisterMetaType<ubuntu::connectivity::NetworkingStatus::Limitations>();
    qRegisterMetaType<QVector<ubuntu::connectivity::NetworkingStatus::Limitations>>();
    qRegisterMetaType<ubuntu::connectivity::NetworkingStatus::Status>();
}

NetworkingStatus::~NetworkingStatus()
{}

QVector<NetworkingStatus::Limitations>
NetworkingStatus::limitations() const
{
    QVector<NetworkingStatus::Limitations> result;
    for (const auto& limitation : d->m_connectivity->limitations())
    {
        result.push_back(
                static_cast<NetworkingStatus::Limitations>(limitation));
    }
    return result;
}

NetworkingStatus::Status
NetworkingStatus::status() const
{
    return static_cast<NetworkingStatus::Status>(d->m_connectivity->status());
}

}
}

#include "networking-status.moc"
