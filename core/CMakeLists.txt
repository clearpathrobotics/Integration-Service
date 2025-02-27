# Copyright 2019 Open Source Robotics Foundation, Inc.
# Copyright (C) 2020 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# is-core library and integration-service executable CMake project

###############################################################################
# CMake build rules for the Integration Service Core library
###############################################################################
cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

###############################################################################
# Build options
# TODO (@jamoralp): Add BUILD_EXECUTABLE option.
###############################################################################
option(IS_COMPILE_DEBUG "Compile the Integration Service project in debug mode." OFF)

option(BUILD_LIBRARY "Compile the Integration Service" ON)

if(DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE_LOWERCASE "" CACHE STRING "Build type to lowercase")
    string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWERCASE)

    if("${CMAKE_BUILD_TYPE_LOWERCASE}" STREQUAL "debug")
        set(IS_COMPILE_DEBUG ON)
    endif()
endif()

project(is-core VERSION "3.1.0" LANGUAGES CXX)

###############################################################################
# Load external CMake Modules.
###############################################################################
if(BUILD_LIBRARY)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SANITIZERS_ROOT}/cmake)

  find_package(Sanitizers QUIET)

  if(SANITIZE_ADDRESS)
      message(STATUS "Preloading AddressSanitizer library could be done using \"${ASan_WRAPPER}\" to run your program.")
  endif()
endif()

###############################################################################
# External dependencies for the Integration Service Core library
###############################################################################
if(BUILD_LIBRARY)
  find_package(xtypes REQUIRED)

  find_package(yaml-cpp REQUIRED)

  find_package(Boost REQUIRED
    COMPONENTS
      program_options
    )
endif()

###############################################################################
# Configure the Integration Service Core library
###############################################################################
if(BUILD_LIBRARY)
  add_library(${PROJECT_NAME}
    SHARED
      src/runtime/FieldToString.cpp
      src/runtime/MiddlewareInterfaceExtension.cpp
      src/runtime/Search.cpp
      src/runtime/StringTemplate.cpp
      src/systemhandle/RegisterSystem.cpp
      src/utils/Log.cpp
      src/Config.cpp
      src/Instance.cpp
  )

  if (Sanitizers_FOUND)
    add_sanitizers(${PROJECT_NAME})
  endif()

  set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION
      ${PROJECT_VERSION}
    SOVERSION
      ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    CXX_STANDARD
      17
    CXX_STANDARD_REQUIRED
      YES
    )

  target_compile_options(${PROJECT_NAME}
    PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-pedantic>
      $<$<CXX_COMPILER_ID:GNU>:-fstrict-aliasing>
      $<$<CXX_COMPILER_ID:GNU>:-Wall>
      $<$<CXX_COMPILER_ID:GNU>:-Wextra>
      $<$<CXX_COMPILER_ID:GNU>:-Wcast-align>
      $<$<CXX_COMPILER_ID:GNU>:-Wshadow>
      $<$<CXX_COMPILER_ID:MSVC>:/W4>
      $<$<CXX_COMPILER_ID:MSVC>:/wd4700>
      $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
      $<$<CXX_COMPILER_ID:MSVC>:/wd4820>
      $<$<CXX_COMPILER_ID:MSVC>:/wd4255>
      $<$<CXX_COMPILER_ID:MSVC>:/wd4668>
    )

  # Generate the export macro header
  include(GNUInstallDirs)
  include(cmake/is_generate_export_header.cmake)
  is_generate_export_header(core)

  # Generate the config.hpp file
  configure_file(${PROJECT_SOURCE_DIR}/include/is/config.hpp.in
    ${PROJECT_BINARY_DIR}/include/is/config.hpp
    )

  # TODO (@jamoralp): check for Windows
  target_link_libraries(${PROJECT_NAME}
    PUBLIC
      yaml-cpp
      xtypes
    PRIVATE
      Boost::program_options
      $<$<PLATFORM_ID:Linux>:dl>
      $<$<PLATFORM_ID:Linux>:stdc++fs>
  )

  target_include_directories(${PROJECT_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      #$<INSTALL_INTERFACE:${xtypes_INCLUDE_DIR}>  #propagate the xtypes headers
  )

  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
      IS_LIBRARY_ARCHITECTURE="${CMAKE_LIBRARY_ARCHITECTURE}"
  )
