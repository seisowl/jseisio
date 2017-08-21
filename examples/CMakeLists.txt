cmake_minimum_required(VERSION 3.5)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # building the examples stand-alone
  cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
  find_package(jseisIO 1.0.0 REQUIRED)

  include_directories(${JSEISIO_INCLUDE_DIRS})

else()
  # building as part of the jseisio package itself
  include_directories(${jseisIO_SOURCE_DIR}/src )
  set(JSEISIO_LIBRARIES jseisIO)
  install(FILES testReader.cpp
                testWriter.cpp
                CMakeLists.txt
                FindjseisIO.cmake
          DESTINATION share/jseisIO/examples/ )
endif()

add_executable(reader testReader.cpp)
target_link_libraries(reader ${JSEISIO_LIBRARIES} pthread)


add_executable(writer testWriter.cpp)
target_link_libraries(writer ${JSEISIO_LIBRARIES} )
