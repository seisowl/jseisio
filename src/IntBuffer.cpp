/***************************************************************************
 IntBuffer.cpp  -  description
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

#include "IntBuffer.h"

namespace jsIO {
IntBuffer::IntBuffer() {
  byteOrder = JSIO_LITTLEENDIAN;
  natOrder = nativeOrder();
  buffer_pos = 0;
}

int IntBuffer::position(unsigned long _buffer_pos) {
  if(_buffer_pos <= size()) buffer_pos = _buffer_pos;
  else {
    return JS_USERERROR;
  }
  return JS_OK;
}

// the endian_swap function should be personilized (for int, int ...) for high performance

void IntBuffer::put(int value) {
  if(natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  char *pV = reinterpret_cast<char *>(&value);
  buffer.insert(buffer_pos * SIZEOFINT, pV, SIZEOFINT);
  buffer_pos += 1;
}

int IntBuffer::put(unsigned long index, int value) {
  if(index < size()) {
    if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFINT);
    char *pV = reinterpret_cast<char *>(&value);
    buffer.insert(index * SIZEOFINT, pV, SIZEOFINT);
    return JS_OK;
  }
  return JS_USERERROR;
}

void IntBuffer::put(const int *src, int len) {
  char *pV = (char *)(src);
  buffer.insert(buffer_pos * SIZEOFINT, pV, len * SIZEOFINT);
  if(natOrder != byteOrder) buffer.swap_endianness(buffer_pos * SIZEOFINT, len, SIZEOFINT);
  buffer_pos += len;
}

int IntBuffer::put(unsigned long pos, const int *src, int len) {
  int ires = JS_OK;
  if(pos + len > size()) {
    return JS_USERERROR;
  }
  char *pV = (char *)(src);
  buffer.insert(pos * SIZEOFINT, pV, len * SIZEOFINT);
  if(natOrder != byteOrder) ires = buffer.swap_endianness(pos * SIZEOFINT, len, SIZEOFINT);
  return ires;
}

int IntBuffer::get() {
  int value = *(reinterpret_cast<int *>(&buffer[buffer_pos * SIZEOFINT]));
  if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFINT);
  buffer_pos += 1;
  return (value);
}

int IntBuffer::get(unsigned long index) {
  if(index < size()) {
    int value = *(reinterpret_cast<int *>(&buffer[index * SIZEOFINT]));
    if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFINT);
    return (value);
  } else {
    return 0;
  }
}

int IntBuffer::get(int *dst, int len) {
  if(buffer_pos + len > size()) {
    return JS_USERERROR;
  }
  const int *buf = (const int *) array();
  memcpy(dst, &buf[buffer_pos], len * SIZEOFINT);
  if(natOrder != byteOrder) endian_swap(dst, len, SIZEOFINT);
  buffer_pos += len;
  return JS_OK;
}

int IntBuffer::get(unsigned long pos, int *dst, int len) {
  if(pos + len > size()) {
    return JS_USERERROR;
  }
  const int *buf = (const int *) array();
  memcpy(dst, &buf[pos], len * SIZEOFINT);
  if(natOrder != byteOrder) endian_swap(dst, len, SIZEOFINT);
  return JS_OK;
}

void IntBuffer::copyBuffer(int *farray, unsigned long farraylen) {
  copyBufferBase((char *) farray, farraylen * SIZEOFINT);
  buffer_pos = 0;
}

void IntBuffer::wrap(int *farray, unsigned long farraylen) {
  wrapBase((char *) farray, farraylen * SIZEOFINT);
  buffer_pos = 0;
}

}
