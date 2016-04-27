
add_definitions(-DNETWORK_SERVICE_BIN="${CMAKE_BINARY_DIR}/src/indicator/indicator-network-service")

include_directories(
    "${CMAKE_SOURCE_DIR}/src/connectivity-api/connectivity-qt"
    "${CMAKE_SOURCE_DIR}/src/qdbus-stubs"
    "${CMAKE_BINARY_DIR}/src/qdbus-stubs"
)

set(
    INTEGRATION_TESTS_SRC
    indicator-network-test-base.cpp
    test-indicator.cpp
    test-indicator-vpn.cpp
    test-connectivity-api.cpp
    test-connectivity-api-modem.cpp
    test-connectivity-api-sim.cpp
    test-connectivity-api-vpn.cpp
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
    ${CONNECTIVITY_QT_LIB_TARGET}
    ${TEST_DEPENDENCIES_LDFLAGS}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${GLIB_LDFLAGS}
)

add_test(
    integration-tests
    integration-tests
)
