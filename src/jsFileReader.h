/***************************************************************************
                          jsFileReader.h  -  description
                             -------------------

    copyright            : (C) 2012 Fraunhofer ITWM

    This file is part of jseisIO.

    jseisIO is free software: you can redistribute it and/or modify
    it under the terms of the Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    jseisIO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Lesser General Public License for more details.

    You should have received a copy of the Lesser General Public License
    along with jseisIO.  If not, see <http://www.gnu.org/licenses/>.

 ***************************************************************************/

#ifndef JSFILEREADER_H
#define JSFILEREADER_H

#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "jsStrDefs.h"
#include "jsDefs.h"
#include "jsByteOrder.h"

namespace jsIO
{
  struct headerWordInfo
  {
    int format;
    int count;
    int offset;
  };

  class TraceProperties;
  class FileProperties;
  class ExtentList;
  class TraceCompressor;
  class CharBuffer;
  class IntBuffer;
  class SeisPEG;
  class TraceMap;
  class catalogedHdrEntry;
  class IOCachedReader;

  /**
   * This class is for reading a dataset in JavaSeis format.
   * It supports all data formats defined in JavaSeis - 
   * FLOAT, INT16, INT08, COMPRESSED_INT16, COMPRESSED_INT08 as well as SEISPEG.
   * Note that the usual read routines jsFileReader::readFrame or jsFileReader::readTrace ARE NOT thread safe 
   * if you use one object instance in multiple threads (actually there is no 
   * performance gain in reading multithreaded from files).
   * However, one can use several threads for decompressing compressed data.
   * For usage examples see examples/testReader.cpp
   * 
   * @author Abel Amirbekyan
   * @since Jul 08, 2011
   * 
  */
  
  class jsFileReader
  {
    public:
      ~jsFileReader();
      
      /**
       * @param _bufferSize cache-size used while reading from files (default: 2MB).
       *                    Set 0 to read directly (not cached)
      */
      jsFileReader(const unsigned long _bufferSize = 2097152); //default: 2MB cache. Set 0, to read directly (not cached)

      /**
       *  @brief Initalizes jsFileReader
       *  @param _jsfilename  the full name of javaseis dataset (i.e. inclusive the path)
       *  @param _NThreads number of threads that can be used in uncompressRawFrame function
      */
      int Init(const std::string _jsfilename, const int _NThreads=1);
      
      ///@return true if the dataset is regular, otherwise returns false
      bool isRegular() const;
      ///@return true if the data is in SeisPEG format, otherwise returns false
      bool isSeisPEG() const; 
      
      ///@return total number of live tracaes in the dataset
      long getNtr() const;
      
      ///@return total number of frames in the dataset
      long getNFrames() const;

      /** 
       * @brief Get the list of header-words defined in data
       * @param[out] names the vector of names of defined of header-words
       * @param[out] descriptions the vector of descriptions of defined of header-words 
       * @return the number of defined header-words, i.e. the lenght of names or descriptions vector
       *
      */ 
      int getHeaderWords(std::vector<std::string> &names, std::vector<std::string> &descriptions) const;
      
      ///@return number of header-words
      int getNumHeaderWords() const; 
      
      ///@return trace header length in bytes
      int getNumBytesInHeader() const;
      
      /** 
       * @brief Get the length (in bytes) of 'raw' frame
       * @details
       * A raw frame is a frame as it is saved on a disk, i.e. possibly in compressed format.
       * That is in the data format is FLOAT, then it returs  getAxisLen(0)*getAxisLen(1)*sizeof(float)
       */
      long getNumBytesInRawFrame() const;
      
      /**
       * @brief Get parameters of defined header words
       * @details
       *   Initalize the array pInfo of type headerWordInfo with the properties of defined header-words.
       *   Note that pointer pInfo must be pre-allocated with the lenght equal to getNumHeaderWords()
      */
      void getHeaderWordsInfo(headerWordInfo *pInfo) const;  

      unsigned long getIOBufferSize() const {return m_IOBufferSize;}
      int getNDim() const;
      int getAxisLen(int index) const;
      int getAxisLogicalValues(int index, std::vector<long> &axis) const;
      int getAxisPhysicalValues(int index, std::vector<double> &axis) const;
      int getAxisLabels(std::vector<std::string> &axis) const;
      int getAxisUnits(std::vector<std::string> &units) const;
      
