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
  url-dispatcher-1
)
include_directories(${GLIB_INCLUDE_DIRS})

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

find_package(Qt5Test REQUIRED)
include_directories(${Qt5Test_INCLUDE_DIRS})

pkg_check_modules(NM REQUIRED NetworkManager REQUIRED)
include_directories(${NM_INCLUDE_DIRS})

pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1 REQUIRED)
include_directories(${QTDBUSTEST_INCLUDE_DIRS})

pkg_check_modules(QTDBUSMOCK REQUIRED libqtdbusmock-1 REQUIRED)
include_directories(${QTDBUSMOCK_INCLUDE_DIRS})

pkg_check_modules(GIO REQUIRED gio-2.0>=${GLIB_REQUIRED_VERSION})
include_directories(${GIO_INCLUDE_DIRS})

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

#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
IF(build_type_lower MATCHES coverage)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
ENDIF()

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(po)

enable_testing()
add_subdirectory(tests)

ADD_CUSTOM_TARGET(
        check
        ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
)

#///! TODO: The following needs to be updated once this project takes form again and new unit tests are written
enable_coverage_report(TARGETS menumodel_cpp indicator-secret-agent
                       FILTER ${CMAKE_SOURCE_DIR}/tests/* ${CMAKE_BINARY_DIR}/*)
