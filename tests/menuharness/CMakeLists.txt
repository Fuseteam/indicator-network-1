pkg_check_modules(UNITY_API libunity-api>=0.1.3 REQUIRED)
include_directories(${UNITY_API_INCLUDE_DIRS})

add_library(
    menuharness
    MatchResult.cpp
    MatchUtils.cpp
    MenuItemMatcher.cpp
    MenuMatcher.cpp
)

target_link_libraries(
    menuharness
    ${GLIB_LDFLAGS}
)
