
set(CONNECTIVITY_QT_INCLUDE_DIR
"${CMAKE_INSTALL_FULL_INCLUDEDIR}/connectivity-api/qt${CONNECTIVITY_QT_VERSION_MAJOR}")

set(PUBLIC_HEADER_DIR "${CMAKE_SOURCE_DIR}/src/connectivity-api/connectivity-qt/ubuntu/connectivity")
set(PUBLIC_HEADERS
    ${PUBLIC_HEADER_DIR}/networking-status.h
)
install(FILES ${PUBLIC_HEADERS}
        DESTINATION "${CONNECTIVITY_QT_INCLUDE_DIR}/ubuntu/connectivity")



set(SYMBOL_MAP "${DATA_DIR}/connectivity-qt.map")

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
    ${CONNECTIVITY_QT_LIB_TARGET}
    SHARED
    ${CONNECTIVITY_QT_SRC}
    ${PUBLIC_HEADERS} # public headers have to be included here for Q_DECL_EXPORT to work properly
)

qt5_use_modules(
    ${CONNECTIVITY_QT_LIB_TARGET}
    Core
    DBus
)

target_link_libraries(
    ${CONNECTIVITY_QT_LIB_TARGET}
    qdbus-stubs
)

set(SO_VERSION ${CONNECTIVITY_QT_VERSION_MAJOR})
set_target_properties(
    ${CONNECTIVITY_QT_LIB_TARGET}
    PROPERTIES
    SOVERSION ${SO_VERSION}
    LINK_FLAGS "-Wl,--version-script,${SYMBOL_MAP}"
    LINK_DEPENDS "${SYMBOL_MAP}"
)

install(
    TARGETS ${CONNECTIVITY_QT_LIB_TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(PC_FILE_TARGET "${CMAKE_BINARY_DIR}/${CONNECTIVITY_QT_LIB_TARGET}.pc")
set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
set(includedir "${CONNECTIVITY_QT_INCLUDE_DIR}")
set(ABSOLUTE_SO_FILE "${CMAKE_INSTALL_FULL_LIBDIR}/lib${CONNECTIVITY_QT_LIB_TARGET}.so.${SO_VERSION}")
configure_file("connectivity-qt.pc.in" ${PC_FILE_TARGET} @ONLY)
install(
  FILES ${PC_FILE_TARGET}
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
