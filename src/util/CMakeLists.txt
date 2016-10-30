set(UTIL_SOURCES
    dbus-property-cache.cpp
    dbus-utils.cpp
    logging.cpp
    unix-signal-handler.cpp
)

add_library(util STATIC ${UTIL_SOURCES})
target_link_libraries(
    util
    qdbus-stubs
)
