
set(
    MENUITEMS_CPP_UNIT_TESTS_SRC
    test-access-point-item.cpp
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
)

target_link_libraries(
    menuitems-cpp-unit-tests
    test-utils
    indicator-network-service-static
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
