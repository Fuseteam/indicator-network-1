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

#include <nmofono/wwan/sim.h>

#include <ofono/dbus.h>

#define slots
#include <qofono-qt5/qofonoconnectionmanager.h>
#undef slots

using namespace std;

namespace nmofono
{
namespace wwan
{
namespace
{

}

Sim::Ptr
Sim::createFromSettings(QSettings *settings, const QString &imsi)
{
    settings->beginGroup(QString("Sims/%1/").arg(imsi));
    QVariant primaryPhoneNumber_var = settings->value("PrimaryPhoneNumber");
    QVariant mcc_var = settings->value("Mcc");
    QVariant mnc_var = settings->value("Mnc");
    QVariant preferredLanguages_var = settings->value("PreferredLanguages");
    QVariant dataRoamingEnabled_var = settings->value("DataRoamingEnabled");
    settings->endGroup();

    if (imsi.isNull() ||
            primaryPhoneNumber_var.isNull() ||
            mcc_var.isNull() ||
            mnc_var.isNull() ||
            preferredLanguages_var.isNull() ||
            dataRoamingEnabled_var.isNull())
    {
        qWarning() << "Corrupt settings for SIM: " << imsi;
        settings->remove(QString("Sims/%1/").arg(imsi));
        return Sim::Ptr();
    }

    return Sim::Ptr(new Sim(imsi,
                            primaryPhoneNumber_var.toString(),
                            mcc_var.toString(),
                            mnc_var.toString(),
                            preferredLanguages_var.toStringList(),
                            dataRoamingEnabled_var.toBool()));
}

void
Sim::saveToSettings(QSettings *settings, Sim::Ptr sim)
{
    settings->beginGroup(QString("Sims/%1/").arg(sim->imsi()));
    settings->setValue("PrimaryPhoneNumber", QVariant(sim->primaryPhoneNumber()));
    settings->setValue("Mcc", sim->mcc());
    settings->setValue("Mnc", sim->mnc());
    settings->setValue("PreferredLanguages", QVariant(sim->preferredLanguages()));
    settings->setValue("DataRoamingEnabled", sim->dataRoamingEnabled());
    settings->endGroup();
    settings->sync();
}

Sim::Ptr Sim::fromQOfonoSimWrapper(const QOfonoSimWrapper *wrapper)
{
    auto sim = Sim::Ptr(new Sim(wrapper->imsi(),
                                wrapper->phoneNumbers().first(), // default to the first number
                                wrapper->mcc(),
                                wrapper->mnc(),
                                wrapper->preferredLanguages(),
                                false));
    sim->setOfonoSimManager(wrapper->ofonoSimManager());
    return sim;
}

class Sim::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT

public:

    Sim& p;

    Sim::PinType m_requiredPin;
    RetriesType m_retries;
    Sim::Status m_status = Sim::Status::missing;

    bool m_requiredPinSet = false;
    bool m_retriesSet = false;
    bool m_statusSet = false;

    int m_index = -1;

    QString m_simIdentifier;

    shared_ptr<QOfonoSimManager> m_simManager;
    shared_ptr<QOfonoConnectionManager> m_connManager;

    bool m_shouldTriggerUnlock = false;

    QSet<QString> m_interfaces;

    QTimer m_updatedTimer;

    QString m_imsi;
    QString m_primaryPhoneNumber;
    QString m_mcc;
    QString m_mnc;
    QStringList m_preferredLanguages;
    bool m_dataRoamingEnabled;
    bool m_mobileDataEnabled = false;

    bool m_locked = false;

    Private(Sim &parent)
        : p(parent)
    {
    }

    Private(Sim& parent, shared_ptr<QOfonoSimManager> simmgr)
        : p(parent)
    {
        simManagerChanged(simmgr);


        /// @todo hook up with system-settings to allow changing the identifier.
        if (m_simIdentifier.isEmpty())
        {
            setSimIdentifier(m_primaryPhoneNumber);
        }

        // Throttle the updates using a timer
        m_updatedTimer.setInterval(0);
        m_updatedTimer.setSingleShot(true);
        connect(&m_updatedTimer, &QTimer::timeout, this, &Private::fireUpdate);
    }

public Q_SLOTS:

    void fireUpdate()
    {
        Q_EMIT p.updated(p);

        if (p.isReadyToUnlock() && m_shouldTriggerUnlock)
        {
            m_shouldTriggerUnlock = false;
            Q_EMIT p.readyToUnlock(p.ofonoPath());
        }
    }

