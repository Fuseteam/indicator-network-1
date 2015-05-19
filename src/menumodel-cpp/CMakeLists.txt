
set(MENUMODEL_CPP_SOURCES
    gio-helpers/util.cpp
    gio-helpers/variant.h

    action.h
    action.cpp
    action-group.cpp
    action-group.h
    action-group-exporter.cpp
    action-group-exporter.h
    action-group-merger.cpp
    action-group-merger.h
    menu.cpp
    menu.h
    menu-exporter.h
    menu-item.h
    menu-item.cpp
    menu-merger.h
    menu-model.h
)

add_library(menumodel_cpp STATIC ${MENUMODEL_CPP_SOURCES})
target_link_libraries(
    menumodel_cpp
    ${GLIB_LIBRARIES}
)

