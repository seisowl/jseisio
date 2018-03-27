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
option(BUILD_SHARED_LIBS "Build jseisIO as shared library" TRUE)
option(BUILD_STATIC_LIBS "Build jseisIO as static library" FALSE)

configure_file(jsVersion.h.in ${CMAKE_CURRENT_BINARY_DIR}/jsVersion.h)

# create the libraries
set(JSIO_src	 xmlParser/tinyxml.cpp
		 xmlParser/tinyxmlparser.cpp
		 xmlParser/tinyxmlerror.cpp
		 xmlParser/tinystr.cpp
		 PropertyDescription.cpp
		 TraceProperties.cpp
		 FileProperties.cpp 
		 AxisLabel.cpp
		 DataDomain.cpp
		 AxisDefinition.cpp
		 GridDefinition.cpp
		 DataType.cpp
		 DataFormat.cpp
		 DataDefinition.cpp
		 Units.cpp
		 jsFileReader.cpp
		 jsFileWriter.cpp
		 jsWriterInput.cpp
		 ByteArray.cpp
		 Buffer.cpp
		 CharBuffer.cpp
		 ShortBuffer.cpp
		 FloatBuffer.cpp
		 IntBuffer.cpp
		 jsByteOrder.cpp
		 stringfuncs.cpp
                 ExtentListEntry.cpp
                 VirtualFolder.cpp
                 VirtualFolders.cpp
                 Parameter.cpp
                 xmlreader.cpp
                 ExtentList.cpp 
                 TraceMap.cpp
                 CustomProperties.cpp
                 IOCachedWriter.cpp
                 IOCachedReader.cpp
                 catalogedHdrEntry.cpp
                 compress/TraceCompressor.cpp
                 compress/Transformer.cpp
                 compress/BlockCompressor.cpp
                 compress/HuffCoder.cpp
                 compress/miniz.c
                 compress/HdrCompressor.cpp
                 compress/CompressedData.cpp
                 compress/SeisPEG.cpp
                 )

add_library(jseisIO ${JSIO_src})

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
              jsStrDefs.h
              ${CMAKE_CURRENT_BINARY_DIR}/jsVersion.h
              stringfuncs.h
              TraceProperties.h
              Units.h
         DESTINATION include/jseisIO )

install(TARGETS jseisIO
        DESTINATION lib )


