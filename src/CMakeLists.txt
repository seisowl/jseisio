cmake_minimum_required(VERSION 2.8)
include(FindOpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else(OPENMP_FOUND)
  message("ERROR: OpenMP could not be found.")
endif(OPENMP_FOUND)

# Let the user decide whether to build jseisIO as a static or shared library
#option(BUILD_SHARED_LIBS "Build jseisIO as shared library" TRUE)

configure_file(jsVersion.h.in ${CMAKE_CURRENT_BINARY_DIR}/jsVersion.h)

# create the libraries
file( GLOB_RECURSE JSIO_src *.cpp *.c )

add_library(jseisIO SHARED ${JSIO_src})
add_library(jseisIO_static STATIC ${JSIO_src})
set_target_properties(jseisIO_static PROPERTIES OUTPUT_NAME jseisIO)

# install rules
install(FILES CharBuffer.h
              AxisLabel.h
              Buffer.h
              ByteArray.h
              catalogedHdrEntry.h
              DataDomain.h
              DataFormat.h
              DataType.h
              jsFileReader.h
              jsFileWriter.h
              jsWriterInput.h
              jsByteOrder.h
              jsDefs.h
              jseisUtil.h
              jsStrDefs.h
              ${CMAKE_CURRENT_BINARY_DIR}/jsVersion.h
              stringfuncs.h
              TraceProperties.h
              Units.h
         DESTINATION include/jseisIO )

install(TARGETS jseisIO
        DESTINATION lib )

install(TARGETS jseisIO_static
        DESTINATION lib )


