/***************************************************************************
 jsWriterInput.cpp  -  description
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

#include "TraceProperties.h"
#include "GridDefinition.h" 
#include "DataDefinition.h"
#include "CustomProperties.h"
#include "PSProLogging.h"

#include "jsWriterInput.h"

namespace jsIO {

DECLARE_LOGGER(jsWriterInputLog);

jsWriterInput::~jsWriterInput() {
  delete gridDef;
  delete dataDef;
  delete traceProps;
  delete customProps;
}

jsWriterInput::jsWriterInput() {
  NExtends = 1;
  numGridAxis = 0;
  seispegPolicy = 0;
  isMapped = true;
  gridDef = new GridDefinition;
  dataDef = new DataDefinition;
  traceProps = new TraceProperties;
  customProps = new CustomProperties;
  IOBufferSize = 2 * 1024 * 1024; //default 2MB
}

void jsWriterInput::CopyClass(const jsWriterInput & Other) {
  numGridAxis = Other.numGridAxis;
  jsfilename = Other.jsfilename;
  description = Other.description;
  NExtends = Other.NExtends;
  isMapped = Other.isMapped;
  IOBufferSize = Other.IOBufferSize;
  virtualFolders = Other.virtualFolders;
  *gridDef = *(Other.gridDef);
  *dataDef = *(Other.dataDef);
  *traceProps = *(Other.traceProps);
  seispegPolicy = Other.seispegPolicy;
  *customProps = *(Other.customProps);
}

jsWriterInput::jsWriterInput(jsWriterInput const& _other) {
  gridDef = new GridDefinition;
  dataDef = new DataDefinition;
  traceProps = new TraceProperties;
  customProps = new CustomProperties;
  if (this != &_other) CopyClass(_other);
}

void jsWriterInput::initData(const DataType &_dataType, const DataFormat &_dataFormat, JS_BYTEORDER _byteOrder) {
  dataDef->Init(_dataType, _dataFormat, _byteOrder);
}

void jsWriterInput::initGridDim(int _numDim) {
  AxisDefinition *axes = new AxisDefinition[_numDim];
  gridDef->Init(_numDim, axes);
  numGridAxis = _numDim;
  delete[] axes;
}

int jsWriterInput::initGridAxis(int _axisInd, AxisLabel _label, Units _units, DataDomain _domain, long _length,
    long _logicalOrigin, long _logicalDelta, double _physicalOrigin, double _physicalDelta, std::string _headerName,
    std::string _headerBinName) {
  if (_axisInd >= 0 && _axisInd < numGridAxis) {
    gridDef->getAxisPtr(_axisInd)->Init(_label, _units, _domain, _length, _logicalOrigin, _logicalDelta,
        _physicalOrigin, _physicalDelta, _headerName, _headerBinName);
    return JS_OK;
  } else {
    ERROR_PRINTF(jsWriterInputLog, "Invalid axis index %d. Must be between 0 and %d", _axisInd, numGridAxis);
    return JS_USERERROR;
  }
}

void jsWriterInput::addDefaultProperties() {
  traceProps->addDefaultProperties();
}

int jsWriterInput::addProperty(std::string _label, std::string _description, std::string _format, int _count) {
  return traceProps->addProperty(_label, _description, _format, _count);
}

int jsWriterInput::addProperty(std::string _label) {
  return traceProps->addProperty(_label);
}

void jsWriterInput::addSurveyGeom(int i1, int i2, int i3, int i4, float f1, float f2, float f3, float f4, float f5,
    float f6) {
  if (customProps != NULL) {
    customProps->survGeom.setGeom(i1, i2, i3, i4, f1, f2, f3, f4, f5, f6);
  } else {
    fprintf(stderr,"Error: you have to first allocate the pointes with allocPointers() function");
  }
}

void jsWriterInput::addCustomProperty(std::string name, std::string type, std::string value) {
  customProps->addProperty(name, type, value);
}

}
