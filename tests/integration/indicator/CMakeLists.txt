
add_definitions(-DNETWORK_SERVICE_BIN="${CMAKE_BINARY_DIR}/src/indicator/indicator-network-service")

set(
    INTEGRATION_TESTS_SRC
    TestIndicatorNetworkService.cpp
)

add_executable(
    integration-tests
    ${INTEGRATION_TESTS_SRC}
)

qt5_use_modules(
    integration-tests
    Core
    DBus
    Test
)

target_link_libraries(
    integration-tests
    test-utils
    menuharness
    ${QTDBUSMOCK_LDFLAGS}
    ${QTDBUSTEST_LDFLAGS}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
)

add_test(
    integration-tests
    integration-tests
)
