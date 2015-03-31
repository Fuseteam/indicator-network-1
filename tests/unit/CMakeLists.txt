#///! TODO: The following should be removed once all dependancy on qmenumodel is removed
pkg_check_modules(QMENUMODEL REQUIRED qmenumodel REQUIRED)
include_directories(${QMENUMODEL_INCLUDE_DIRS})

add_subdirectory(indicator)
add_subdirectory(menumodel-cpp)
add_subdirectory(secret-agent)
