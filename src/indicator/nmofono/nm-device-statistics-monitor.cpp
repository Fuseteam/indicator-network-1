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

#include <nmofono/nm-device-statistics-monitor.h>
#include <nmofono/ethernet/ethernet-link.h>

#include <NetworkManager.h>

#include <QDBusInterface>

#include <QMap>
#include <QDebug>

#include <NetworkManagerDeviceStatisticsInterface.h>

#include <gio/gio.h>
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#include "wifi/wifi-link-impl.h"
#include "wwan/modem.h"

using namespace std;

namespace nmofono
{

class NMDeviceStatisticsMonitor::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT

    NMDeviceStatisticsMonitor& p;

public:

    QMap<QString, std::shared_ptr<OrgFreedesktopNetworkManagerDeviceStatisticsInterface>> m_interfaces;
    QTimer m_txTimer;
    QTimer m_rxTimer;

    bool m_tx{false};
    bool m_rx{false};

    bool m_enabled{false};

    /***************/
    // This QDBusInterface magic is blatantly adapted from unity8: src/plugins/Powerd/Powerd.cpp
    // BTW: the service com.canonical.Unity.Screen is provided by lp:repowerd
    // BUGISH: no xml2qdbus goodness for us
    //         https://bugs.launchpad.net/ubuntu/+source/repowerd/+bug/1637730

    enum Status {
        Off,
        On,
    };

    QDBusInterface m_unityScreen{QStringLiteral("com.canonical.Unity.Screen"),
                                 QStringLiteral("/com/canonical/Unity/Screen"),
                                 QStringLiteral("com.canonical.Unity.Screen"),
                                 QDBusConnection::systemBus()};
    Status m_status {Status::On}; // fingers crossed..
                                  // BUG: https://bugs.launchpad.net/ubuntu/+source/repowerd/+bug/1637722
    /***************/

    Private(NMDeviceStatisticsMonitor& parent)
        : p(parent)
    {
        GSettings *settings{nullptr};
        if (qEnvironmentVariableIsSet("INDICATOR_NETWORK_UNDER_TESTING"))
        {
            Q_ASSERT(qEnvironmentVariableIsSet("INDICATOR_NETWORK_TESTING_GSETTINGS_INI"));
            Q_ASSERT(qEnvironmentVariableIsSet("GSETTINGS_SCHEMA_DIR"));

            GSettingsBackend *backend = g_keyfile_settings_backend_new(qgetenv("INDICATOR_NETWORK_TESTING_GSETTINGS_INI"),
                                                                       "/com/canonical/indicator/network/",
                                                                       "root");

            settings = g_settings_new_with_backend("com.canonical.indicator.network",
                                                   backend);
            g_object_unref(backend);
        }
        else
        {
            settings = g_settings_new("com.canonical.indicator.network");
        }
        m_enabled = g_settings_get_boolean(settings,
                                           "data-usage-indication");
        g_object_unref(settings);
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        qDebug() << "enabled:" << m_enabled;
#endif

        m_unityScreen.connection().connect(QStringLiteral("com.canonical.Unity.Screen"),
                                           QStringLiteral("/com/canonical/Unity/Screen"),
                                           QStringLiteral("com.canonical.Unity.Screen"),
                                           QStringLiteral("DisplayPowerStateChange"),
                                           this,
                                           SLOT(handleDisplayPowerStateChange(int, int)));

        m_txTimer.setInterval(1000);
        m_txTimer.setSingleShot(true);
        connect(&m_rxTimer, &QTimer::timeout, this, &Private::txShot);

        m_rxTimer.setInterval(1000);
        m_rxTimer.setSingleShot(true);
        connect(&m_rxTimer, &QTimer::timeout, this, &Private::rxShot);
    }

    ~Private()
    {
        for (auto path : m_interfaces.keys())
        {
            resetInterface(path);
        }
    }

    void setTx(bool value)
    {
        if (m_tx == value)
        {
            return;
        }

        m_tx = value;
        Q_EMIT p.txChanged();
    }

    void setRx(bool value)
    {
        if (m_rx == value)
        {
            return;
        }

        m_rx = value;
        Q_EMIT p.rxChanged();
    }


