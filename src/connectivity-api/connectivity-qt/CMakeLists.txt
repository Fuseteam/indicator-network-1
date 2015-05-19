
set(API_VERSION 1)
set(SOVERSION 1)
set(SYMBOL_MAP "${DATA_DIR}/connectivity-qt.map")
set(OUTPUT_NAME "connectivity-qt${SOVERSION}")
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")

set(
    CONNECTIVITY_QT_SRC
    connectivityqt/internal/dbus-property-cache.cpp
    connectivityqt/connectivity.cpp
    ubuntu/connectivity/networking-status.cpp
)

set_source_files_properties(
    "${DATA_DIR}/org.freedesktop.DBus.Properties.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.NetworkingStatus.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.Private.xml"
    PROPERTIES
    NO_NAMESPACE YES
)


qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/com.ubuntu.connectivity1.NetworkingStatus.xml"
    NetworkingStatusInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/com.ubuntu.connectivity1.Private.xml"
    NetworkingStatusPrivateInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/org.freedesktop.DBus.Properties.xml"
    PropertiesInterface
)

add_library(
    connectivity-qt
    SHARED
    ${CONNECTIVITY_QT_SRC}
)

qt5_use_modules(
    connectivity-qt
    Core
    DBus
)

target_link_libraries(
    connectivity-qt
    qdbus-stubs
)

set_target_properties(
    connectivity-qt
    PROPERTIES
    OUTPUT_NAME ${OUTPUT_NAME}
    VERSION ${API_VERSION}.0.0
    SOVERSION ${SOVERSION}
    LINK_FLAGS "-Wl,--version-script,${SYMBOL_MAP}"
    LINK_DEPENDS "${SYMBOL_MAP}"
)

install(
    TARGETS connectivity-qt
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION "include"
    FILES_MATCHING PATTERN "*.h"
    PATTERN "internal" EXCLUDE
)