      /**
       * @brief Returns frame size (in bytes) on disk
       * @details In case of FLOAT data it is equal to getAxisLen(0)*getAxisLen(1)*sizeof(float), but in case of
       * compressed data it is different.
      */
      long getFrameSizeOnDisk() const {return m_frameSize;}
      
      JS_BYTEORDER getByteOrder() const {return m_byteOrder;}
      std::string getByteOrderAsString() const;
      std::string getTraceFormatName() const;
      std::string getDescriptiveName() const{return m_descriptiveName;}
      std::string getDataType() const;

      /**
       * @brief Returns an access to the header-word named _name in a form of catalogedHdrEntry class
      */ 
      catalogedHdrEntry getHdrEntry(std::string _name) const;
      
      /**
      * @brief Returns list of all header-words in a form of catalogedHdrEntry class
      */ 
      std::vector<catalogedHdrEntry> getHdrEntries() const;

      //in all following function  *position is an array of integers defining the position of a frame/trace   accorind to the logical coordintes
      //len(position)=fileProps.numDimensions-1;

      //you can read also headers together with frames with readFrame function 
      
      /**
       * @brief Reads the header of the trace given by its position
       * @param _position position of the trace given in logical coordinates
       * @param[out] headbuf a pre-allocated buffer (with a size at least getNumBytesInHeader()) to save the header
       * @return JS_OK if successful
      */
      int readTraceHeader(const int* _position, char *headbuf);
      
      /**
       * @brief Reads the header of the trace given by its its global index
       * @param _traceIndex global trace index
       * @param[out] headbuf a pre-allocated buffer (with a size at least getNumBytesInHeader()) to save the header
       * @return JS_OK if successful
      */
      int readTraceHeader(const long _traceIndex, char *headbuf);
      
      /**
       * @brief Reads the header of the frame given by its position
       * @param _position position of the frame given in logical coordinates
       * @param[out] headbuf a pre-allocated buffer (with a size at least getAxisLen(1)*getNumBytesInHeader()) to save the header
       * @return the number of live traces in the frame
      */
      int readFrameHeader(const int* _position, char *headbuf);
      
      /**
       * @brief Reads the header of the frame given by its global index
       * @param _frameIndex global frame index
       * @param[out] headbuf a pre-allocated buffer (with a size at least getAxisLen(1)*getNumBytesInHeader()) to save the header
       * @return the number of live traces in the frame
      */
      int readFrameHeader(const long _frameIndex, char *headbuf);//headbuf must be pre-allocated with the size = numOfTracesInFrame * getNumBytesInHeader()

      /**
       * @brief Reads the trace given by its position
       * @param _position position of the trace given in logical coordinates
       * @param[out] trace a pre-allocated float array (with a length at least getAxisLen(0)) to save the trace
       * @return JS_OK if successful
      */
      int readTrace(const int* _position, float *trace);
      
      /**
       * @brief Reads the trace given by its its global index
       * @param _traceIndex global index of the trace
       * @param[out] trace a pre-allocated float array (with a length at least getAxisLen(0)) to save the trace
       * @return JS_OK if successful
      */
      int readTrace(const long _traceIndex, float *trace); 

      
      /**
       * @brief Reads multiple traces
       * @param _firstTraceIndex global index of the first trace
       * @param _numOfTraces number of traces to read
       * @param[out] buffer a pre-allocated float array (with a length getAxisLen(0) * _numOfTraces) to save traces
       * @param[out] headbuf (if not NULL) a pre-allocated buffer (with a size _numOfTraces*getNumBytesInHeader()) to save traces headers
       * @return the number of live traces actually read (<0 in case of error)
      */      
      long readTraces(const long _firstTraceIndex,  const long _numOfTraces, float *buffer, char *headbuf=NULL);

      /**
       * @brief Reads headers of multiple traces
       * @param _firstTraceIndex global index of the first trace
       * @param _numOfTraces number of trace-headers to read
       * @param[out] headbuf a pre-allocated buffer (with a size _numOfTraces*getNumBytesInHeader()) to save traces headers
       * @return the number of live traces actually read (<0 in case of error)
      */
      long readTraceHeaders(const long _firstTraceIndex,  const long _numOfTraces, char *headbuf);


      /**
       * @brief Converts live trace index to global trace index
       * @details 
       *  For example liveToGlobalTraceIndex(10, n) will iniialize n with the global index of 10-th live trace.
       *  Live traces are in range [0,getNtr()]
      */
      int liveToGlobalTraceIndex(const long _liveTraceIndex, long &_globalTraceIndex);