endif()
###############################################################################
# Configure the Integration Service executable
###############################################################################
if(BUILD_LIBRARY)
  add_executable(integration-service src/integration-service.cpp)

  target_link_libraries(integration-service
    PRIVATE
      is-core
      #${PROJECT_NAME}
  )
endif()
###############################################################################
# Install the Integration Service Core library, executable and targets
###############################################################################
if(BUILD_LIBRARY)
  set(is_core_config_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

  # Install is-core library
  install(
    TARGETS
      ${PROJECT_NAME}
    EXPORT
      ${PROJECT_NAME}Targets
    RUNTIME DESTINATION
    ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION
      ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION
      ${CMAKE_INSTALL_LIBDIR}
    COMPONENT
      libraries
    )

  # Install CMake modules
  install(
    EXPORT
      ${PROJECT_NAME}Targets
    DESTINATION
      ${is_core_config_dir}
    NAMESPACE
      is::
    FILE
      ${PROJECT_NAME}-target.cmake
    )

  # Install headers
  install(
    DIRECTORY
      ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION
      ${CMAKE_INSTALL_INCLUDEDIR}
    )

  # Install Config.cmake file
  install(
    FILES
      ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION
      ${is_core_config_dir}
    )

  if (WIN32)
    install(
      FILES
        ${CMAKE_CURRENT_LIST_DIR}/windows/local_setup_windows.bat
      DESTINATION
        ${CMAKE_INSTALL_PREFIX}/../
      COMPONENT
        ${PROJECT_NAME}
      )
  endif()

  # Install integration-service executable
  install(
    TARGETS
      integration-service
    EXPORT
      integration-serviceTargets
    RUNTIME
      DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY
      DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE
      DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT
      executables
    )

  # Install core middleware extension development tools
  install(
    DIRECTORY
      ${CMAKE_CURRENT_LIST_DIR}/cmake
    DESTINATION
      ${is_core_config_dir}
    )

  install(
    DIRECTORY
      ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION
      ${is_core_config_dir}
    )
endif()
###############################################################################
# Add the Integration Service Core testing subdirectory
###############################################################################
if(BUILD_LIBRARY)
  include(CTest)
  add_subdirectory(test)
endif()

###############################################################################
# Integration Service Core API Reference
###############################################################################
if(BUILD_API_REFERENCE)
  find_package(Doxygen REQUIRED)
  # Create doxygen directories
  add_custom_target(doc-dirs
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/doxygen
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/doxygen/html
      COMMENT "Creating documentation directories" VERBATIM)

  set(IS_CORE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/is/")
  file(GLOB_RECURSE HPP_FILES "${IS_CORE_INCLUDE_DIR}/*.h*")

  # Doxygen related variables
  set(DOXYGEN_INPUT_DIR "${IS_CORE_INCLUDE_DIR}/")
  set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/doxygen")
  set(DOXYGEN_INDEX_FILE "${PROJECT_BINARY_DIR}/doxygen/xml/index.xml")
  set(DOXYFILE_IN "${PROJECT_SOURCE_DIR}/../utils/api_reference/doxygen-config.in")
  set(DOXYFILE_OUT ${PROJECT_BINARY_DIR}/doxygen-config)
  set(DOXYGEN_GENERATE_TAGFILE "is_core.tag")
  set(DOXYGEN_HTML_DIR "${PROJECT_BINARY_DIR}/doxygen/html")

  # Configure doxygen
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  # Doxygen command
  add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
      DEPENDS ${HPP_FILES}
      MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
      COMMENT "Generating doxygen documentation")

  # Generate API reference
  add_custom_target(doxygen ALL
      DEPENDS ${DOXYGEN_INDEX_FILE}
      COMMENT "Generated API documentation with doxygen" VERBATIM)
  add_dependencies(doxygen doc-dirs)

  # Install doxygen generated XML files
  install(DIRECTORY ${PROJECT_BINARY_DIR}/doxygen/xml
      DESTINATION doxygen
      COMPONENT doxygen)
  set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Integration Service Core doxygen")
  set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION
      "eProsima Integration Service Core doxygen documentation")
  set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} doxygen)

  install(
    FILES
      ${PROJECT_SOURCE_DIR}/../utils/api_reference/doxygen-config.in
    DESTINATION
      ${CMAKE_INSTALL_PREFIX}
  )
endif()