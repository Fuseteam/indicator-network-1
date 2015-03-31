
set(CONNECTIVITY_CPP_VERSION_MAJOR 0)
set(CONNECTIVITY_CPP_VERSION_MINOR 0)
set(CONNECTIVITY_CPP_VERSION_PATCH 1)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/backend"
    "${CMAKE_CURRENT_BINARY_DIR}/backend"
)
pkg_check_modules(
    PROPERTIESCPP REQUIRED
    properties-cpp
)
include_directories(${PROPERTIESCPP_INCLUDE_DIRS})

pkg_check_modules(
    DBUSCPP REQUIRED
    dbus-cpp
)
include_directories(${DBUSCPP_INCLUDE_DIRS})

pkg_check_modules(
    NETWORKMANAGER REQUIRED
    NetworkManager
)
include_directories(${NETWORKMANAGER_INCLUDE_DIRS})

# list these here to have them visible in Qt Creator
set(CONNECTIVITY_INTERFACES
    include/connectivity/networking/link.h
    include/connectivity/networking/manager.h
    include/connectivity/networking/service.h

    include/connectivity/networking/service/tethering/service.h
    include/connectivity/networking/service/tor/service.h
    include/connectivity/networking/service/vpn/service.h

    include/connectivity/networking/wifi/access-point.h
    include/connectivity/networking/wifi/link.h

    include/connectivity/networking/wwan/link.h
)
add_custom_target(QtCreatorHeaderHack SOURCES ${CONNECTIVITY_INTERFACES})

add_subdirectory(data)
#add_subdirectory(doc)
#add_subdirectory(examples)
add_subdirectory(src)
