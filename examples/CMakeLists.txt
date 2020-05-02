cmake_minimum_required(VERSION 2.8)
include(FindOpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else(OPENMP_FOUND)
  message("ERROR: OpenMP could not be found.")
endif(OPENMP_FOUND)


if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # building the examples stand-alone

  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
  find_package(jseisIO 1.0.0 REQUIRED)

  include_directories(${JSEISIO_INCLUDE_DIRS})

else()
  # building as part of the jseisio package itself
  include_directories(${jseisIO_SOURCE_DIR}/src )
  set(JSEISIO_LIBRARIES jseisIO)
  install(FILES testReader.cpp
                testWriter.cpp
				testJseisND.cpp
                CMakeLists.txt
                FindjseisIO.cmake
          DESTINATION share/jseisIO/examples/ )
endif()

add_executable(reader testReader.cpp)
target_link_libraries(reader ${JSEISIO_LIBRARIES} pthread)


add_executable(writer testWriter.cpp)
target_link_libraries(writer ${JSEISIO_LIBRARIES} pthread)

add_executable(jsnd testJseisND.cpp)
target_link_libraries(jsnd ${JSEISIO_LIBRARIES} pthread)
