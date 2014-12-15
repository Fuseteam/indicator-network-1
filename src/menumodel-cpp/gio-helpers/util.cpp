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

std::mutex GMainLoopDispatch::_lock;
std::list<GMainLoopDispatch::Func *> GMainLoopDispatch::_funcs;

gboolean
GMainLoopDispatch::dispatch_cb(gpointer)
{
    std::unique_lock<std::mutex> lock(_lock);
    for (auto func : _funcs) {
        lock.unlock();
        (*func)();
        lock.lock();
        delete func;
    }
    _funcs.clear();
    return G_SOURCE_REMOVE;
}

GMainLoopDispatch::GMainLoopDispatch(Func func, int priority, bool force_delayed)
{
    std::unique_lock<std::mutex> lock(_lock);

    if (!force_delayed &&
         g_main_context_acquire(g_main_context_default())) {
        // already running inside GMainLoop..
        // free the lock and dispatch immediately.
        lock.unlock();
        func();
        g_main_context_release(g_main_context_default());
    } else {
        std::function<void()> *funcPtr = new std::function<void()>(func);
        if (_funcs.empty()) {
            g_idle_add_full(priority,
                            GSourceFunc(GMainLoopDispatch::dispatch_cb),
                            NULL,
                            NULL);
        }
        _funcs.push_back(funcPtr);
    }
}

