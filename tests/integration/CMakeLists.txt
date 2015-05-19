
add_definitions(-DNETWORK_SERVICE_BIN="${CMAKE_BINARY_DIR}/src/indicator/indicator-network-service")

include_directories(
    "${CMAKE_SOURCE_DIR}/src/connectivity-api/connectivity-qt"
    "${CMAKE_SOURCE_DIR}/src/qdbus-stubs"
)

set(
    INTEGRATION_TESTS_SRC
    indicator-network-test-base.cpp
    test-indicator.cpp
    test-connectivity-api.cpp
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
    ${CONNECTIVITY_QT_LIB_TARGET}
    ${QTDBUSMOCK_LDFLAGS}
    ${QTDBUSTEST_LDFLAGS}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
)

add_test(
    integration-tests
    integration-tests
)
