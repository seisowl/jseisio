/***************************************************************************
 catalogedHdrEntry.cpp  -  description
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
#include <string>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <signal.h>
#include <stdexcept>

#include "PropertyDescription.h"
#include "PSProLogging.h"
#include "catalogedHdrEntry.h"
#include "Assertion.h"

namespace jsIO {
DECLARE_LOGGER(catalogedHdrEntryLog);

catalogedHdrEntry::catalogedHdrEntry() {
  offset = -1;
  byteOrder = JSIO_LITTLEENDIAN;
  natOrder = nativeOrder();
  format = PropertyDescription::HDR_FORMAT_FLOAT;
  count = 1;
}

void catalogedHdrEntry::Init(std::string _name, std::string _description, int _format, int _count, int _offset) {
  name = _name;
  description = _description;
  format = _format;
  count = _count;
  offset = _offset;
}

float catalogedHdrEntry::getFloatVal(char *headerBuf) {
  if(format == PropertyDescription::HDR_FORMAT_FLOAT) {
    float v = *(reinterpret_cast<float *>(&headerBuf[offset]));
    if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(float));
    return v;
  } else if(format == PropertyDescription::HDR_FORMAT_DOUBLE) {
    double v = *(reinterpret_cast<double *>(&headerBuf[offset]));
    if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(double));
    return v;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-float/double header as a float");
    return std::numeric_limits<float>::max();
  }
}

double catalogedHdrEntry::getDoubleVal(char *headerBuf) {
  if(format == PropertyDescription::HDR_FORMAT_DOUBLE) {
    double v = *(reinterpret_cast<double *>(&headerBuf[offset]));
    if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(double));
    return v;
  } else if(format == PropertyDescription::HDR_FORMAT_FLOAT) {
    float v = *(reinterpret_cast<float *>(&headerBuf[offset]));
    if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(float));
    return v;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-double/float header as a double");
    return std::numeric_limits<double>::max();
  }
}

long catalogedHdrEntry::getLongVal(char *headerBuf) {
  if(format == PropertyDescription::HDR_FORMAT_LONG) {
    long v = *(reinterpret_cast<long *>(&headerBuf[offset]));
    if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(long));
    return v;
  } else if(format == PropertyDescription::HDR_FORMAT_INTEGER) {
    int v = *(reinterpret_cast<int *>(&headerBuf[offset]));
    if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(int));
    return v;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-long/int header as a long");
    return std::numeric_limits<long>::max();
  }
}

int catalogedHdrEntry::getIntVal(char *headerBuf) {
  if(format == PropertyDescription::HDR_FORMAT_INTEGER) {
    int v = *(reinterpret_cast<int *>(&headerBuf[offset]));
    if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(int));
    return v;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-int header as an int %d", format);
    return std::numeric_limits<int>::max();
  }
}

short catalogedHdrEntry::getShortVal(char *headerBuf) {
  if(format == PropertyDescription::HDR_FORMAT_SHORT) {
    short v = *(reinterpret_cast<short *>(&headerBuf[offset]));
    if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(short));
    return v;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-short header as a short");
    return std::numeric_limits<short>::max();
  }
}

int catalogedHdrEntry::setFloatVal(char *headerBuf, float val) {
  if(format == PropertyDescription::HDR_FORMAT_FLOAT) {
    if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(float));
    char *pV = reinterpret_cast<char *>(&val);
    memcpy(&headerBuf[offset], pV, sizeof(float));
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write float value in a non-float header %s", name.c_str());
    return JS_USERERROR;
  }
}

int catalogedHdrEntry::setDoubleVal(char *headerBuf, double val) {
  if(format == PropertyDescription::HDR_FORMAT_DOUBLE) {
    if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(double));
    char *pV = reinterpret_cast<char *>(&val);
    memcpy(&headerBuf[offset], pV, sizeof(double));
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write double value in a non-double header %s", name.c_str());
    return JS_USERERROR;
  }
}

int catalogedHdrEntry::setLongVal(char *headerBuf, long val) {
  if(format == PropertyDescription::HDR_FORMAT_LONG) {
    if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(long));
    char *pV = reinterpret_cast<char *>(&val);
    memcpy(&headerBuf[offset], pV, sizeof(long));
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write long value in a non-long header %s", name.c_str());
    return JS_USERERROR;
  }
}

int catalogedHdrEntry::setIntVal(char *headerBuf, int val) {
  if(format == PropertyDescription::HDR_FORMAT_INTEGER) {
    if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(int));
    char *pV = reinterpret_cast<char *>(&val);
    memcpy(&headerBuf[offset], pV, sizeof(int));
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write int value in a non-int header '%s'", name.c_str());
    raise(SIGSEGV);
    return JS_USERERROR;
  }
}

int catalogedHdrEntry::setShortVal(char *headerBuf, short val) {
  if(format == PropertyDescription::HDR_FORMAT_SHORT) {
    if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(short));
    char *pV = reinterpret_cast<char *>(&val);
    memcpy(&headerBuf[offset], pV, sizeof(short));
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write short value in a non-short header %s", name.c_str());;
    return JS_USERERROR;
  }
}

int catalogedHdrEntry::getByteCount() {
  switch(format) {
  case PropertyDescription::HDR_FORMAT_BYTE:
    return 1;
  case PropertyDescription::HDR_FORMAT_SHORT:
    return 2;
  case PropertyDescription::HDR_FORMAT_INTEGER:
    return 4;
  case PropertyDescription::HDR_FORMAT_LONG:
    return 8;
  case PropertyDescription::HDR_FORMAT_FLOAT:
    return 4;
  case PropertyDescription::HDR_FORMAT_DOUBLE:
    return 8;
  }
  return 0;
}

std::string catalogedHdrEntry::getFormatAsStr() {
  switch(format) {
  case PropertyDescription::HDR_FORMAT_BYTE:
    return "char";
  case PropertyDescription::HDR_FORMAT_SHORT:
    return "short";
  case PropertyDescription::HDR_FORMAT_INTEGER:
    return "int32";
  case PropertyDescription::HDR_FORMAT_LONG:
    return "int64";
  case PropertyDescription::HDR_FORMAT_FLOAT:
    return "float";
  case PropertyDescription::HDR_FORMAT_DOUBLE:
    return "double";
  }
  return "undefined";
}

void catalogedHdrEntry::getFloatVals(signed char *headerBuf, float *vals, int nBytesHeader, int n1, int n2) {
  if(format == PropertyDescription::HDR_FORMAT_FLOAT) {
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        size_t off = offv * nBytesHeader + offset;
        float v = *(reinterpret_cast<float *>(&headerBuf[off]));
        if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(float));
        vals[offv] = v;
      }
    }
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-float header as a float");
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        vals[offv] = std::numeric_limits<float>::max();
      }
    }
  }
}

void catalogedHdrEntry::getDoubleVals(signed char *headerBuf, double *vals, int nBytesHeader, int n1, int n2) {
  if(format == PropertyDescription::HDR_FORMAT_DOUBLE) {
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        size_t off = offv * nBytesHeader + offset;
        double v = *(reinterpret_cast<double *>(&headerBuf[off]));
        if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(double));
        vals[offv] = v;
      }
    }
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-double header as a double");
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        vals[offv] = std::numeric_limits<double>::max();
      }
    }
  }
}

void catalogedHdrEntry::getIntVals(signed char *headerBuf, int *vals, int nBytesHeader, int n1, int n2) {
  if(format == PropertyDescription::HDR_FORMAT_INTEGER) {
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        size_t off = offv * nBytesHeader + offset;
        int v = *(reinterpret_cast<int *>(&headerBuf[off]));
        if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(int));
        vals[offv] = v;
      }
    }
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-int header as a int");
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        vals[offv] = std::numeric_limits<int>::max();
      }
    }
  }
}

void catalogedHdrEntry::getShortVals(signed char *headerBuf, short *vals, int nBytesHeader, int n1, int n2) {
  if(format == PropertyDescription::HDR_FORMAT_SHORT) {
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        size_t off = offv * nBytesHeader + offset;
        short v = *(reinterpret_cast<short *>(&headerBuf[off]));
        if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(short));
        vals[offv] = v;
      }
    }
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-short header as a short");
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        vals[offv] = std::numeric_limits<short>::max();
      }
    }
  }
}

void catalogedHdrEntry::getLongVals(signed char *headerBuf, long *vals, int nBytesHeader, int n1, int n2) {
  if(format == PropertyDescription::HDR_FORMAT_LONG) {
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        size_t off = offv * nBytesHeader + offset;
        long v = *(reinterpret_cast<long *>(&headerBuf[off]));
        if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(long));
        vals[offv] = v;
      }
    }
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-long header as a long");
    #pragma omp parallel for
    for(int i2 = 0; i2 < n2; i2++) {
      for(int i1 = 0; i1 < n1; i1++) {
        size_t offv = (size_t) i2 * n1 + i1;
        vals[offv] = std::numeric_limits<long>::max();
      }
    }
  }
}

int catalogedHdrEntry::setFloatVector(char *headerBuf, std::vector<float> vec) {
  if(format == PropertyDescription::HDR_FORMAT_FLOAT) {
    int nbytes = getByteCount();
    int vecsize = vec.size();
    int ncnt = std::min(vecsize, count);
    for(int i = 0; i < ncnt; i++) {
      float val = vec[i];
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(float));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(float));
    }
    for(int i = ncnt; i < count; i++) {
      float val = - FLT_MAX;
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(float));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(float));
    }
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write float value in a non-float header %s", name.c_str());
    return JS_USERERROR;
  }
}

int catalogedHdrEntry::setDoubleVector(char *headerBuf, std::vector<double> vec) {
  if(format == PropertyDescription::HDR_FORMAT_DOUBLE) {
    int nbytes = getByteCount();
    int vecsize = vec.size();
    int ncnt = std::min(vecsize, count);
    for(int i = 0; i < ncnt; i++) {
      double val = vec[i];
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(double));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(double));
    }
    for(int i = ncnt; i < count; i++) {
      double val = - DBL_MAX;
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(double));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(double));
    }
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write double value in a non-double header %s", name.c_str());
    return JS_USERERROR;
  }
}


int catalogedHdrEntry::setIntVector(char *headerBuf, std::vector<int> vec) {
  if(format == PropertyDescription::HDR_FORMAT_INTEGER) {
    int nbytes = getByteCount();
    int vecsize = vec.size();
    int ncnt = std::min(vecsize, count);
    for(int i = 0; i < ncnt; i++) {
      int val = vec[i];
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(int));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(int));
    }
    for(int i = ncnt; i < count; i++) {
      int val = - INT_MAX;
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(int));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(int));
    }
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write int value in a non-int header %s", name.c_str());
    return JS_USERERROR;
  }
}


int catalogedHdrEntry::setShortVector(char *headerBuf, std::vector<short> vec) {
  if(format == PropertyDescription::HDR_FORMAT_SHORT) {
    int nbytes = getByteCount();
    int vecsize = vec.size();
    int ncnt = std::min(vecsize, count);
    for(int i = 0; i < ncnt; i++) {
      short val = vec[i];
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(short));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(short));
    }
    for(int i = ncnt; i < count; i++) {
      short val = - SHRT_MAX;
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(short));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(short));
    }
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write short value in a non-short header %s", name.c_str());;
    return JS_USERERROR;
  }
}


int catalogedHdrEntry::setLongVector(char *headerBuf, std::vector<long> vec) {
  if(format == PropertyDescription::HDR_FORMAT_LONG) {
    int nbytes = getByteCount();
    int vecsize = vec.size();
    int ncnt = std::min(vecsize, count);
    for(int i = 0; i < ncnt; i++) {
      long val = vec[i];
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(long));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(long));
    }
    for(int i = ncnt; i < count; i++) {
      long val = - LONG_MAX;
      if(byteOrder != natOrder) endian_swap(&val, 1, sizeof(long));
      char *pV = reinterpret_cast<char *>(&val);
      memcpy(&headerBuf[offset + i * nbytes], pV, sizeof(long));
    }
    return JS_OK;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to write long value in a non-long header %s", name.c_str());
    return JS_USERERROR;
  }
}


std::vector<float> catalogedHdrEntry::getFloatVector(char *headerBuf) {
  std::vector<float> vec;
  if(format == PropertyDescription::HDR_FORMAT_FLOAT) {
    int nbytes = getByteCount();
    for(int i = 0; i < count; i++) {
      float v = *(reinterpret_cast<float *>(&headerBuf[offset + i * nbytes]));
      if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(float));
      if(v > - FLT_MAX) {
        vec.push_back(v);
      } else return vec;
    }
    return vec;
  } else if(format == PropertyDescription::HDR_FORMAT_DOUBLE) {
    int nbytes = getByteCount();
    std::vector<float> vec;
    for(int i = 0; i < count; i++) {
      double v = *(reinterpret_cast<double *>(&headerBuf[offset + i * nbytes]));
      if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(double));
      if(v > - DBL_MAX) {
        vec.push_back((float)v);
      } else return vec;
    }
    return vec;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-float/double header as a float");
    return vec;
  }
}


std::vector<double> catalogedHdrEntry::getDoubleVector(char *headerBuf) {
  std::vector<double> vec;
  if(format == PropertyDescription::HDR_FORMAT_DOUBLE) {
    int nbytes = getByteCount();
    for(int i = 0; i < count; i++) {
      double v = *(reinterpret_cast<double *>(&headerBuf[offset + i * nbytes]));
      if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(double));
      if(v > - DBL_MAX) {
        vec.push_back(v);
      } else return vec;
    }
    return vec;
  } else if(format == PropertyDescription::HDR_FORMAT_FLOAT) {
    int nbytes = getByteCount();
    for(int i = 0; i < count; i++) {
      float v = *(reinterpret_cast<float *>(&headerBuf[offset + i * nbytes]));
      if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(float));
      if(v > - FLT_MAX) {
        vec.push_back((double)v);
      } else return vec;
    }
    return vec;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-double/float header as a double");
    return vec;
  }

}

std::vector<int> catalogedHdrEntry::getIntVector(char *headerBuf) {
  std::vector<int> vec;
  if(format == PropertyDescription::HDR_FORMAT_INTEGER) {
    int nbytes = getByteCount();
    for(int i = 0; i < count; i++) {
      int v = *(reinterpret_cast<int *>(&headerBuf[offset + i * nbytes]));
      if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(int));
      if(v > - INT_MAX) {
        vec.push_back(v);
      } else return vec;
    }
    return vec;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-int header as an int %d", format);
    return vec;
  }
}

std::vector<short> catalogedHdrEntry::getShortVector(char *headerBuf) {
  std::vector<short> vec;
  if(format == PropertyDescription::HDR_FORMAT_SHORT) {
    int nbytes = getByteCount();
    for(int i = 0; i < count; i++) {
      int v = *(reinterpret_cast<short *>(&headerBuf[offset + i * nbytes]));
      if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(short));
      if(v > - SHRT_MAX) {
        vec.push_back(v);
      } else return vec;
    }
    return vec;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-int header as an int %d", format);
    return vec;
  }

}

std::vector<long> catalogedHdrEntry::getLongVector(char *headerBuf) {
  std::vector<long> vec;
  if(format == PropertyDescription::HDR_FORMAT_LONG) {
    int nbytes = getByteCount();
    for(int i = 0; i < count; i++) {
      long v = *(reinterpret_cast<long *>(&headerBuf[offset + i * nbytes]));
      if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(long));
      if(v > - LONG_MAX) {
        vec.push_back(v);
      } else return vec;
    }
    return vec;
  } else if(format == PropertyDescription::HDR_FORMAT_INTEGER) {
    int nbytes = getByteCount();
    for(int i = 0; i < count; i++) {
      int v = *(reinterpret_cast<int *>(&headerBuf[offset + i * nbytes]));
      if(byteOrder != natOrder) endian_swap(&v, 1, sizeof(int));
      if(v > - INT_MAX) {
        vec.push_back((long)v);
      } else return vec;
    }
    return vec;
  } else {
    ERROR_PRINTF(catalogedHdrEntryLog, "You are trying to read non-long/int header as a long");
    return vec;
  }
}

}
