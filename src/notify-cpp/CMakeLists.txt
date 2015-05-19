include_directories("${CMAKE_SOURCE_DIR}/src/")

set(NOTIFY_CPP_SOURCES
    notification.cpp
    snapdecision/sim-unlock.cpp
)

add_library(notify_cpp STATIC ${NOTIFY_CPP_SOURCES})
target_link_libraries(
    notify_cpp
    ${GLIB_LIBRARIES}
)
