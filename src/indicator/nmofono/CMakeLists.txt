# Copyright © 2013 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License version 3,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authors:
#     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>

set(NMOFONO_PLATFORM_SOURCES
    kill-switch.cpp
    manager.cpp
    manager.h
    service.cpp
    service.h
    wifi/access-point.h
    wifi/access-point.cpp
    wifi/grouped-access-point.h
    wifi/grouped-access-point.cpp
    wifi/link.h
    wifi/link.cpp
    ../util.cpp

    #ofono_nm_connectivity_manager.cpp
    set_name_for_thread.cpp
    set_name_for_thread.h
)

add_library(platform_nmofono STATIC ${NMOFONO_PLATFORM_SOURCES})
target_link_libraries(
    platform_nmofono
    ${DBUSCPP_LIBRARIES}
    ${GLIB_LIBRARIES}
)