       /**
       * @brief Reads multiple traces within live traces
       * @param _firstTraceIndex global index (within all live traces) of the first trace
       * @param _numOfTraces number of live traces to read
       * @param[out] buffer a pre-allocated float array (with a length getAxisLen(0) * _numOfTraces) to save traces
       * @param[out] headbuf (if not NULL) a pre-allocated buffer (with a size _numOfTraces*getNumBytesInHeader()) to save traces headers
       * @return the number of live traces actually read (<0 in case of error)
      */
      long readWithinLiveTraces(const long _firstTraceIndex,  const long _numOfTraces, float *buffer, char *headbuf=NULL);


      // Note that extensively random use of readWithinLiveTraces or readWithinLiveTraceHeaders functions can be noticeably slow in 
      // comparison to other reading routines, because each time for computing the file-offset of given live trace we have to 
      // read TraceMap from the beginning until the corresponding position.
      // In case of sequentially use, we remember previous indices, so we don't need to read TraceMap from the beginning.

      /**
       * @brief Reads headers of multiple traces within live traces
       * @param _firstTraceIndex global index (within all live traces) of the first trace
       * @param _numOfTraces number of live trace-headers to read
       * @param[out] headbuf a pre-allocated buffer (with a size _numOfTraces*getNumBytesInHeader()) to save traces headers
       * @return the number of live traces actually read (<0 in case of error)
      */
      long readWithinLiveTraceHeaders(const long _firstTraceIndex,  const long _numOfTraces, char *headbuf);


      /**
       * @brief Reads the frame given by its position
       * @param _position position of the frame given in logical coordinates
       * @param[out] frame a pre-allocated float array (with a length at least  getAxisLen(0) * getAxisLen(1)) to save the frame.
       * @param[out] headbuf if not NULL, then a pre-allocated buffer (with a size at least getAxisLen(1)*getNumBytesInHeader()) to save the frame header.
       * @return the number of live traces in the frame
       * @details
       *  The reading of header together with traces will be faster only for SeisPEG data, since there the header is compressed with the data
      */
      int readFrame(const int* _position, float *frame, char *headbuf=NULL);
      
      /**
       * @brief Reads the frame given by its global index
       * @param _frameIndex _frameIndex index of the frame
       * @param[out] frame a pre-allocated float array (with a length at least  getAxisLen(0) * getAxisLen(1)) to save the frame
       * @param[out] headbuf if not NULL, then a pre-allocated buffer (with a size at least getAxisLen(1)*getNumBytesInHeader()) to save the frame header
       * @return the number of live traces in the frame
      */
      int readFrame(const long _frameIndex, float *frame, char *headbuf=NULL);

      /**
       * @brief Reads multiple raw frames
       * @details
       *   A raw frame is a frame as it is saved on a disk, i.e. possibly in compressed format.
       *   Call this function from one thread only to read multile frames from a disk. 
       *   Afterwards, these raw frames can be uncompressed (if format is not FLOAT) with uncompressRawFrame
       *   using multiple threads.
       * @param _position position of the first frame given in logical coordinates
       * @param NFrames the number of frames to read
       * @param[out] rawframe a pre-allocated buffer (with a length at least  NFrames * getNumBytesInRawFrame() ) to save the frame
       * @param[out] numLiveTraces an array with a number of live trace in each read frames
       * @return JS_OK if successful
      */
      int readRawFrames(const int* _position, int NFrames, char *rawframe, int *numLiveTraces);  
      
      /**
       * @brief Reads multiple raw frames
       * @details
       *   A raw frame is a frame as it is saved on a disk, i.e. possibly in compressed format.
       *   Call this function from one thread only to read multile frames from a disk. 
       *   Afterwards, these raw frames can be uncompressed (if format is not FLOAT) with uncompressRawFrame
       *   using multiple threads.
       * @param _frameindex position of the first frame given in logical coordinates
       * @param NFrames the number of frames to read
       * @param[out] rawframe a pre-allocated buffer (with a length at least  NFrames * getNumBytesInRawFrame() ) to save the frame
       * @param[out] numLiveTraces an array with a number of live trace in each read frames
       * @return JS_OK if successful
      */
      int readRawFrames(const long _frameindex, int NFrames, char *rawframe, int *numLiveTraces);
      
