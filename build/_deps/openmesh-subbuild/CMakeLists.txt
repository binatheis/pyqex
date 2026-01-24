# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.24.2)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(openmesh-populate NONE)


# Pass through things we've already detected in the main project to avoid
# paying the cost of redetecting them again in ExternalProject_Add()
set(GIT_EXECUTABLE [==[C:/Program Files/Git/mingw64/bin/git.exe]==])
set(GIT_VERSION_STRING [==[2.45.1.windows.1]==])
set_property(GLOBAL PROPERTY _CMAKE_FindGit_GIT_EXECUTABLE_VERSION
  [==[C:/Program Files/Git/mingw64/bin/git.exe;2.45.1.windows.1]==]
)


include(ExternalProject)
ExternalProject_Add(openmesh-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://gitlab.vci.rwth-aachen.de:9000/OpenMesh/OpenMesh.git" "GIT_TAG" "OpenMesh-8.1" "GIT_SHALLOW" "TRUE"
                    SOURCE_DIR          "C:/Dev/pyqex/build/_deps/openmesh-src"
                    BINARY_DIR          "C:/Dev/pyqex/build/_deps/openmesh-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


