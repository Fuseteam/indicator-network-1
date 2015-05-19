# Copyright © 2013 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License version 3,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authors:
#     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>

string(TOLOWER "${CMAKE_BUILD_TYPE}" typelower)
set(EXTRACT_PRIVATE NO)
set(INTERNAL_DOCS NO)
set(SOURCE_DIRS "${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}")

if("${typelower}" STREQUAL "debug")
  option(BUILD_DOC "Build documentation by default" FALSE)
else()
  option(BUILD_DOC "Build documentation by default" TRUE)
endif()

option(PRIVATE_DOCS "Build private documentation" FALSE)

find_package(Doxygen)

if(${BUILD_DOC})
  set(IS_ALL ALL)
endif()
if(${PRIVATE_DOCS})
  set(EXTRACT_PRIVATE YES)
  set(INTERNAL_DOCS YES)
  set(SOURCE_DIRS "${SOURCE_DIRS} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/dbus")
endif()

if (DOXYGEN_FOUND)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in 
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(connectivity-doc ${IS_ALL}
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
  install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION ${CMAKE_INSTALL_DOCDIR})
  install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/xml
    DESTINATION ${CMAKE_INSTALL_DOCDIR})

endif (DOXYGEN_FOUND)

# list these here to have them visible in Qt Creator
set(DOC_PAGES
    pages/mainpage.dox
    pages/using.dox
    pages/networking-status.dox
)
add_custom_target(QtCreatorDocsHack SOURCES ${DOC_PAGES})
