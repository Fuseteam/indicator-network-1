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

#include "modem.h"

#include <menumodel-cpp/gio-helpers/util.h>

class Modem::Private : public std::enable_shared_from_this<Private>
{
public:
    core::Property<bool> m_online;

    org::ofono::Interface::Modem::Ptr m_ofonoModem;
    core::Property<Modem::SimStatus> m_simStatus;
    core::Property<Modem::PinType> m_requiredPin;
    core::Property<std::map<Modem::PinType, std::uint8_t>> m_retries;

    core::Property<std::string> m_operatorName;
    core::Property<org::ofono::Interface::NetworkRegistration::Status> m_status;
    core::Property<std::int8_t> m_strength;
    core::Property<org::ofono::Interface::NetworkRegistration::Technology> m_technology;

    core::Property<bool> m_dataEnabled;

    core::Property<std::string> m_simIdentifier;
    int m_index = -1;

    void connectionManagerChanged(org::ofono::Interface::ConnectionManager::Ptr conmgr);
    void networkRegistrationChanged(org::ofono::Interface::NetworkRegistration::Ptr netreg);
    void simManagerChanged(org::ofono::Interface::SimManager::Ptr simmgr);

    void update();

    Private() = delete;
    Private(org::ofono::Interface::Modem::Ptr ofonoModem);
    void ConstructL();
};

Modem::Private::Private(org::ofono::Interface::Modem::Ptr ofonoModem)
    : m_ofonoModem{ofonoModem}
{}

void
Modem::Private::ConstructL()
{
    auto that = shared_from_this();

    m_online.set(m_ofonoModem->online.get());
    m_ofonoModem->online.changed().connect([that](bool value){
        GMainLoopDispatch([that, value]() {
            that->m_online.set(value);
        });
    });

    std::unique_lock<std::mutex> lock(m_ofonoModem->_lock);
    auto simmgr = m_ofonoModem->simManager.get();
    auto netreg = m_ofonoModem->networkRegistration.get();
    auto conmgr = m_ofonoModem->connectionManager.get();
    lock.unlock();

    simManagerChanged(simmgr);
    m_ofonoModem->simManager.changed().connect([that](org::ofono::Interface::SimManager::Ptr simmgr)
    {
        that->simManagerChanged(simmgr);
    });

    networkRegistrationChanged(netreg);
    m_ofonoModem->networkRegistration.changed().connect([that](org::ofono::Interface::NetworkRegistration::Ptr netreg)
    {
        that->networkRegistrationChanged(netreg);
    });

    connectionManagerChanged(conmgr);
    m_ofonoModem->connectionManager.changed().connect([that](org::ofono::Interface::ConnectionManager::Ptr conmgr)
    {
        that->connectionManagerChanged(conmgr);
    });

    /// @todo hook up with system-settings to allow changing the identifier.
    ///       for now just provide the defaults
    const auto path = m_ofonoModem->object->path().as_string();
    if (path == "/ril_0") {
        m_simIdentifier.set("SIM 1");
        m_index = 1;
    } else if (path == "/ril_1") {
        m_simIdentifier.set("SIM 2");
        m_index = 2;
    } else {
        m_simIdentifier.set(path);
    }
}


void
Modem::Private::update()
{
    std::unique_lock<std::mutex> lock(m_ofonoModem->_lock);
    auto simmgr = m_ofonoModem->simManager.get();
    auto netreg = m_ofonoModem->networkRegistration.get();
    auto conmgr = m_ofonoModem->connectionManager.get();
    lock.unlock();

    if (simmgr) {
        // update requiredPin
        switch(simmgr->pinRequired.get())
        {
        case org::ofono::Interface::SimManager::PinType::none:
            m_requiredPin.set(PinType::none);
            break;
        case org::ofono::Interface::SimManager::PinType::pin:
            m_requiredPin.set(PinType::pin);
            break;
        case org::ofono::Interface::SimManager::PinType::puk:
            m_requiredPin.set(PinType::puk);
            break;
        default:
            throw std::runtime_error("Ofono requires a PIN we have not been prepared to handle (" +
                                     org::ofono::Interface::SimManager::pin2str(simmgr->pinRequired.get()) +
                                     "). Bailing out.");
        }

        // update retries
        std::map<Modem::PinType, std::uint8_t> tmp;
        for (auto element : simmgr->retries.get()) {
            switch(element.first) {
            case org::ofono::Interface::SimManager::PinType::pin:
                tmp[Modem::PinType::pin] = element.second;
                break;
            case org::ofono::Interface::SimManager::PinType::puk:
                tmp[Modem::PinType::puk] = element.second;
                break;
            default:
                // don't care
                break;
            }
        }
        m_retries.set(tmp);

        // update simStatus
        if (!simmgr->present.get()) {
            m_simStatus.set(SimStatus::missing);
        } else if (m_requiredPin == PinType::none){
            m_simStatus.set(SimStatus::ready);
        } else {
            if (m_retries->count(PinType::puk) != 0 && m_retries->at(PinType::puk) == 0)
                m_simStatus.set(SimStatus::permanentlyLocked);
            else
                m_simStatus.set(SimStatus::locked);
        }

    } else {
        m_requiredPin.set(PinType::none);
        m_retries.set({});
        m_simStatus.set(SimStatus::not_available);
    }

    if (netreg) {
        m_operatorName.set(netreg->operatorName.get());
        m_status.set(netreg->status.get());
        m_strength.set(netreg->strength.get());
        m_technology.set(netreg->technology.get());
    } else {
        m_operatorName.set("");
        m_status.set(org::ofono::Interface::NetworkRegistration::Status::unknown);
        m_strength.set(-1);
        m_technology.set(org::ofono::Interface::NetworkRegistration::Technology::notAvailable);
    }

    if (conmgr) {
        m_dataEnabled.set(conmgr->powered.get());
    } else {
        m_dataEnabled.set(false);
    }
}

