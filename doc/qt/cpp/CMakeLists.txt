
add_subdirectory(examples)

include(UseDoxygen OPTIONAL)

add_doxygen(
    qtdoc
    PROJECT_NAME
        "connectivity-api"
    INPUT
        "${CMAKE_SOURCE_DIR}/src/connectivity-api/connectivity-qt"
        "${CMAKE_CURRENT_SOURCE_DIR}/pages"
    STRIP_FROM_PATH
        "${CMAKE_SOURCE_DIR}/src/connectivity-api/connectivity-qt"
    STRIP_FROM_INC_PATH
        "${CMAKE_SOURCE_DIR}/src/connectivity-api/connectivity-qt"
    EXCLUDE_PATTERNS
        */internal/*
    EXCLUDE_SYMBOLS
	*::internal*
        *::Priv
    INSTALL
        "${INSTALL_DOCDIR}/cpp"
    XML
    ALL
)

