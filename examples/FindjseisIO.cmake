# - Try to find the jseisIO processing library
# Once done this will define
#
#  JSEISIO_FOUND - System has the jseisIO library
#  JSEISIO_INCLUDE_DIRS - The include directories to use for jseisIO
#  JSEISIO_LIBRARIES - The libraries needed to use jseisIO
#  JSEISIO_VERSION_STRING - the version of jseisIO. This is only supported when using CMake >= 2.8.3

#=============================================================================
# Copyright 2013 Fraunhofer ITWM <neundorf@itwm.fraunhofer.de>
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

find_library(JSEISIO_LIBRARY NAMES jseisIO )
find_path(JSEISIO_INCLUDE_DIR jsFileWriter.h PATH_SUFFIXES jseisIO)

set(JSEISIO_INCLUDE_DIRS "${JSEISIO_INCLUDE_DIR}" )
set(JSEISIO_LIBRARIES "${JSEISIO_LIBRARY}" )


# extract version from jsVersion.h
if(JSEISIO_INCLUDE_DIR AND EXISTS "${JSEISIO_INCLUDE_DIR}/jsVersion.h")
    file(STRINGS "${JSEISIO_INCLUDE_DIR}/jsVersion.h" jseisIO_version_str
         REGEX "^#define[\t ]+JSVERSION_STRING[\t ]+\".*\"")
    string(REGEX REPLACE "^#define[\t ]+JSVERSION_STRING[\t ]+\"([^\"]*)\".*" "\\1"
           JSEISIO_VERSION_STRING "${jseisIO_version_str}")
    unset(jseisIO_version_str)
endif()

include(FindPackageHandleStandardArgs)

if (CMAKE_VERSION VERSION_LESS 2.8.3)
    find_package_handle_standard_args(jseisIO DEFAULT_MSG  JSEISIO_LIBRARY JSEISIO_INCLUDE_DIR)
else()
    find_package_handle_standard_args(jseisIO REQUIRED_VARS JSEISIO_LIBRARY JSEISIO_INCLUDE_DIR
                                      VERSION_VAR JSEISIO_VERSION_STRING)
endif()
