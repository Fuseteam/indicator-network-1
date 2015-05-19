
include_directories(${CMAKE_SOURCE_DIR}/src/connectivity-api/connectivity-qt)
add_executable(
  networking_status
  example_networking_status.cpp
)
target_link_libraries(networking_status ${CONNECTIVITY_QT_LIB_TARGET})
