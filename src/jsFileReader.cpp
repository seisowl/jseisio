/***************************************************************************
                          jsFileReader.cpp -  description
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

#include <limits>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "jsFileReader.h"
#include "GridDefinition.h" 
#include "TraceProperties.h"
#include "PropertyDescription.h"
#include "FileProperties.h"
#include "CustomProperties.h"
#include "TraceMap.h"
#include "compress/TraceCompressor.h"
#include "compress/SeisPEG.h"

#include "IOCachedReader.h"

#include "PSProLogging.h"
#include "ExtentList.h"

namespace jsIO
{
  DECLARE_LOGGER(jsFileReaderLog);

  jsFileReader::~jsFileReader()
  {
    Close();
  }

  jsFileReader::jsFileReader(const unsigned long _bufferSize)
  {
    m_curr_trffd = -1;
    m_curr_trhfd = -1;
    m_traceBufferArray = NULL;
    m_traceBuffer = NULL;
    m_headerBufferArray=NULL;
    m_headerBuffer = NULL;
    m_headerBufferView = NULL;
    m_trMap=NULL;

    m_traceProps = NULL;
    m_fileProps = NULL;
    m_customProps = NULL;
    m_TrFileExtents = NULL;
    m_TrHeadExtents = NULL;

    m_IOBufferSize = _bufferSize;
    m_pCachedReaderHD = NULL;
    m_pCachedReaderTR = NULL;

    m_traceCompressor=NULL;
    m_seispegCompressor=NULL;

    m_frame = NULL;
    m_frameHeader = NULL;
    m_frameInd=-1;
    m_frameHeaderInd = -1;

    m_bInit=false;
  }

void jsFileReader::closefp() {
	if (m_trMap != NULL) {
		m_trMap->closefp();
		delete m_trMap;
		m_trMap = NULL;
	}
	if (m_curr_trffd > 0) {
		::close (m_curr_trffd);
		m_curr_trffd = -1;
	}
	if (m_curr_trhfd > 0) {
		::close (m_curr_trhfd);
		m_curr_trhfd = -1;
	}
}

  void jsFileReader::Close()
  {
    if(m_traceBufferArray!=NULL)delete []m_traceBufferArray;
    if(m_traceBuffer!=NULL) delete []m_traceBuffer;
    if(m_headerBufferArray!=NULL)delete []m_headerBufferArray;
    if(m_headerBuffer!=NULL) delete []m_headerBuffer;
    if(m_headerBufferView!=NULL)delete []m_headerBufferView;
    if(m_traceProps!=NULL) delete m_traceProps;
    if(m_customProps!=NULL) delete m_customProps;
    if(m_fileProps!=NULL) delete m_fileProps;
    if(m_TrFileExtents!=NULL) delete m_TrFileExtents;
    if(m_TrHeadExtents!=NULL) delete m_TrHeadExtents;

    if(m_traceCompressor!=NULL)delete []m_traceCompressor;
    if(m_seispegCompressor!=NULL)delete []m_seispegCompressor;

    if(m_frame!=NULL)delete []m_frame;
    if(m_frameHeader!=NULL)delete []m_frameHeader;

	closefp();

    if(m_pCachedReaderHD!=NULL)delete m_pCachedReaderHD;
    if(m_pCachedReaderTR!=NULL)delete m_pCachedReaderTR;
  }

  int jsFileReader::Init(const std::string _jsfilename, const int _NThreads)
  {
    TRACE_PRINTF(jsFileReaderLog, "Init JS_Reader : %s", _jsfilename.c_str());
    Close();

    m_filename= _jsfilename;
    if(m_filename[m_filename.length()-1]!='/') m_filename.append(1,'/');

    if(_NThreads<1 || _NThreads>1024){
      ERROR_PRINTF(jsFileReaderLog, "Invalid number of threads. %d must be between 0 and 1024",_NThreads);
      return JS_USERERROR;
    }
    m_NThreads = _NThreads;

    int ires;
    std::ifstream ifile;
    std::string fname = m_filename + "/"+JS_FILE_PROPERTIES_XML;
    ifile.open(fname.c_str(), std::ifstream::in);
    if(! ifile.good() ) {
      ERROR_PRINTF(jsFileReaderLog, "Invalid JavaSeis Format. Error while opening file %s", fname.c_str());
      return JS_USERERROR;
    }

    m_descriptiveName = "Undefined";
    readSingleProperty(m_filename, JS_FILE_STUB,"DescriptiveName",m_descriptiveName);

    m_traceProps = new TraceProperties;
    m_fileProps = new FileProperties;
    m_customProps = new CustomProperties;
    m_TrFileExtents = new ExtentList;
    m_TrHeadExtents = new ExtentList;
 
    std::string xmlString;
  // get length of file:
    ifile.seekg (0, std::ios::end);
    long length = ifile.tellg();
    ifile.seekg (0, std::ios::beg);
  // read the file into xmlString
    char *buffer = new char [length+1];
    ifile.read (buffer,length);
    ifile.close();
    buffer[length]='\0'; 
    xmlString.append(buffer);
    delete[]buffer;

    ires = m_fileProps->load(xmlString);
    if(ires != JS_OK){
      ERROR_PRINTF(jsFileReaderLog, "Invalid JavaSeis XML file %s", fname.c_str());
      return JS_USERERROR;
    }

    ires = m_traceProps->load(xmlString);
    if(ires != JS_OK){
      ERROR_PRINTF(jsFileReaderLog, "Invalid JavaSeis XML file %s", fname.c_str());
      return JS_USERERROR;
    }

    ires = m_customProps->load(xmlString);
    //if(ires != JS_OK){
    //   ERROR_PRINTF(jsFileReaderLog, "Invalid JavaSeis XML file %s", fname.c_str());
    //   return JS_USERERROR;
    //}

    ires = initExtents(m_filename);
    if(ires != JS_OK){
      ERROR_PRINTF(jsFileReaderLog, "Invalid JavaSeis XML file %s", fname.c_str());
      return JS_USERERROR;
    }
        
    m_byteOrder = m_fileProps->byteOrder;
    m_currIndexOfTrFileExtent = -1;
    m_curr_trffd = -1;

    m_currIndexOfTrHeadExtent = -1;
    m_curr_trhfd = -1;

    m_numSamples = m_fileProps->axisLengths[GridDefinition::SAMPLE_INDEX];
    m_numTraces = m_fileProps->axisLengths[GridDefinition::TRACE_INDEX];
    m_compess_traceSize = TraceCompressor::getRecordLength(m_fileProps->traceFormat, m_numSamples);
    m_frameSize = m_numTraces * m_compess_traceSize;

    //                
    m_headerLengthBytes = m_traceProps->getRecordLength();
    m_headerLengthWords =  ceil(m_headerLengthBytes /4.);

    m_frameHeaderLength = ((long) m_headerLengthBytes * (long) m_numTraces);

    m_bIsFloat = false;
    if(m_fileProps->traceFormat.getName()==DataFormat::FLOAT.getName())
      m_bIsFloat = true;

    m_TotalNumOfFrames = 1;
    for(int i=2;i<m_fileProps->numDimensions;i++)
      m_TotalNumOfFrames *= m_fileProps->axisLengths[i];

    m_TotalNumOfTraces = m_TotalNumOfFrames * m_fileProps->axisLengths[1];

    m_pCachedReaderHD = new IOCachedReader(-1, m_IOBufferSize, 1);
    m_pCachedReaderTR = new IOCachedReader(-1, m_IOBufferSize, 1);

  //*** check Regularity and compute total number of live traces 
    if(m_fileProps->isMapped==false){ //if not mapped, then it must be regular
      m_bIsMapped=false;
      m_bIsRegular=true;
      int NDim = m_fileProps->numDimensions;
      m_TotalNumOfLiveTraces = 1; 
      for(int i=1;i<NDim;i++){
        m_TotalNumOfLiveTraces *= m_fileProps->axisLengths[i];
      }
    }else{
    //else: read TraceMap and check whether it contains a value that differs from m_numTraces
    //if so, then it is not regular, otherwise regular
      m_bIsMapped=true;
      m_bIsRegular=true;
      std::ifstream ifile;
      std::string fname = m_filename + "/"+JS_TRACE_MAP;
      ifile.open(fname.c_str(), std::ifstream::in);
      if(! ifile.good() ) {
        ERROR_PRINTF(jsFileReaderLog, "Invalid JavaSeis Format. Error while opening file %s", fname.c_str());
        return JS_USERERROR;
      }

      int numReadedInts=0;
      int maxInt2read = 4096;
      int *iBuffer = new int[maxInt2read];
      m_TotalNumOfLiveTraces = 0; 
      while(ifile.good() && numReadedInts<m_TotalNumOfFrames){
        int ints2Read = (m_TotalNumOfFrames-numReadedInts > maxInt2read)?maxInt2read:m_TotalNumOfFrames-numReadedInts;
        ifile.read ((char*)iBuffer,ints2Read*sizeof(int));
        for(int i=0;i<ints2Read;i++){
          m_TotalNumOfLiveTraces += iBuffer[i];
          if(iBuffer[i]!=m_numTraces){
            m_bIsRegular=false;
//              TRACE_PRINTF(jsFileReaderLog, "Not Regular %d!=%d", iBuffer[i],m_numTraces);
          }
        }
        numReadedInts += ints2Read;
      }
      ifile.close();
      delete[]iBuffer;
    }
  //***
    TRACE_PRINTF(jsFileReaderLog,"Data Format=%s",m_fileProps->traceFormat.getName().c_str());
    TRACE_PRINTF(jsFileReaderLog,"Is Regular = %s",m_bIsRegular?"TRUE":"FALSE");
    TRACE_PRINTF(jsFileReaderLog,"traceLen=%d, TotalNumOfFrames=%ld, TotalNumOfTraces=%ld",
                 m_compess_traceSize,m_TotalNumOfFrames, m_TotalNumOfLiveTraces);

    m_traceBufferArray = new char [m_NThreads*m_frameSize];
    m_headerBufferArray = new char [m_NThreads*m_frameHeaderLength];

    m_traceBuffer = new CharBuffer[m_NThreads]; // !!!!!!!!!!!!!!!!!!!
    m_headerBuffer = new CharBuffer[m_NThreads];
    m_headerBufferView = new IntBuffer[m_NThreads];

    for(int i=0;i<m_NThreads;i++){
      m_traceBuffer[i].setByteOrder(m_byteOrder);
      m_traceBuffer[i].wrap(&m_traceBufferArray[i*m_frameSize], m_frameSize); //during the multi-thread execution should be "wrap"-ed respectively
      m_headerBuffer[i].wrap(&m_headerBufferArray[i*m_frameHeaderLength], m_frameHeaderLength); //during the multi-thread execution should be "wrap"-ed respectively
      m_headerBuffer[i].setByteOrder(m_byteOrder);
      m_headerBuffer[i].asIntBuffer(m_headerBufferView[i]);
    }

    if(m_bIsMapped){
      m_trMap = new TraceMap;
      int trMap_numDim=m_fileProps->numDimensions;
      long * trMap_axes = new long[trMap_numDim];
      for(int i=0;i<trMap_numDim;i++)
        trMap_axes[i]=m_fileProps->axisLengths[i];

      m_trMap->Init(trMap_axes, trMap_numDim, m_byteOrder,  m_filename, "r");

      delete[]trMap_axes;
    }

    m_prev_firstTr1 = 0;
    m_prev_numTraces1 = 0;
    m_prev_frInd1 = 0;

    m_prev_firstTr2 = 0;
    m_prev_numTraces2 = 0;
    m_prev_frInd2 = 0;

    if(m_fileProps->traceFormat.getName()!=DataFormat::SEISPEG.getName()){
      m_traceCompressor = new TraceCompressor[m_NThreads];
      for(int i=0;i<m_NThreads;i++){
        m_traceCompressor[i].Init(m_fileProps->traceFormat, m_numSamples, &m_traceBuffer[i]);
      }
      m_bSeisPEG_data=false;
    }else{
      m_bSeisPEG_data=true;
    //** read 1 frame from the first TraceFile and use it to initalize m_seispegCompressor
      std::string fname = (*m_TrFileExtents)[0].getPath();
      std::ifstream infile(fname.c_str(), std::ifstream::in);
      if(! infile.good() )  return JS_USERERROR;
      infile.read (m_traceBufferArray,m_frameSize);
      infile.close();
      m_seispegCompressor = new SeisPEG[m_NThreads];
      for(int i=0;i<m_NThreads;i++){
        int ires = m_seispegCompressor[i].Init(m_traceBufferArray);
        if(ires!=JS_OK){
          delete[] m_seispegCompressor;
          m_seispegCompressor=NULL;
          ERROR_PRINTF(jsFileReaderLog, "Invalid JavaSeis SeisPEG file");
          return JS_USERERROR;
        }
      }
    //** 
    }


    m_traceProps->setBuffer(m_headerBuffer);

    TRACE_PRINTF(jsFileReaderLog, "Init DONE!\n");

    m_bInit=true;
    return JS_OK;
  }

  long jsFileReader::getNtr() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_TotalNumOfLiveTraces;
  }

  long jsFileReader::getNFrames() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_TotalNumOfFrames;
  }

  long jsFileReader::readTraces(const long _firstTraceIndex,  const long _numOfTraces, float *buffer, char *headbuf)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return -1;
    }
    if(_firstTraceIndex<0 || _firstTraceIndex>=m_TotalNumOfTraces){
      ERROR_PRINTF(jsFileReaderLog, "Invalid trace index. %ld must be in [0,%ld)", _firstTraceIndex, m_TotalNumOfTraces);
      return -1;
    }
    if(_firstTraceIndex+_numOfTraces>m_TotalNumOfTraces){
      ERROR_PRINTF(jsFileReaderLog, "Can't read %ld traces starting from the trace #%ld, because there are only %ld traces total",
      _numOfTraces, _firstTraceIndex, m_TotalNumOfTraces);
      return -1;
    }

    long frameInd; //index of the frame where trace with the index _firstTraceIndex is located
    int trInd; //index of the trace # _firstTraceIndex  within the frame with index frameInd

    long numOfProcessedTraces = 0;

    if (m_bIsRegular){
      frameInd = (long) (_firstTraceIndex / m_numTraces);
      trInd = _firstTraceIndex - frameInd*m_numTraces;
    }else{
      frameInd = (long) (_firstTraceIndex / m_numTraces);
      trInd = _firstTraceIndex - frameInd*m_numTraces;

      int numLiveTraces = m_trMap->getFold(frameInd);
      if(trInd>=numLiveTraces) //the first trace to be read is dead,
                               //so start from the next live frame
      {
        numOfProcessedTraces += (m_numTraces - trInd);
        trInd = 0;
        frameInd ++;

        if(frameInd>=m_TotalNumOfFrames)
        {
          return 0;
        }
        numLiveTraces = m_trMap->getFold(frameInd);
        while(frameInd<m_TotalNumOfFrames-1 && numLiveTraces==0)
        {
          frameInd ++;
          numOfProcessedTraces += m_numTraces;
          if(numOfProcessedTraces >=_numOfTraces)
          {
            return 0;
          }
          numLiveTraces = m_trMap->getFold(frameInd);
        }
      }
    }
/*
    float *frame = new float[m_numTraces*m_numSamples];
    char *frameHeader = NULL;
    if(headbuf!=NULL){
      frameHeader = new char [m_numTraces*m_headerLengthBytes];
    }
*/
    //** allocate buffer at first use
    if(m_frame==NULL){
      m_frame = new float[m_numTraces*m_numSamples];
    }
    if(m_frameHeader==NULL)
    {
      m_frameHeader = new char [m_numTraces*m_headerLengthBytes];
    }
    //**

    float *frame = m_frame;
    char *frameHeader = NULL;
    if(headbuf!=NULL){
      frameHeader = m_frameHeader;
    }

    unsigned long pos=0;
    unsigned long posHeader=0;
    long nReadTraces=0; //the number of live traces read

    int nLiveTr=m_numOfFrameLiveTraces;
    if(m_frameInd!=frameInd) //read only if not in buffer
    {
      // read first nLiveTr-trInd traces
      nLiveTr=readFrame(frameInd, frame, frameHeader);
//       printf("Read new frame %ld, nLiveTr=%d\n",frameInd, nLiveTr);
      if(nLiveTr<0){
        return JS_USERERROR;
      }
      m_frameInd = frameInd;
      m_numOfFrameLiveTraces=nLiveTr;
    }

    int numTraces = std::min((long)(nLiveTr-trInd),(long)(_numOfTraces-numOfProcessedTraces));
    if(numTraces<0 || numTraces>nLiveTr)
    {
      ERROR_PRINTF(jsFileReaderLog, "Invalid trace index.");
      return JS_USERERROR;
    }

    memcpy((char*)&buffer[pos],(char*)&frame[trInd*m_numSamples], (numTraces*m_numSamples) * sizeof(float));
    if(headbuf){
      memcpy((char*)&headbuf[posHeader],(char*)&frameHeader[trInd*m_headerLengthBytes], (numTraces*m_headerLengthBytes));
    }

    pos += numTraces*m_numSamples;
    posHeader += numTraces*m_headerLengthBytes;
    nReadTraces += numTraces;
    numOfProcessedTraces += m_numTraces-trInd;
    frameInd++;

    // read the rest of traces
    while(numOfProcessedTraces<_numOfTraces)
    {
      nLiveTr=readFrame(frameInd, frame, frameHeader);
      if(nLiveTr<0)
      {
        return JS_USERERROR;
      }
      m_frameInd = frameInd;
      m_numOfFrameLiveTraces=nLiveTr;

      numTraces = std::min((int)nLiveTr,(int)(_numOfTraces-numOfProcessedTraces));
      memcpy((char*)&buffer[pos],(char*)&frame[0], (numTraces*m_numSamples) * sizeof(float));
      if(headbuf)
      {
        memcpy((char*)&headbuf[posHeader],(char*)&frameHeader[0], (numTraces*m_headerLengthBytes));
      }
      pos += numTraces*m_numSamples;
      posHeader += numTraces*m_headerLengthBytes;
      nReadTraces += numTraces;
      numOfProcessedTraces += m_numTraces;
      frameInd++;
    }

    return nReadTraces;
  }


  long jsFileReader::readTraceHeaders(const long _firstTraceIndex,  const long _numOfTraces, char *headbuf)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return -1;
    }
    if(_firstTraceIndex<0 || _firstTraceIndex>=m_TotalNumOfTraces){
      ERROR_PRINTF(jsFileReaderLog, "Invalid trace index. %ld must be in [0,%ld)", _firstTraceIndex, m_TotalNumOfTraces);
      return -1;
    }
    if(_firstTraceIndex+_numOfTraces>m_TotalNumOfTraces){
      ERROR_PRINTF(jsFileReaderLog, "Can't read %ld traces starting from the trace #%ld, because there are only %ld traces total",
      _numOfTraces, _firstTraceIndex, m_TotalNumOfTraces);
      return -1;
    }

    long frameInd; //index of the frame where trace with the index _firstTraceIndex is located
    int trInd; //index of the trace # _firstTraceIndex  within the frame with index frameInd

    long numOfProcessedTraces = 0;

    if (m_bIsRegular){
      frameInd = (long) (_firstTraceIndex / m_numTraces);
      trInd = _firstTraceIndex - frameInd*m_numTraces;
    }else{
      frameInd = (long) (_firstTraceIndex / m_numTraces);
      trInd = _firstTraceIndex - frameInd*m_numTraces;

      int numLiveTraces = m_trMap->getFold(frameInd);
      if(trInd>=numLiveTraces) //the first trace to be read is dead,
                               //so start from the next live frame
      {
        numOfProcessedTraces += (m_numTraces - trInd);
        trInd = 0;
        frameInd ++;

        if(frameInd>=m_TotalNumOfFrames)
        {
          return 0;
        }
        numLiveTraces = m_trMap->getFold(frameInd);
        while(frameInd<m_TotalNumOfFrames-1 && numLiveTraces==0)
        {
          frameInd ++;
          numOfProcessedTraces += m_numTraces;
          if(numOfProcessedTraces >=_numOfTraces)
          {
            return 0;
          }
          numLiveTraces = m_trMap->getFold(frameInd);
        }
      }
    }

    if(m_frameHeader==NULL)  //allocate buffer at first use
    {
      m_frameHeader = new char [m_numTraces*m_headerLengthBytes];
    }

    char *frameHeader = m_frameHeader;

    unsigned long posHeader=0;
    long nReadTraces=0; //the number of live traces read

    int nLiveTr=m_numOfFrameHeaderLiveTraces;
    if(m_frameHeaderInd != frameInd) //read only if not in buffer
    {
      // read first nLiveTr-trInd traces
      nLiveTr=readFrameHeader(frameInd, frameHeader);
      if(nLiveTr<0)
      {
        return JS_USERERROR;
      }
      m_frameHeaderInd = frameInd;
      m_numOfFrameHeaderLiveTraces = nLiveTr;
    }

    int numTraces = std::min((long)(nLiveTr-trInd),(long)(_numOfTraces-numOfProcessedTraces));
    if(numTraces<0 || numTraces>nLiveTr)
    {
      ERROR_PRINTF(jsFileReaderLog, "Invalid trace index.");
      return JS_USERERROR;
    }

    memcpy((char*)&headbuf[posHeader],(char*)&frameHeader[trInd*m_headerLengthBytes], (numTraces*m_headerLengthBytes));
    posHeader += numTraces*m_headerLengthBytes;
    nReadTraces += numTraces;
    numOfProcessedTraces += m_numTraces-trInd;
    frameInd++;

    // read the rest of traces
    while(numOfProcessedTraces<_numOfTraces)
    {
      nLiveTr=readFrameHeader(frameInd, frameHeader);
      if(nLiveTr<0)
      {
        return JS_USERERROR;
      }
      m_frameHeaderInd = frameInd;
      m_numOfFrameHeaderLiveTraces = nLiveTr;

      numTraces = std::min((int)nLiveTr,(int)(_numOfTraces-numOfProcessedTraces));
      memcpy((char*)&headbuf[posHeader],(char*)&frameHeader[0], (numTraces*m_headerLengthBytes));
      posHeader += numTraces*m_headerLengthBytes;
      nReadTraces += numTraces;
      numOfProcessedTraces += m_numTraces;
      frameInd++;
    }

    return nReadTraces;
  }


  int jsFileReader::liveToGlobalTraceIndex(const long _liveTraceIndex, long &_globalTraceIndex)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(_liveTraceIndex<0 || _liveTraceIndex>=m_TotalNumOfLiveTraces){
      ERROR_PRINTF(jsFileReaderLog, "Invalid live trace index. %ld must be in [0,%ld)", _liveTraceIndex, m_TotalNumOfLiveTraces);
      return JS_USERERROR;
    }

    long frameInd; //index of the frame where trace with the index _liveTraceIndex is located
    int trInd; //index of the trace # _liveTraceIndex  within the frame with index frameInd

    if (m_bIsRegular){
      _globalTraceIndex = _liveTraceIndex;
    }else{
      long numTraces = 0;
      long frInd = 0;
      int  nTookFrom=0;

      if(_liveTraceIndex<m_prev_firstTr1 ||
         (m_prev_firstTr1==0 && m_prev_firstTr2==0))
      {
        nTookFrom=0;
      }
      else if(_liveTraceIndex>=m_prev_firstTr2)
      {
        numTraces = m_prev_numTraces2;
        frInd = m_prev_frInd2;
        nTookFrom=2;
      }
      else if(_liveTraceIndex>=m_prev_firstTr1)
      {
        numTraces = m_prev_numTraces1;
        frInd = m_prev_frInd1;
        nTookFrom=1;
      }

      int numLiveTraces; //number of live traces in current frame
      numLiveTraces = m_trMap->getFold(frInd);
      do{
        numLiveTraces = m_trMap->getFold(frInd);
        if(numLiveTraces<0){
          ERROR_PRINTF(jsFileReaderLog, "Corrupted TraceMap");
          return JS_USERERROR;
        }
        numTraces += numLiveTraces;
        frInd++;
      }while(numTraces<=_liveTraceIndex);

      frameInd = frInd - 1;
      trInd = _liveTraceIndex - (numTraces - numLiveTraces);

      if(nTookFrom==0)
      {
        m_prev_firstTr1 = _liveTraceIndex;
        m_prev_numTraces1 = numTraces - numLiveTraces;
        m_prev_frInd1 = frameInd;

        m_prev_firstTr2 = _liveTraceIndex;
        m_prev_numTraces2 = numTraces - numLiveTraces;
        m_prev_frInd2 = frameInd;
      }
      else if(nTookFrom==1)
      {
        m_prev_firstTr1 = _liveTraceIndex;
        m_prev_numTraces1 = numTraces - numLiveTraces;
        m_prev_frInd1 = frameInd;
      }
      else if(nTookFrom==2)
      {
        m_prev_firstTr2 = _liveTraceIndex;
        m_prev_numTraces2 = numTraces - numLiveTraces;
        m_prev_frInd2 = frameInd;
      }

      _globalTraceIndex = frameInd * m_numTraces + trInd;
    }
