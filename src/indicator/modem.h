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

#ifndef MODEM_H
#define MODEM_H

#include <string>
#include <memory>

#include <core/property.h>

#include "dbus-cpp/services/ofono.h"

/**
 * all signals and property changes emitted from GMainLoop
 */
class Modem
{
    class Private;
    std::shared_ptr<Private> d;

public:
    enum class PinType
    {
        none,
        pin,
        puk
    };

    enum class SimStatus
    {
        missing,
        error,
        locked,
        permanentlyLocked,
        ready,
        not_available
    };

    typedef std::shared_ptr<Modem> Ptr;
    typedef std::weak_ptr<Modem> WeakPtr;

    struct Compare
    {
        bool operator()(int lhs, int rhs)
        {
            if (lhs == -1 && rhs == -1)
                return false;
            if (lhs == -1)
                return false;
            if (rhs == -1)
                return true;
            return lhs < rhs;
        }
    };

    Modem() = delete;
    explicit Modem(org::ofono::Interface::Modem::Ptr ofonoModem);
    virtual ~Modem();

    org::ofono::Interface::Modem::Ptr ofonoModem() const;

    bool enterPin(PinType type,
                  const std::string &pin);

    bool resetPin(PinType type,
                  const std::string &puk,
                  const std::string &pin);

    bool changePin(PinType type,
                   const std::string &oldPin,
                   const std::string &newPin);

    const core::Property<bool> &online();

    const core::Property<SimStatus> &simStatus();
    const core::Property<PinType> &requiredPin();
    const core::Property<std::map<PinType, std::uint8_t>> &retries();

    const core::Property<std::string> &operatorName();
    const core::Property<org::ofono::Interface::NetworkRegistration::Status> &status();
    const core::Property<std::int8_t> &strength();
    const core::Property<org::ofono::Interface::NetworkRegistration::Technology> &technology();

    const core::Property<std::string> &simIdentifier();
    int index();

    const std::string &name() const;

    static const std::string strengthIcon(int8_t strength)
    {
        /* Using same values as used by Android, not linear (LP: #1329945)*/
        if (strength >= 39)
            return "gsm-3g-full";
        else if (strength >= 26)
            return "gsm-3g-high";
        else if (strength >= 16)
            return "gsm-3g-medium";
        else if (strength >= 6)
            return "gsm-3g-low";
        else
            return "gsm-3g-none";
    }

    static const std::string technologyIcon(org::ofono::Interface::NetworkRegistration::Technology tech)
    {
        switch (tech){
        case org::ofono::Interface::NetworkRegistration::Technology::notAvailable:
        case org::ofono::Interface::NetworkRegistration::Technology::gsm:
            return "network-cellular-pre-edge";
        case org::ofono::Interface::NetworkRegistration::Technology::edge:
            return "network-cellular-edge";
        case org::ofono::Interface::NetworkRegistration::Technology::umts:
            return "network-cellular-3g";
        case org::ofono::Interface::NetworkRegistration::Technology::hspa:
            return "network-cellular-hspa";
        /// @todo oFono can't tell us about hspa+ yet
        //case org::ofono::Interface::NetworkRegistration::Technology::hspaplus:
        //    break;
        case org::ofono::Interface::NetworkRegistration::Technology::lte:
            return "network-cellular-lte";
        }
        // shouldn't be reached
        return "";
    }
};

#endif
