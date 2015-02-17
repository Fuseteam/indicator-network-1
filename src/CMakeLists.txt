
if(NOT build_type_lower MATCHES debug)
    set(COMMON_FLAGS "${COMMON_FLAGS} -Werror")
endif()

add_subdirectory(dbus-cpp)
add_subdirectory(connectivity-cpp)
add_subdirectory(indicator)
add_subdirectory(menumodel-cpp)
add_subdirectory(notify-cpp)
add_subdirectory(secret-agent)
add_subdirectory(url-dispatcher-cpp)
