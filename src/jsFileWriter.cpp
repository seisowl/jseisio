/***************************************************************************
                          jsFileWriter.cpp -  description
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
  
#include "jsFileWriter.h"
#include "GridDefinition.h" 
#include "DataDefinition.h"
#include "TraceProperties.h"
#include "FileProperties.h"
#include "CustomProperties.h"
#include "PropertyDescription.h"
#include "ExtentList.h"
#include "TraceMap.h"
#include "jsWriterInput.h"

#include "ExtentList.h"

#include "Units.h"
#include "DataDomain.h"
#include "AxisLabel.h"

#include "IOCachedWriter.h"

#include "PSProLogging.h"
#include "compress/TraceCompressor.h"
#include "compress/SeisPEG.h"

namespace jsIO
{
  DECLARE_LOGGER(jsFileWriterLog);

  jsFileWriter::~jsFileWriter()
  {
    Close();
    if(m_gridDef!=NULL) delete m_gridDef;
    if(m_dataDef!=NULL) delete m_dataDef;

    if(m_traceProps!=NULL) delete m_traceProps;
    if(m_fileProps!=NULL) delete m_fileProps;
    if(m_customProps!=NULL) delete m_customProps;
    if(m_TrFileExtents!=NULL) delete m_TrFileExtents;
    if(m_TrHeadExtents!=NULL) delete m_TrHeadExtents;

    if(m_trMap!=NULL) delete m_trMap;

    if(m_pCachedWriterHD!=NULL) delete m_pCachedWriterHD;
    if(m_pCachedWriterTR!=NULL) delete m_pCachedWriterTR;
  }

  jsFileWriter::jsFileWriter()
  {
    m_bInit=false;
    m_traceProps = new TraceProperties;
    m_fileProps = new FileProperties;
    m_customProps = new CustomProperties;

    m_TrFileExtents = new ExtentList;
    m_TrHeadExtents = new ExtentList;

    m_gridDef = new GridDefinition;
    m_dataDef = new DataDefinition;

    m_trMap = NULL;

    m_traceCompressor=NULL;
    m_seispegCompressor=NULL;

    m_traceBufferArray = NULL;
    m_traceBuffer = NULL;
    m_headerBufferArray=NULL;
    m_headerBuffer = NULL;
    m_seispegHeaderBuffer = NULL;
    
    m_pCachedWriterHD = NULL;
    m_pCachedWriterTR = NULL;

    m_curr_trffd = -1;
    m_curr_trhfd = -1; 
    m_currIndexOfTrFileExtent = -1;
    m_currIndexOfTrHeadExtent = -1;
  
    m_bTraceMapWritten=false;

//   plogfile = fopen ("/m/scratch/supertmp/abel/jsWriterLog.txt","w");
  }

  void jsFileWriter::Close()
  {
//   fclose(plogfile);
    if(m_seispegCompressor!=NULL)
    {
      TRACE_PRINTF(jsFileWriterLog, "SeisPEG Compression Ration =%g",m_seispegCompressor->getCompressionRatio());
    }

    if(m_traceBufferArray!=NULL)delete []m_traceBufferArray;
    if(m_traceBuffer!=NULL) delete m_traceBuffer;
    if(m_headerBufferArray!=NULL)delete []m_headerBufferArray;
    if(m_headerBuffer!=NULL) delete m_headerBuffer;
    if(m_seispegHeaderBuffer!=NULL)delete m_seispegHeaderBuffer;

    if(m_traceCompressor!=NULL)delete m_traceCompressor;
    if(m_seispegCompressor!=NULL)delete m_seispegCompressor;
 
    if(m_pCachedWriterHD!=NULL) m_pCachedWriterHD->flush();
    if(m_pCachedWriterTR!=NULL) m_pCachedWriterTR->flush();

    ::close(m_curr_trffd);
    ::close(m_curr_trhfd);

    m_traceCompressor=NULL;
    m_seispegCompressor=NULL;

    m_traceBufferArray = NULL;
    m_traceBuffer = NULL;
    m_headerBufferArray=NULL;
    m_headerBuffer = NULL;
    m_seispegHeaderBuffer = NULL;
    
    m_curr_trffd = -1;
    m_curr_trhfd = -1; 
    m_currIndexOfTrFileExtent = -1;
    m_currIndexOfTrHeadExtent = -1;
    
    m_bInit=false;
  }

  int jsFileWriter::Init(const jsWriterInput *_writerInput)
  {
    *m_gridDef = *(_writerInput->gridDef);
    *m_dataDef = *(_writerInput->dataDef);
    *m_traceProps = *(_writerInput->traceProps);
    m_filename = _writerInput->jsfilename;
    m_description = _writerInput->description;
    m_numExtends = _writerInput->NExtends;
    m_virtualFolders = _writerInput->virtualFolders;
    if(m_virtualFolders.size()==0) //not defined
    {
      m_virtualFolders.push_back(m_filename);
    }
//     m_customProps->survGeom = *(_writerInput->geometry);
    *m_customProps = *(_writerInput->customProps);
        
    m_seispegPolicy = _writerInput->seispegPolicy;
        
    m_IOBufferSize = _writerInput->IOBufferSize;

    TRACE_PRINTF(jsFileWriterLog,"Init File Properties");

    if(m_filename[m_filename.length()-1]!='/') m_filename.append(1,'/');

    m_fileProps->dataType = m_dataDef->getDataType(); 
    m_fileProps->traceFormat = m_dataDef->getTraceFormat();

    m_fileProps->comments="www.javaseis.org - JavaSeis File Properties "+JS_VERSION;
    m_fileProps->version="2006.3";
    m_fileProps->byteOrder = nativeOrder();
    m_fileProps->isMapped = _writerInput->isMapped;

    m_byteOrder = m_fileProps->byteOrder ;


    m_numDim = m_gridDef->getNumDimensions();

    m_fileProps->Init(m_numDim);

//************
    for(int i=0;i<m_numDim;i++)
    {
      m_fileProps->axisLabels[i] = m_gridDef->getAxisLabel(i);
      m_fileProps->axisLabelsStr[i]=m_gridDef->getAxisLabelString(i);

      m_fileProps->axisUnits[i] = m_gridDef->getAxisUnits(i);
      m_fileProps->axisUnitsStr[i]=m_gridDef->getAxisUnitsString(i);

      m_fileProps->axisDomains[i] = m_gridDef->getAxisDomain(i);
      m_fileProps->axisDomainsStr[i]=m_gridDef->getAxisDomainString(i);

      m_fileProps->axisLengths[i] = m_gridDef->getAxisLength(i);

      m_fileProps->logicalOrigins[i]= m_gridDef->getAxisLogicalOrigin(i);
      m_fileProps->logicalDeltas[i] = m_gridDef->getAxisLogicalDelta(i);

      m_fileProps->physicalOrigins[i]= m_gridDef->getAxisPhysicalOrigin(i);
      m_fileProps->physicalDeltas[i] = m_gridDef->getAxisPhysicalDelta(i);
    }
//***************
    m_fileProps->headerLengthBytes = m_traceProps->getRecordLength();

    m_TotalNumOfFrames = 1;
    for(int i=2;i<m_fileProps->numDimensions;i++)
      m_TotalNumOfFrames *= m_fileProps->axisLengths[i];
 
    m_TotalNumOfTr = m_TotalNumOfFrames * m_fileProps->axisLengths[1];

    m_numSamples = m_fileProps->axisLengths[GridDefinition::SAMPLE_INDEX];
    m_numTraces = m_fileProps->axisLengths[GridDefinition::TRACE_INDEX];
    m_traceSize = m_numSamples * sizeof(float);
    m_compess_traceSize = TraceCompressor::getRecordLength(m_fileProps->traceFormat, m_numSamples);
    m_frameSize = m_numTraces * m_compess_traceSize;
    m_traceFileSize = m_TotalNumOfTr * m_compess_traceSize;

    m_headerLengthBytes = m_traceProps->getRecordLength();
    m_frameHeaderSize = ((long) m_headerLengthBytes * (long) m_numTraces);
    m_headerFileSize = m_TotalNumOfTr * m_headerLengthBytes;
    m_headerLengthWords = ceil(m_headerLengthBytes/4.f);
//   fprintf(plogfile,"m_headerLengthBytes=%d, m_numTraces=%d, m_headerFileSize=%d\n",m_headerLengthBytes, m_numTraces, m_headerFileSize);

    TRACE_PRINTF(jsFileWriterLog,"Init Virtual Folders");

    VirtualFolders vFolders;
    for(int i=0;i<m_virtualFolders.size();i++)
    {
      vFolders.addFolder(m_virtualFolders[i]);
    }
    
    long trExtSize = ExtentList::computeExtentSize(m_traceFileSize, m_numExtends, m_frameSize);
    long hdExtSize = ExtentList::computeExtentSize(m_headerFileSize, m_numExtends, m_frameHeaderSize);

    TRACE_PRINTF(jsFileWriterLog,"Init TraceFile extents");  
    m_TrFileExtents->Init(JS_TRACE_DATA, m_numExtends, m_traceFileSize, trExtSize, vFolders);

    TRACE_PRINTF(jsFileWriterLog,"Init TraceHeader extents");
    m_TrHeadExtents->Init(JS_TRACE_HEADERS, m_numExtends, m_headerFileSize, hdExtSize, vFolders);

    TRACE_PRINTF(jsFileWriterLog,"traceLen=%d, TotalNumOfFrames=%ld, TotalNumOfTraces=%ld", m_compess_traceSize,m_TotalNumOfFrames, m_TotalNumOfTr);
    
    
    m_bisFloat = false;
    m_bSeisPEG_data=false;
    if(m_fileProps->traceFormat.getName()==DataFormat::FLOAT.getName())
    {
      m_bisFloat = true;
    }
    else if(m_fileProps->traceFormat.getName()==DataFormat::SEISPEG.getName())
    {
      m_bSeisPEG_data=true;
    }
    
    
    int trBufferArrayLen=m_frameSize;
    if(m_bSeisPEG_data)
    {
      trBufferArrayLen = m_frameSize + SeisPEG::getOutputHdrBufferSize(m_headerLengthWords, m_numTraces);
    }
    
    m_traceBufferArray = new char [trBufferArrayLen];
    m_headerBufferArray = new char [m_frameHeaderSize];

    m_traceBuffer = new CharBuffer; 
    m_headerBuffer = new CharBuffer;
    m_seispegHeaderBuffer = new IntBuffer;
    
    m_traceBuffer->setByteOrder(m_byteOrder);
    m_traceBuffer->wrap(m_traceBufferArray, m_frameSize); 
    m_headerBuffer->wrap(m_headerBufferArray, m_frameHeaderSize); 
    m_headerBuffer->setByteOrder(m_byteOrder);

    if(m_bSeisPEG_data)
    {
      SeisPEG_Policy policy = (m_seispegPolicy==0) ? SEISPEG_POLICY_FASTEST : SEISPEG_POLICY_MAX_COMPRESSION;
      m_seispegCompressor = new SeisPEG(m_numSamples, m_numTraces, 0.1, policy);
    }
    else
    {
      m_traceCompressor = new TraceCompressor;
      m_traceCompressor->Init(m_fileProps->traceFormat, m_numSamples, m_traceBuffer);
    }


    m_traceProps->setBuffer(m_headerBuffer);

    m_curr_trffd = -1;
    m_curr_trhfd = -1;

    m_pCachedWriterHD = new IOCachedWriter(-1, m_IOBufferSize);
    m_pCachedWriterTR = new IOCachedWriter(-1, m_IOBufferSize);

//**********

    m_bInit=true;
    return JS_OK;
  }

  int jsFileWriter::writeMetaData()
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog,"Properties must be initialized first");
      return JS_USERERROR;
    }

//*****  create folders and remove contet if already exist
    VirtualFolders vFolders;
    for(int i=0;i<m_virtualFolders.size();i++)
    {
      vFolders.addFolder(m_virtualFolders[i]);
    }
    vFolders.createFolders();
    vFolders.removeFoldersContents();

//*****  write xml files
    int ires;

    ires = writeSingleProperty(m_filename, JS_FILE_STUB,"DescriptiveName",m_description);
    if(ires !=JS_OK) return ires;

    ires = writeSingleProperty(m_filename, JS_HAS_TRACES_FILE,"HasTraces","true");
    if(ires !=JS_OK) return ires;


    std::string fPropsXMLstr;
    m_fileProps->save(fPropsXMLstr);

    std::string tPropsXMLstr;
    m_traceProps->save(tPropsXMLstr);

    std::string cPropsXMLstr="";
    m_customProps->save(cPropsXMLstr);


    std::string XMLstr =  "<parset name=\"JavaSeis Metadata\">\n" + 
        fPropsXMLstr +  
        tPropsXMLstr +
        cPropsXMLstr +
        "</parset>" ;

    std::string filePropsFileXML = m_filename + JS_FILE_PROPERTIES_XML;

    FILE *pfile = fopen (filePropsFileXML.c_str(),"w");
    if ( pfile == NULL ){ 
      ERROR_PRINTF(jsFileWriterLog,"Can't open file %s.\n",filePropsFileXML.c_str());
      return JS_USERERROR;
    }
    fprintf(pfile,"%s\n",XMLstr.c_str());
    fclose(pfile);
  

//***** write TraceFile.xml TraceHeaders.xml and VirtualFoldrs.xml
    ires = m_TrFileExtents->getVirtualFolders().save(m_filename); 
    if(ires!=JS_OK) return ires;
    ires = m_TrFileExtents->saveXML(m_filename);
    if(ires!=JS_OK) return ires;
    ires = m_TrHeadExtents->saveXML(m_filename);
    if(ires!=JS_OK) return ires;

//*** init TraceMap if mapped
    if(m_fileProps->isMapped)
    {
      m_trMap = new TraceMap;
      long * trMap_axes = new long[m_numDim];
      for(int i=0;i<m_numDim;i++)
        trMap_axes[i]=m_gridDef->getAxis(i).getLength();
      m_trMap->Init(trMap_axes, m_numDim, m_byteOrder,  m_filename, "w");
      m_trMap->intializeTraceMapOnDisk();
      delete[]trMap_axes;
    }

    return JS_OK;
  }


  int jsFileWriter::writeSingleProperty(std::string datasetPath, std::string  fileName,
                                        std::string  propertyName, std::string  propertyValue)
  {
    time_t rawtime;
    time ( &rawtime );
    std::string  comments = "# www.javaseis.org - JavaSeis File Properties " + JS_VERSION + 
        "\n# " + ctime (&rawtime);

    // if( opendir(datasetPath.c_str()) == NULL)
      mkdir(datasetPath.c_str(),0777);

    if(datasetPath[datasetPath.length()-1]!='/') datasetPath.append(1,'/');

    std::string fpath =  datasetPath + fileName;
    FILE *pfile = fopen (fpath.c_str(),"w");
    if ( pfile == NULL ){ 
      ERROR_PRINTF(jsFileWriterLog,"Can't open file %s.\n",fpath.c_str());
      return JS_USERERROR;
    }

    fprintf(pfile,"%s",comments.c_str());
    fprintf(pfile,"%s=%s\n",propertyName.c_str(), propertyValue.c_str());
    fclose(pfile);
    return JS_OK;
  
  }

//retrun global offset in TraceFile(s) or TraceHeader(s) corresponding to indices (array indices directly)
//for TraceFile(s) len1d must be equal to m_traceSize, ie.len1d=m_compess_traceLen, and for 
//TraceHeader(s) len1d=m_headerLengthBytes
  long jsFileWriter::getOffsetInExtents(int* indices, int len1d)
  {
    int ind_len= m_fileProps->numDimensions-1;
    unsigned long glb_offset = 0;
    long volsize=1;
    for(int i=0;i<ind_len;i++){
      if(indices[i]<0 || indices[i]>=m_fileProps->axisLengths[ind_len-i]){
        ERROR_PRINTF(jsFileWriterLog,"index %d must be positive and smaller than %ld", indices[i],m_fileProps->axisLengths[i]);
        return JS_USERERROR;
      } 
      volsize=len1d;
      for(int j=ind_len-i-1;j>=1;j--){
        volsize *= m_fileProps->axisLengths[j];
      }
      glb_offset += indices[i]*volsize;
    }
    return glb_offset;
  }

  int jsFileWriter::writeHeaderBuffer(long offset, const char *buf, long buflen)
  {
    int lowInd = m_TrHeadExtents->getExtentIndex(offset+1);
    int upInd  = m_TrHeadExtents->getExtentIndex(offset+buflen);

    if(lowInd<0 || upInd<0 || upInd<lowInd){
      ERROR_PRINTF(jsFileWriterLog, "Can't write %ld bytes starting from offset %ld in TraceHeader(s)", buflen, offset);
      return JS_USERERROR;
    }

    long rest_buflen = buflen;
    long bytes2write = buflen;
    long loc_offset_trHeader = offset - (*m_TrHeadExtents)[lowInd].getStartOffset();
    for(int extInd=lowInd; extInd<=upInd;extInd++){
      long extSize = (*m_TrHeadExtents)[extInd].getExtentSize();
      if(loc_offset_trHeader + rest_buflen >extSize){
        bytes2write = extSize - loc_offset_trHeader;
      }

      if(m_currIndexOfTrHeadExtent!=extInd){
        m_pCachedWriterHD->flush();
          ::close(m_curr_trhfd);
          std::string fname = (*m_TrHeadExtents)[extInd].getPath();
          m_curr_trhfd = :: open(fname.c_str(), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
          if( m_curr_trhfd < 0 ){
            return JS_WARNING;
          }
          m_currIndexOfTrHeadExtent=extInd;
          m_pCachedWriterHD->setNewFileDescriptor(m_curr_trhfd);
      }

//     long bytesWritten = ::pwrite (m_curr_trhfd, &buf[buflen-rest_buflen], bytes2write, loc_offset_trHeader);
//      printf("extInd=%d, m_curr_trffd=%d, rest_buflen=%ld, loc_offset_trFile=%lu, bytesWritten=%ld, bytes2write=%ld\n",extInd,m_curr_trhfd,rest_buflen,loc_offset_trHeader, bytesWritten, bytes2write);
//     if(  bytesWritten != bytes2write){
      bool ret = m_pCachedWriterHD->write(loc_offset_trHeader, (unsigned char*) &buf[buflen-rest_buflen], bytes2write);
      if(!ret){
        m_currIndexOfTrHeadExtent=-1;
          ::close(m_curr_trhfd);
          m_curr_trhfd=-1;
          return JS_WARNING;
      }

      rest_buflen -= bytes2write;
      loc_offset_trHeader=0;
    }

    return JS_OK;
  }



  int jsFileWriter::writeTraceBuffer(long offset, const char *buf, long buflen)
  {
    int lowInd = m_TrFileExtents->getExtentIndex(offset+1);
    int upInd  = m_TrFileExtents->getExtentIndex(offset+buflen);

    if(lowInd<0 || upInd<0 || upInd<lowInd){
      ERROR_PRINTF(jsFileWriterLog, "Can't write %ld bytes starting from offset %ld in TraceFile(s)", buflen, offset);
      return JS_USERERROR;
    }

    long rest_buflen = buflen;
    long bytes2write = buflen;
    long loc_offset_trFile = offset - (*m_TrFileExtents)[lowInd].getStartOffset();
    for(int extInd=lowInd; extInd<=upInd;extInd++){
      long extSize = (*m_TrFileExtents)[extInd].getExtentSize();
      if(loc_offset_trFile + rest_buflen >extSize){
        bytes2write = extSize - loc_offset_trFile;
      }

      if(m_currIndexOfTrFileExtent!=extInd){
        m_pCachedWriterTR->flush();
          ::close(m_curr_trffd);
          std::string fname = (*m_TrFileExtents)[extInd].getPath();
          m_curr_trffd = :: open(fname.c_str(), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
          if( m_curr_trffd < 0 ){
            return JS_WARNING;
          }
          m_currIndexOfTrFileExtent=extInd;
          m_pCachedWriterTR->setNewFileDescriptor(m_curr_trffd);
      }
     
//       long bytesWritten = ::pwrite (m_curr_trffd, &buf[buflen-rest_buflen], bytes2write, loc_offset_trFile);
//      printf("extInd=%d, m_curr_trffd=%d, rest_buflen=%ld, loc_offset_trFile=%lu, bytesWritten=%ld, bytes2write=%ld\n",extInd,m_curr_trffd,rest_buflen,loc_offset_trFile, bytesWritten, bytes2write);
//       if(  bytesWritten != bytes2write){
      bool ret = m_pCachedWriterTR->write(loc_offset_trFile, (unsigned char*) &buf[buflen-rest_buflen], bytes2write);
      if(!ret){
        m_currIndexOfTrFileExtent=-1;
        ::close(m_curr_trffd);
        m_curr_trffd=-1;
        return JS_WARNING;
      }

      rest_buflen -= bytes2write;
      loc_offset_trFile=0;
    }

    return JS_OK;
  }

  int jsFileWriter::writeFrames(int* position, float *frames, int nFrames)
  {
//     position[m_fileProps->numDimensions-2]=0;
    long frameIndex = getFrameIndex(position);
    return writeFrames(frameIndex, frames, nFrames);
  }


  int jsFileWriter::writeFrames(int frameIndex, float *frames, int nFrames)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog,"Properties must be initialized first");
      return JS_USERERROR;
    }
    if(!m_bisFloat)
    {
      ERROR_PRINTF(jsFileWriterLog,"This function can be used only for FLOAT data");
      return JS_USERERROR;
    }
    if(frameIndex<0 || frameIndex+nFrames>=m_TotalNumOfFrames){
      ERROR_PRINTF(jsFileWriterLog,"Invalid frame index. [%d,%d] must be in [0,%ld)", frameIndex, frameIndex+nFrames, m_TotalNumOfFrames);
      return JS_USERERROR;
    }
    long offset = frameIndex * m_frameSize;

    int ires = writeTraceBuffer(offset, (char*)frames, m_frameSize*nFrames);
    if(ires!=JS_OK){
      ERROR_PRINTF(jsFileWriterLog,"write error");
      return ires;
    }
    
  //in case of regular data TraceMap may be written at once with WriteTraceMap4RegularData function
  //(should be faster than with m_trMap->putFold)
    if(!m_bTraceMapWritten) 
    {
      for(int i=0;i<nFrames;i++)
      {
        int ires = m_trMap->putFold(frameIndex+i, m_numTraces);
        if(  ires != JS_OK ){
          ERROR_PRINTF(jsFileWriterLog, "Can't write TraceMap file");
          return ires;
        }
      }
    }
    
    return JS_OK;
  }
  

  int jsFileWriter::writeTrace(long traceIndex, const float *trace,  const char *headbuf)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog,"Properties must be initialized first");
      return JS_USERERROR;
    }
    if(!m_bisFloat)
    {
      ERROR_PRINTF(jsFileWriterLog,"This function can be used only for FLOAT data");
      return JS_USERERROR;
    }
    if(traceIndex<0 || traceIndex>=m_TotalNumOfTr){
      ERROR_PRINTF(jsFileWriterLog,"Invalid trace index. %ld must be in [0,%ld)", traceIndex, m_TotalNumOfTr);
      return JS_USERERROR;
    }
 
    long offset = traceIndex * m_traceSize;

    int ires = writeTraceBuffer(offset, (char*)trace, m_traceSize);
    if(ires!=JS_OK){
      ERROR_PRINTF(jsFileWriterLog,"Can't write trace into the file");
      return ires;
    }
    
    if(headbuf!=NULL)//write trace header
    {
      long glb_hd_offset = traceIndex * m_headerLengthBytes;
      ires = writeHeaderBuffer(glb_hd_offset, headbuf, m_headerLengthBytes);
      if(  ires != JS_OK ){
        ERROR_PRINTF(jsFileWriterLog, "Can't write trace header into the file");
        return ires;
      }
   }
    
    return JS_OK;
  }





  long jsFileWriter::getFrameIndex(const int* position)
  {
    int numAxis= m_fileProps->numDimensions-1;
    int pos[3]={0,0,0}; //max dim to determine frameIndex (since max dim=5)
    for(int i=0;i<numAxis-1;i++) {
      pos[i]=(int)((position[i]-m_fileProps->logicalOrigins[numAxis-i])/m_fileProps->logicalDeltas[ numAxis-i]);
      if(pos[i]<0 || pos[i]>m_fileProps->axisLengths[numAxis-i])
      {
        ERROR_PRINTF(jsFileWriterLog, "Unable to locate a frame with value %d in dimension %d", position[i],i);
        return -1;
      }  
    }

    long frIndex = 0;
    for(int i=0;i<numAxis;i++){
      long volsize=1;
      for(int j=2;j<numAxis-i;j++){
        volsize *= m_fileProps->axisLengths[j];
      }
      frIndex += pos[i]*volsize;
    } 
//   printf("for pos(%d,%d,%d), frame index = %d\n",position[0],position[1],position[2],frIndex);
    return frIndex;

  }

  long jsFileWriter::getNtr()
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog,"Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_TotalNumOfTr;
  }

  long jsFileWriter::getNFrames()
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog,"Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_TotalNumOfFrames;
  }

  int jsFileWriter::writeFrame(const int* position, const float *frame, const char *headbuf, int numLiveTraces)
  {
//     position[m_fileProps->numDimensions-2]=0;
    long frameIndex = getFrameIndex(position);
    return writeFrame(frameIndex, frame, headbuf, numLiveTraces);
  }

// write frame trace data and header 
// (headbuf must be initalized with m_traceProps->getTraceHeader function)
  int jsFileWriter::writeFrame(long frameIndex, const float *frame, const char *headbuf, int numLiveTraces)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(frameIndex<0 || frameIndex>=m_TotalNumOfFrames){
      ERROR_PRINTF(jsFileWriterLog, "Invalid frame index. %ld must be in [0,%ld)\n", frameIndex, m_TotalNumOfFrames);
      return JS_USERERROR;
    }

    bool bWriteTraceMap=true;
    if(numLiveTraces<0){
      numLiveTraces = m_numTraces; //default value
      bWriteTraceMap=false;
    } 
  
    long glb_offset = frameIndex * m_frameSize;
    long bytesInFrame =  numLiveTraces * m_compess_traceSize;

  //write frame data
    if(m_bisFloat){//if float, there is no need to uncompress, we can write directly  (should be faster)
//     if(nativeOrder()!=m_byteOrder){ 
//         endian_swap((void*)frame, bytesInFrame, sizeof(float));
//     }    
    //write only with native order
      int ires = writeTraceBuffer(glb_offset, (char*)frame, bytesInFrame);
      if(  ires != JS_OK ){
        ERROR_PRINTF(jsFileWriterLog, "Can't write frame into the file");
        return ires;
      }   
    }else{
    //if dataFormat is not FLOAT - uncompress 
      if(m_bSeisPEG_data)
      {
        if(headbuf!=NULL)
        {
          m_seispegHeaderBuffer->wrap((int*)headbuf, m_headerLengthWords*numLiveTraces);
          bytesInFrame = m_seispegCompressor->compress((float*)frame, numLiveTraces, m_seispegHeaderBuffer, m_headerLengthWords, m_traceBufferArray);
          m_seispegCompressor->updateStatistics(numLiveTraces, m_numSamples, m_headerLengthWords, bytesInFrame);
        }
        else
        {
          bytesInFrame = m_seispegCompressor->compress((float*)frame, numLiveTraces, m_traceBufferArray);
          m_seispegCompressor->updateStatistics(numLiveTraces, m_numSamples, 0, bytesInFrame);
        }
      }
      else
      {  
        m_traceBuffer->position(0); 
        m_traceCompressor->packFrame(numLiveTraces, frame);
      }  

      int ires = writeTraceBuffer(glb_offset, &m_traceBufferArray[0], bytesInFrame);
      if(  ires != JS_OK ){
        ERROR_PRINTF(jsFileWriterLog, "Can't write frame into the file");
        return ires;
      }

    }

    if(headbuf!=NULL && !m_bSeisPEG_data)//write frame header. In case of SeisPEG header is written with frame data
    {
    //    (in SeisPEG case this should be done with the writing of data)
      long glb_hd_offset = frameIndex * m_frameHeaderSize;
      long bytesInHdFrame =  numLiveTraces * m_headerLengthBytes;
      int ires = writeHeaderBuffer(glb_hd_offset, headbuf, bytesInHdFrame);
      if(  ires != JS_OK ){
        ERROR_PRINTF(jsFileWriterLog, "Can't write frame header into the file");
        return ires;
      }
    }

  //in case of regular data TraceMap may be written at once with WriteTraceMap4RegularData function
  //(should be faster than with m_trMap->putFold)
    if(bWriteTraceMap==true) 
    {
      int ires = m_trMap->putFold(frameIndex, numLiveTraces);
      if(  ires != JS_OK ){
        ERROR_PRINTF(jsFileWriterLog, "Can't write TraceMap file");
        return ires;
      }
    }

    return numLiveTraces;
  }

  int jsFileWriter::writeFrameHeader(long frameIndex, char *headbuf)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(frameIndex<0 || frameIndex>=m_TotalNumOfFrames){
      ERROR_PRINTF(jsFileWriterLog, "Invalid frame index. %ld must be in [0,%ld)", frameIndex, m_TotalNumOfFrames);
      return JS_USERERROR;
    }
    long glb_hd_offset = frameIndex * m_frameHeaderSize;
    int ires = writeHeaderBuffer(glb_hd_offset, headbuf, m_frameHeaderSize);
    if(  ires != JS_OK ){
      ERROR_PRINTF(jsFileWriterLog, "Can't write frame header into the file");
      return ires;
    }
    return JS_OK;
  }

  int jsFileWriter::writeFrameHeader(int* position,char *headbuf)
  {
//     position[m_fileProps->numDimensions-2]=0;
    long frameIndex = getFrameIndex(position);
    return writeFrameHeader(frameIndex, headbuf);
  }

  int jsFileWriter::writeTraceMap4RegularData()
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }

    if(m_fileProps->isMapped){
//     TraceMap trMap(long* _axisLengths,  int _numAxis, ByteOrder _byteOrder, std::string path, std::string mode);
      std::string traceMapfile =  m_filename + JS_TRACE_MAP;
      FILE *pfile = fopen (traceMapfile.c_str(),"w");
      if ( pfile == NULL ){ 
        ERROR_PRINTF(jsFileWriterLog,"Can't open file %s.\n",traceMapfile.c_str());
        return JS_USERERROR;
      }

      int *ibuf = new int[m_TotalNumOfFrames];
      for(int i=0;i<m_TotalNumOfFrames;i++)
        ibuf[i]=m_numTraces;
 
      fwrite ((char*)ibuf, sizeof(int), m_TotalNumOfFrames, pfile);
      delete[]ibuf;

      fclose(pfile);
    }
    m_bTraceMapWritten = true;
    return JS_OK;
  }



// leftJustify : left justify input frame and header buffer (headbuf) 
//   modify input frame and headbuf and 
//  return number of live traces in input frame
// Parameters:
//   frame - traces to left justify  
//   headbuf - corresponding header data (must be initialized with TraceProper::getTraceHeader function) 
//   numTraces - number of traces in frame and headbuf
  int jsFileWriter::leftJustify(float *frame, char *headbuf, int numTraces)
  {
    std::string sLiveTraceFlag = "TRC_TYPE";

    if(m_traceProps->exists(sLiveTraceFlag)!=true){
      ERROR_PRINTF(jsFileWriterLog, "There is no header TRC_TYPE (indicating live or dead trace) in the dataset");
      return JS_USERERROR;
    }
    int keyoff = m_traceProps-> getKeyOffset(sLiveTraceFlag);

    char *tmpHrdBuf = new char[numTraces*m_headerLengthBytes];
    float *tmpFrameBuf = new float[numTraces*m_numSamples];
  
    memcpy(tmpHrdBuf, headbuf, numTraces*m_headerLengthBytes);
    memcpy(tmpFrameBuf, frame, numTraces*m_numSamples*sizeof(float));

    int i=0;
    int ifull=0;
    while(i<numTraces){
      int trc_type = *(reinterpret_cast<int*>( &headbuf[i*m_headerLengthBytes + keyoff]));
//       printf("---   i=%d, trc_type=%d, ifull=%d\n", i, trc_type, ifull);
      int j=i;
      if(trc_type==0){
        do{
          j++;
          trc_type = *(reinterpret_cast<int*>( &headbuf[j*m_headerLengthBytes + keyoff]));
        }while(trc_type==0 && j<m_numTraces);
        memcpy(&tmpFrameBuf[ifull*m_numSamples], &frame[j*m_numSamples], (numTraces-j)*m_numSamples*sizeof(float));
        memcpy(&tmpHrdBuf[ifull*m_headerLengthBytes], &headbuf[j*m_headerLengthBytes], (numTraces-j)*m_headerLengthBytes);
      }
      else{
	ifull++;
      }
      i++;
    }

    memcpy(headbuf, tmpHrdBuf, numTraces*m_headerLengthBytes);
    memcpy(frame, tmpFrameBuf, numTraces*m_numSamples*sizeof(float));

    delete [] tmpHrdBuf;
    delete [] tmpFrameBuf;

    return ifull;
  }


  float* jsFileWriter::allocFrameBuf()
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog,"Properties must be initialized first");
      return NULL;
    }
    return new float[m_numSamples*m_numTraces];
  }

  char* jsFileWriter::allocHdrBuf(bool initVals)
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog,"Properties must be initialized first");
      return NULL;
    }
    char *hdrBuf = new char[m_headerLengthBytes*m_numTraces];
    memset(hdrBuf, 0, m_headerLengthBytes*m_numTraces);

    if(initVals) // init with SeisSpace standard values
    {
      float TFULL_E = m_gridDef->getAxisPhysicalOrigin(0) + m_gridDef->getAxisLength(0)*m_gridDef->getAxisPhysicalDelta(0);
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

  catalogedHdrEntry jsFileWriter::getHdrEntry(std::string _name) const
  {
    return m_traceProps->getHdrEntry(_name);
  }

  std::vector<catalogedHdrEntry> jsFileWriter::getHdrEntries() const
  {
    return m_traceProps->getHdrEntries();
  }

  catalogedHdrEntry jsFileWriter::getAxisHdrEntry(int _axisInd) const
  {
    if(_axisInd>=0 && _axisInd<m_numDim)
    {
      return getHdrEntry(m_gridDef->getAxisPtr(_axisInd)->getHeaderName());
    }
    else
    {
      ERROR_PRINTF(jsFileWriterLog, "Invalid axis index %d. Must be between 0 and %d", _axisInd, m_numDim);
      return catalogedHdrEntry();
    }
  }
  
  catalogedHdrEntry jsFileWriter::getAxisBinHdrEntry(int _axisInd) const
  {
    if(_axisInd>=0 && _axisInd<m_numDim)
    {
      return getHdrEntry(m_gridDef->getAxisPtr(_axisInd)->getHeaderBinName());
    }
    else
    {
      ERROR_PRINTF(jsFileWriterLog, "Invalid axis index %d. Must be between 0 and %d", _axisInd, m_numDim);
      return catalogedHdrEntry();
    }
  }

  int jsFileWriter::getNDim() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_fileProps->numDimensions;
  }

  int jsFileWriter::getAxisLen(int index) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(index>=0 && index <getNDim()) return m_fileProps->axisLengths[index];
    return JS_USERERROR;
  }

 int jsFileWriter::getAxisLogicalValues(int index, std::vector<long> &axis) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    int NDim = getNDim();
    if(index<0 || index>=NDim){
      ERROR_PRINTF(jsFileWriterLog, "Axis index %d must be smaller than %d\n", index, NDim);
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

  int jsFileWriter::getAxisPhysicalValues(int index, std::vector<double> &axis) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    int NDim = getNDim();
    if(index<0 || index>=NDim){
      ERROR_PRINTF(jsFileWriterLog, "Axis index %d must be smaller than %d\n", index, NDim);
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

  int jsFileWriter::getAxisLabels(std::vector<std::string> &axis) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    axis.clear();
    for(int i=0;i<m_fileProps->numDimensions;i++)
      axis.push_back(m_fileProps->axisLabelsStr[i]);

    return m_fileProps->numDimensions;
  }

  int jsFileWriter::getAxisUnits(std::vector<std::string> &units) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    } 
    units.clear();
    for(int i=0;i<m_fileProps->numDimensions;i++)
      units.push_back(m_fileProps->axisUnitsStr[i]);

    return m_fileProps->numDimensions;
  }

  std::string jsFileWriter::getVersion() const
  {
    return m_fileProps->version;
  }

  int jsFileWriter::getNumOfExtents() const
  {
    return m_TrFileExtents->getNumExtents();
  }

  int jsFileWriter::getNumOfVirtualFolders() const
  {
    return m_TrFileExtents->getNumvFolders();
  }

  std::string jsFileWriter::getByteOrderAsString() const
  {
    if(m_byteOrder==JSIO_LITTLEENDIAN){
      return "LittleEndian";
    }
    return "BigEndian";
  }
  
  std::string jsFileWriter::getTraceFormatName() const
  {
    return m_fileProps->traceFormat.getName();
  }

  std::string jsFileWriter::getDataType() const
  {
    return m_fileProps->dataType.getName();
  }

  int jsFileWriter::getNumBytesInRawFrame() const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    return m_frameSize;
  }

}


