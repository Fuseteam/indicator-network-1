/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include <algorithm>
#include <notify-cpp/snapdecision/sim-unlock.h>
#include "sim-unlock-dialog.h"
#include "modem-manager.h"

#include <QMap>

#define slots
#include <qofono-qt5/qofonomanager.h>
#include <qofono-qt5/qofonomodem.h>
#undef slots

using namespace std;

class ModemManager::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT
public:
    ModemManager& p;

    QMap<QString, Modem::Ptr> m_modems;

    shared_ptr<QOfonoManager> m_ofono;

    SimUnlockDialog::Ptr m_unlockDialog;

    QList<Modem::Ptr> m_pendingUnlocks;

    Private(ModemManager& parent) :
        p(parent)
    {
        m_unlockDialog = make_shared<SimUnlockDialog>();
        connect(m_unlockDialog.get(), &SimUnlockDialog::ready, this, &Private::sim_unlock_ready);

        m_ofono = make_shared<QOfonoManager>();
        connect(m_ofono.get(), &QOfonoManager::modemsChanged, this, &Private::modems_changed);
        modems_changed(m_ofono->modems());
    }

    ~Private()
    {
    }

public Q_SLOTS:
    void sim_unlock_ready()
    {
        if (!m_pendingUnlocks.empty())
        {
            auto modem = m_pendingUnlocks.front();
            m_pendingUnlocks.pop_front();
            if (modem->requiredPin() != Modem::PinType::none)
            {
                m_unlockDialog->unlock(modem);
            }
        }
    }

    void modems_changed(const QStringList& value)
    {
        QSet<QString> modemPaths(value.toSet());
        QSet<QString> currentModemPaths(m_modems.keys().toSet());

        auto toRemove = currentModemPaths;
        toRemove.subtract(modemPaths);

        auto toAdd = modemPaths;
        toAdd.subtract(currentModemPaths);

        for (const auto& path : toRemove)
        {
            auto modem = m_modems.take(path);
            if (m_pendingUnlocks.contains(modem))
            {
                m_unlockDialog->cancel();
                m_pendingUnlocks.removeOne(modem);
            }
        }

        for (const auto& path : toAdd)
        {
            auto modemInterface = make_shared<QOfonoModem>();
            modemInterface->setModemPath(path);

            auto modem = make_shared<Modem>(modemInterface);
            m_modems[path] = modem;
        }

        Q_EMIT p.modemsUpdated(m_modems.values());
        m_unlockDialog->setShowSimIdentifiers(m_modems.size() > 1);
    }
};

ModemManager::ModemManager()
    : d{new Private(*this)}
{
}

ModemManager::~ModemManager()
{}

void
ModemManager::unlockModem(Modem::Ptr modem)
{
    try {
        if (!d->m_modems.values().contains(modem)
                || d->m_unlockDialog->modem() == modem
                || std::count(d->m_pendingUnlocks.begin(), d->m_pendingUnlocks.end(), modem) != 0)
            return;

        if (d->m_unlockDialog->state() == SimUnlockDialog::State::ready
                && d->m_pendingUnlocks.size() == 0)
            d->m_unlockDialog->unlock(modem);
        else
            d->m_pendingUnlocks.push_back(modem);
    } catch(const std::exception &e) {
        // Something unexpected has happened. As an example, unity8 might have
        // crashed taking the notification server with it. There is no graceful
        // and reliable way to recover so die and get restarted.
        // See also https://bugs.launchpad.net/unity-notifications/+bug/1238990
        qWarning() << __PRETTY_FUNCTION__ << " sim unlocking failed: " << QString::fromStdString(e.what());
    }
}

void
ModemManager::unlockAllModems()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << __PRETTY_FUNCTION__;
#endif
    for (auto& m : d->m_modems)
    {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        qDebug() << "Unlocking " << m->simIdentifier();
#endif
        unlockModem(m);
    }
}

void
ModemManager::unlockModemByName(const QString &name)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << __PRETTY_FUNCTION__ ;
#endif
    auto it = d->m_modems.find(name);
    if (it != d->m_modems.cend())
    {
        unlockModem(it.value());
    }
}


QList<Modem::Ptr>
ModemManager::modems()
{
    return d->m_modems.values();
}

#include "modem-manager.moc"
