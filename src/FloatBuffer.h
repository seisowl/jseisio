/***************************************************************************
 FloatBuffer.h  -  description
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
#ifndef FLOATBUFFER_H
#define FLOATBUFFER_H

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "Buffer.h"
#include "ByteArray.h"
#include "jsByteOrder.h"
#include "jsDefs.h"
using std::string;
using std::vector;
#include <sstream>
#include <limits>    // std::cout, std::fixed
#include <iomanip>

namespace jsIO {
vector<float> str2floats(const string str);
string floats2str(const vector<float> &v);

class FloatBuffer: public Buffer {
public:
  FloatBuffer();
  virtual ~FloatBuffer() {
  }

  JS_BYTEORDER getByteOrder() {
    return byteOrder;
  }

  void setByteOrder(JS_BYTEORDER order) {
    byteOrder = order;
  }

  unsigned long position() const {
    return buffer_pos;
  }

  int position(unsigned long _buffer_pos);

  void copyBuffer(float *farray, unsigned long farraylen) {
    copyBufferBase((char*)farray, farraylen * SIZEOFFLOAT);
    buffer_pos = 0;
  }

  void wrap(float *farray, unsigned long farraylen) {
    wrapBase((char*)farray, farraylen * SIZEOFFLOAT);
    buffer_pos = 0;
  }

  size_t size() const {
    return (size_t)(sizeBase() / SIZEOFFLOAT);
  }

  size_t capacity() const {
    return capacityBase() / SIZEOFFLOAT;
  }

  void resize(size_t sz) {
    resize(sz * SIZEOFFLOAT);
  }

  void reserve(size_t sz) {
    reserveBase(sz * SIZEOFFLOAT);
  }

  const float* array() const {
    return (const float*)(arrayBase());
  }

  float get();
  float get(unsigned long index);
  int get(float *dst, int len);
  int get(unsigned long pos, float *dst, int len);

  void put(float value);
  int put(unsigned long index, float value);
  void put(const float *src, int len);
  int put(unsigned long pos, const float *src, int len);

private:
  JS_BYTEORDER byteOrder;
  JS_BYTEORDER natOrder;

  unsigned long buffer_pos;
  static const short SIZEOFFLOAT = 4;
};
}

#endif

