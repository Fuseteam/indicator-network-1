
file(GLOB_RECURSE SCRIPT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.sh
)
add_custom_target(scriptfiles SOURCES ${SCRIPT_FILES})
