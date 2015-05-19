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

#pragma once

#include <nmofono/manager.h>

#include "menumodel-cpp/gio-helpers/variant.h"

/**
 * all signals and property changes emitted from GMainLoop
 */
class RootState: public QObject
{
    Q_OBJECT

    class Private;
    std::shared_ptr<Private> d;

public:
    typedef std::shared_ptr<RootState> Ptr;

    RootState(nmofono::Manager::Ptr manager);
    virtual ~RootState();

    Q_PROPERTY(Variant state READ state NOTIFY stateUpdated)
    const Variant& state() const;

Q_SIGNALS:
    void stateUpdated(const Variant& state);
};
