
set(URL_DISPATCHER_CPP_SOURCES
  url-dispatcher.cpp
)

add_library(url_dispatcher_cpp STATIC ${URL_DISPATCHER_CPP_SOURCES})
target_link_libraries(
    url_dispatcher_cpp
    ${GLIB_LIBRARIES}
    ${URL_DISPATCHER_LIBRARIES}
)