      /**
       * @brief Uncompress raw frames
       * @details
       *   Thread-parallel decompression of raw frames read with readRawFrames function
       * @param rawframe a raw frame (read with readRawFrames)
       * @param numLiveTraces number of live traces in rawframe
       * @param iThread current thread ID (must be smaller than _NThreads parameter used in Init function)
       * @param[out] frame a pre-allocated float array (with a length at least  getAxisLen(0) * getAxisLen(1) ) to save the frame
       * @param[out] headbuf in case of SeisPEG data must be pre-allocated buffer 
                     (with a size at least getAxisLen(1)*getNumBytesInHeader()) to save the frame header.
      */
      int uncompressRawFrame(char *rawframe, int numLiveTraces, int iThread,  float *frame, char* headbuf=NULL);

      /**
       * @brief Returns the number of live traces in frame with global index _frameIndex 
      */
      int getNumOfLiveTraces(int _frameIndex) const;
      
      /**
       * @brief Returns the version number (as a string) of dataset 
      */
      std::string getVersion() const;
      
      /**
      * @brief Read a parameter from CustomProperties
      * @details Returns the value of parameter given as _property defined in CustomProperties part of FileProperties.xml
      * or an empty string in case of error.
      * For example getCustomProperty("Stacked"), or getCustomProperty("FieldInstruments/earlyGain")
      */  
      std::string getCustomProperty(std::string _property);
      
      /**
       * @brief Returns the number of extents in dataset 
      */
      int getNumOfExtents() const;

      /**
       * @brief Returns the number of virtual folders in dataset 
      */
      int getNumOfVirtualFolders() const;
      
    private:
      int m_NThreads;

      TraceProperties* m_traceProps;
      FileProperties* m_fileProps;

      std::string m_filename;
  
      bool m_bInit;

      long m_TotalNumOfLiveTraces;
      long m_TotalNumOfTraces;
      long m_TotalNumOfFrames;

      JS_BYTEORDER m_byteOrder;

      bool m_bIsFloat; //true is data format is FLOAT
      bool m_bIsRegular;
      bool m_bIsMapped;

      int m_numSamples;
      int m_numTraces;
      int m_compess_traceSize;
      long m_frameSize;

      int m_headerLengthBytes;
      int m_headerLengthWords;
      long m_frameHeaderLength;

      unsigned long m_IOBufferSize; 
      IOCachedReader *m_pCachedReaderHD;
      IOCachedReader *m_pCachedReaderTR;

      TraceMap *m_trMap;

      ExtentList *m_TrFileExtents;
      ExtentList *m_TrHeadExtents;

      TraceCompressor *m_traceCompressor; // Null if using SeisPEG.
      SeisPEG *m_seispegCompressor; // Null if not using SeisPEG.
      bool m_bSeisPEG_data; //true is using SeisPEG.

      int m_currIndexOfTrFileExtent;
      int m_curr_trffd;
      int m_currIndexOfTrHeadExtent;
      int m_curr_trhfd;

    //tmp buffers
      char *m_traceBufferArray;
      CharBuffer *m_traceBuffer;

      char *m_headerBufferArray;
      CharBuffer *m_headerBuffer; 
      IntBuffer *m_headerBufferView;

      std::string m_descriptiveName;
    
      //these variables used in liveToGlobalTraceIndex to mimimize the access to TraceMap in 
      //case of sequentially calling liveToGlobalTraceIndex
      long m_prev_firstTr1; //liveTraceIndex preciously used in liveToGlobalTraceIndex
      long m_prev_frInd1;  //global frame index corresponding to liveTraceIndex
      long m_prev_numTraces1; //number of live traces total in all frames till m_prev_frInd
     
      //same as above, we just keep information on two access of liveToGlobalTraceIndex
      long m_prev_firstTr2;
      long m_prev_numTraces2;
      long m_prev_frInd2;
      //
      //tmp buffers for readTraces function
      float *m_frame;
      char  *m_frameHeader;
      long   m_frameInd;
      long   m_frameHeaderInd;
      int    m_numOfFrameLiveTraces;
      int    m_numOfFrameHeaderLiveTraces;

    private:
      int  initExtents(const std::string &jsfilename);
      
      long getFrameIndex(const int* position) const;
      long getTraceIndex(const int* position)  const;

      long getOffsetInExtents(int* position, int len1d) const;
      int readTraceBuffer(long offset, char *buf, long buflen);
      int readHeaderBuffer(long offset, char *buf, long buflen);

      void Close(); 

      int readSingleProperty(const std::string &_datasetPath, const std::string  &_fileName, 
                             const std::string  propertyName, std::string &propertyValue) const;

  };
}


#endif



