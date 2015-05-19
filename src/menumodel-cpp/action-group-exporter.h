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

#pragma once

#include <memory>
#include <gio/gio.h>

#include "gio-helpers/util.h"

#include "action-group.h"

class ActionGroupExporter: public QObject
{
    Q_OBJECT

    typedef std::shared_ptr<GSimpleActionGroup> GSimpleActionGroupPtr;
    GSimpleActionGroupPtr make_gsimpleactiongroup_ptr();

    std::string m_path;
    std::shared_ptr<SessionBus> m_sessionBus;
    GSimpleActionGroupPtr m_gSimpleActionGroup;
    gint m_exportId;
    ActionGroup::Ptr m_actionGroup;

public:
    typedef std::shared_ptr<ActionGroupExporter> Ptr;
    typedef std::unique_ptr<ActionGroupExporter> UPtr;

    ActionGroupExporter(SessionBus::Ptr sessionBus, ActionGroup::Ptr actionGroup, const std::string &path);

    ~ActionGroupExporter();

private:
    void waitForFirstSignalEmission();

private Q_SLOTS:
    void actionAdded(Action::Ptr);

    void actionRemoved(Action::Ptr);
};
