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


include_directories(${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR})

if (NOT DEFINED GTEST_ROOT)
    set(GTEST_ROOT /usr/src/gtest)
endif()

set(GTEST_SRC_DIR "${GTEST_ROOT}/src")
set(GTEST_INCLUDE_DIR ${GTEST_ROOT})

add_library(connectivity-gtest STATIC
${GTEST_SRC_DIR}/gtest-all.cc
${GTEST_SRC_DIR}/gtest_main.cc

)

set_target_properties(connectivity-gtest PROPERTIES INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR})
target_link_libraries(connectivity-gtest ${CMAKE_THREAD_LIBS_INIT})

include_directories(${GTEST_INCLUDE_DIRS})

add_definitions(-DCORE_DBUS_ENABLE_GOOGLE_TEST_FIXTURE)

pkg_check_modules(PROCESS_CPP process-cpp)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_data.h @ONLY
)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src/connectivity-cpp/include
    ${CMAKE_SOURCE_DIR}/src/connectivity-cpp/dbus-cpp
)

add_executable (
    test_networking_status
    test_networking_status.cpp
)

add_executable (
    test_flight_mode
    test_flight_mode.cpp
)

add_executable (
    test_wifi_link
    test_wifi_link.cpp
)

add_executable (
    test_wifi_link_dedup
    test_wifi_link_dedup.cpp
)

add_executable (
    test_wifi_connect
    test_wifi_connect.cpp
)

add_executable (
    test_wifi_link_enable_disable
    test_wifi_link_enable_disable.cpp
)

target_link_libraries(
    test_networking_status
    connectivity-cpp
    connectivity-gtest
    ${PROCESS_CPP_LIBRARIES}
)

target_link_libraries(
    test_flight_mode
    connectivity-cpp
    connectivity-gtest
    ${PROCESS_CPP_LIBRARIES}
)

target_link_libraries(
    test_wifi_link
    connectivity-cpp
    connectivity-gtest
    ${PROCESS_CPP_LIBRARIES}
)

target_link_libraries(
    test_wifi_link_dedup
    connectivity-cpp
    connectivity-gtest
    ${PROCESS_CPP_LIBRARIES}
)

target_link_libraries(
    test_wifi_connect
    connectivity-cpp
    connectivity-gtest
    ${PROCESS_CPP_LIBRARIES}
)

target_link_libraries(
    test_wifi_link_enable_disable
    connectivity-cpp
    connectivity-gtest
    ${PROCESS_CPP_LIBRARIES}
)

add_test(test_networking_status ${CMAKE_CURRENT_BINARY_DIR}/test_networking_status)
add_test(test_flight_mode ${CMAKE_CURRENT_BINARY_DIR}/test_flight_mode)
add_test(test_wifi_link ${CMAKE_CURRENT_BINARY_DIR}/test_wifi_link)
add_test(test_wifi_link_dedup ${CMAKE_CURRENT_BINARY_DIR}/test_wifi_link_dedup)
add_test(test_wifi_connect ${CMAKE_CURRENT_BINARY_DIR}/test_wifi_connect)
add_test(test_wifi_link_enable_disable ${CMAKE_CURRENT_BINARY_DIR}/test_wifi_link_enable_disable)