    void simManagerChanged(shared_ptr<QOfonoSimManager> simmgr)
    {
        if (m_simManager == simmgr)
        {
            return;
        }

        m_simManager = simmgr;
        if (m_simManager)
        {
            connect(m_simManager.get(),
                    &QOfonoSimManager::pinRequiredChanged, this,
                    &Private::update);

            connect(m_simManager.get(),
                    &QOfonoSimManager::pinRetriesChanged, this,
                    &Private::update);

            connect(m_simManager.get(),
                    &QOfonoSimManager::enterPinComplete, this,
                    &Private::enterPinComplete);

            connect(m_simManager.get(),
                    &QOfonoSimManager::resetPinComplete, this,
                    &Private::resetPinComplete);
        }

        update();

        Q_EMIT p.presentChanged(m_simManager.get() != nullptr);
    }

    void connManagerChanged(shared_ptr<QOfonoConnectionManager> connmgr)
    {
        if (m_connManager == connmgr)
        {
            return;
        }

        m_connManager = connmgr;
        if (m_connManager)
        {
            connect(m_connManager.get(),
                    &QOfonoConnectionManager::poweredChanged, this,
                    &Private::update);
            connect(m_connManager.get(),
                    &QOfonoConnectionManager::roamingAllowedChanged, this,
                    &Private::update);

            m_connManager->setPowered(m_mobileDataEnabled);
            m_connManager->setRoamingAllowed(m_dataRoamingEnabled);
        }

        update();
    }

    void update()
    {
        if (m_simManager) {
            // update requiredPin
            switch(m_simManager->pinRequired())
            {
            case QOfonoSimManager::PinType::NoPin:
                setRequiredPin(PinType::none);
                break;
            case QOfonoSimManager::PinType::SimPin:
                setRequiredPin(PinType::pin);
                break;
            case QOfonoSimManager::PinType::SimPuk:
                setRequiredPin(PinType::puk);
                break;
            default:
                throw std::runtime_error("Ofono requires a PIN we have not been prepared to handle (" +
                                         to_string(m_simManager->pinRequired()) +
                                         "). Bailing out.");
            }

            m_requiredPinSet = true;

            bool retriesWasSet = true;
            // update retries
            RetriesType tmp;
            QVariantMap retries = m_simManager->pinRetries();
            QMapIterator<QString, QVariant> i(retries);
            while (i.hasNext()) {
                i.next();
                QOfonoSimManager::PinType type = (QOfonoSimManager::PinType) i.key().toInt();
                int count = i.value().toInt();
                if (count < 0)
                {
                    retriesWasSet = false;
                }
                switch(type)
                {
                    case QOfonoSimManager::PinType::SimPin:
                        tmp[Sim::PinType::pin] = count;
                        break;
                    case QOfonoSimManager::PinType::SimPuk:
                        tmp[Sim::PinType::puk] = count;
                        break;
                    default:
                        break;
                }
            }
            setRetries(tmp);

            m_retriesSet = retriesWasSet;

        } else {
            setRequiredPin(PinType::none);
            setRetries({});

            m_requiredPinSet = false;
            m_retriesSet = false;
        }

        if (m_connManager)
        {
            bool powered = m_connManager->powered();
            bool roamingAllowed = m_connManager->roamingAllowed();

            /*
             * Connectivity Service is the policy manager for the system.
             * If ofono has different settings force them back to stored values.
             */
            if (m_mobileDataEnabled != powered)
            {
                m_connManager->setPowered(m_mobileDataEnabled);
            }
            if (m_dataRoamingEnabled != roamingAllowed)
            {
                m_connManager->setRoamingAllowed(m_dataRoamingEnabled);
            }
        }
        else
        {
            /* empty */
        }
    }

    void enterPinComplete(QOfonoSimManager::Error error, const QString &errorString)
    {
        if (error == QOfonoSimManager::Error::NoError)
        {
            Q_EMIT p.enterPinSuceeded();
        }
        else
        {
            Q_EMIT p.enterPinFailed(errorString);
        }
    }

    void resetPinComplete(QOfonoSimManager::Error error, const QString &errorString)
    {
        if (error == QOfonoSimManager::Error::NoError)
        {
            Q_EMIT p.resetPinSuceeded();
        }
        else
        {
            Q_EMIT p.resetPinFailed(errorString);
        }
    }

    void setSimIdentifier(const QString& simIdentifier)
    {
        if (m_simIdentifier == simIdentifier)
        {
            return;
        }

        m_simIdentifier = simIdentifier;
        Q_EMIT p.simIdentifierUpdated(m_simIdentifier);
    }

    void setRequiredPin(Sim::PinType requiredPin)
    {
        if (m_requiredPin == requiredPin)
        {
            return;
        }

        m_requiredPin = requiredPin;
        Q_EMIT p.requiredPinUpdated(m_requiredPin);
    }

