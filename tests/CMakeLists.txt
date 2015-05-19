
include(FindGMock)

find_package("Valgrind" REQUIRED)

pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1 REQUIRED)
include_directories(${QTDBUSTEST_INCLUDE_DIRS})

pkg_check_modules(QTDBUSMOCK REQUIRED libqtdbusmock-1 REQUIRED)
include_directories(${QTDBUSMOCK_INCLUDE_DIRS})

find_package(Qt5Test REQUIRED)
include_directories(${Qt5Test_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(autopilot)
add_subdirectory(integration)
add_subdirectory(data)
add_subdirectory(menuharness)
add_subdirectory(unit)
add_subdirectory(utils)
