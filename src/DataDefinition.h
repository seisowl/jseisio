/***************************************************************************
 DataDefinition.h  -  description
 -------------------
 * This class provides a type-safe representation of domains,
 * such as space, time, and frequency.

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

#ifndef DATADEFINITION_H
#define DATADEFINITION_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "stringfuncs.h"
#include "jsDefs.h"
#include "jsByteOrder.h"
#include "DataType.h"
#include "DataFormat.h"

namespace jsIO {
class DataDefinition {
public:
  ~DataDefinition() {
  }
  DataDefinition() {
  }
  DataDefinition(const DataType &_dataType, const DataFormat &_dataFormat, JS_BYTEORDER _byteOrder);
  void Init(const DataType &_dataType, const DataFormat &_traceFormat, JS_BYTEORDER _byteOrder);

  DataDefinition getDefault();

  DataType getDataType() {
    return (m_dataType);
  }
  DataFormat getTraceFormat() {
    return (m_traceFormat);
  }
  JS_BYTEORDER getByteOrder() {
    return (m_byteOrder);
  }
  std::string getDataTypeString() {
    return (m_dataType.toString());
  }
  std::string getTraceFormatString() {
    return (m_traceFormat.toString());
  }
  std::string getByteOrderString();
  int getBytesPerSample();

private:
  JS_BYTEORDER m_byteOrder;
  DataFormat m_traceFormat;
  DataType m_dataType;
};
}

#endif