    void setRetries(const RetriesType& retries)
    {
        if (m_retries == retries)
        {
            return;
        }

        m_retries = retries;
        Q_EMIT p.retriesUpdated();
    }

    void setStatus(Sim::Status value)
    {
        if (m_status == value)
        {
            return;
        }

        m_status = value;
        Q_EMIT p.statusUpdated();
    }

    void setOfono(std::shared_ptr<QOfonoSimManager> simmgr)
    {
        simManagerChanged(simmgr);
        if (simmgr)
        {
            m_connManager = std::make_shared<QOfonoConnectionManager>(this);
            m_connManager->setModemPath(simmgr->modemPath());
            connManagerChanged(m_connManager);
        }
        else
        {
            m_connManager = std::shared_ptr<QOfonoConnectionManager>();
            connManagerChanged(m_connManager);
        }
    }


};

Sim::Sim(const QString &imsi,
         const QString &primaryPhoneNumber,
         const QString &mcc,
         const QString &mnc,
         const QStringList &preferredLanguages,
         bool dataRoamingEnabled)
    : d{new Private(*this)}
{
    d->m_imsi = imsi;
    d->m_primaryPhoneNumber = primaryPhoneNumber;
    d->m_mcc = mcc;
    d->m_mnc = mnc;
    d->m_preferredLanguages = preferredLanguages;
    d->m_dataRoamingEnabled = dataRoamingEnabled;
}

Sim::~Sim()
{}

void
Sim::enterPin(PinType type, const QString &pin)
{
    if (!d->m_simManager)
    {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        break;
    case PinType::pin:
        d->m_simManager->enterPin(QOfonoSimManager::PinType::SimPin,
                                pin);
        break;
    case PinType::puk:
        d->m_simManager->enterPin(QOfonoSimManager::PinType::SimPuk,
                                pin);
        break;
    }
}


void
Sim::resetPin(PinType type, const QString &puk, const QString &pin)
{
    if (!d->m_simManager) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        break;
    case PinType::puk:
        d->m_simManager->resetPin(QOfonoSimManager::PinType::SimPuk,
                                puk,
                                pin);
        break;
    default:
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Not Supported.");
    }
}

Sim::PinType
Sim::requiredPin() const
{
    return d->m_requiredPin;
}

const Sim::RetriesType&
Sim::retries() const
{
    return d->m_retries;
}

const QString&
Sim::simIdentifier() const
{
    return d->m_simIdentifier;
}

bool
Sim::isReadyToUnlock() const
{
    return d->m_statusSet && d->m_requiredPinSet && d->m_retriesSet
            && (d->m_requiredPin != PinType::none);
}

void
Sim::notifyWhenReadyToUnlock()
{
    d->m_shouldTriggerUnlock = true;
}


Sim::Status Sim::status() const
{
    return d->m_status;
}

QString Sim::imsi() const
{
    return d->m_imsi;
}

QString Sim::primaryPhoneNumber() const
{
    return d->m_primaryPhoneNumber;
}

bool Sim::locked() const
{
    return d->m_locked;
}

bool Sim::present() const
{
    return d->m_simManager.get() != nullptr;
}

QString Sim::mcc() const
{
    return d->m_mcc;
}

QString Sim::mnc() const
{
    return d->m_mnc;
}

QList<QString> Sim::preferredLanguages() const
{
    return d->m_preferredLanguages;
}

bool Sim::dataRoamingEnabled() const
{
    return d->m_dataRoamingEnabled;
}

void Sim::setDataRoamingEnabled(bool value)
{
    if (d->m_dataRoamingEnabled == value)
        return;
    d->m_dataRoamingEnabled = value;
    Q_EMIT dataRoamingEnabledChanged(value);
}

bool Sim::mobileDataEnabled() const
{
    return d->m_mobileDataEnabled;
}

void Sim::setMobileDataEnabled(bool value)
{
    if (d->m_mobileDataEnabled == value)
        return;
    d->m_mobileDataEnabled = value;
    if (d->m_connManager)
    {
        d->m_connManager->setPowered(d->m_mobileDataEnabled);
    }
    Q_EMIT mobileDataEnabledChanged(value);
}

void Sim::unlock()
{

}

QString Sim::ofonoPath() const
{
    if (d->m_simManager) {
        return d->m_simManager->objectPath();
    }
    return QString();
}

void Sim::setOfonoSimManager(std::shared_ptr<QOfonoSimManager> simmgr)
{
    d->setOfono(simmgr);
}


}
}

#include "sim.moc"
