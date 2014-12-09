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

#ifndef MODEM_MANAGER_H
#define MODEM_MANAGER_H

#include "modem.h"

#include <memory>
#include <core/property.h>

class ModemManager
{
    class Private;
    std::shared_ptr<Private> d;

public:

    typedef std::shared_ptr<ModemManager> Ptr;

    ModemManager();
    ~ModemManager();

    /**
     * must be called from GMainLoop
     */
    void unlockModem(Modem::Ptr modem);

    /**
     * must be called from GMainLoop
     */
    void unlockAllModems();

    /**
     * must be called from GMainLoop
     */
    void unlockModemByName(const std::string &name);

    /**
     * changed() emitted from GMainLoop
     */
    const core::Property<std::set<Modem::Ptr>> &modems();
};

#endif