void
Modem::Private::networkRegistrationChanged(org::ofono::Interface::NetworkRegistration::Ptr netreg)
{
    auto that = shared_from_this();
    if (netreg) {
        netreg->operatorName.changed().connect([that](const std::string &)
        {
            GMainLoopDispatch([that]() { that->update(); });
        });
        netreg->status.changed().connect([that](org::ofono::Interface::NetworkRegistration::Status)
        {
            GMainLoopDispatch([that]() { that->update(); });
        });

        netreg->strength.changed().connect([that](std::int8_t)
        {
            GMainLoopDispatch([that]() { that->update(); });
        });

        netreg->technology.changed().connect([that](org::ofono::Interface::NetworkRegistration::Technology)
        {
            GMainLoopDispatch([that]() { that->update(); });
        });

    }

    GMainLoopDispatch([that]() { that->update(); });
}

void
Modem::Private::connectionManagerChanged(org::ofono::Interface::ConnectionManager::Ptr conmgr)
{
    auto that = shared_from_this();
    if (conmgr) {
        conmgr->powered.changed().connect([that](bool)
        {
            GMainLoopDispatch([that]() { that->update(); });
        });
    }

    GMainLoopDispatch([that]() { that->update(); });
}


void
Modem::Private::simManagerChanged(org::ofono::Interface::SimManager::Ptr simmgr)
{
    auto that = shared_from_this();
    if (simmgr) {
        simmgr->present.changed().connect([that](bool)
        {
            GMainLoopDispatch([that]() { that->update(); });
        });

        simmgr->pinRequired.changed().connect([that](org::ofono::Interface::SimManager::PinType)
        {
            GMainLoopDispatch([that]() { that->update(); });
        });

        simmgr->retries.changed().connect([that](std::map<org::ofono::Interface::SimManager::PinType, std::uint8_t>)
        {
            GMainLoopDispatch([that]() { that->update(); });
        });

    }
    GMainLoopDispatch([that]() { that->update(); });
}

Modem::Modem(org::ofono::Interface::Modem::Ptr ofonoModem)
    : d{new Private(ofonoModem)}
{
    d->ConstructL();
}

Modem::~Modem()
{}

org::ofono::Interface::Modem::Ptr
Modem::ofonoModem() const
{
    return d->m_ofonoModem;
}

bool
Modem::enterPin(PinType type, const std::string &pin)
{
    std::unique_lock<std::mutex> lock(d->m_ofonoModem->_lock);
    auto simmgr = d->m_ofonoModem->simManager.get();
    lock.unlock();

    if (!simmgr) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        return true;
    case PinType::pin:
        return simmgr->enterPin(org::ofono::Interface::SimManager::PinType::pin,
                                pin);
    case PinType::puk:
        return simmgr->enterPin(org::ofono::Interface::SimManager::PinType::puk,
                                pin);
        break;
    }

    throw std::logic_error("code should not be reached.");
}


bool
Modem::resetPin(PinType type, const std::string &puk, const std::string &pin)
{
    std::unique_lock<std::mutex> lock(d->m_ofonoModem->_lock);
    auto simmgr = d->m_ofonoModem->simManager.get();
    lock.unlock();

    if (!simmgr) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        return true;
    case PinType::puk:
        return simmgr->resetPin(org::ofono::Interface::SimManager::PinType::puk,
                                puk,
                                pin);
    default:
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Not Supported.");
    }
}

bool
Modem::changePin(PinType type, const std::string &oldPin, const std::string &newPin)
{
    std::unique_lock<std::mutex> lock(d->m_ofonoModem->_lock);
    auto simmgr = d->m_ofonoModem->simManager.get();
    lock.unlock();

    if (!simmgr) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        return true;
    case PinType::pin:
        return simmgr->changePin(org::ofono::Interface::SimManager::PinType::pin,
                                 oldPin,
                                 newPin);
    case PinType::puk:
        return simmgr->changePin(org::ofono::Interface::SimManager::PinType::puk,
                                 oldPin,
                                 newPin);
        break;
    }

    throw std::logic_error("code should not be reached.");
}

const core::Property<bool> &
Modem::online()
{
    return d->m_online;
}

const core::Property<Modem::SimStatus> &
Modem::simStatus()
{
    return d->m_simStatus;
}

const core::Property<Modem::PinType> &
Modem::requiredPin()
{
    return d->m_requiredPin;
}

const core::Property<std::map<Modem::PinType, std::uint8_t> > &
Modem::retries()
{
    return d->m_retries;
}

const core::Property<std::string> &
Modem::operatorName()
{
    return d->m_operatorName;
}

const core::Property<org::ofono::Interface::NetworkRegistration::Status> &
Modem::status()
{
    return d->m_status;
}

const core::Property<std::int8_t> &
Modem::strength()
{
    return d->m_strength;
}

const core::Property<org::ofono::Interface::NetworkRegistration::Technology> &
Modem::technology()
{
    return d->m_technology;
}
const core::Property<std::string> &
Modem::simIdentifier()
{
    return d->m_simIdentifier;
}

const core::Property<bool> &
Modem::dataEnabled()
{
    return d->m_dataEnabled;
}

int
Modem::index()
{
    return d->m_index;
}

const std::string &
Modem::name() const
{
    return d->m_ofonoModem->object->path().as_string();
}