//     printf(" _liveTraceIndex=%ld, _globalTraceIndex=%ld\n", _liveTraceIndex, _globalTraceIndex);
    return JS_OK;
  }


  long jsFileReader::readWithinLiveTraces(const long _firstTraceIndex,  const long _numOfTraces, float *buffer, char* headbuf)
  {
    long first_globalTraceIndex=-1;
    long last_globalTraceIndex=-1;
    long numTraces = 0;
    int ires;

    ires = liveToGlobalTraceIndex(_firstTraceIndex, first_globalTraceIndex);
    if(ires!=JS_OK)
    {
      ERROR_PRINTF(jsFileReaderLog, "Invalid trace index");
      return -1;
    }

    if(_numOfTraces==1)
    {
      numTraces = 1;
    }
    else
    {
      ires = liveToGlobalTraceIndex(_firstTraceIndex + _numOfTraces-1, last_globalTraceIndex);
      if(ires!=JS_OK)
      {
        ERROR_PRINTF(jsFileReaderLog, "Invalid trace index");
        return -1;
      }
      numTraces = 1 + last_globalTraceIndex - first_globalTraceIndex;
    }

//     printf("_firstTraceIndex=%ld, _numOfTraces=%ld, first_globalTraceIndex=%ld, numTraces=%ld\n",
//               _firstTraceIndex, _numOfTraces, first_globalTraceIndex, numTraces);
    return readTraces(first_globalTraceIndex,  numTraces, buffer, headbuf);
  }

  long jsFileReader::readWithinLiveTraceHeaders(const long _firstTraceIndex,  const long _numOfTraces, char *headbuf)
  {
    long first_globalTraceIndex=-1;
    long last_globalTraceIndex=-1;
    long numTraces = 0;
    int ires;

    ires = liveToGlobalTraceIndex(_firstTraceIndex, first_globalTraceIndex);
    if(ires!=JS_OK)
    {
      ERROR_PRINTF(jsFileReaderLog, "Invalid trace index");
      return -1;
    }

    if(_numOfTraces==1)
    {
      numTraces = 1;
    }
    else
    {
      ires = liveToGlobalTraceIndex(_firstTraceIndex + _numOfTraces-1, last_globalTraceIndex);
      if(ires!=JS_OK)
      {
        ERROR_PRINTF(jsFileReaderLog, "Invalid trace index");
        return -1;
      }
      numTraces = 1 + last_globalTraceIndex - first_globalTraceIndex;
    }

    return readTraceHeaders(first_globalTraceIndex,  numTraces, headbuf);
  }

  int jsFileReader::initExtents(const std::string &_filename)
  {
    TRACE_PRINTF(jsFileReaderLog,"Initalizing extents info ...");

    int ires;
    //VirtualFolders vFolders;
    vFolders = new VirtualFolders;
    ires  = vFolders->load(_filename);
    if(ires<0){
      ERROR_PRINTF(jsFileReaderLog, "Unable to load VirtualFolders from  %s\n", _filename.c_str());
      return JS_USERERROR;
    }

    m_TrFileExtents->setVirtualFolders(*vFolders);
    m_TrHeadExtents->setVirtualFolders(*vFolders);

    TRACE_PRINTF(jsFileReaderLog,"Init TraceData extents ...");
    ires = m_TrFileExtents->InitFromXML(_filename,JS_TRACE_DATA_XML);
    if(ires!=JS_OK){
      ERROR_PRINTF(jsFileReaderLog,"TraceFile extents could not be initialized.");
      return ires;
    }

    TRACE_PRINTF(jsFileReaderLog,"Init TraceHeader extents ...");
    ires = m_TrHeadExtents->InitFromXML(_filename, JS_TRACE_HEADERS_XML);
    if(ires!=JS_OK){
      ERROR_PRINTF(jsFileReaderLog,"TraceHeader extents could not be initialized.");
      return ires;
    }
    TRACE_PRINTF(jsFileReaderLog,"Initalizing Extents info . DONE");
    return ires;
  }

  bool jsFileReader::isRegular() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_bIsRegular;
  }


  int jsFileReader::getHeaderWords(std::vector<std::string> &names, std::vector<std::string> &descriptions) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }

    names.clear();
    descriptions.clear();
    int N = m_traceProps->getNumProperties();
    PropertyDescription property;
    for(int i=0;i<N;i++){
      m_traceProps->getTraceProperty(i, property);
      names.push_back(property.getLabel());
      descriptions.push_back(property.getDescription());
    }
    return N;
  }

  float* jsFileReader::allocFrameBuf()
    {
      if(!m_bInit){
        ERROR_PRINTF(jsFileReaderLog,"Properties must be initialized first");
        return NULL;
      }
      return new float[m_numSamples*m_numTraces];
    }

  char* jsFileReader::allocHdrBuf(bool initVals)
    {
      if(!m_bInit){
        ERROR_PRINTF(jsFileReaderLog,"Properties must be initialized first");
        return NULL;
      }
      char *hdrBuf = new char[m_headerLengthBytes*m_numTraces];
      memset(hdrBuf, 0, m_headerLengthBytes*m_numTraces);

      if(initVals) // init with SeisSpace standard values
      {
        float TFULL_E = m_fileProps->physicalOrigins[0] + m_fileProps->axisLengths[0]*m_fileProps->physicalDeltas[0];
        float TLIVE_E = TFULL_E;
        for(int i=0;i<m_numTraces;i++)
        {
          getHdrEntry("TRC_TYPE").setIntVal(&hdrBuf[i*m_headerLengthBytes], 1);
          getHdrEntry("TLIVE_S").setFloatVal(&hdrBuf[i*m_headerLengthBytes], 0.0f);
          getHdrEntry("TFULL_S").setFloatVal(&hdrBuf[i*m_headerLengthBytes], 0.0f);
          getHdrEntry("TLIVE_E").setFloatVal(&hdrBuf[i*m_headerLengthBytes], TLIVE_E);
          getHdrEntry("TFULL_E").setFloatVal(&hdrBuf[i*m_headerLengthBytes], TFULL_E);
          getHdrEntry("LEN_SURG").setFloatVal(&hdrBuf[i*m_headerLengthBytes], 0.0f);
          getHdrEntry("TOT_STAT").setFloatVal(&hdrBuf[i*m_headerLengthBytes], 0.0f);
          getHdrEntry("NA_STAT").setFloatVal(&hdrBuf[i*m_headerLengthBytes], 0.0f);
          getHdrEntry("AMP_NORM").setFloatVal(&hdrBuf[i*m_headerLengthBytes], 1.0f);
          getHdrEntry("TR_FOLD").setFloatVal(&hdrBuf[i*m_headerLengthBytes], 1.0f);
          getHdrEntry("SKEWSTAT").setFloatVal(&hdrBuf[i*m_headerLengthBytes], 0.0f);
          getHdrEntry("PAD_TRC").setIntVal(&hdrBuf[i*m_headerLengthBytes], 0);
          getHdrEntry("NMO_APLD").setIntVal(&hdrBuf[i*m_headerLengthBytes], 1);
        }
      }

      return hdrBuf;
    }

  int jsFileReader::getAxisLabels(std::vector<std::string> &axis) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    axis.clear();
    for(int i=0;i<m_fileProps->numDimensions;i++)
      axis.push_back(m_fileProps->axisLabelsStr[i]);

    return m_fileProps->numDimensions;
  }

  int jsFileReader::getAxisDomains(std::vector<std::string> &domain) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    domain.clear();
    for(int i=0;i<m_fileProps->numDimensions;i++)
      domain.push_back(m_fileProps->axisDomainsStr[i]);

    return m_fileProps->numDimensions;
  }


  int jsFileReader::getAxisUnits(std::vector<std::string> &units) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    units.clear();
    for(int i=0;i<m_fileProps->numDimensions;i++)
      units.push_back(m_fileProps->axisUnitsStr[i]);

    return m_fileProps->numDimensions;
  }


  int jsFileReader::getNDim() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_fileProps->numDimensions;
  }


  bool jsFileReader::isSeisPEG() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_bSeisPEG_data;
  }

  int jsFileReader::getAxisLogicalValues(int index, std::vector<long> &axis) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    int NDim = getNDim();
    if(index<0 || index>=NDim){
      ERROR_PRINTF(jsFileReaderLog, "Axis index %d must be smaller than %d\n", index, NDim);
      return JS_USERERROR;
    }
    axis.clear();
    int axisLen = m_fileProps->axisLengths[index];

    for(int i=0;i<axisLen;i++){
      long fval;
      fval = m_fileProps->logicalOrigins[index] + i*m_fileProps->logicalDeltas[index];
      axis.push_back(fval);
    }
    return axisLen;
  }

  int jsFileReader::getAxisPhysicalValues(int index, std::vector<double> &axis) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    int NDim = getNDim();
    if(index<0 || index>=NDim){
      ERROR_PRINTF(jsFileReaderLog, "Axis index %d must be smaller than %d\n", index, NDim);
      return JS_USERERROR;
    }
    axis.clear();
    int axisLen = m_fileProps->axisLengths[index];

    for(int i=0;i<axisLen;i++){
      double fval;
      fval = m_fileProps->physicalOrigins[index] + i*m_fileProps->physicalDeltas[index];
      axis.push_back(fval);
    }
    return axisLen;
  }

