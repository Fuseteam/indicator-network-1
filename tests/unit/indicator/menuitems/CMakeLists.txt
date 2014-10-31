
set(
    MENUITEMS_CPP_UNIT_TESTS_SRC
    test-access-point-item.cpp
    ${CMAKE_SOURCE_DIR}/src/indicator/menuitems/access-point-item.cpp
    test-switch-item.cpp
)

add_executable(
    menuitems-cpp-unit-tests
    ${MENUITEMS_CPP_UNIT_TESTS_SRC}
)

qt5_use_modules(
    menuitems-cpp-unit-tests
    Core
    DBus
    Test
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src/connectivity-cpp/include
    ${CMAKE_SOURCE_DIR}/src/connectivity-cpp/dbus-cpp
)

target_link_libraries(
    menuitems-cpp-unit-tests
    test-utils
    menumodel_cpp
    ${QTDBUSMOCK_LDFLAGS}
    ${QTDBUSTEST_LDFLAGS}
    ${QMENUMODEL_LDFLAGS}
    ${GIO_LDFLAGS}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
)

add_valgrind_test(
    menuitems-cpp-unit-tests
    menuitems-cpp-unit-tests
)
