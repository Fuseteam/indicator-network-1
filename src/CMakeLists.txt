include_directories("${CMAKE_SOURCE_DIR}/src/")
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")
include_directories("${CMAKE_BINARY_DIR}/src/qdbus-stubs")

add_subdirectory(qdbus-stubs)
add_subdirectory(util)

add_subdirectory(connectivity-api)

add_subdirectory(agent)
add_subdirectory(indicator)
add_subdirectory(menumodel-cpp)
add_subdirectory(qpowerd)
add_subdirectory(notify-cpp)
add_subdirectory(url-dispatcher-cpp)
