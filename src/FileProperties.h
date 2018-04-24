/***************************************************************************
 FileProperties.h  -  description
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

#ifndef FILEPROPERTIES_H
#define FILEPROPERTIES_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <utility> // make_pair
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "xmlreader.h"

#include "AxisLabel.h"
#include "Units.h"
#include "DataDomain.h"
#include "DataType.h"
#include "DataFormat.h"
#include "jsByteOrder.h"

#include "stringfuncs.h"
#include "jsDefs.h"

#include <stdexcept>

namespace jsIO {
class FileProperties {
public:
  static const std::string COMMENTS;
  static const std::string DATA_DIMENSIONS;
  static const std::string DATA_TYPE;
  static const std::string BYTEORDER;
  static const std::string TRACE_FORMAT;
  static const std::string MAPPED;
  static const std::string PHYSICAL_ORIGINS;
  static const std::string PHYSICAL_DELTAS;
  static const std::string AXIS_UNITS;
  static const std::string AXIS_LABELS;
  static const std::string AXIS_DOMAINS;
  static const std::string AXIS_LENGTHS;
  static const std::string HEADER_LENGTH_BYTES;
  static const std::string HEADER_LENGTH;
  static const std::string LOGICAL_ORIGINS;
  static const std::string LOGICAL_DELTAS;
  static const std::string JAVASEIS_VERSION;
  static const std::string DESCRIPTIVE_NAME;
  static const std::string HAS_TRACES;

  static const std::string SCALCO;
  static const std::string SCALEL;

public:
  ~FileProperties();
  FileProperties();
  void Init(int _numDimensions);

  int load(std::string &XMLstring);
  int save(std::string &XMLstring);

public:
  //should be private, just put here temporary for convenience
  DataType dataType;
  DataFormat traceFormat;
  std::string comments;
  std::string version;
  JS_BYTEORDER byteOrder;
  bool isMapped;

  int numDimensions;
  AxisLabel *axisLabels;
  Units *axisUnits;
  DataDomain *axisDomains;
  std::string *axisLabelsStr;
  std::string *axisUnitsStr;
  std::string *axisDomainsStr;
  long *axisLengths;
  long *logicalOrigins;
  long *logicalDeltas;
  double *physicalOrigins;
  double *physicalDeltas;

  int headerLengthBytes;

private:

};
}

#endif

