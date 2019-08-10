/***************************************************************************
                          jsFileWriter.h -  description
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

#ifndef JSFILEWRITER_H
#define JSFILEWRITER_H

#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

#include "jsStrDefs.h"
#include "jsDefs.h"
#include "jsByteOrder.h"
#include "jsFileReader.h"

namespace jsIO
{
  class TraceProperties;
  class FileProperties;
  class CustomProperties;
  class ExtentList;
  class SeisPEG;
  class TraceMap;
  class TraceCompressor;
  class CharBuffer;
  class IntBuffer;

  class GridDefinition;
  class DataDefinition;
  class catalogedHdrEntry;

  class IOCachedWriter;

  class jsWriterInput; //for general non-regular data
  
  /**
   * This class is for writing dataset in JavaSeis format.
   * It supports all data formats defined in JavaSeis -
   * FLOAT, INT16, INT08, COMPRESSED_INT16, COMPRESSED_INT08 as well as SEISPEG.
   * Note this class IS NOT thread safe if you use one object instance in multiple threads.
   * For usage examples see examples/testWriter.cpp
   *
   * @author Abel Amirbekyan
   * @since Nov 07, 2011
   */

  class jsFileWriter
  {
    public:
      jsFileWriter();
      ~jsFileWriter();

      ///Initializes the object using parameters defined in _wrtData
      int Init(const jsWriterInput* _wrtData);

      // init by jsFileReader
      int Init( jsFileReader* jsReader);

      // set file name and description name
      int setFileName(const std::string _filename);
      int initDataType(const std::string dataType, std::string dataFormat, bool isMapped, int nextends, std::string vpath="");

      void initGridDim(int numDim);
      int initGridAxis(int _axisInd, std::string axislabel,  std::string units,  std::string domain,
                       long length, long logicalOrigin, long logicalDelta,
                       double physicalOrigin, double physicalDelta);
      int addProperty(std::string _label, std::string _description, std::string _format, int _count);
      void addSurveyGeom(int minILine, int maxILine, int minXLine, int maxXLine,
    		  float xILine1End, float yILine1End, float xILine1Start, float yILine1Start, float xXLine1End, float yXLine1End);
      void addCustomProperty(std::string name, std::string type, std::string value);
      void add_secondary_path(std::string _path){m_virtualFolders.push_back(_path);}

      //Initializes the object using parameters assigned for mannual setup
      int Initialize();

      // change the grid of axis after Init( jsFileReader* jsReader) only, before Initialize and writeMetaData
      int updateGridAxis(int axisInd, long length, long logicalOrigin, long logicalDelta,
                       double physicalOrigin, double physicalDelta);

      /**
       * @brief Writes metadata (all XML and text files) for the dataset,
       * remove: defines how to deal with trace and header files keep(0)/create(1)/copy(2); default 1 to create
       * @details This should be called only from master node.
      */ 
      int writeMetaData(const int remove = 1);

      /**
       * @brief Writes TraceMap file filled with a const value
       * @details Actually for regular data, where each frame has const amount of live traces
       * TraceMap is not necessary and the dataset can be defined as not mapped (by setting jsIO::jsWriterInit::setMaped(false)).
       * Howvers
      */
      int writeTraceMap4RegularData();

      ///@return Total number of tracaes in the dataset
      long getNtr();

      ///@return Total number of frames in the dataset
      long getNFrames();

      /**
       * @brief Allocates a buffer where a frame can be stored
       * @details Allocates a float buffer (with new[] command) with a size of a full frame and return the pointer.
       * It is up to the user to free the buffer afterwards
      */
      float* allocFrameBuf();

      ///Returns the size of an array (in floats) that will be allocated by allocFrameBuf() function
      int getFrameBufSize(){return m_numSamples*m_numTraces;}
      
      /**
       * @brief Allocates a buffer where a frame header can be stored
       * @param initVals If true, then the header-buffer the SeisSpace standard headers automatically will be initalized a corresponding values.
       * @details Allocates a char buffer (with new[] command) with frame-header size.
       * It is up to the user to free the buffer afterwards 
       */
      char* allocHdrBuf(bool initVals=true);

      ///Returns the size of an array (in bytes) that will be allocated by allocHdrBuf() function
      int getHdrBufSize(){return m_headerLengthBytes*m_numTraces;}

      ///Get access to header-word / property with a name _name
      catalogedHdrEntry getHdrEntry(std::string _name) const;

      ///Returns header-word/property conected with the physical value of axis with index _axisInd
      catalogedHdrEntry getAxisHdrEntry(int _axisInd) const;

      ///Returns header-word/property conected with the logical value of axis with index _axisInd
      catalogedHdrEntry getAxisBinHdrEntry(int _axisInd) const;

      ///Get list of all header-words
      std::vector<catalogedHdrEntry> getHdrEntries() const;

      ///Returns the size of one trace header
      int getTraceHeaderSize()const {return m_headerLengthBytes;}

      /**
       * @brief Get the length (in bytes) of 'raw' frame
       * @details
       * A raw frame is a frame as it is saved on a disk, i.e. possibly in compressed format.
       * That is in the data format is FLOAT, then it returs  getAxisLen(0)*getAxisLen(1)*sizeof(float)
       */
      int getNumBytesInRawFrame() const;

      unsigned long getIOBufferSize() const {return m_IOBufferSize;}
      int getNDim() const;
      int getAxisLen(int index) const;
      int getAxisLogicalOrigin(int index) const;
      int getAxisLogicalDelta(int index) const;
      double getAxisPhysicalOrigin(int index) const;
      double getAxisPhysicalDelta(int index) const;

      int getAxisLogicalValues(int index, std::vector<long> &axis) const;
      int getAxisPhysicalValues(int index, std::vector<double> &axis) const;
      int getAxisLabels(std::vector<std::string> &axis) const;
      int getAxisUnits(std::vector<std::string> &units) const;

      JS_BYTEORDER getByteOrder() const {return m_byteOrder;}
      std::string getByteOrderAsString() const;
      std::string getTraceFormatName() const;
      std::string getDataType() const;

      std::string getVersion() const;
      int getNumOfExtents() const;
      int getNumOfVirtualFolders() const;

      int indexToLogical(int* position) const; // *input position must be in index, and will convert to logical corrdinates

      int logicalToIndex(int* position) const; // *input position must be in logical corrdinates and will convert to index

      /**
       * @brief Left justify input frame and header buffer (headbuf)
       * @details In JavaSeis dataset all live traces in a frame must be so called left-justified.
       * This function rearrange traces and corresponging headers in the frame, so all live traces
       * (i.e. traces where header-word "TRC_TYPE" is not 0) come first (i.e. are left-justified)
       * @param frame input data frame
       * @param headbuf input header frame
       * @param numTraces number of traces in frame and headbuf (must be equal)
       * @return number of live traces in input frame
      */
      int leftJustify(float* frame, char* headbuf, int numTraces);

      /**
       * @brief Writes frame to the dataset
       * @param position position of the frame given in logical coordinates
       * @param frame the frame to be written
       * @param headbuf the corresponding frame header. If not NULL then it will be written in
       *                the corresponding position in TraceHeader(s)
       * @param numLiveTraces the number of live traces in the frame. If positive, then it will be written in
       *                      the corresponding position in TraceMap
       * @return the number of written traces
       * @details For writing non-regular data all four parameters must be set. In case of regular data it will be faster
       * to write TraceMap (using writeTraceMap4RegularData()) and TraceHeader(s) (using writeFrameHeader) separately
       * and here set/use only first two parameters.
       *
     */
      int writeFrame(int* position, float* frame, char* headbuf=NULL, int numLiveTraces=-1);

      /**
       * @brief Writes frame to the dataset
       * @param frameIndex global index of the frame
       * @param frame the frame to be written
       * @param headbuf the corresponding frame header. If not NULL then it will be written in
       *                the corresponding position in TraceHeader(s)
       * @param numLiveTraces the number of live traces in the frame. If positive, then it will be written in
       *                      the corresponding position in TraceMap
       * @return the number of written traces
       * @details For writing non-regular data all four parameters must be set. In case of regular data it will be faster
       * to write TraceMap (using writeTraceMap4RegularData()) and TraceHeader(s) (using writeFrameHeader) separately
       * and here set/use only first two parameters.
       *
       */
      int writeFrame(long frameIndex, float* frame,  char* headbuf=NULL, int numLiveTraces=-1);

      /**
       * @brief Writes frame header to the dataset
       * @details In case of regular data, where we need NOT to to run leftJustify function
       * the headers can be written separetly from traces, which in general should be faster
       * @param position position of the frame header given in logical coordinates
       * @param headbuf the buffer where frame header should be written
       * @return JS_OK if successful
      */
      int writeFrameHeader(int* position, char* headbuf);

       /**
       * @brief Writes frame header to the dataset
       * @details In case of regular data, where we need NOT to to run leftJustify function
       * the headers can be written separetly from traces, which in general should be faster
       * @param frameIndex global index of the frame header
       * @param headbuf the buffer where frame header should be written
       * @return JS_OK if successful
      */
      int writeFrameHeader(long frameIndex, char* headbuf);

       /**
       * @brief Writes several frames at once
       * @details Note that this methods works correct ONLY for FLOAT data format and ONLY for
       * regular data, i.e. all traces in frame are live.
       * @param  position position of the first frame given in logical coordinates
       * @param frames frames to be written
       * @param nFrames number of frames to be written
       * @return JS_OK if successful
       * @details This function works only in case of FLOAT data format
      */
      int writeFrames(int* position, float* frames, int nFrames);

      /**
       * @brief Writes several frames at once (works only for FLOAT data format)
       * @param frameIndex global index of the frame given in logical coordinates
       * @param frames frames to be written
       * @param nFrames number of frames to be written
       * @return JS_OK if successful
      */
      int writeFrames(long frameIndex, float* frames, int nFrames);

      /**
       * @brief Writes single trace (works only for FLOAT and regular data)
       * @param  traceIndex  global index of the trace to be written
       * @param trace trace to be written
       * @param headbuf the corresponding trace header. If not NULL then it will be written in
       *                the corresponding position in TraceHeader(s)
       * @return JS_OK if successful
      */
      int writeTrace(long traceIndex, float* trace, char* headbuf=NULL);


      ///Closes the dataset and flushes all caches
      void Close();
      
    private:
      GridDefinition *m_gridDef;

      TraceProperties *m_traceProps;
      FileProperties *m_fileProps;

      CustomProperties *m_customProps;

      std::string m_filename;
      std::string m_description;

      int m_numExtends;
      std::vector<std::string> m_virtualFolders;

      bool m_bInit;
      bool m_bTraceMapWritten;

      long m_TotalNumOfTr;
      long m_TotalNumOfFrames;

      long m_traceFileSize;
      long m_headerFileSize;

      int m_numSamples;
      int m_numTraces;
      int m_traceSize;
      long m_frameSize;
      int m_compess_traceSize;

      int m_headerLengthBytes;
      long m_frameHeaderSize;
      int m_headerLengthWords;
      int m_seispegPolicy;

      int m_trBufferArrayLen;
      
      TraceMap *m_trMap;
      jsFileReader  *m_jsReader;

      ExtentList *m_TrFileExtents;
      ExtentList *m_TrHeadExtents;

      bool m_bSeisPEG_data; //true is using SeisPEG.
      bool m_bisFloat;//true is format=float
      JS_BYTEORDER m_byteOrder;

      unsigned long m_IOBufferSize; 
//      IOCachedWriter *m_pCachedWriterHD;
//      IOCachedWriter *m_pCachedWriterTR;

//      int m_currIndexOfTrFileExtent;
//      int m_curr_trffd;
//      int m_currIndexOfTrHeadExtent;
//      int m_curr_trhfd;

      int m_numDim;

    private:
      int writeSingleProperty(std::string datasetPath, std::string fileName, std::string propertyName,
                              std::string propertyValue);

      int writeTraceBuffer(long offset, char* buf, long buflen);
      int writeHeaderBuffer(long offset, char* buf, long buflen);

      long getFrameIndex(int* position);    // position must be in logical coordinate
      long getOffsetInExtents(int* indices, int len1d); // indices must be in index

      void axisGridToProps(GridDefinition *gridDef);
};
}

#endif