//retrun global offset in TraceFile(s) or TraceHeader(s) corresponding to position (array position directly)
//for TraceFile(s) len1d must be equal to traceLen, ie.len1d=m_compess_traceSize, and for 
//TraceHeader(s) len1d=m_headerLengthBytes
  long jsFileReader::getOffsetInExtents(int* indices, int len1d) const
  {
    int ind_len= m_fileProps->numDimensions;
    unsigned long glb_offset = indices[0];
    long volsize=1;
    for(int i=1;i<ind_len;i++){
      if(indices[i]<0 || indices[i]>=m_fileProps->axisLengths[i]){
        ERROR_PRINTF(jsFileReaderLog, "index %d must be positive and smaller than %ld", indices[i],m_fileProps->axisLengths[i]);
        return JS_USERERROR;
      }
      volsize=len1d;
      for(int j=i;j>=2;j--){
        volsize *= m_fileProps->axisLengths[j];
      }
      glb_offset += indices[i]*volsize;
    }
    return glb_offset;
  }

  int jsFileReader::readTrace(const int* _position, float *trace)
  {
    long traceIndex = getTraceIndex(_position);
    return readTrace(traceIndex, trace);
  }

  int jsFileReader::readTrace(const long _traceIndex, float *trace)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(_traceIndex<0 || _traceIndex>=m_TotalNumOfTraces){
      ERROR_PRINTF(jsFileReaderLog, "Invalid trace index. %ld must be in [0,%ld)", _traceIndex, m_TotalNumOfTraces);
      return JS_USERERROR;
    }
    long glb_offset = _traceIndex * m_compess_traceSize;
    if(m_bIsFloat){//if float, there is no need to uncompress, we can read directly into trace (should be faster)
      int ires = readTraceBuffer(glb_offset, (char*)trace, m_compess_traceSize);
      if(  ires != JS_OK ) return ires;
      if(nativeOrder()!=m_byteOrder)
        endian_swap((void*)trace, m_numSamples, sizeof(float));
    }else if (!m_bSeisPEG_data){
    //read trace from the TraceFile(s)
      int ires = readTraceBuffer(glb_offset, &m_traceBufferArray[0], m_compess_traceSize);
      if(  ires != JS_OK ) return ires;
      m_traceBuffer->position(0);
      m_traceCompressor->unpackFrame(1, trace);
    }
    else{ //  is SeisPEG
      ERROR_PRINTF(jsFileReaderLog, "This function can't be used for SeisPEG formated data. Use readFrame instead.");
      return JS_USERERROR;
    }

    return  JS_OK;
  }


  int jsFileReader::readFrame(const long _frameIndex, float *frame, char *headbuf)
  {
    if(!m_bInit)
    {
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(_frameIndex<0 || _frameIndex>=m_TotalNumOfFrames)
    {
      ERROR_PRINTF(jsFileReaderLog, "Invalid frame index. %ld must be in [0,%ld)\n", _frameIndex, m_TotalNumOfFrames);
      return JS_USERERROR;
    }

    long glb_offset = _frameIndex * m_frameSize;
    int numLiveTraces = getNumOfLiveTraces(_frameIndex);
    if(numLiveTraces==0) return 0;

    long bytesInFrame =  numLiveTraces * m_compess_traceSize;

    if(headbuf!=NULL && !m_bSeisPEG_data)
    {
      long glb_head_offset = _frameIndex * m_frameHeaderLength;
      long bytesInHeaderFrame =  numLiveTraces * m_headerLengthBytes;
      int ires = readHeaderBuffer(glb_head_offset, (char*)&m_headerBufferArray[0], bytesInHeaderFrame);
      if(  ires != JS_OK )
      {
        ERROR_PRINTF(jsFileReaderLog, "Can't read frame header from the file");
        return ires;
      }
      m_traceProps->getTraceHeader(0, numLiveTraces, headbuf);
    }

    if(m_bIsFloat)//if float, there is no need to uncompress, we can read directly into frame (should be faster)
    {
      int ires = readTraceBuffer(glb_offset, (char*)frame, bytesInFrame);
      if(  ires != JS_OK )
      {
        ERROR_PRINTF(jsFileReaderLog, "Can't read frame from the file");
        return ires;
      }
      if(nativeOrder()!=m_byteOrder)
        endian_swap((void*)frame, m_numSamples*m_numTraces, sizeof(float));
    }
    else
    {
    //read gather from the TraceFile(s)
      int ires = readTraceBuffer(glb_offset, &m_traceBufferArray[0], bytesInFrame);
      if(  ires != JS_OK ){
        ERROR_PRINTF(jsFileReaderLog, "Can't read frame from the file");
        return ires;
      }
    //if dataFormat is not FLOAT - uncompress 
      if(m_bSeisPEG_data)
      {
        if(headbuf!=NULL)
        {
          m_seispegCompressor->uncompress(m_traceBufferArray, bytesInFrame, frame, numLiveTraces, m_headerBufferView, m_headerLengthBytes);
          m_traceProps->getTraceHeader(0, numLiveTraces, headbuf);
        }
        else
        {
          m_seispegCompressor->uncompress(m_traceBufferArray, bytesInFrame, frame, numLiveTraces);
        }
      }
      else
      {
        m_traceBuffer->position(0);
        m_traceCompressor->unpackFrame(numLiveTraces, frame);
      }
    }
    return numLiveTraces;
  }

  int jsFileReader::readFrame(const int* _position, float *frame, char *headbuf)
  {
//     _position[m_fileProps->numDimensions-2]=0;
    long frameIndex = getFrameIndex(_position);
    return readFrame(frameIndex, frame, headbuf);
  }

  int jsFileReader::indexToLogical(int* position) const // *input position must be in index, and will convert to logical corrdinates
  {
    int numAxis= m_fileProps->numDimensions;
    for(int i=0;i<numAxis;i++)
    {
      position[i]=(int)(m_fileProps->logicalOrigins[i] + position[i] * m_fileProps->logicalDeltas[i]);
    }
  }

  int jsFileReader::logicalToIndex(int* position) const // *input position must be in logical corrdinates and will convert to index
  {
    int numAxis= m_fileProps->numDimensions;
    for(int i=0;i<numAxis;i++)
    {
      position[i]=(int)((position[i]-m_fileProps->logicalOrigins[i])/m_fileProps->logicalDeltas[i]);
      if(position[i]<0 || position[i]>m_fileProps->axisLengths[i])
      {
        ERROR_PRINTF(jsFileReaderLog, "Unable to locate a frame with value %d in dimension %d", position[i],i);
        return -1;
      }
    }
  }

  long jsFileReader::getFrameIndex(const int* position) const // *position must be in logical coordinate
  {
    int numAxis= m_fileProps->numDimensions;
    int *index = new int[numAxis];
    for (int i = 0; i < numAxis; i++) {
            index[i] = position[i];
    }
    logicalToIndex(index);

    long frIndex = index[2];
    for(int i=3;i<numAxis;i++)
    {
      long volsize=1;
      for(int j=2;j<i;j++)
      {
        volsize *= m_fileProps->axisLengths[j];
      }
      frIndex += index[i]*volsize;
    }
//   printf("for pos(%d,%d,%d), frame index = %d\n",position[0],position[1],position[2],frIndex);
    return frIndex;
  }

  long jsFileReader::getTraceIndex(const int* position)  const // *position must be in logical coordinate
  {
    int numAxis= m_fileProps->numDimensions;
    int *index = new int[numAxis];
    for (int i = 0; i < numAxis; i++) {
            index[i] = position[i];
    }
    logicalToIndex(index);

    long trIndex = index[1];
    for(int i=2;i<numAxis;i++)
    {
      long volsize=1;
      for(int j=1;j<i;j++)
      {
        volsize *= m_fileProps->axisLengths[j];
      }
      trIndex += index[i]*volsize;
    }
    return trIndex;
  }

  int jsFileReader::getNumOfLiveTraces(int _frameIndex) const
  {
    int numLiveTraces = m_numTraces;
    if (!m_bIsRegular) numLiveTraces = m_trMap->getFold(_frameIndex);
    return numLiveTraces;
  }

  int jsFileReader::readRawFrames(const int* _position, int _NFrames, char *rawframe, int *numLiveTraces)
  {
//   printf("position(%d,%d)\n", position[0],position[1]);
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
//     _position[m_fileProps->numDimensions-2]=0;
    long frameIndex = getFrameIndex(_position);

    return readRawFrames(frameIndex, _NFrames, rawframe, numLiveTraces);
  }


  int jsFileReader::readRawFrames(const long _frameIndex, int _NFrames, char *rawframe, int *numLiveTraces)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(_frameIndex<0 || _frameIndex>=m_TotalNumOfFrames){
      ERROR_PRINTF(jsFileReaderLog, "Invalid frame index. %ld must be in [0,%ld)\n", _frameIndex, m_TotalNumOfFrames);
      return JS_USERERROR;
    }
    if( _frameIndex+_NFrames>m_TotalNumOfFrames){
      TRACE_PRINTF(jsFileReaderLog, "Can't read %d frames starting from the frame #%ld, because there are only %ld frames total.\n  \
          Read %ld frames indstead.", _NFrames, _frameIndex, m_TotalNumOfFrames, m_TotalNumOfFrames-_frameIndex);
      _NFrames= m_TotalNumOfFrames-_frameIndex;
    }

    long glb_offset = _frameIndex * m_frameSize;
    for(int i=0;i<_NFrames;i++)
    {
      numLiveTraces[i] = getNumOfLiveTraces(_frameIndex+i);
      if(numLiveTraces[i]<0){
        ERROR_PRINTF(jsFileReaderLog, "Can't read from TraceMap");
        return JS_USERERROR;
      }
    }
    long numBytes2Read = m_frameSize*_NFrames;
    int ires = readTraceBuffer(glb_offset, rawframe, numBytes2Read);
    if(  ires != JS_OK ){
      ERROR_PRINTF(jsFileReaderLog, "Can't read frame from the file");
      return ires;
    }

    return JS_OK;
  }


  int jsFileReader::uncompressRawFrame(char *rawframe, int numLiveTraces, int iThread, float *frame, char* headbuf)
  {
    if(headbuf!=NULL && !m_bSeisPEG_data){
      TRACE_PRINTF(jsFileReaderLog, "Can't initialize header for non SeisPEG data.\n \
                                     For header initialization of non-SeisPEG data use readFrameHeader() function");
//       return JS_USERERROR;
    }

    if(headbuf==NULL && m_bSeisPEG_data){
      ERROR_PRINTF(jsFileReaderLog, "For SeisPEG data headbuf must be pre-allocated.");
      return JS_USERERROR;
    }

    if(numLiveTraces<=0)
      return numLiveTraces;

    if(m_bIsFloat){//if float, there is no need to uncompress, we can read directly into frame (should be faster)
      memcpy((char*)frame, (char*)rawframe, m_numSamples*numLiveTraces*sizeof(float));
      if(nativeOrder()!=m_byteOrder)
        endian_swap((void*)rawframe, m_numSamples*numLiveTraces, sizeof(float));

    }else{
    //if dataFormat is not FLOAT - uncompress 
      if(m_bSeisPEG_data){
        if(headbuf!=NULL){
          m_seispegCompressor[iThread].uncompress(rawframe, m_frameSize, frame, numLiveTraces, (int*)headbuf, m_headerLengthWords);
          if(nativeOrder()!=m_byteOrder)
            m_traceProps->swapHeaders(headbuf, numLiveTraces);
        }else
          m_seispegCompressor[iThread].uncompress(rawframe, m_frameSize, frame, numLiveTraces);
      }else{
         m_traceCompressor[iThread].updateBuffer(rawframe, m_frameSize);
         m_traceCompressor[iThread].unpackFrame(numLiveTraces, frame);
      }
    }

    return numLiveTraces;
  }

