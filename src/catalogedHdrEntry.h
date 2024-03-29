/***************************************************************************
 catalogedHdrEntry.h  -  description
 -------------------

 // this class contatins similar functions like in TraceProperties and PropertyDescription
 // made for fast and maybe more convenient access to headers

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
#ifndef CATALOGEDHDRENTRY_H
#define CATALOGEDHDRENTRY_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

#include "jsByteOrder.h"

namespace jsIO {
/**
 * This class provides a convenient access to header-words defined in dataset
 */
class catalogedHdrEntry {

  friend class TraceProperties;

public:
  catalogedHdrEntry();
private:
  std::string name;
  std::string description;
  int format;
  int count;
  int offset;

  JS_BYTEORDER byteOrder;
  JS_BYTEORDER natOrder;

private:
  void Init(std::string _name, std::string _description, int _format, int _count, int _offset);

public:
  bool isInitialized() {
    return offset >= 0;
  }

  std::string getName() {
    return name;
  }

  std::string getDescription() {
    return description;
  }

  int getCount() {
    return count;
  }

  int getByteCount();

  int getFormat() {
    return format;
  }

  std::string getFormatAsStr();

  int getOffset() {
    return offset;
  }

  JS_BYTEORDER getByteOrder() {
    return byteOrder;
  }
  void setByteOrder(JS_BYTEORDER order) {
    byteOrder = order;
  }

  float getFloatVal(char *headerBuf);
  double getDoubleVal(char *headerBuf);
  int getIntVal(char *headerBuf);
  short getShortVal(char *headerBuf);
  long getLongVal(char *headerBuf);

  void getFloatVals(signed char *headerBuf, float *vals, int nBytesHeader, int n1, int n2);
  void getDoubleVals(signed char *headerBuf, double *vals, int nBytesHeader, int n1, int n2);
  void getIntVals(signed char *headerBuf, int *vals, int nBytesHeader, int n1, int n2);
  void getShortVals(signed char *headerBuf, short *vals, int nBytesHeader, int n1, int n2);
  void getLongVals(signed char *headerBuf, long *vals, int nBytesHeader, int n1, int n2);

  int setFloatVal(char *headerBuf, float val);
  int setDoubleVal(char *headerBuf, double val);
  int setIntVal(char *headerBuf, int val);
  int setShortVal(char *headerBuf, short val);
  int setLongVal(char *headerBuf, long val);

  int setFloatVector(char *headerBuf, std::vector<float> vec);
  int setDoubleVector(char *headerBuf, std::vector<double> vec);
  int setIntVector(char *headerBuf, std::vector<int> vec);
  int setShortVector(char *headerBuf, std::vector<short> vec);
  int setLongVector(char *headerBuf, std::vector<long> vec);

  std::vector<float> getFloatVector(char *headerBuf);
  std::vector<double> getDoubleVector(char *headerBuf);
  std::vector<int> getIntVector(char *headerBuf);
  std::vector<short> getShortVector(char *headerBuf);
  std::vector<long> getLongVector(char *headerBuf);

};
}

#endif
