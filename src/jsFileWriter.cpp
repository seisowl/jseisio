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
#include "DataType.h"
#include "DataFormat.h"
#include "jseisUtil.h"

// #include "IOCachedWriter.h"

#include "PSProLogging.h"
#include "compress/TraceCompressor.h"
#include "compress/SeisPEG.h"

namespace jsIO
{
  DECLARE_LOGGER(jsFileWriterLog);

  jsFileWriter::~jsFileWriter()
  {
    Close();
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

    m_trMap = NULL;
    m_jsReader = NULL;
    
//    m_pCachedWriterHD = NULL;
//    m_pCachedWriterTR = NULL;
//    m_curr_trffd = -1;
//    m_curr_trhfd = -1;
//    m_currIndexOfTrFileExtent = -1;
//    m_currIndexOfTrHeadExtent = -1;
  
    m_bTraceMapWritten=false;

//   plogfile = fopen ("/m/scratch/supertmp/abel/jsWriterLog.txt","w");
  }

  void jsFileWriter::Close()
  {
    m_bInit=false;
    if(m_gridDef!=NULL) delete m_gridDef;
    //    if(m_dataDef!=NULL) delete m_dataDef;

    if(m_traceProps!=NULL) delete m_traceProps;
    if(m_fileProps!=NULL) delete m_fileProps;
    if(m_customProps!=NULL) delete m_customProps;
    if(m_TrFileExtents!=NULL) delete m_TrFileExtents;
    if(m_TrHeadExtents!=NULL) delete m_TrHeadExtents;

    if(m_trMap!=NULL) delete m_trMap;

    //    if(m_pCachedWriterHD!=NULL) delete m_pCachedWriterHD;
    //    if(m_pCachedWriterTR!=NULL) delete m_pCachedWriterTR;
  }

  // setup names, default values which can be over written later
  int jsFileWriter::setFileName(const std::string _filename) {
	  m_filename = _filename;
	  if(m_filename[m_filename.length()-1]!='/') m_filename.append(1,'/');
	  std::string descname;
	  std::string fname = jseisUtil::fullname(_filename.c_str(), descname);
	  m_description = descname;
	  m_fileProps->isMapped = true;
	  m_IOBufferSize = (4*1024*1024); //4MB
	  m_numExtends = 3;
	  m_seispegPolicy = 0;
	  initDataType("CUSTOM", "FLOAT", true, 4);
	  m_traceProps->addDefaultProperties();
  }

  // set the description name, setFileName() handles description properly. sometimes useful
  int jsFileWriter::setFileDescription(const std::string _descname) {
	  m_description = _descname;
  }

  int jsFileWriter::initDataType(const std::string dataType, std::string dataFormat, bool isMapped, int nextends, std::string vpath) {
	  m_fileProps->dataType =  DataType::get(dataType);
	  m_fileProps->traceFormat = DataFormat::get(dataFormat);
	  m_fileProps->isMapped = isMapped;
	  m_numExtends = nextends;
	  if (!vpath.empty()) {
		  VirtualFolders vFolders;
		  std::string vFolderPathRest = vFolders.getPathRest(m_filename);
		  if (vFolderPathRest[vFolderPathRest.length() - 1] != '/') vFolderPathRest.append(1, '/');
		  if (vFolderPathRest[0] == '/' && vpath[vpath.length() - 1] == '/')
			  vFolderPathRest=vFolderPathRest.substr(1,vFolderPathRest.length());
		  m_virtualFolders.push_back(vpath + vFolderPathRest);
		  TRACE_PRINTF(jsWriterInputLog, "virtualFolder : %s", (vpath + vFolderPathRest).c_str());
	  }
  }

  void jsFileWriter::initGridDim(int numDim){
	  AxisDefinition *axes = new AxisDefinition[numDim];
	  m_gridDef->Init(numDim, axes);
	  m_numDim = numDim;
	  delete[]axes;
  }

