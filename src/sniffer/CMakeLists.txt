
include_directories("${CMAKE_BINARY_DIR}/src/qdbus-stubs")
include_directories("${CMAKE_SOURCE_DIR}/src/qdbus-stubs")

add_executable(i-n-sniffer
eventprinter.cpp
eventprinter.h
i-n-sniffer.cpp
)

qt5_use_modules(i-n-sniffer Core DBus)

target_link_libraries(
  i-n-sniffer
  qdbus-stubs
  ${QOFONO_LIBRARIES}
)

install(
  TARGETS i-n-sniffer
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
