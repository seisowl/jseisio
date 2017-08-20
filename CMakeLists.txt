cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

project(jseisIO)

# Set some compiler flags, and append flags which have been preset by the user:
if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "-w1 -ww1 -ww111 -ww174 -ww177 -ww271 -ww304 -ww593 -ww654 -we1595 -vec-report0 -xSSE2 -DNDEBUG -O3 ${CMAKE_CXX_FLAGS}" )
  set(CMAKE_SHARED_LINKER_FLAGS -static-intel)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  set(CMAKE_CXX_FLAGS "-Wall -Wno-switch -Wno-sign-compare -DNDEBUG -O2 ${CMAKE_CXX_FLAGS}" )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
  set(CMAKE_CXX_FLAGS "-Wall -Wno-switch -Wno-sign-compare -Wno-gnu -DNDEBUG -O2 ${CMAKE_CXX_FLAGS}" )
endif()

# Let the user decide whether to build jseisIO as a static or shared library
option(BUILD_SHARED_LIBS "Build jseisIO as shared library" FALSE)

# Setup version numbers. They are used for generating the version header jsVersion.h
# and for the package names.
set(JSEISIO_VERSION_MAJOR 1)
set(JSEISIO_VERSION_MINOR 0)
set(JSEISIO_VERSION_PATCH 0)

add_subdirectory(src)

add_subdirectory(examples)

install(FILES readme COPYING.LESSER DESTINATION share/jseisIO/ )

# setup packaging

# check whether we are on a 64bit system and adjust CPACK_SYSTEM_NAME accordingly:
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-x86_64")
else()
  set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-i386")
endif()

set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_VERSION_MAJOR ${JSEISIO_VERSION_MAJOR} )
set(CPACK_PACKAGE_VERSION_MINOR ${JSEISIO_VERSION_MINOR} )
set(CPACK_PACKAGE_VERSION_PATCH ${JSEISIO_VERSION_PATCH} )

# Setup the source package:
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.svn/;.+~;html-docs") #${CPACK_SOURCE_IGNORE_FILES}")

# add potentially existing build directories to the ignore-files:
file(GLOB buildDirs */CMakeCache.txt)
foreach(bdir ${buildDirs})
  get_filename_component(dir "${bdir}" PATH)
  list(APPEND CPACK_SOURCE_IGNORE_FILES "${dir}" )
endforeach()

include(CPack)
