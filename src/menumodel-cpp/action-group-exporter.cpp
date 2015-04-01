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

#include "action-group-exporter.h"

#include <unity/util/ResourcePtr.h>

using namespace std;
namespace util = unity::util;

ActionGroupExporter::ActionGroupExporter(SessionBus::Ptr sessionBus,
                                         ActionGroup::Ptr actionGroup,
                                         const std::string &path)
    : m_path(path),
      m_sessionBus(sessionBus),
      m_exportId {0},
      m_actionGroup {actionGroup}
{
    m_gSimpleActionGroup = make_gsimpleactiongroup_ptr();

    GError *error = NULL;
    m_exportId = g_dbus_connection_export_action_group(m_sessionBus->bus().get(),
                                                       path.c_str(),
                                                       G_ACTION_GROUP(m_gSimpleActionGroup.get()),
                                                       &error);
    if (error) {
        if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
            std::cerr << "Error exporting action group: " << error->message;
        }
        g_error_free(error);
        /// @todo throw something
        return;
    }


    for (auto action : actionGroup->actions()) {
        g_action_map_add_action(G_ACTION_MAP(m_gSimpleActionGroup.get()), action->gaction().get());
    }
    connect(actionGroup.get(), &ActionGroup::actionAdded, this, &ActionGroupExporter::actionAdded);
    connect(actionGroup.get(), &ActionGroup::actionRemoved, this, &ActionGroupExporter::actionRemoved);

    waitForFirstSignalEmission();
}

ActionGroupExporter::~ActionGroupExporter()
{
    if (!m_exportId)
        return;
    g_dbus_connection_unexport_action_group(m_sessionBus->bus().get(), m_exportId);
}

ActionGroupExporter::GSimpleActionGroupPtr
ActionGroupExporter::make_gsimpleactiongroup_ptr()
{
    return std::shared_ptr<GSimpleActionGroup>(g_simple_action_group_new(),
                                               GObjectDeleter());
}

void ActionGroupExporter::actionAdded(Action::Ptr action)
{
    g_action_map_add_action(G_ACTION_MAP(m_gSimpleActionGroup.get()), action->gaction().get());
}

void ActionGroupExporter::actionRemoved(Action::Ptr action)
{
    g_action_map_remove_action(G_ACTION_MAP(m_gSimpleActionGroup.get()),
                                       action->name().c_str());
}

void ActionGroupExporter::waitForFirstSignalEmission()
{
    shared_ptr<GMainLoop> loop(g_main_loop_new(nullptr, false), &g_main_loop_unref);

    /* Our two exit criteria */
    util::ResourcePtr<gulong,
        function<void(guint)>> signal(
            g_dbus_connection_signal_subscribe(m_sessionBus->bus().get(),
                g_dbus_connection_get_unique_name(m_sessionBus->bus().get()),
                "org.gtk.Actions",
                "Changed",
                m_path.c_str(),
                nullptr,
                G_DBUS_SIGNAL_FLAGS_NONE,
                [](GDBusConnection *,
                        const gchar *,
                        const gchar *,
                        const gchar *,
                        const gchar *,
                        GVariant *,
                        gpointer user_data)
                {
                    g_main_loop_quit((GMainLoop*) user_data);
                },
                loop.get(),
                nullptr),
        [this](guint s)
        {
            g_dbus_connection_signal_unsubscribe (m_sessionBus->bus().get(), s);
        });

    util::ResourcePtr<guint, function<void(guint)>> timer(g_timeout_add(200,
            [](gpointer user_data) -> gboolean
            {
                g_main_loop_quit((GMainLoop *)user_data);
                return G_SOURCE_CONTINUE;
            },
            loop.get()),
            &g_source_remove);

    /* Wait for sync */
    g_main_loop_run(loop.get());
}
