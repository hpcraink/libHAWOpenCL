#.rst:
# FindLibHAWOpenCL
# ----------------
# Based on CMake's libArchive.cmake
#
# Find HAWOpenCL library and headers
#
# The module defines the following variables:
#
# ::
#
#   LibHAWOpenCL_FOUND        - true if libHAWOpenCL was found
#   LibHAWOpenCL_INCLUDE_DIRS - include search path
#   LibHAWOpenCL_LIBRARIES    - libraries to link
#   LibHAWOpenCL_VERSION      - libHAWOpenCL 3-component version number

#=============================================================================
# Copyright 2010 Kitware, Inc.
# Copyright 2016 Rainer Keller, HFT Stuttgart
# Copyright 2018-2019 Rainer Keller, HS Esslingen
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(LibHAWOpenCL_INCLUDE_DIR
  NAMES HAWOpenCL.h
  )

find_library(LibHAWOpenCL_LIBRARY
  NAMES HAWOpenCL libHAWOpenCL
  )

# Extract the version number from the header.
if(LibHAWOpenCL_INCLUDE_DIR AND EXISTS "${LibHAWOpenCL_INCLUDE_DIR}/HAWOpenCL.h")
  # The version string appears in one of two known formats in the header:
  #  #define HAWOPENCL_LIBRARY_VERSION "libHAWOpenCL 1.0.0"
  # Match either format.
  set(_LibHAWOpenCL_VERSION_REGEX "^#[ \t]*define[ \t]+HAWOPENCL[_A-Z]+VERSION[_A-Z]*[ \t]+\"libHAWOpenCL +([0-9]+)\\.([0-9]+)\\.([0-9]+)[^\"]*\".*$")
  file(STRINGS "${LibHAWOpenCL_INCLUDE_DIR}/HAWOpenCL.h" _LibHAWOpenCL_VERSION_STRING LIMIT_COUNT 1 REGEX "${_LibHAWOpenCL_VERSION_REGEX}")
  if(_LibHAWOpenCL_VERSION_STRING)
    string(REGEX REPLACE "${_LibHAWOpenCL_VERSION_REGEX}" "\\1.\\2.\\3" LibHAWOpenCL_VERSION "${_LibHAWOpenCL_VERSION_STRING}")
  endif()
  unset(_LibHAWOpenCL_VERSION_REGEX)
  unset(_LibHAWOpenCL_VERSION_STRING)
endif()

# Handle the QUIETLY and REQUIRED arguments and set LibHAWOpenCL_FOUND
# to TRUE if all listed variables are TRUE.
# (Use ${CMAKE_ROOT}/Modules instead of ${CMAKE_CURRENT_LIST_DIR} because CMake
#  itself includes this FindLibArchive/FindLibHAWOpenCL when built with an older CMake that does
#  not provide it.  The older CMake also does not have CMAKE_CURRENT_LIST_DIR.)
include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(LibHAWOpenCL
                                  REQUIRED_VARS LibHAWOpenCL_LIBRARY LibHAWOpenCL_INCLUDE_DIR
                                  VERSION_VAR LibHAWOpenCL_VERSION
  )
set(LibHAWOpenCL_FOUND ${LIBHAWOPENCL_FOUND})
unset(LIBHAWOPENCL_FOUND)

if(LibHAWOpenCL_FOUND)
  set(LibHAWOpenCL_INCLUDE_DIRS ${LibHAWOpenCL_INCLUDE_DIR})
  set(LibHAWOpenCL_LIBRARIES    ${LibHAWOpenCL_LIBRARY})
endif()
