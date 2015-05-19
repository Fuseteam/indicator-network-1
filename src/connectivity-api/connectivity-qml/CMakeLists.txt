 
include(QmlPlugins)

find_package(Qt5Quick REQUIRED)

include_directories(
  "${CMAKE_SOURCE_DIR}/src/connectivity-api/connectivity-qt"
)

set(CONNECTIVITY_QML_SRC
  plugin.cpp
)

add_library(connectivity-qml MODULE
  ${CONNECTIVITY_QML_SRC}
)

qt5_use_modules(connectivity-qml
  Qml
)

target_link_libraries(
  connectivity-qml
  connectivity-qt
)

add_qmlplugin(
  Ubuntu.Connectivity
  1.0
  Ubuntu/Connectivity
  TARGET_PREFIX Connectivity
  TARGETS connectivity-qml
)
