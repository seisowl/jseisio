cmake_minimum_required(VERSION 3.5)
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
        DESTINATION lib64 )



find_package(Doxygen)
if (DOXYGEN_FOUND)
  option(GENERATE_DOCS_ALWAYS "Always generate doxygen documentation when building all" TRUE)
  if(GENERATE_DOCS_ALWAYS)
    set(DOCS_IN_ALL "ALL")
  else()
    unset(DOCS_IN_ALL)
  endif()

  add_custom_target(docs ${DOCS_IN_ALL}
                    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/JavaSeisIOlib.doxyfile
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

  if(GENERATE_DOCS_ALWAYS)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/html-docs/html" DESTINATION share/jseisIO/ )
  endif()

endif (DOXYGEN_FOUND)