//read buflen number of bytes from TraceFile(s) into buf
//buf must be pre-allocated with len=buflen
  int jsFileReader::readTraceBuffer(long offset, char *buf, long buflen)
  {
    int lowInd = m_TrFileExtents->getExtentIndex(offset+1);
    int upInd  = m_TrFileExtents->getExtentIndex(offset+buflen);
//   printf("\t %ld, %ld, %d, %d\n", offset, buflen, lowInd, upInd);
//   if(lowInd!=upInd) {
//      printf("%ld, %ld, %d, %d\n", offset, buflen, lowInd, upInd);
//    }

    if(lowInd<0 || upInd<0 || upInd<lowInd)
    {
      ERROR_PRINTF(jsFileReaderLog, "Can't read %ld bytes starting from offset %ld in TraceFile(s)", buflen, offset);
      return JS_USERERROR;
    }

    long rest_buflen = buflen;
    long bytes2read = buflen;
    long loc_offset_trFile = offset - (*m_TrFileExtents)[lowInd].getStartOffset();
    for(int extInd=lowInd; extInd<=upInd;extInd++)
    {
      long extSize = (*m_TrFileExtents)[extInd].getExtentSize();
      if(loc_offset_trFile + rest_buflen >extSize){
        bytes2read = extSize - loc_offset_trFile;
      }

      if(m_currIndexOfTrFileExtent!=extInd)
      {
//          TRACE_VAR(jsFileReaderLog,offset);
//          TRACE_VAR(jsFileReaderLog,extInd);
          ::close(m_curr_trffd);
          std::string fname = (*m_TrFileExtents)[extInd].getPath();
          m_curr_trffd = :: open(fname.c_str(), O_RDONLY);
          TRACE_PRINTF(jsFileReaderLog, "Open new file %d, %d, %s, %d", m_currIndexOfTrFileExtent,extInd, fname.c_str(), m_curr_trffd );
          if( m_curr_trffd < 0 ){
            return JS_WARNING;
          }
          m_currIndexOfTrFileExtent=extInd;
          m_pCachedReaderTR->setNewFile(m_curr_trffd,(*m_TrFileExtents)[extInd].getExtentSizeOnDisk());
      }
//      printf("extInd=%d, m_curr_trffd=%d, rest_buflen=%ld, loc_offset_trFile=%lu\n",extInd,m_curr_trffd,rest_buflen,loc_offset_trFile);
//      ::pread (m_curr_trffd, &buf[buflen-rest_buflen], bytes2read, loc_offset_trFile);
      m_pCachedReaderTR->read(loc_offset_trFile, (unsigned char*) &buf[buflen-rest_buflen], bytes2read);
/*
      if( ::pread (m_curr_trffd, &buf[buflen-rest_buflen], bytes2read, loc_offset_trFile) != bytes2read){
      m_currIndexOfTrFileExtent=-1;
        ::close(m_curr_trffd);
      m_curr_trffd=-1;
      return JS_WARNING;
    }
*/
      rest_buflen -= bytes2read;
      bytes2read = buflen - rest_buflen;
      loc_offset_trFile=0;
    }

    return JS_OK;
  }


