
set(CONNECTIVITY_QT_INCLUDE_DIR
"${CMAKE_INSTALL_FULL_INCLUDEDIR}/connectivity-api/qt${CONNECTIVITY_QT_VERSION_MAJOR}")

install(
  DIRECTORY
    "ubuntu"
    "connectivityqt"
  DESTINATION
    "${CONNECTIVITY_QT_INCLUDE_DIR}"
  FILES_MATCHING
    PATTERN "*internal*" EXCLUDE
    PATTERN "*.h"
)


set(SYMBOL_MAP "${DATA_DIR}/connectivity-qt.map")

set(
    CONNECTIVITY_QT_SRC
    connectivityqt/connectivity.cpp
    connectivityqt/modem.cpp
    connectivityqt/modems-list-model.cpp
    connectivityqt/sim.cpp
    connectivityqt/sims-list-model.cpp
    connectivityqt/openvpn-connection.cpp
    connectivityqt/pptp-connection.cpp
    connectivityqt/vpn-connection.cpp
    connectivityqt/vpn-connections-list-model.cpp
    ubuntu/connectivity/networking-status.cpp
)

set_source_files_properties(
    "${DATA_DIR}/com.ubuntu.connectivity1.NetworkingStatus.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.Private.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.Modem.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.Sim.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.vpn.VpnConnection.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.vpn.VpnConnection.OpenVpn.xml"
    "${DATA_DIR}/com.ubuntu.connectivity1.vpn.VpnConnection.Pptp.xml"
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
    "${DATA_DIR}/com.ubuntu.connectivity1.Modem.xml"
    ModemInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/com.ubuntu.connectivity1.Sim.xml"
    SimInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/com.ubuntu.connectivity1.vpn.VpnConnection.xml"
    VpnConnectionInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/com.ubuntu.connectivity1.vpn.VpnConnection.OpenVpn.xml"
    OpenVpnConnectionInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_QT_SRC
    "${DATA_DIR}/com.ubuntu.connectivity1.vpn.VpnConnection.Pptp.xml"
    PptpConnectionInterface
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
    util
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
