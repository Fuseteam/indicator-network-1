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

// guards access to __funcs
std::mutex GMainLoopDispatch::_lock;
std::list<GMainLoopDispatch::Func> GMainLoopDispatch::__funcs;

gboolean
GMainLoopDispatch::dispatch_cb(gpointer)
{
    std::unique_lock<std::mutex> lock(_lock);
    auto funcs = __funcs;
    __funcs.clear();
    lock.unlock();

    for (auto &func : funcs) {
        func();
    }
    return G_SOURCE_REMOVE;
}

GMainLoopDispatch::GMainLoopDispatch(Func func, int priority, bool force_delayed)
{
    if (!force_delayed &&
         g_main_context_acquire(g_main_context_default())) {
        // already running inside GMainLoop.
        func();
        g_main_context_release(g_main_context_default());
    } else {
        std::unique_lock<std::mutex> lock(_lock);
        if (__funcs.empty()) {
            g_idle_add_full(priority,
                            GSourceFunc(GMainLoopDispatch::dispatch_cb),
                            NULL,
                            NULL);
        }
        __funcs.push_back(func);
    }
}

