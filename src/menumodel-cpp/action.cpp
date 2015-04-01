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

#include "action.h"

void
Action::activate_cb(GSimpleAction *,
                    GVariant      *parameter,
                    gpointer       user_data)
{
    Variant param;
    if (parameter != nullptr) {
        param = Variant::fromGVariant(g_variant_ref(parameter));
    }
    Action *that = static_cast<Action *>(user_data);
    Q_EMIT that->activated(param);
}

void
Action::change_state_cb(GSimpleAction *,
                        GVariant      *value,
                        gpointer       user_data)
{
    Variant new_value = Variant::fromGVariant(g_variant_ref(value));

    Action *that =  static_cast<Action *>(user_data);
    Q_EMIT that->stateUpdated(new_value);
}

Action::Action(const std::string &name, const
       GVariantType *parameterType,
       const Variant &state)
    : m_name {name}
{
    /// @todo validate that name is valid.

    if (state) {
        m_gaction = make_gaction_ptr(g_simple_action_new_stateful(name.c_str(),
                                                                  parameterType,
                                                                  state));
    } else {
        m_gaction = make_gaction_ptr(g_simple_action_new(name.c_str(),
                                                         parameterType));
    }

    m_activateHandlerId = g_signal_connect(m_gaction.get(),
                                           "activate",
                                           G_CALLBACK(Action::activate_cb),
                                           this);
    m_changeStateHandlerId = 0;
    m_changeStateHandlerId = g_signal_connect(m_gaction.get(),
                                              "change-state",
                                              G_CALLBACK(Action::change_state_cb),
                                              this);
}

Action::~Action()
{
    g_signal_handler_disconnect(m_gaction.get(), m_activateHandlerId);
    if (m_changeStateHandlerId)
    {
        g_signal_handler_disconnect(m_gaction.get(), m_changeStateHandlerId);
    }
}

std::string
Action::name()
{
    return m_name;
}

void
Action::setEnabled(bool enabled)
{
    if (g_action_get_enabled(m_gaction.get()) == enabled)
    {
        return;
    }

    g_simple_action_set_enabled(G_SIMPLE_ACTION(m_gaction.get()), enabled);
}

void
Action::setState(const Variant &value)
{
    if (value == state())
    {
        return;
    }

    g_simple_action_set_state(G_SIMPLE_ACTION(m_gaction.get()), value);

    Q_EMIT stateUpdated(state());
}

Variant
Action::state()
{
    return Variant::fromGVariant(
            g_action_get_state(G_ACTION(m_gaction.get())));
}

Action::GActionPtr
Action::gaction()
{
    return m_gaction;
}
