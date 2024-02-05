/***************************************************************************
 ShortBuffer.cpp  -  description
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

#include "ShortBuffer.h"

namespace jsIO {
ShortBuffer::ShortBuffer() {
  byteOrder = JSIO_LITTLEENDIAN;
  natOrder = nativeOrder();
  buffer_pos = 0;
}

int ShortBuffer::position(unsigned long _buffer_pos) {
  if(_buffer_pos <= size()) buffer_pos = _buffer_pos;
  else {
    return JS_USERERROR;
  }
  return JS_OK;
}

// the endian_swap function should be persolaized (for short, int ...) for high performance

void ShortBuffer::put(short value) {
  if(natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  char *pV = reinterpret_cast<char *>(&value);
  buffer.insert(buffer_pos * SIZEOFSHORT, pV, SIZEOFSHORT);
  buffer_pos += 1;
}

int ShortBuffer::put(unsigned long index, short value) {
  if(index < size()) {
    if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFSHORT);
    char *pV = reinterpret_cast<char *>(&value);
    buffer.insert(index * SIZEOFSHORT, pV, SIZEOFSHORT);
    return JS_OK;
  } else {
    return JS_USERERROR;
  }
}

void ShortBuffer::put(const short *src, int len) {
  char *pV = (char *)(src);
  buffer.insert(buffer_pos * SIZEOFSHORT, pV, len * SIZEOFSHORT);
  if(natOrder != byteOrder) buffer.swap_endianness(buffer_pos * SIZEOFSHORT, len, SIZEOFSHORT);
  buffer_pos += len;
}

int ShortBuffer::put(unsigned long pos, const short *src, int len) {
  int ires = JS_OK;
  if(pos + len > size()) {
    return JS_USERERROR;
  }
  char *pV = (char *)(src);
  buffer.insert(pos * SIZEOFSHORT, pV, len * SIZEOFSHORT);
  if(natOrder != byteOrder) ires = buffer.swap_endianness(pos * SIZEOFSHORT, len, SIZEOFSHORT);
  return ires;
}

short ShortBuffer::get() {
  short value = *(reinterpret_cast<short *>(&buffer[buffer_pos * SIZEOFSHORT]));
  if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFSHORT);
  buffer_pos += 1;
  return (value);
}

short ShortBuffer::get(unsigned long index) {
  if(index < size()) {
    short value = *(reinterpret_cast<short *>(&buffer[index * SIZEOFSHORT]));
    if(natOrder != byteOrder) endian_swap(&value, 1, SIZEOFSHORT);
    return (value);
  } else {
    return 0;
  }
}

int ShortBuffer::get(short *dst, int len) {
  if(buffer_pos + len > size()) {
    return JS_USERERROR;
  }
  const short *buf = (const short *) array();
  memcpy(dst, &buf[buffer_pos], len * SIZEOFSHORT);
  if(natOrder != byteOrder) endian_swap(dst, len, SIZEOFSHORT);
  buffer_pos += len;
  return JS_OK;
}

int ShortBuffer::get(unsigned long pos, short *dst, int len) {
  if(pos + len > size()) {
    return JS_USERERROR;
  }
  const short *buf = (const short *) array();
  memcpy(dst, &buf[pos], len * SIZEOFSHORT);
  if(natOrder != byteOrder) endian_swap(dst, len, SIZEOFSHORT);
  return JS_OK;
}
}

