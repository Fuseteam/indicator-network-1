project(indicator-network C CXX)
cmake_minimum_required(VERSION 2.8.9)

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

if("${build_type_lower}" STREQUAL debug)
  set(TRACE_DEFAULT TRUE)
else()
  set(TRACE_DEFAULT FALSE)
endif()

option(trace_messages "Print debug trace messages." ${TRACE_DEFAULT})
option(REMOTE_BUILD "Remote build (skip docs, translations, tests)." FALSE)

if(${trace_messages})
  add_definitions(-DINDICATOR_NETWORK_TRACE_MESSAGES)
endif()

add_definitions(
  -DQT_NO_KEYWORDS=1
)

find_package(PkgConfig REQUIRED)
include(EnableCoverageReport)
include(GNUInstallDirs)
include(UseGSettings)

set(GETTEXT_PACKAGE indicator-network)
set(LOCALE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/locale")

# Workaround for libexecdir on debian
if (EXISTS "/etc/debian_version")
  set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR})
  set(CMAKE_INSTALL_FULL_LIBEXECDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}")
endif()

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")

set(GLIB_REQUIRED_VERSION 2.26)

pkg_check_modules(
  GLIB REQUIRED
  glib-2.0>=${GLIB_REQUIRED_VERSION}
  gio-2.0>=${GLIB_REQUIRED_VERSION}
)
include_directories(${GLIB_INCLUDE_DIRS})

pkg_check_modules(
  UNITY_API REQUIRED
  libunity-api
)
include_directories(${UNITY_API_INCLUDE_DIRS})

set(OFONO_REQUIRED_VERSION 1.12)
pkg_check_modules(
  OFONO REQUIRED
  ofono>=${OFONO_REQUIRED_VERSION}
)
include_directories(${OFONO_INCLUDE_DIRS})

set(NOTIFY_REQUIRED_VERSION 0.7.5)
pkg_check_modules(
  NOTIFY REQUIRED
  libnotify>=${NOTIFY_REQUIRED_VERSION}
)
include_directories(${NOTIFY_INCLUDE_DIRS})

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

find_package(Qt5DBus COMPONENTS Qt5DBusMacros REQUIRED)
include_directories(${Qt5DBus_INCLUDE_DIRS})

pkg_check_modules(NM REQUIRED NetworkManager REQUIRED)
include_directories(${NM_INCLUDE_DIRS})

pkg_check_modules(URL_DISPATCHER REQUIRED url-dispatcher-1)
include_directories(${URL_DISPATCHER_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${CMAKE_BINARY_DIR})

set(COMMON_FLAGS "-Wall -Wextra -Wpedantic -fno-permissive -fPIC -fvisibility=hidden -pthread")

# "nice bug" in cmake... http://www.cmake.org/Bug/view.php?id=15058
# let's not set C_FLAGS as it will break pthreads detection \o/
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ${COMMON_FLAGS} -fno-strict-aliasing -fvisibility-inlines-hidden")

configure_file(
  "config.h.in"
  "config.h"
)

set(CONNECTIVITY_QT_VERSION_MAJOR 1)
set(CONNECTIVITY_QT_VERSION_MINOR 0)
set(CONNECTIVITY_QT_LIB_TARGET connectivity-qt${CONNECTIVITY_QT_VERSION_MAJOR})

add_subdirectory(data)
add_subdirectory(src)

if(NOT REMOTE_BUILD)

add_subdirectory(po)
add_subdirectory(doc)

enable_testing()
add_subdirectory(tests)

ADD_CUSTOM_TARGET(
        check
        ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
)

enable_coverage_report(
  TARGETS
    indicator-network-service
    menumodel_cpp
    notify_cpp
    qdbus-stubs
    url_dispatcher_cpp
  TESTS
    unit-tests
    integration-tests
  FILTER
    ${CMAKE_SOURCE_DIR}/tests/*
    ${CMAKE_BINARY_DIR}/*
)

endif()
