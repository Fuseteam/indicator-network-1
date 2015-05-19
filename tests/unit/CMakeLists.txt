
#///! TODO: The following should be removed once all dependancy on qmenumodel is removed
pkg_check_modules(QMENUMODEL REQUIRED qmenumodel REQUIRED)
include_directories(${QMENUMODEL_INCLUDE_DIRS})

include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/indicator"
    "${CMAKE_SOURCE_DIR}/src/qdbus-stubs"
    "${CMAKE_BINARY_DIR}/src/qdbus-stubs"
    ${CMAKE_CURRENT_BINARY_DIR}
)

###########################
# Secret agent mini binary
###########################

add_definitions(
    -DSECRET_AGENT_BIN="${CMAKE_CURRENT_BINARY_DIR}/secret-agent-test-bin"
)

add_executable(
    secret-agent-test-bin
    secret-agent/secret-agent-main.cpp
)

qt5_use_modules(
    secret-agent-test-bin
    Core
)

target_link_libraries(
    secret-agent-test-bin
    indicator-network-service-static
)

###################
# Unit test binary
###################

set(
    UNIT_TESTS_SRC

    indicator/menuitems/test-access-point-item.cpp
    indicator/menuitems/test-switch-item.cpp

    menumodel-cpp/test-menu-exporter.cpp

    secret-agent/test-secret-agent.cpp
)

set_source_files_properties(
    "${DATA_DIR}/nm-secret-agent.xml"
    PROPERTIES
    INCLUDE "dbus-types.h"
)

qt5_add_dbus_interface(
    UNIT_TESTS_SRC
    "${DATA_DIR}/nm-secret-agent.xml"
    SecretAgentInterface
)

add_executable(
    unit-tests
    ${UNIT_TESTS_SRC}
)

qt5_use_modules(
    unit-tests
    Core
    DBus
    Test
)

target_link_libraries(
    unit-tests
    test-utils
    indicator-network-service-static
    ${QTDBUSMOCK_LDFLAGS}
    ${QTDBUSTEST_LDFLAGS}
    ${QMENUMODEL_LDFLAGS}
    ${GLIB_LDFLAGS}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
)

add_valgrind_test(
    unit-tests
    unit-tests
)