  int jsFileWriter::initGridAxis(int axisInd, std::string axislabel,  std::string units,  std::string domain,
                   long length, long logicalOrigin, long logicalDelta,
                   double physicalOrigin, double physicalDelta) {
	  if(axisInd>=0 && axisInd<m_numDim)
	  {
		m_gridDef->getAxisPtr(axisInd)->Init( AxisLabel(axislabel, axislabel), Units(units),  DataDomain(domain),
				length, logicalOrigin, logicalDelta, physicalOrigin, physicalDelta,  "",  "");
	    return JS_OK;
	  }
	  else
	  {
	    ERROR_PRINTF(jsWriterInputLog, "Invalid axis index %d. Must be between 0 and %d", axisInd, m_numDim);
	    return JS_USERERROR;
	  }
  }

  int jsFileWriter::addProperty(std::string _label, std::string _description, std::string _format, int _count)
  {
     return m_traceProps->addProperty(_label, _description, _format, _count);
  }

  void jsFileWriter::addSurveyGeom(int minILine, int maxILine, int minXLine, int maxXLine,
		  float xILine1End, float yILine1End, float xILine1Start, float yILine1Start, float xXLine1End, float yXLine1End)
  {
     m_customProps->survGeom.setGeom(minILine, maxILine, minXLine, maxXLine, xILine1End, yILine1End, xILine1Start, xILine1Start, xXLine1End, yXLine1End);
  }

  void jsFileWriter::addCustomProperty(std::string name, std::string type, std::string value)
  {
	  m_customProps->addProperty(name, type, value);
  }

  void jsFileWriter::axisGridToProps(GridDefinition *gridDef) {
	m_numDim = gridDef->getNumDimensions();
    m_fileProps->Init(m_numDim);
	for (int i = 0; i < m_numDim; i++) {
		m_fileProps->axisLabels[i] = gridDef->getAxisLabel(i);
		m_fileProps->axisLabelsStr[i] = gridDef->getAxisLabelString(i);
		m_fileProps->axisUnits[i] = gridDef->getAxisUnits(i);
		m_fileProps->axisUnitsStr[i] = gridDef->getAxisUnitsString(i);
		m_fileProps->axisDomains[i] = gridDef->getAxisDomain(i);
		m_fileProps->axisDomainsStr[i] = gridDef->getAxisDomainString(i);
		m_fileProps->axisLengths[i] = gridDef->getAxisLength(i);
		m_fileProps->logicalOrigins[i] = gridDef->getAxisLogicalOrigin(i);
		m_fileProps->logicalDeltas[i] = gridDef->getAxisLogicalDelta(i);
		m_fileProps->physicalOrigins[i] = gridDef->getAxisPhysicalOrigin(i);
		m_fileProps->physicalDeltas[i] = gridDef->getAxisPhysicalDelta(i);
	}
  }

  int jsFileWriter::updateGridAxis(int axisInd, long length, long logicalOrigin, long logicalDelta,
                   double physicalOrigin, double physicalDelta) {
	  if(axisInd>=0 && axisInd<m_numDim)
	  {
		  m_fileProps->axisLengths[axisInd] = length;
		  m_fileProps->logicalOrigins[axisInd] = logicalOrigin;
		  m_fileProps->logicalDeltas[axisInd] = logicalDelta;
		  m_fileProps->physicalOrigins[axisInd] = physicalOrigin;
		  m_fileProps->physicalDeltas[axisInd] = physicalDelta;
		  return JS_OK;
	  }
	  else
	  {
	    ERROR_PRINTF(jsWriterInputLog, "Invalid axis index %d. Must be between 0 and %d", axisInd, m_numDim);
	    return JS_USERERROR;
	  }
  }

  void copy_file( const char* srce_file, const char* dest_file )
  {
      std::ifstream srce( srce_file, std::ios::binary ) ;
      std::ofstream dest( dest_file, std::ios::binary ) ;
      dest << srce.rdbuf() ;
      srce.close();
      dest.close();
  }

  /**
  * init from  jsWriterInput
  */
  int jsFileWriter::Init(const jsWriterInput *_writerInput)
  {
	*m_gridDef =  *(_writerInput->gridDef);

//***************
    *m_traceProps = *(_writerInput->traceProps);
    *m_customProps = *(_writerInput->customProps);

    m_filename = _writerInput->jsfilename;
    if(m_filename[m_filename.length()-1]!='/') m_filename.append(1,'/');
    m_description = _writerInput->description;
    m_numExtends = _writerInput->NExtends;
    m_virtualFolders = _writerInput->virtualFolders;
    m_seispegPolicy = _writerInput->seispegPolicy;
    m_IOBufferSize = _writerInput->IOBufferSize;
    m_fileProps->dataType = _writerInput->dataDef->getDataType();
    m_fileProps->traceFormat = _writerInput->dataDef->getTraceFormat();
    m_fileProps->isMapped = _writerInput->isMapped;

	axisGridToProps(m_gridDef);

    // Initialize();
  }

