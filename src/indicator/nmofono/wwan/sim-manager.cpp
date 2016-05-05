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

#include <nmofono/wwan/sim-manager.h>

#include <nmofono/wwan/qofono-sim-wrapper.h>

#define slots
#include <qofono-qt5/qofonomanager.h>
#include <qofono-qt5/qofonomodem.h>
#include <qofono-qt5/qofonosimmanager.h>
#undef slots
#include <ofono/dbus.h>

using namespace std;

namespace nmofono
{
namespace wwan
{
namespace
{

}

class SimManager::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT

public:

    SimManager& p;

    QMap<QString, Sim::Ptr> m_knownSims;

    shared_ptr<QOfonoManager> m_ofono;
    QMap<QString, shared_ptr<QOfonoModem>> m_ofonoModems;
    QMap<QString, QOfonoSimWrapper::Ptr> m_wrappers;

    QSettings *m_settings;

    Private(SimManager& parent)
        : p(parent)
    {
        m_settings = new QSettings(QSettings::IniFormat,
                                   QSettings::UserScope,
                                   "Ubuntu",
                                   "connectivityservice",
                                   this);

        QVariant ret;
        ret = m_settings->value("KnownSims");
        if (ret.isNull())
        {
            /* This is the first time we are running on a system.
             */
            m_settings->setValue("KnownSims", QVariant(QVariant::StringList));
        }
        else
        {
            QStringList imsis = ret.toStringList();
            for(auto imsi : imsis) {
                auto sim = wwan::Sim::createFromSettings(m_settings, imsi);
                connect(sim.get(), &Sim::dataRoamingEnabledChanged, this, &Private::simDataRoamingEnabledChanged);
                if (!sim)
                {
                    continue;
                }
                m_knownSims[sim->imsi()] = sim;
            }
        }

        m_ofono = make_shared<QOfonoManager>();
        connect(m_ofono.get(), &QOfonoManager::modemsChanged, this, &Private::modemsChanged);
        modemsChanged(m_ofono->modems());
    }

public Q_SLOTS:

    void modemsChanged(const QStringList& value)
    {
        QSet<QString> modemPaths(value.toSet());
        QSet<QString> currentModemPaths(m_ofonoModems.keys().toSet());

        auto toRemove = currentModemPaths;
        toRemove.subtract(modemPaths);

        auto toAdd = modemPaths;
        toAdd.subtract(currentModemPaths);

        for (const auto& path : toRemove)
        {
            if (!m_ofonoModems.contains(path))
            {
                qWarning("Something went horribly wrong.");
                continue;
            }
            auto modem = m_ofonoModems.take(path);
            if (m_wrappers.contains(path))
            {
                auto wrapper = m_wrappers[path];
                if (m_knownSims.contains(wrapper->imsi()))
                {
                    auto sim = m_knownSims[wrapper->imsi()];
                    sim->setOfonoSimManager(std::shared_ptr<QOfonoSimManager>());
                }
                m_wrappers.remove(path);
            }
        }

        for (const auto& path : toAdd)
        {
            auto modem = make_shared<QOfonoModem>();
            modem->setModemPath(path);

            if (m_ofonoModems.contains(path))
            {
                qWarning("Something went horribly wrong.");
            }
            m_ofonoModems[path] = modem;

            connect(modem.get(), &QOfonoModem::interfacesChanged, this, &Private::modemInterfacesChanged);
            // use Q_EMIT to get the sender() set.
            Q_EMIT modem->interfacesChanged(modem->interfaces());
        }
    }

    void modemInterfacesChanged() {
        QOfonoModem *modem = qobject_cast<QOfonoModem*>(sender());
        if (!modem)
        {
            qWarning() << "Unable to cast sender().";
            return;
        }

        QSet<QString> interfaces(modem->interfaces().toSet());
        if (interfaces.contains(OFONO_SIM_MANAGER_INTERFACE))
        {
            if (!m_wrappers.contains(modem->modemPath()))
            {
                auto simmgr = make_shared<QOfonoSimManager>();
                simmgr->setModemPath(modem->modemPath());

                auto wrapper = make_shared<wwan::QOfonoSimWrapper>(simmgr);

                connect(wrapper.get(), &wwan::QOfonoSimWrapper::presentChanged, this, &Private::ofonoSimPresentChanged);
                connect(wrapper.get(), &wwan::QOfonoSimWrapper::readyChanged, this, &Private::ofonoSimReady);

                m_wrappers[modem->modemPath()] = wrapper;
            }

        } else {
            if (m_wrappers.contains(modem->modemPath()))
            {
                auto wrapper = m_wrappers[modem->modemPath()];
                if (m_knownSims.contains(wrapper->imsi()))
                {
                    auto sim = m_knownSims[wrapper->imsi()];
                    sim->setOfonoSimManager(std::shared_ptr<QOfonoSimManager>());
                }
                m_wrappers.remove(modem->modemPath());
            }
        }
    }

    void ofonoSimPresentChanged(bool present)
    {
        auto wrapper = qobject_cast<QOfonoSimWrapper*>(sender());
        if (!wrapper) {
            qWarning("casting failed.");
        }

        if (!present) {
            if (m_knownSims.contains(wrapper->imsi()))
            {
                auto sim = m_knownSims[wrapper->imsi()];
                sim->setOfonoSimManager(std::shared_ptr<QOfonoSimManager>());
            }
        }
    }

    void ofonoSimReady(bool value)
    {
        if (!value)
        {
            // handled in present changed
            return;
        }

        auto wrapper = qobject_cast<wwan::QOfonoSimWrapper*>(sender());
        if (!wrapper)
        {
            qWarning("casting failed.");
            return;
        }

        auto wrappers = m_wrappers;

        bool found = false;
        for (auto i : m_knownSims.values())
        {
            if (wrapper->imsi() == i->imsi())
            {
                found = true;
                i->setOfonoSimManager(wrapper->ofonoSimManager());
                break;
            }
        }
        if (!found)
        {
            auto sim = Sim::fromQOfonoSimWrapper(wrapper);
            connect(sim.get(), &Sim::dataRoamingEnabledChanged, this, &Private::simDataRoamingEnabledChanged);
            wwan::Sim::saveToSettings(m_settings, sim);
            m_knownSims[sim->imsi()] = sim;
            m_settings->setValue("KnownSims", QVariant(m_knownSims.keys()));
            Q_EMIT p.simAdded(sim);
        }
    }

    void simDataRoamingEnabledChanged(bool value)
    {
        Q_UNUSED(value)

        auto sim_raw = qobject_cast<Sim*>(sender());
        auto sim = m_knownSims[sim_raw->imsi()];
        if (!sim)
        {
            qWarning("foobar");
            return;
        }
        Sim::saveToSettings(m_settings, sim);
    }

};



SimManager::SimManager()
    : d{new Private(*this)}
{
}

SimManager::~SimManager()
{}

QList<Sim::Ptr>
SimManager::knownSims() const
{
    return d->m_knownSims.values();
}

}
}

#include "sim-manager.moc"
