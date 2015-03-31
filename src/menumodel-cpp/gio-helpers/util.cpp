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

#include "util.h"

#include <unity/util/ResourcePtr.h>
#include <glib.h>

using namespace std;
namespace util = unity::util;

void runGMainloop(guint ms)
{
    shared_ptr<GMainLoop> loop(g_main_loop_new(nullptr, false), &g_main_loop_unref);
    util::ResourcePtr<guint, function<void(guint)>> timer(g_timeout_add(ms,
        [](gpointer user_data) -> gboolean
        {
            g_main_loop_quit((GMainLoop *)user_data);
            return G_SOURCE_CONTINUE;
        },
        loop.get()),
        &g_source_remove);
    g_main_loop_run(loop.get());
}
