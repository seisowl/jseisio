/***************************************************************************
 FloatBuffer.cpp  -  description
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

#include "FloatBuffer.h"

namespace jsIO {
vector<float> str2floats(const string str) {
  std::stringstream iss(str);
  vector<float> v;

  float val;
  char c;
  while(iss >> val) {
    v.push_back(val);
    while((c = iss.peek()) == ',' || c == ' ')
      iss.get();
  }
  return v;
}

string floats2str(const vector<float> &v) { // %8g
  std::ostringstream ss;
  size_t len = v.size();
  for(size_t i = 0; i < len; i++) {
    ss << (i == 0 ? "" : ",") << std::setprecision(std::numeric_limits<float>::max_digits10) << v[i];
  }
  return ss.str();
}


FloatBuffer::FloatBuffer() {
  byteOrder = JSIO_LITTLEENDIAN;
  natOrder = nativeOrder();
  buffer_pos = 0;
}

int FloatBuffer::position(unsigned long _buffer_pos) {
  if(_buffer_pos <= size()) {
    buffer_pos = _buffer_pos;
    return JS_OK;
  }
  return JS_USERERROR;
}

void FloatBuffer::put(float value) {
  if(natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  char *pV = reinterpret_cast<char *>(&value);
  buffer.insert(buffer_pos * SIZEOFFLOAT, pV, SIZEOFFLOAT);
  buffer_pos += 1;
}

int FloatBuffer::put(unsigned long index, float value) {
  if(index < size()) {
    if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFFLOAT);
    char *pV = reinterpret_cast<char *>(&value);
    buffer.insert(index * SIZEOFFLOAT, pV, SIZEOFFLOAT);
    return JS_OK;
  }
  return JS_USERERROR;
}

void FloatBuffer::put(const float *src, int len) {
  char *pV = (char *)(src);
  buffer.insert(buffer_pos * SIZEOFFLOAT, pV, len * SIZEOFFLOAT);
  if(natOrder != byteOrder) buffer.swap_endianness(buffer_pos * SIZEOFFLOAT, len, SIZEOFFLOAT);
  buffer_pos += len;
}

int FloatBuffer::put(unsigned long pos, const float *src, int len) {
  int ires = JS_OK;
  if(pos + len > size()) {
    return JS_USERERROR;
  }
  char *pV = (char *)(src);
  buffer.insert(pos * SIZEOFFLOAT, pV, len * SIZEOFFLOAT);
  if(natOrder != byteOrder) ires = buffer.swap_endianness(pos * SIZEOFFLOAT, len, SIZEOFFLOAT);
  return ires;
}

float FloatBuffer::get() {
  float value = *(reinterpret_cast<float *>(&buffer[buffer_pos * SIZEOFFLOAT]));
  if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFFLOAT);
  buffer_pos += 1;
  return (value);
}

float FloatBuffer::get(unsigned long index) {
  if(index < size()) {
    float value = *(reinterpret_cast<float *>(&buffer[index * SIZEOFFLOAT]));
    if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFFLOAT);
    return (value);
  } else {
    return 0;
  }
}

int FloatBuffer::get(float *dst, int len) {
  if(buffer_pos + len > size()) {
    return JS_USERERROR;
  }
  const float *buf = (const float *) array();
  memcpy(dst, &buf[buffer_pos], len * SIZEOFFLOAT);
  if(natOrder != byteOrder) endian_swap(dst, len, SIZEOFFLOAT);
  buffer_pos += len;
  return JS_OK;
}

int FloatBuffer::get(unsigned long pos, float *dst, int len) {
  if(pos + len > size()) {
    return JS_USERERROR;
  }
  const float *buf = (const float *) array();
  memcpy(dst, &buf[pos], len * SIZEOFFLOAT);
  if(natOrder != byteOrder) endian_swap(dst, len, SIZEOFFLOAT);
  return JS_OK;
}
}

