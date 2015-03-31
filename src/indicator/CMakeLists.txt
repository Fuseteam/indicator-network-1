
pkg_check_modules(
    NETWORKMANAGER REQUIRED
    NetworkManager
)
include_directories(${NETWORKMANAGER_INCLUDE_DIRS})

pkg_check_modules(
    QOFONO REQUIRED
    qofono-qt5
)
include_directories(${QOFONO_INCLUDE_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_BINARY_DIR}/src/qdbus-stubs")
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")

# list these here to have them visible in Qt Creator
file(
        GLOB_RECURSE NETWORK_SERVICE_HEADERS
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
add_custom_target(QtCreatorHeaderHack SOURCES ${NETWORK_SERVICE_HEADERS})

set(NETWORK_SERVICE_SOURCES
    ${NETWORK_SERVICE_HEADERS}

    nmofono/kill-switch-impl.cpp
    nmofono/manager.cpp
    nmofono/manager-impl.cpp
    nmofono/wifi/access-point.cpp
    nmofono/wifi/access-point-impl.cpp
    nmofono/wifi/grouped-access-point-impl.cpp
    nmofono/wifi/wifi-link-impl.cpp

    factory.cpp
    indicator-menu.cpp
    menu-builder.cpp
    modem.cpp
    modem-manager.cpp
    sim-unlock-dialog.cpp
    root-state.cpp
    sections/quick-access-section.cpp
    sections/wifi-section.cpp
    sections/wwan-section.cpp

    connectivity-service/connectivity-service.cpp

    menuitems/access-point-item.cpp
    menuitems/switch-item.cpp
    menuitems/text-item.cpp
    menuitems/wifi-link-item.cpp
    menuitems/wwan-link-item.cpp
    menuitems/modem-info-item.cpp
    
    util/unix-signal-handler.cpp
)

qt5_add_dbus_adaptor(
    NETWORK_SERVICE_SOURCES
    "${DATA_DIR}/com.ubuntu.connectivity1.NetworkingStatus.xml"
    connectivity-service/connectivity-service.h
    connectivity_service::ConnectivityService
    NetworkingStatusAdaptor
)

qt5_add_dbus_adaptor(
    NETWORK_SERVICE_SOURCES
    "${DATA_DIR}/com.ubuntu.connectivity1.Private.xml"
    connectivity-service/connectivity-service.h
    connectivity_service::PrivateService
    NetworkingStatusPrivateAdaptor
)

add_library(
  indicator-network-service-static
  STATIC
  ${NETWORK_SERVICE_SOURCES}
)

target_link_libraries(
    indicator-network-service-static
    menumodel_cpp
    notify_cpp
    url_dispatcher_cpp
    qdbus-stubs
    ${DBUSCPP_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    ${QOFONO_LIBRARIES}
)

qt5_use_modules(
    indicator-network-service-static
    Core
    DBus
)

###########################
# Executable
###########################

add_executable(
  indicator-network-service
  main.cpp
)

target_link_libraries(
    indicator-network-service
    indicator-network-service-static
)

qt5_use_modules(
    indicator-network-service
    Core
    DBus
)

###########################
# Installation
###########################

install(
  TARGETS indicator-network-service
  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/indicator-network/"
)
