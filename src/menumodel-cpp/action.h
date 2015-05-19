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
#include <functional>

#include <gio/gio.h>

#include "gio-helpers/variant.h"

#include <QObject>

class Action: public QObject
{
    Q_OBJECT

    typedef std::shared_ptr<GAction> GActionPtr;
    GActionPtr make_gaction_ptr(GSimpleAction *action) { return std::shared_ptr<GAction>(G_ACTION(action), GObjectDeleter()); }

    GActionPtr m_gaction;
    std::string m_name;
    gulong m_activateHandlerId;
    gulong m_changeStateHandlerId;

    static void activate_cb(GSimpleAction *,
                            GVariant      *parameter,
                            gpointer       user_data);

    static void change_state_cb(GSimpleAction *,
                                GVariant      *value,
                                gpointer       user_data);
public:
    typedef std::shared_ptr<Action> Ptr;

    Action(const std::string &name, const
           GVariantType *parameterType = nullptr,
           const Variant &state = Variant());

    ~Action();

    std::string name();

    Q_PROPERTY(Variant state READ state WRITE setState NOTIFY stateUpdated)
    Variant state();

    GActionPtr gaction();

public Q_SLOTS:
    void setState(const Variant &value);

    void setEnabled(bool enabled);

Q_SIGNALS:
    void activated(const Variant&);

    void stateUpdated(const Variant&);
};
