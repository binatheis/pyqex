#
# Copyright 2013 Computer Graphics Group, RWTH Aachen University
# Author: Hans-Christian Ebke <ebke@cs.rwth-aachen.de>
#
# This file is part of QEx.
#
# QEx is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# QEx is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with QEx.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.15)

# OpenMesh 8.1 has old cmake_minimum_required, allow it with newer CMake
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

project(QEx VERSION 1.0.0 LANGUAGES C CXX)

include(FetchContent)

# Modern CMake settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(BUILD_PYTHON_BINDINGS "Build Python bindings via pybind11" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_DEMOS "Build demo programs" OFF)
option(BUILD_UNIT_TESTS "Build unit tests" OFF)
option(STL_RANGE_CHECKS "Include STL range checks in debug mode" OFF)
option(FETCH_OPENMESH "Use FetchContent to download OpenMesh" ON)

# Get OpenMesh
if(FETCH_OPENMESH)
    message(STATUS "Fetching OpenMesh...")
    FetchContent_Declare(
        OpenMesh
        GIT_REPOSITORY https://gitlab.vci.rwth-aachen.de:9000/OpenMesh/OpenMesh.git
        GIT_TAG OpenMesh-8.1
        GIT_SHALLOW TRUE
    )
    # Configure OpenMesh build options
    set(OPENMESH_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(OPENMESH_DOCS OFF CACHE BOOL "" FORCE)
    set(BUILD_APPS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(OpenMesh)

    set(OPENMESH_INCLUDE_DIRS ${openmesh_SOURCE_DIR}/src)
    set(OPENMESH_LIBRARIES OpenMeshCore)
else()
    # Use system OpenMesh
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
    find_package(OpenMesh REQUIRED)
endif()

# Core library sources
set(QEX_SOURCES
    interfaces/c/qex.cc
    src/predicates.c
    src/MeshExtractor.cc
)

# Create library
add_library(QEx ${QEX_SOURCES})
add_library(QEx::QEx ALIAS QEx)

target_include_directories(QEx
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/interfaces/c>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${OPENMESH_INCLUDE_DIRS}
)

target_link_libraries(QEx PRIVATE ${OPENMESH_LIBRARIES})

# Compiler flags for exact predicates (SSE required on x86, no x87)
# Note: -msse flags only apply to x86/x86_64, not ARM
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i[3-6]86)$")
        target_compile_options(QEx PRIVATE -Wall -msse -mfpmath=sse -pedantic)
    else()
        target_compile_options(QEx PRIVATE -Wall -pedantic)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i[3-6]86)$")
        target_compile_options(QEx PRIVATE -Wall -msse -mfpmath=sse -pedantic -Wextra)
    else()
        target_compile_options(QEx PRIVATE -Wall -pedantic -Wextra)
    endif()
elseif(MSVC)
    target_compile_definitions(QEx PRIVATE _USE_MATH_DEFINES)
    # MSVC uses SSE by default on x64, exactinit() handles precision
endif()

# Export symbols on Windows
target_compile_definitions(QEx PRIVATE QEX_EXPORT_SYMBOLS)

# STL range checks in debug mode
if(STL_RANGE_CHECKS)
    target_compile_definitions(QEx PRIVATE
        $<$<CONFIG:Debug>:_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC>
    )
endif()

# Python bindings
if(BUILD_PYTHON_BINDINGS)
    # Fetch pybind11
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.11.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(pybind11)

    pybind11_add_module(_pyqex python/bindings.cpp)
    target_link_libraries(_pyqex PRIVATE QEx ${OPENMESH_LIBRARIES})
    target_include_directories(_pyqex PRIVATE
        ${CMAKE_SOURCE_DIR}/interfaces/c
        ${CMAKE_SOURCE_DIR}/src
        ${OPENMESH_INCLUDE_DIRS}
    )
    # QEX_EXPORT_SYMBOLS needed so qex.h uses dllexport, not dllimport (for static linking)
    target_compile_definitions(_pyqex PRIVATE QEX_EXPORT_SYMBOLS)
    if(MSVC)
        target_compile_definitions(_pyqex PRIVATE _USE_MATH_DEFINES)
    endif()

    install(TARGETS _pyqex LIBRARY DESTINATION pyqex)
endif()

# Demos
if(BUILD_DEMOS)
    add_subdirectory(demo/minimal_c)
    add_subdirectory(demo/cmdline_tool)
endif()

# Unit tests
if(BUILD_UNIT_TESTS)
    enable_testing()
    if(NOT GTEST_DIR)
        message(FATAL_ERROR "GTEST_DIR unset")
    endif()
    add_subdirectory(${GTEST_DIR} gtest)
    add_subdirectory(tests)
endif()

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_BINARY_DIR}/doc/Doxyfile @ONLY)
    add_custom_target(qex_doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
        COMMENT "Generating Doxygen documentation"
        VERBATIM
    )
endif()

# Export for dependent projects
if(NOT PROJECT_IS_TOP_LEVEL)
    set(QEX_FOUND TRUE PARENT_SCOPE)
    set(QEX_LIBRARIES QEx PARENT_SCOPE)
    set(QEX_LIBRARY QEx PARENT_SCOPE)
    set(QEX_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/interfaces/c" PARENT_SCOPE)
endif()
