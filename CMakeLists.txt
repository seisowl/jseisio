cmake_minimum_required(VERSION 2.8)

project(jseisIO)

include(FindOpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else(OPENMP_FOUND)
  message("ERROR: OpenMP could not be found.")
endif(OPENMP_FOUND)

# Let the user decide whether to build jseisIO as a static or shared library
option(BUILD_SHARED_LIBS "Build jseisIO as shared library" TRUE)
option(BUILD_STATIC_LIBS "Build jseisIO as static library" FALSE)

# Setup version numbers. They are used for generating the version header jsVersion.h
# and for the package names.
set(JSEISIO_VERSION_MAJOR 1)
set(JSEISIO_VERSION_MINOR 1)
set(JSEISIO_VERSION_PATCH 0)

add_subdirectory(src)

add_subdirectory(examples)

find_package(Doxygen)
if (DOXYGEN_FOUND)
  option(GENERATE_DOCS_ALWAYS "Always generate doxygen documentation when building all" TRUE)
  if(GENERATE_DOCS_ALWAYS)
    set(DOCS_IN_ALL "ALL")
  else()
    unset(DOCS_IN_ALL)
  endif()

  add_custom_target(docs ${DOCS_IN_ALL}
                    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/JavaSeisIOlib.doxyfile
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src )

  if(GENERATE_DOCS_ALWAYS)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/html-docs/html" DESTINATION share/jseisIO/ )
  endif()

endif (DOXYGEN_FOUND)

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