//read buflen number of bytes from TraceHeader(s) into buf
//buf must be pre-allocated with len=buflen
  int jsFileReader::readHeaderBuffer(long offset, char *buf, long buflen)
  {
    int lowInd = m_TrHeadExtents->getExtentIndex(offset+1);
    int upInd  = m_TrHeadExtents->getExtentIndex(offset+buflen);

    if(lowInd<0 || upInd<0 || upInd<lowInd){
      return JS_USERERROR;
    }

    long rest_buflen = buflen;
    long bytes2read = buflen;
    long loc_offset_trFile = offset - (*m_TrHeadExtents)[lowInd].getStartOffset();
    for(int extInd=lowInd; extInd<=upInd;extInd++){
      long extSize = (*m_TrHeadExtents)[extInd].getExtentSize();
      if(loc_offset_trFile + rest_buflen >extSize){
        bytes2read = extSize - loc_offset_trFile;
      }

      if(m_currIndexOfTrHeadExtent!=extInd){
          ::close(m_curr_trhfd);
          std::string fname = (*m_TrHeadExtents)[extInd].getPath();
          m_curr_trhfd = :: open(fname.c_str(), O_RDONLY);
//        printf("****** open new header file pInd=%d, cInd=%d, name=%s, fd=%d\n", m_currIndexOfTrHeadExtent,extInd, fname.c_str(), m_curr_trhfd );
          if( m_curr_trhfd < 0 ){
            return JS_WARNING;
          }
          m_currIndexOfTrHeadExtent=extInd;
          m_pCachedReaderHD->setNewFile(m_curr_trhfd, (*m_TrHeadExtents)[extInd].getExtentSizeOnDisk());
      }
//       printf("lowInd=%d, upInd=%d, extInd=%d, m_curr_trhfd=%d, bytes2read=%ld, loc_offset_trFile=%lu, glb_offest=%ld\n",lowInd, upInd, extInd,m_curr_trhfd, bytes2read, loc_offset_trFile, offset);
//     long bread = ::pread(m_curr_trhfd, &buf[buflen-rest_buflen], bytes2read, loc_offset_trFile);
//     if(bread != bytes2read)
//     {
      if(m_pCachedReaderHD->read(loc_offset_trFile, (unsigned char*) &buf[buflen-rest_buflen], bytes2read)!=true){
//       printf("*************** lowInd=%d, upInd=%d, extInd=%d, m_curr_trhfd=%d, bytes2read=%ld, loc_offset_trFile=%lu\t %s \n",lowInd, upInd, extInd,m_curr_trhfd,bytes2read, loc_offset_trFile, strerror(errno));
        m_currIndexOfTrHeadExtent=-1;
          ::close(m_curr_trhfd);
          m_curr_trhfd=-1;
          return JS_WARNING;
      }

      rest_buflen -= bytes2read;
      loc_offset_trFile=0;
    }

    return JS_OK;
  }


  int jsFileReader::getNumHeaderWords() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_traceProps->getNumProperties();
  }

  void jsFileReader::getHeaderWordsInfo(headerWordInfo *pInfo) const
  {
    int N = getNumHeaderWords();
    PropertyDescription property;
    for(int i=0;i<N;i++){
      m_traceProps->getTraceProperty(i, property);
      pInfo[i].format = property.getFormat();
      pInfo[i].count  = property.getCount();
      pInfo[i].offset = property.getOffset();
    }
    return;
  }

  float jsFileReader::getFloatHdrVal(std::string _name, char * headerBuf)
  {
	catalogedHdrEntry hdr = getHdrEntry(_name);
	return hdr.getFloatVal(headerBuf);
  }

  double jsFileReader::getDoubleHdrVal(std::string _name, char * headerBuf)
  {
	catalogedHdrEntry hdr = getHdrEntry(_name);
	return hdr.getDoubleVal(headerBuf);
  }

  long jsFileReader::getLongHdrVal(std::string _name, char * headerBuf)
  {
	catalogedHdrEntry hdr = getHdrEntry(_name);
	return hdr.getLongVal(headerBuf);
  }

  int jsFileReader::getIntHdrVal(std::string _name, char * headerBuf)
  {
	catalogedHdrEntry hdr = getHdrEntry(_name);
	return hdr.getIntVal(headerBuf);
  }

  short jsFileReader::getShortHdrVal(std::string _name, char * headerBuf)
  {
	catalogedHdrEntry hdr = getHdrEntry(_name);
	return hdr.getShortVal(headerBuf);
  }


  int jsFileReader::getNumBytesInHeader() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }  
    return m_headerLengthBytes;
  }

  long jsFileReader::getNumBytesInRawFrame() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_frameSize;
  }

  int jsFileReader::getAxisLen(int index) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(index>=0 && index <getNDim()) return m_fileProps->axisLengths[index];
    return JS_USERERROR;
  }

  int jsFileReader::getAxisLogicalOrigin(int index) const
   {
     if(!m_bInit){
       ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
       return JS_USERERROR;
     }
     if(index>=0 && index <getNDim()) return m_fileProps->logicalOrigins[index];
     return JS_USERERROR;
   }

   int jsFileReader::getAxisLogicalDelta(int index) const
   {
       if(!m_bInit){
         ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
         return JS_USERERROR;
       }
       if(index>=0 && index <getNDim()) return m_fileProps->logicalDeltas[index];
       return JS_USERERROR;
   }

   double jsFileReader::getAxisPhysicalOrigin(int index) const
   {
     if(!m_bInit){
       ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
       return JS_USERERROR;
     }
     if(index>=0 && index <getNDim()) return m_fileProps->physicalOrigins[index];
     return JS_USERERROR;
   }

   double jsFileReader::getAxisPhysicalDelta(int index) const
   {
       if(!m_bInit){
         ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
         return JS_USERERROR;
       }
       if(index>=0 && index <getNDim()) return m_fileProps->physicalDeltas[index];
       return JS_USERERROR;
   }

   catalogedHdrEntry jsFileReader::getAxisHdrEntry(int _axisInd) const
   {
     if(_axisInd>=0 && _axisInd<getNDim())
     {
       return getHdrEntry( (m_fileProps->axisLabels[_axisInd]).getName());
     }
     else
     {
       ERROR_PRINTF(jsFileWriterLog, "Invalid axis index %d. Must be between 0 and %d", _axisInd, getNDim());
       return catalogedHdrEntry();
     }
   }

  int jsFileReader::readTraceHeader(const int* _position, char *headbuf)
  {
    long traceIndex = getTraceIndex(_position);
    return readTraceHeader(traceIndex, headbuf);
  }


  int jsFileReader::readTraceHeader(const long _traceIndex, char *headbuf)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(_traceIndex<0 || _traceIndex>=m_TotalNumOfTraces){
      ERROR_PRINTF(jsFileReaderLog, "Invalid trace index. %ld must be in [0,%ld)\n", _traceIndex, m_TotalNumOfTraces);
      return JS_USERERROR;
    }
    long glb_offset = _traceIndex * m_headerLengthBytes;
    int ires = readHeaderBuffer(glb_offset, (char*)&m_headerBufferArray[0], m_headerLengthBytes);
    if(  ires != JS_OK ) return ires;
    m_traceProps->getTraceHeader(0, 1, headbuf);
    return JS_OK;
  }


  int jsFileReader::readFrameHeader(const int* _position, char *headbuf)
  {
//     position[m_fileProps->numDimensions-2]=0;
    long frameIndex = getFrameIndex(_position);
    return readFrameHeader(frameIndex, headbuf);
  }

  int jsFileReader::readFrameHeader(const long _frameIndex, char *headbuf)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(_frameIndex<0 || _frameIndex>=m_TotalNumOfFrames){
      ERROR_PRINTF(jsFileReaderLog, "Invalid frame index. %ld must be in [0,%ld)\n", _frameIndex, m_TotalNumOfFrames);
      return JS_USERERROR;
    }
 
    int numLiveTraces =  getNumOfLiveTraces(_frameIndex);
    if(!m_bSeisPEG_data){
      long glb_offset = _frameIndex * m_frameHeaderLength;
      long bytesInFrameHeader =  numLiveTraces * m_headerLengthBytes;
      
      int ires = readHeaderBuffer(glb_offset, (char*)&m_headerBufferArray[0], bytesInFrameHeader);
      if(  ires != JS_OK ) return ires;
      m_traceProps->getTraceHeader(0, numLiveTraces, headbuf);

    }else{
      long glb_offset = _frameIndex * m_frameSize;
      long bytesInFrame =  numLiveTraces * m_compess_traceSize;

      int ires = readTraceBuffer(glb_offset, &m_traceBufferArray[0], bytesInFrame);
      if(  ires != JS_OK ){
        ERROR_PRINTF(jsFileReaderLog, "Can't read frame from the file");
        return ires;
      }   
      m_seispegCompressor->uncompressHdrs(m_traceBufferArray, bytesInFrame, (int*)m_headerBufferArray, m_headerLengthBytes);
      m_traceProps->getTraceHeader(0, numLiveTraces, headbuf);
    } 
    return numLiveTraces;
  }


  std::string jsFileReader::getTraceFormatName() const
  {
    return m_fileProps->traceFormat.getName();
  }


  std::string jsFileReader::getDataType() const
  {
    return m_fileProps->dataType.getName();
  }


  std::string jsFileReader::getByteOrderAsString() const
  {
    if(m_byteOrder==JSIO_LITTLEENDIAN){
      return "LittleEndian";
    }
    return "BigEndian";
  }

  int jsFileReader::readSingleProperty(const std::string &_datasetPath, const std::string &_fileName,
                                       const std::string _propertyName, std::string &propertyValue) const
  {
    std::string fpath =  _datasetPath + _fileName;
    std::ifstream infile;
    infile.open( fpath.c_str(), std::ifstream::in);

    std::string line;

    while(infile.good()){
      std::getline(infile, line);
      ltrimStr(line);
      if(line.substr(0, _propertyName.size())==_propertyName){
        line = line.substr(_propertyName.size(), line.size());
        ltrimStr(line);
        if(line[0]=='='){
          propertyValue = line.substr(1, line.size());
          infile.close();
          return JS_OK;
        }
      }
    }
    infile.close();

    return JS_WARNING;
  }
  
  std::string jsFileReader::getVersion() const
  {
    return m_fileProps->version;
  }
  
  catalogedHdrEntry jsFileReader::getHdrEntry(std::string _name) const
  {
    return m_traceProps->getHdrEntry(_name);
  }

  std::vector<catalogedHdrEntry> jsFileReader::getHdrEntries() const
  {
    return m_traceProps->getHdrEntries();
  }

  int jsFileReader::getHdrEntries(std::vector<std::string> &hdrEntries) const
  {
	  int N = getNumHeaderWords();
	  std::vector<jsIO::catalogedHdrEntry, std::allocator<jsIO::catalogedHdrEntry> > hdrs =  m_traceProps->getHdrEntries();
	  for(int i=0;i<N;i++){
		  hdrEntries.push_back(hdrs[i].getName().c_str());
	  }
      return N;
  }

  std::string jsFileReader::getCustomProperty(std::string _property)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileReaderLog, "Properties must be initialized first");
      return "";
    }
        
    std::ifstream ifile;
    std::string fname = m_filename + "/"+JS_FILE_PROPERTIES_XML;
    ifile.open(fname.c_str(), std::ifstream::in);
    if(! ifile.good() ) {
      ERROR_PRINTF(jsFileReaderLog, "Invalid JavaSeis Format. Error while opening file %s", fname.c_str());
      return "";
    }
    ifile.close();
    
    xmlreader xmlReader;
    xmlReader.parseFile(fname.c_str());
    xmlElement* pCustomPropsEl = xmlReader.getBlock("CustomProperties");
    if(pCustomPropsEl==0){
      ERROR_PRINTF(jsFileReaderLog, "There is no CustomProperties part.");
      return "";
    }  
  
    std::vector<std::string> vProps;
    
    std::istringstream ss(_property);
    while (!ss.eof())
    {
      std::string x;
      std::getline( ss, x, '/' ); 
      vProps.push_back(x);
    }
    
    if(vProps.size()==0)
    {
      ERROR_PRINTF(jsFileReaderLog, "Can't read %s from CustomProperties", _property.c_str());
      return ""; 
    }
    
    xmlElement *pEl = pCustomPropsEl; 
    for(int i=0;i<vProps.size()-1;i++)
    {
      pEl = xmlReader.FirstChildBlock(pEl, vProps[i], true);
      if(pEl==0)
      {
        return ""; 
      }
    }

    pEl = xmlReader.FirstChildElement(pEl, vProps[vProps.size()-1], true);
    if(pEl==0)
    {
      return ""; 
    }
    std::string sVal = xmlReader.getText(pEl);

    sVal.erase(0, sVal.find_first_not_of(" \t"));//ltrim
    sVal.erase(sVal.find_last_not_of(" \t")+1, sVal.size());//rtrim

    return sVal;
  }
  
  int jsFileReader::getNumOfExtents() const
  {
    return m_TrFileExtents->getNumExtents();
  }

  int jsFileReader::getNumOfVirtualFolders() const
  {
    return m_TrFileExtents->getNumvFolders();
  }
}
