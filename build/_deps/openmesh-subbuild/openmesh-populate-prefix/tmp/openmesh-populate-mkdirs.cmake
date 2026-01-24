# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Dev/pyqex/build/_deps/openmesh-src"
  "C:/Dev/pyqex/build/_deps/openmesh-build"
  "C:/Dev/pyqex/build/_deps/openmesh-subbuild/openmesh-populate-prefix"
  "C:/Dev/pyqex/build/_deps/openmesh-subbuild/openmesh-populate-prefix/tmp"
  "C:/Dev/pyqex/build/_deps/openmesh-subbuild/openmesh-populate-prefix/src/openmesh-populate-stamp"
  "C:/Dev/pyqex/build/_deps/openmesh-subbuild/openmesh-populate-prefix/src"
  "C:/Dev/pyqex/build/_deps/openmesh-subbuild/openmesh-populate-prefix/src/openmesh-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Dev/pyqex/build/_deps/openmesh-subbuild/openmesh-populate-prefix/src/openmesh-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Dev/pyqex/build/_deps/openmesh-subbuild/openmesh-populate-prefix/src/openmesh-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
