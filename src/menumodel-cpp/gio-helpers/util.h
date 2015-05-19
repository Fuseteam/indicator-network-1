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

#include <gio/gio.h>
#include <memory>
#include <iostream>
#include <vector>
#include <list>

void runGMainloop(guint ms = 50);

struct GObjectDeleter {
    GObjectDeleter() noexcept {}
   ~GObjectDeleter() noexcept {}

    void operator() (void *ptr) const
    {
        if (!ptr)
            return;
        GObject *obj = G_OBJECT(ptr);
        if (obj) {
            g_clear_object(&obj);
        }
    }
};

struct GVariantDeleter {
    GVariantDeleter() noexcept {}
    ~GVariantDeleter() noexcept {}

    void operator() (GVariant *ptr) const
    {
        if (!ptr)
            return;
        if (g_variant_is_floating(ptr))
            g_variant_ref_sink(ptr);
        g_variant_unref(ptr);
    }
};

namespace {

typedef std::shared_ptr<GVariant> GVariantPtr;
inline GVariantPtr make_gvariant_ptr(GVariant *ptr)
{
    if (ptr && g_variant_is_floating(ptr))
        g_variant_ref_sink(ptr);
    return GVariantPtr(ptr, GVariantDeleter());
}

typedef std::shared_ptr<GMenu> GMenuPtr;
inline GMenuPtr make_gmenu_ptr() { return std::shared_ptr<GMenu>(g_menu_new(), GObjectDeleter()); }

typedef std::shared_ptr<GMenuItem> GMenuItemPtr;
inline GMenuItemPtr make_gmenuitem_ptr(GMenuItem *gmenuitem) { return std::shared_ptr<GMenuItem>(gmenuitem, GObjectDeleter()); }

// returns the index of the appended item.
inline int append_item_to_gmenu(GMenu *menu, GMenuItem *item)
{
    int n_items = g_menu_model_get_n_items(G_MENU_MODEL(menu));
    g_menu_insert_item(menu, n_items, item);
    return n_items;
}

}

class SessionBus
{
    std::shared_ptr<GDBusConnection> m_bus;

public:
    typedef std::shared_ptr<SessionBus> Ptr;
    typedef std::unique_ptr<SessionBus> UPtr;

    SessionBus()
    {
        GError *error = nullptr;

        gchar *address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SESSION,
                                                         nullptr, &error);
        if (!address)
        {
            g_assert(error != nullptr);
            if (error->domain != G_IO_ERROR
                    || error->code != G_IO_ERROR_CANCELLED)
            {
                std::cerr << "Error getting the bus address: " << error->message;
            }
            g_error_free(error);
            /// @todo throw something
            return;
        }

        error = nullptr;
        m_bus.reset(
                g_dbus_connection_new_for_address_sync(
                        address,
                        (GDBusConnectionFlags) (G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT
                                | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
                        nullptr,
                        nullptr,
                        &error),
                GObjectDeleter());
        g_free(address);

        if (!m_bus) {
            g_assert(error != nullptr);
            if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
                std::cerr << "Error getting the bus: " << error->message;
            }
            g_error_free(error);
            /// @todo throw something
            return;
        }

        g_dbus_connection_set_exit_on_close(m_bus.get(), FALSE);
    }

    std::shared_ptr<GDBusConnection> bus() const
    {
        return m_bus;
    }

    std::string
    address ()
    {
        return std::string(g_dbus_connection_get_unique_name(m_bus.get()));
    }
};

class BusName
{
    guint m_busOwnId;
    std::function<void(std::string)> m_acquired;
    std::function<void(std::string)> m_lost;
    std::shared_ptr<SessionBus> m_bus;

    static void nameAcquired(GDBusConnection *,
                             const gchar *name,
                             gpointer user_data)
    {
        BusName *that = static_cast<BusName *>(user_data);
        that->m_acquired(name);
    }

    static void nameLost(GDBusConnection *,
                         const gchar *name,
                         gpointer user_data)
    {
        BusName *that = static_cast<BusName *>(user_data);
        that->m_lost(name);
    }

public:
    typedef std::unique_ptr<BusName> UPtr;

    BusName(std::string name,
            std::function<void(std::string)> acquired,
            std::function<void(std::string)> lost,
            std::shared_ptr<SessionBus> bus)
        : m_acquired {acquired},
          m_lost {lost},
          m_bus {bus}
    {
        m_busOwnId =
            g_bus_own_name_on_connection(m_bus->bus().get(),
                                         name.c_str(),
                                         G_BUS_NAME_OWNER_FLAGS_NONE,
                                         (GBusNameAcquiredCallback)nameAcquired,
                                         (GBusNameLostCallback)nameLost,
                                         this,
                                         nullptr);

    }
    ~BusName()
    {
        if (m_busOwnId != 0)
            g_bus_unown_name(m_busOwnId);
    }
};
