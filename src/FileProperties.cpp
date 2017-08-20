/***************************************************************************
                          FileProperties.cpp -  description
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
  
#include "FileProperties.h"

#include "PSProLogging.h"

namespace jsIO
{
  DECLARE_LOGGER(FilePropertiesLog);
 
  const std::string FileProperties::COMMENTS = "Comments";
  const std::string FileProperties::DATA_DIMENSIONS = "DataDimensions";
  const std::string FileProperties::DATA_TYPE = "DataType";
  const std::string FileProperties::BYTEORDER = "ByteOrder";
  const std::string FileProperties::TRACE_FORMAT = "TraceFormat";
  const std::string FileProperties::MAPPED = "Mapped";
  const std::string FileProperties::PHYSICAL_ORIGINS = "PhysicalOrigins";
  const std::string FileProperties::PHYSICAL_DELTAS = "PhysicalDeltas";
  const std::string FileProperties::AXIS_UNITS = "AxisUnits";
  const std::string FileProperties::AXIS_LABELS = "AxisLabels";
  const std::string FileProperties::AXIS_DOMAINS = "AxisDomains";
  const std::string FileProperties::AXIS_LENGTHS = "AxisLengths";
  const std::string FileProperties::HEADER_LENGTH_BYTES = "HeaderLengthBytes";
  const std::string FileProperties::HEADER_LENGTH = "HeaderLength";
  const std::string FileProperties::LOGICAL_ORIGINS = "LogicalOrigins";
  const std::string FileProperties::LOGICAL_DELTAS = "LogicalDeltas";
  const std::string FileProperties::JAVASEIS_VERSION = "JavaSeisVersion";
  const std::string FileProperties::DESCRIPTIVE_NAME = "DescriptiveName";
  const std::string FileProperties::HAS_TRACES = "HasTraces";

// const std::string FileProperties::SCALCO = "scalco";
// const std::string FileProperties::SCALEL = "scalel";


  FileProperties::~FileProperties(){
    if(physicalDeltas != NULL){
      delete[] axisLabels;
      delete[] axisUnits;
      delete[] axisDomains;
      delete[] axisLabelsStr;
      delete[] axisUnitsStr;
      delete[] axisDomainsStr;
      delete[] axisLengths;
      delete[] logicalOrigins;
      delete[] logicalDeltas;
      delete[] physicalOrigins;
      delete[] physicalDeltas;
    }
  }

  FileProperties::FileProperties(){
    numDimensions=0;
    physicalDeltas = NULL;
//   scalco = 1; //default value
//   scalel = 1; //default value
  }

  void FileProperties::Init(int _numDimensions){
    numDimensions=_numDimensions;
    axisLabels = new AxisLabel[numDimensions];
    axisUnits = new Units[numDimensions];
    axisDomains = new DataDomain[numDimensions];
    axisLabelsStr = new std::string[numDimensions];
    axisUnitsStr = new std::string[numDimensions];
    axisDomainsStr = new std::string[numDimensions];
    axisLengths = new long[numDimensions];
    logicalOrigins =  new long[numDimensions];
    logicalDeltas =  new long[numDimensions];
    physicalOrigins = new double[numDimensions];
    physicalDeltas = new double[numDimensions];
  }

  int FileProperties::load(std::string &XMLstring)
  {
    TRACE_PRINTF(FilePropertiesLog, "loading FileProperties ...");

    xmlreader reader;
    reader.parse(XMLstring);

    xmlElement* parSetFileProps=0;
    parSetFileProps=reader.getBlock("FileProperties");
  
    if(parSetFileProps==0){
      ERROR_PRINTF(FilePropertiesLog, "Error in XML string. There is no FileProperties part.");
      return JS_USERERROR;
    }  

    xmlElement* parEntries = 0;
    parEntries = reader.FirstChildElement(parSetFileProps, DATA_DIMENSIONS);

    Parameter par;
    if(parEntries!=0){
      int ires = reader.load2Parameter(parEntries, &par);
      if(ires!=JS_OK){
        ERROR_PRINTF(FilePropertiesLog, "Can't read xml tag %s",DATA_DIMENSIONS.c_str());
        return ires;
      }
      par.valuesAsInts(&numDimensions);
    } 

    TRACE_PRINTF(FilePropertiesLog, "#Dimenstions=%d",numDimensions);

    if(numDimensions<=0 || numDimensions>5){
      ERROR_PRINTF(FilePropertiesLog, "Wrong or missing %s entry in XML string.\n",DATA_DIMENSIONS.c_str());
      return JS_USERERROR;	
    } 

    dataType = DataType::UNKNOWN;
    traceFormat = DataFormat::FLOAT;
    byteOrder = JSIO_LITTLEENDIAN;
    isMapped=false;

    axisLabels = new AxisLabel[numDimensions];
    axisUnits = new Units[numDimensions];
    axisDomains = new DataDomain[numDimensions];
    axisLabelsStr = new std::string[numDimensions];
    axisUnitsStr = new std::string[numDimensions];
    axisDomainsStr = new std::string[numDimensions];
    axisLengths = new long[numDimensions];
    logicalOrigins = new long[numDimensions];
    logicalDeltas = new long[numDimensions];
    physicalOrigins = new double[numDimensions];
    physicalDeltas = new double[numDimensions];

//    parEntries = parSetFileProps->FirstChildElement("par");
    xmlElement *parElement;
    parElement = reader.FirstChildElement(parSetFileProps);
    std::string curVal;
    std::string curValText;
    do{
      int ires=reader.load2Parameter(parElement, &par);
      if(ires!=JS_OK){
        ERROR_PRINTF(FilePropertiesLog, "error in xml string");
        return ires;
      }
      curVal = par.getName();
//        printf("curVal=%s\n",curVal.c_str());
      if(curVal==COMMENTS){
        par.valuesAsStrings(&curValText);
        comments=curValText;
      }else if(curVal==JAVASEIS_VERSION){
        par.valuesAsStrings(&curValText);
        version=curValText;
      }else if(curVal==DATA_TYPE){
        par.valuesAsStrings(&curValText);
        dataType =  DataType::get(curValText);
      }else if(curVal==TRACE_FORMAT){
        par.valuesAsStrings(&curValText);
        traceFormat =  DataFormat::get(curValText);
      }else if(curVal==BYTEORDER){
        par.valuesAsStrings(&curValText);
        if( curValText=="BIG_ENDIAN") byteOrder=JSIO_BIGENDIAN;
      }else if(curVal==MAPPED){
//            par.valuesAsStrings(&curValText);
        par.valuesAsBooleans(&isMapped);
//            printf("*************** %s=%d\n",curVal.c_str(),isMapped);
//  	  if(curValText=="true") isMapped=true;
      }else if(curVal==AXIS_LABELS){
        par.valuesAsStrings(axisLabelsStr);
        for(int i=0;i<numDimensions;i++)
          axisLabels[i].Init(axisLabelsStr[i],axisLabelsStr[i]);
      }else if(curVal==AXIS_UNITS){
        par.valuesAsStrings(axisUnitsStr);
//         str2sarr(par.valuesAsStrings(), numDimensions, axisUnitsStr);
        for(int i=0;i<numDimensions;i++)
          axisUnits[i].Init(axisUnitsStr[i]);
      }else if(curVal==AXIS_DOMAINS){
        par.valuesAsStrings(axisDomainsStr);
//         str2sarr(par.valuesAsStrings(), numDimensions, axisDomainsStr);
        for(int i=0;i<numDimensions;i++)
          axisDomains[i].Init(axisDomainsStr[i]);
      }else if(curVal==AXIS_LENGTHS){
//         str2larr(par.valuesAsStrings(), numDimensions, axisLengths);
        par.valuesAsLongs(axisLengths);
      }else if(curVal==LOGICAL_ORIGINS){
//         str2larr(par.valuesAsStrings(), numDimensions, logicalOrigins);
        par.valuesAsLongs(logicalOrigins);
      }else if(curVal==LOGICAL_DELTAS){
//       	   str2larr(par.valuesAsStrings(), numDimensions, logicalDeltas);
        par.valuesAsLongs(logicalDeltas);
      }else if(curVal==PHYSICAL_ORIGINS){
//       	   str2darr(par.valuesAsStrings(), numDimensions, physicalOrigins);
        par.valuesAsDoubles(physicalOrigins);
      }else if(curVal==PHYSICAL_DELTAS){
//       	   str2darr(par.valuesAsStrings(), numDimensions, physicalDeltas);
        par.valuesAsDoubles(physicalDeltas);
      }else if(curVal==HEADER_LENGTH_BYTES){
//       	   headerLengthBytes = atoi(par.valuesAsStrings());
        par.valuesAsInts(&headerLengthBytes);
      }
      parElement=reader.NextSiblingElement(parElement,"par");
// /********* here comes all other properties
    }while(parElement!=NULL);

    TRACE_PRINTF(FilePropertiesLog, "FileProperties loaded successfully.");

    return JS_OK;
  }

  int FileProperties::save(std::string &XMLstring)
  {
    std::string tmp;
    XMLstring = "  <parset name=\"FileProperties\">\n";
    XMLstring +="    <par name=\""+COMMENTS+"\" type=\"string\"> \""+ comments +"\" </par>\n";
    XMLstring +="    <par name=\""+JAVASEIS_VERSION+"\" type=\"string\"> "+ version +" </par>\n";
    XMLstring +="    <par name=\""+DATA_TYPE+"\" type=\"string\"> "+ dataType.toString() +" </par>\n";
    std::string sTrFormat=traceFormat.toString();
    std::size_t found =sTrFormat.find(' ');
    if(found!=std::string::npos){
      sTrFormat = "\""+sTrFormat+"\"";
    }  
    XMLstring +="    <par name=\""+TRACE_FORMAT+"\" type=\"string\"> "+ sTrFormat +" </par>\n";
    if(byteOrder==JSIO_LITTLEENDIAN) tmp="LITTLE_ENDIAN"; else tmp="BIG_ENDIAN";
    XMLstring +="    <par name=\""+BYTEORDER+"\" type=\"string\"> "+ tmp +" </par>\n";
    if(isMapped) tmp="true"; else tmp="false";
    XMLstring +="    <par name=\""+MAPPED+"\" type=\"boolean\"> "+ tmp +" </par>\n";
    XMLstring +="    <par name=\""+DATA_DIMENSIONS+"\" type=\"int\"> "+ num2Str(numDimensions) +" </par>\n";
    tmp="\n";
    for(int i=0;i<numDimensions;i++) 
      tmp+="      "+axisLabels[i].getName()+"\n";
    XMLstring +="    <par name=\""+AXIS_LABELS+"\" type=\"string\">"+ tmp +"    </par>\n";
    tmp="\n";
    for(int i=0;i<numDimensions;i++) 
      tmp+="      "+axisUnits[i].getName()+"\n";
    XMLstring +="    <par name=\""+AXIS_UNITS+"\" type=\"string\">"+ tmp +"    </par>\n";
    tmp="\n";
    for(int i=0;i<numDimensions;i++)
      tmp+="      "+axisDomains[i].getName()+"\n";
    XMLstring +="    <par name=\""+AXIS_DOMAINS+"\" type=\"string\">"+ tmp +"    </par>\n";
    tmp="\n";
    for(int i=0;i<numDimensions;i++) 
      tmp+="      "+num2Str(axisLengths[i])+"\n";
    XMLstring +="    <par name=\""+AXIS_LENGTHS+"\" type=\"long\">"+ tmp +"    </par>\n";
    tmp="\n";
    for(int i=0;i<numDimensions;i++) 
      tmp+="      "+num2Str(logicalOrigins[i])+"\n";
    XMLstring +="    <par name=\""+LOGICAL_ORIGINS+"\" type=\"long\">"+ tmp +"    </par>\n";
    tmp="\n";
    for(int i=0;i<numDimensions;i++) 
      tmp+="      "+num2Str(logicalDeltas[i])+"\n";
    XMLstring +="    <par name=\""+LOGICAL_DELTAS+"\" type=\"long\">"+ tmp +"    </par>\n";
    tmp="\n";
    for(int i=0;i<numDimensions;i++)
      tmp+="      "+num2Str(physicalOrigins[i])+"\n";
    XMLstring +="    <par name=\""+PHYSICAL_ORIGINS+"\" type=\"double\">"+ tmp +"    </par>\n";
    tmp="\n";
    for(int i=0;i<numDimensions;i++) 
      tmp+="      "+num2Str(physicalDeltas[i])+"\n";
    XMLstring +="    <par name=\""+PHYSICAL_DELTAS+"\" type=\"double\">"+ tmp +"    </par>\n";
    XMLstring +="    <par name=\""+HEADER_LENGTH_BYTES+"\" type=\"int\"> "+ num2Str(headerLengthBytes) +" </par>\n";
    XMLstring +="  </parset>\n";

    return JS_OK;
  }

} 
 