  /**
    * init from jsFileReader
    */
    int jsFileWriter::Init( jsFileReader* jsReader)
    {
//       *m_gridDef = *(jsReader->gridDef);
//       *m_fileProps = *(jsReader->m_fileProps);
//
       m_jsReader = jsReader;
       *m_traceProps = *(jsReader->m_traceProps);
 	   *m_customProps = *(jsReader->m_customProps);
       m_numExtends = jsReader->getNumOfExtents();
       // m_virtualFolders.empty(); // set to use primary
       m_virtualFolders = jsReader->vFolders->getNewPaths(m_filename);

       m_seispegPolicy = 0; //jsReader->m_bSeisPEG_data;
       m_IOBufferSize = jsReader->m_IOBufferSize;
       m_fileProps->dataType = DataType::get(jsReader->getDataType());
       m_fileProps->traceFormat = DataFormat::get(jsReader->getTraceFormatName());
       m_fileProps->isMapped = jsReader->m_bIsMapped;

       m_numDim = jsReader->getNDim();
       m_fileProps->Init(m_numDim);
       for (int i = 0; i < m_numDim; i++) {
       		m_fileProps->axisLabels[i] = jsReader->m_fileProps->axisLabels[i];
       		m_fileProps->axisLabelsStr[i] = jsReader->m_fileProps->axisLabelsStr[i];
       		m_fileProps->axisUnits[i] = jsReader->m_fileProps->axisUnits[i];
       		m_fileProps->axisUnitsStr[i] = jsReader->m_fileProps->axisUnitsStr[i];
       		m_fileProps->axisDomains[i] = jsReader->m_fileProps->axisDomains[i];
       		m_fileProps->axisDomainsStr[i] = jsReader->m_fileProps->axisDomainsStr[i];
       		m_fileProps->axisLengths[i] = jsReader->m_fileProps->axisLengths[i];
       		m_fileProps->logicalOrigins[i] = jsReader->m_fileProps->logicalOrigins[i];
       		m_fileProps->logicalDeltas[i] = jsReader->m_fileProps->logicalDeltas[i];
       		m_fileProps->physicalOrigins[i] = jsReader->m_fileProps->physicalOrigins[i];
       		m_fileProps->physicalDeltas[i] = jsReader->m_fileProps->physicalDeltas[i];
       	}

       // Initialize();

    }

  // this for private, and called by
  int jsFileWriter::Initialize()
  {
    if (m_jsReader == NULL) axisGridToProps(m_gridDef);

	TRACE_PRINTF(jsFileWriterLog,"Init File Properties");
    if(m_filename[m_filename.length()-1]!='/') m_filename.append(1,'/');

    if(m_virtualFolders.size()==0) //not defined
    {
      m_virtualFolders.push_back(m_filename);
    }

    m_fileProps->comments="www.javaseis.org - JavaSeis File Properties "+JS_VERSION;
    m_fileProps->version="2006.3";
    m_fileProps->byteOrder = nativeOrder();
    m_byteOrder = m_fileProps->byteOrder ;

//************
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

  //  TRACE_PRINTF(jsFileWriterLog,"Init Virtual Folders");

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
    
    
    m_trBufferArrayLen=m_frameSize;
    if(m_bSeisPEG_data)
    {
      m_trBufferArrayLen = m_frameSize + SeisPEG::getOutputHdrBufferSize(m_headerLengthWords, m_numTraces);
    }
    
//    m_curr_trffd = -1;
//    m_curr_trhfd = -1;
//
//    m_pCachedWriterHD = new IOCachedWriter(-1, m_IOBufferSize);
//    m_pCachedWriterTR = new IOCachedWriter(-1, m_IOBufferSize);

//**********

    if(m_fileProps->isMapped && m_jsReader != NULL && m_filename.compare(m_jsReader->m_filename) == 0)
    {
       if(m_trMap!=NULL) delete m_trMap;
       m_trMap = new TraceMap;
       long * trMap_axes = new long[m_numDim];
       for(int i=0;i<m_numDim;i++)
          trMap_axes[i]=m_fileProps->axisLengths[i];
       m_trMap->Init(trMap_axes, m_numDim, m_byteOrder,  m_filename, "rw");
       delete[]trMap_axes;
    }

    m_bInit=true;
    return JS_OK;
  }

