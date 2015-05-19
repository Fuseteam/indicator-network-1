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


add_executable(
  example_airplane_mode
  example_airplane_mode.cpp
)
target_link_libraries(example_airplane_mode connectivity-cpp)

add_executable(
  example_available_links
  example_available_links.cpp
)
target_link_libraries(example_available_links connectivity-cpp)


add_executable(
  example_connect_to_wifi_network
  example_connect_to_wifi_network.cpp
)
target_link_libraries(example_connect_to_wifi_network connectivity-cpp)


add_executable(
  example_networking_status
  example_networking_status.cpp
)
target_link_libraries(example_networking_status connectivity-cpp)


add_executable(
  example_tor_over_vpn
  example_tor_over_vpn.cpp
)
target_link_libraries(example_tor_over_vpn connectivity-cpp)