    void setUpInterface(const QString &path)
    {
        if (!m_interfaces.contains(path))
        {
            return;
        }

        if (!m_enabled)
        {
            return;
        }

        auto iface = m_interfaces[path];

        connect(iface.get(), &OrgFreedesktopNetworkManagerDeviceStatisticsInterface::PropertiesChanged, this, &Private::propertiesChanged);
        iface->setRefreshRateMs(500);
    }

    void resetInterface(const QString &path)
    {
        if (!m_interfaces.contains(path))
        {
            return;
        }
        auto iface = m_interfaces[path];
        disconnect(iface.get(), &OrgFreedesktopNetworkManagerDeviceStatisticsInterface::PropertiesChanged, this, &Private::propertiesChanged);
        iface->setRefreshRateMs(0);
    }

    void connectAllInterfaces()
    {
        for (auto path : m_interfaces.keys())
        {
            setUpInterface(path);
        }
    }

    void disconnectAllInterfaces()
    {
        for (auto path : m_interfaces.keys())
        {
            resetInterface(path);
        }
    }


public Q_SLOTS:

    void handleDisplayPowerStateChange(int status, int reason)
    {
        Q_UNUSED(reason)

        if (m_status != (Status)status) {
            m_status = (Status)status;
            switch (m_status)
            {
            case Status::On:
            {
                connectAllInterfaces();
                break;
            }
            case Status::Off:
                disconnectAllInterfaces();
                break;
            }
        }
    }

    void propertiesChanged(const QVariantMap &properties)
    {
        if (properties.contains("TxBytes"))
        {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
            OrgFreedesktopNetworkManagerDeviceStatisticsInterface *iface = qobject_cast<OrgFreedesktopNetworkManagerDeviceStatisticsInterface*>(sender());
            qDebug() << "TxBytes updated on" << iface->path();
#endif
               setTx(true);
               m_txTimer.start();
        }

        if (properties.contains("RxBytes"))\
        {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
            OrgFreedesktopNetworkManagerDeviceStatisticsInterface *iface = qobject_cast<OrgFreedesktopNetworkManagerDeviceStatisticsInterface*>(sender());
            qDebug() << "RxBytes updated on" << iface->path();
#endif
            setRx(true);
            m_rxTimer.start();
        }
    }

    void txShot()
    {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        qDebug() << "";
#endif
        setTx(false);
    }

    void rxShot()
    {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        qDebug() << "";
#endif
        setRx(false);
    }


};


NMDeviceStatisticsMonitor::NMDeviceStatisticsMonitor()
    : d{new Private(*this)}
{
}

NMDeviceStatisticsMonitor::~NMDeviceStatisticsMonitor()
{

}

void
NMDeviceStatisticsMonitor::addLink(Link::SPtr link)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << "adding" << link->name();
#endif

    QString path;

    if (std::dynamic_pointer_cast<wifi::WifiLinkImpl>(link))
    {
        path = std::dynamic_pointer_cast<wifi::WifiLinkImpl>(link)->device_path().path();
    }
    else if (std::dynamic_pointer_cast<wwan::Modem>(link))
    {
        path = std::dynamic_pointer_cast<wwan::Modem>(link)->nmPath();
    }
    else if (std::dynamic_pointer_cast<ethernet::EthernetLink>(link))
    {
        path = std::dynamic_pointer_cast<ethernet::EthernetLink>(link)->devicePath().path();
    }

    if (path.isEmpty())
    {
        qWarning() <<  "Could not determine NMDevice path for link with name:" << link->name();
        return;
    }

    auto dev = make_shared<OrgFreedesktopNetworkManagerDeviceStatisticsInterface>(
                NM_DBUS_SERVICE,
                path,
                QDBusConnection::systemBus());

    d->m_interfaces[path] = dev;

    d->setUpInterface(path);
}

void
NMDeviceStatisticsMonitor::remove(const QString &nmPath)
{
    if (d->m_interfaces.contains(nmPath))
    {
        d->resetInterface(nmPath);
        d->m_interfaces.remove(nmPath);
    }
}

bool
NMDeviceStatisticsMonitor::tx() const
{
    return d->m_tx;
}

bool
NMDeviceStatisticsMonitor::rx() const
{
    return d->m_rx;
}

}

#include "nm-device-statistics-monitor.moc"