  // remove flag: create(1)/copy(2); default 1;
  int jsFileWriter::writeMetaData(const int remove)
  {
	// call actural init based on the setup parameters
	if (!m_bInit) Initialize();

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
    if (remove==1) vFolders.removeFoldersContents();

//*****  write xml files
    int ires;

    ires = vFolders.mkdirp(m_filename.c_str(), 0777);
    if (ires != JS_OK) {
          TRACE_PRINTF(jsFileWriterLog, "Can't create direcory %s", m_filename.c_str());
          return ires;
    }

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
      if(m_trMap!=NULL) delete m_trMap;
      m_trMap = new TraceMap;
      long * trMap_axes = new long[m_numDim];
      for(int i=0;i<m_numDim;i++)
        trMap_axes[i]=m_fileProps->axisLengths[i];
      m_trMap->Init(trMap_axes, m_numDim, m_byteOrder,  m_filename, "w");
      if (m_jsReader != NULL && remove==2) {
    	  // copy foldmap file;
    	  copy_file((m_jsReader->m_filename + JS_TRACE_MAP).c_str(), (m_filename + JS_TRACE_MAP).c_str());
      }
      else if (remove==1) m_trMap->intializeTraceMapOnDisk();

      delete[]trMap_axes;
    }

    //init the trace and header files for later parallel thread write only
    for (int i =0; i < m_TrFileExtents->getNumExtents(); i++) {
    	std::string fname = (*m_TrFileExtents)[i].getPath();
    	if (m_jsReader != NULL && remove==2) {
    	   // copy from m_jsReader
    	   copy_file((*(m_jsReader->m_TrFileExtents))[i].getPath().c_str(), fname.c_str());
    	}
        else {
    	  int fd = :: open(fname.c_str(), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		  if( fd < 0 ) return JS_WARNING;
		  ::close(fd);
    	}
    }
    for (int i =0; i < m_TrHeadExtents->getNumExtents(); i++) {
    	std::string fname = (*m_TrHeadExtents)[i].getPath();
    	if (m_jsReader != NULL && remove==2 ) {
    		// copy from m_jsReader
    	  copy_file((*(m_jsReader->m_TrHeadExtents))[i].getPath().c_str(), fname.c_str());
    	}
    	else {
    	  int fd = :: open(fname.c_str(), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		  if( fd < 0 ) return JS_WARNING;
		  ::close(fd);
    	}
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

    if( opendir(datasetPath.c_str()) == NULL)
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
    int ind_len= m_fileProps->numDimensions;
    unsigned long glb_offset = indices[0];
    long volsize=1;
    for(int i=1;i<ind_len;i++){
      if(indices[i]<0 || indices[i]>=m_fileProps->axisLengths[i]){
        ERROR_PRINTF(jsFileWriterLog,"index %d must be positive and smaller than %ld", indices[i],m_fileProps->axisLengths[i]);
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

  int jsFileWriter::writeHeaderBuffer(long offset, char *buf, long buflen)
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
    // IOCachedWriter* pCachedWriterHD = new IOCachedWriter(-1, m_IOBufferSize);

    for(int extInd=lowInd; extInd<=upInd;extInd++){
      long extSize = (*m_TrHeadExtents)[extInd].getExtentSize();
      if(loc_offset_trHeader + rest_buflen >extSize){
        bytes2write = extSize - loc_offset_trHeader;
      }

      std::string fname = (*m_TrHeadExtents)[extInd].getPath();
      int curr_trhfd = :: open(fname.c_str(), O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
      if( curr_trhfd < 0 ){
            return JS_WARNING;
      }
      // pCachedWriterHD->setNewFileDescriptor(curr_trhfd);

      long bytesWritten = ::pwrite (curr_trhfd, &buf[buflen-rest_buflen], bytes2write, loc_offset_trHeader);
//       printf("extInd=%d, m_curr_trffd=%d, rest_buflen=%ld, loc_offset_trFile=%lu, bytesWritten=%ld, bytes2write=%ld\n",extInd,m_curr_trhfd,rest_buflen,loc_offset_trHeader, bytesWritten, bytes2write);
      ::close(curr_trhfd);
      if(  bytesWritten != bytes2write){
//       bool ret = pCachedWriterHD->write(loc_offset_trHeader, (unsigned char*) &buf[buflen-rest_buflen], bytes2write);
//       if(!ret){
          return JS_WARNING;
      }

      rest_buflen -= bytes2write;
      loc_offset_trHeader=0;
    }

    return JS_OK;
  }



  int jsFileWriter::writeTraceBuffer(long offset, char *buf, long buflen)
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
    // IOCachedWriter* pCachedWriterTR = new IOCachedWriter(-1, m_IOBufferSize);

    for(int extInd=lowInd; extInd<=upInd;extInd++){
      long extSize = (*m_TrFileExtents)[extInd].getExtentSize();
      if(loc_offset_trFile + rest_buflen >extSize){
        bytes2write = extSize - loc_offset_trFile;
      }

      std::string fname = (*m_TrFileExtents)[extInd].getPath();
      int curr_trffd = :: open(fname.c_str(), O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
      if( curr_trffd < 0 ){
         return JS_WARNING;
      }
      // pCachedWriterTR->setNewFileDescriptor(curr_trffd);
     
      long bytesWritten = ::pwrite (curr_trffd, &buf[buflen-rest_buflen], bytes2write, loc_offset_trFile);
//      printf("extInd=%d, m_curr_trffd=%d, rest_buflen=%ld, loc_offset_trFile=%lu, bytesWritten=%ld, bytes2write=%ld\n",extInd,m_curr_trffd,rest_buflen,loc_offset_trFile, bytesWritten, bytes2write);
      ::close(curr_trffd);
      if(  bytesWritten != bytes2write){
//      bool ret = pCachedWriterTR->write(loc_offset_trFile, (unsigned char*) &buf[buflen-rest_buflen], bytes2write);
//      if(!ret){
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


  int jsFileWriter::writeFrames(long frameIndex, float *frames, int nFrames)
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
    if(frameIndex<0 || frameIndex+nFrames-1 >= m_TotalNumOfFrames){
      ERROR_PRINTF(jsFileWriterLog,"Invalid frame index. [%ld,%ld] must be in [0,%ld)", frameIndex, frameIndex+nFrames-1, m_TotalNumOfFrames);
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


  int jsFileWriter::writeTrace(long traceIndex, float *trace,  char *headbuf)
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


 int jsFileWriter::indexToLogical(int* position) const // *input position must be in index, and will convert to logical corrdinates
  {
    int numAxis= m_fileProps->numDimensions;
    for(int i=0;i<numAxis;i++)
    {
      position[i]=(int)(m_fileProps->logicalOrigins[i] + position[i] * m_fileProps->logicalDeltas[i]);
    }
  }

  int jsFileWriter::logicalToIndex(int* position) const // *input position must be in logical corrdinates and will convert to index
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


  long jsFileWriter::getFrameIndex(int* position) // *input position must be in logical corrdinates
  {
    int numAxis= m_fileProps->numDimensions;
    int pos[5]={0,0,0,0,0}; //max dim to determine frameIndex (since max dim=5)
    for(int i=0;i<numAxis;i++) {
      pos[i]=(int)((position[i]-m_fileProps->logicalOrigins[i])/m_fileProps->logicalDeltas[i]);
      if(pos[i]<0 || pos[i]>m_fileProps->axisLengths[i])
      {
        ERROR_PRINTF(jsFileWriterLog, "Unable to locate a frame with value %d in dimension %d", position[i],i);
        return -1;
      }
    }

    long frIndex = pos[2];
    for(int i=3;i<numAxis;i++){
      long volsize=1;
      for(int j=2;j<i;j++){
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

  int jsFileWriter::writeFrame(int* position, float *frame, char *headbuf, int numLiveTraces)
  {
//     position[m_fileProps->numDimensions-2]=0;
    long frameIndex = getFrameIndex(position);
    return writeFrame(frameIndex, frame, headbuf, numLiveTraces);
  }

// write frame trace data and header 
// (headbuf must be initalized with m_traceProps->getTraceHeader function)
  int jsFileWriter::writeFrame(long frameIndex, float *frame, char *headbuf, int numLiveTraces)
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

    if (numLiveTraces > 0) { // we do  need write anything.

    long glb_offset = frameIndex * m_frameSize;
    long bytesInFrame =  numLiveTraces * m_compess_traceSize;

    //////////////ji need change
    // m_traceProps->setBuffer(headerBuffer);

    ///////////////ji

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
      char* traceBufferArray = new char [m_trBufferArrayLen];

      if(m_bSeisPEG_data)
      {

    	SeisPEG_Policy policy = (m_seispegPolicy==0) ? SEISPEG_POLICY_FASTEST : SEISPEG_POLICY_MAX_COMPRESSION;
        SeisPEG* seispegCompressor = new SeisPEG(m_numSamples, m_numTraces, 0.1, policy);
        if(headbuf!=NULL)
        {
		  IntBuffer* seispegHeaderBuffer;
          seispegHeaderBuffer = new IntBuffer;
          seispegHeaderBuffer->wrap((int*)headbuf, m_headerLengthWords*numLiveTraces);
          bytesInFrame = seispegCompressor->compress((float*)frame, numLiveTraces, seispegHeaderBuffer, m_headerLengthWords, traceBufferArray);
          seispegCompressor->updateStatistics(numLiveTraces, m_numSamples, m_headerLengthWords, bytesInFrame);
        }
        else
        {
          bytesInFrame = seispegCompressor->compress((float*)frame, numLiveTraces, traceBufferArray);
          seispegCompressor->updateStatistics(numLiveTraces, m_numSamples, 0, bytesInFrame);
        }
      }
      else
      {  
        TraceCompressor* traceCompressor = new TraceCompressor;
        CharBuffer* traceBuffer = new CharBuffer;
        traceBuffer->setByteOrder(m_byteOrder);
        traceBuffer->wrap(traceBufferArray, m_frameSize);
        traceCompressor->Init(m_fileProps->traceFormat, m_numSamples, traceBuffer);

        traceBuffer->position(0);
        traceCompressor->packFrame(numLiveTraces, frame);
      }  

      int ires = writeTraceBuffer(glb_offset, traceBufferArray, bytesInFrame);
      if(  ires != JS_OK ){
        ERROR_PRINTF(jsFileWriterLog, "Can't write frame into the file");
        return ires;
      }

      delete [] traceBufferArray;
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

    }

  //in case of regular data TraceMap may be written at once with WriteTraceMap4RegularData function
  //(should be faster than with m_trMap->putFold)
  // even if number of live trace is 0, we need update foldmap
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
      if((trc_type==12 || trc_type==0)){
        do{
          j++;
          trc_type = *(reinterpret_cast<int*>( &headbuf[j*m_headerLengthBytes + keyoff]));
        }while((trc_type==12 || trc_type==0) && j<m_numTraces);
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
      return getHdrEntry( (m_fileProps->axisLabels[_axisInd]).getName());
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
    	return getHdrEntry( (m_fileProps->axisLabels[_axisInd]).getName());
     // return getHdrEntry(m_gridDef->getAxisPtr(_axisInd)->getHeaderBinName());
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

  int jsFileWriter::getAxisLogicalOrigin(int index) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(index>=0 && index <getNDim()) return m_fileProps->logicalOrigins[index];
    return JS_USERERROR;
  }

  int jsFileWriter::getAxisLogicalDelta(int index) const
  {
      if(!m_bInit){
        ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
        return JS_USERERROR;
      }
      if(index>=0 && index <getNDim()) return m_fileProps->logicalDeltas[index];
      return JS_USERERROR;
  }

  double jsFileWriter::getAxisPhysicalOrigin(int index) const
  {
    if(!m_bInit){
      ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
      return JS_USERERROR;
    }
    if(index>=0 && index <getNDim()) return m_fileProps->physicalOrigins[index];
    return JS_USERERROR;
  }

  double jsFileWriter::getAxisPhysicalDelta(int index) const
  {
      if(!m_bInit){
        ERROR_PRINTF(jsFileWriterLog, "Properties must be initialized first");
        return JS_USERERROR;
      }
      if(index>=0 && index <getNDim()) return m_fileProps->physicalDeltas[index];
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

