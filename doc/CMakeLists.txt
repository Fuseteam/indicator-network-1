set(INSTALL_DOCDIR ${CMAKE_INSTALL_DATAROOTDIR}/doc/connectivity-api)

option(BUILD_DOC "Build documentation by default" TRUE)
option(PRIVATE_DOCS "Build private documentation" FALSE)

add_subdirectory(qt)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/
    DESTINATION ${INSTALL_DOCDIR}/dbus/
)
