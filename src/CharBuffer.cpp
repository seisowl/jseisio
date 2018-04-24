/***************************************************************************
 CharBuffer.cpp  -  description
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
#include "ShortBuffer.h"
#include "IntBuffer.h"

#include "CharBuffer.h"

namespace jsIO {
CharBuffer::CharBuffer() {
  byteOrder = JSIO_LITTLEENDIAN;
  natOrder = nativeOrder();
  buffer_pos = 0;
}

int CharBuffer::position(unsigned long _buffer_pos) {
  if (_buffer_pos <= size()) {
    buffer_pos = _buffer_pos;
    return JS_OK;
  }
//   else{
//      ERROR_PRINTF(CharBufferLog, "Illegal buffer position. %lu must be smaller than %lu", _buffer_pos, size());
//   }
  return JS_USERERROR;
}

void CharBuffer::asByteBuffer(CharBuffer &bBuf) {
  bBuf.wrap((char*) array(), size());
}

void CharBuffer::asFloatBuffer(FloatBuffer &fBuf) {
  fBuf.wrap((float*) array(), (unsigned long) (size() / sizeof(float)));
}

void CharBuffer::asShortBuffer(ShortBuffer &sBuf) {
  sBuf.wrap((short*) array(), (unsigned long) (size() / sizeof(short)));
}

void CharBuffer::asIntBuffer(IntBuffer &iBuf) {
  iBuf.wrap((int*) array(), (unsigned long) (size() / sizeof(int)));
}

char CharBuffer::get() {
  char ch = buffer[buffer_pos];
  buffer_pos += 1;
  return ch;
}

char CharBuffer::get(unsigned long index) {
  if (index < size()) return buffer[index];
  else {
//      ERROR_PRINTF(CharBufferLog, "Error: %lu must be in [0,%lu) \n",index, size());
    return 0;
  }
}

int CharBuffer::get(char *dst, int len) {
  if (buffer_pos + len > size()) {
//         ERROR_PRINTF(CharBufferLog, " %lu + %lu must smaller than %lu \n",buffer_pos, len, size());
    return JS_USERERROR;
  }
  const char *buf = (const char *) array();
  memcpy(dst, &buf[buffer_pos], len);
  buffer_pos += len;
  return JS_OK;
}

int CharBuffer::get(unsigned long pos, char *dst, int len) {
  if (pos + len > size()) {
//         ERROR_PRINTF(CharBufferLog, "%d must be smaller than the size of the remaining buffer %lu\n",len, size()-pos);
    return JS_USERERROR;
  }
  const char *buf = (const char *) array();
  memcpy(dst, &buf[pos], len);
  return JS_OK;
}

void CharBuffer::put(char ch) {
  buffer[buffer_pos] = ch;
  buffer_pos += 1;
}

int CharBuffer::put(unsigned long index, char ch) {
  if (index >= size()) {
//      printf("Error: %lu must be in [0,%lu) \n",index, size());
    return JS_USERERROR;
  }
  buffer[index] = ch;
  return JS_OK;
}

void CharBuffer::put(char *src, int len) {
  buffer.insert(buffer_pos, src, len);
  buffer_pos += len;
}

int CharBuffer::put(unsigned long pos, char *src, int len) {
  if (pos > 0 && pos < size()) buffer.insert(pos, src, len);
  else return JS_USERERROR;
//       printf("Error: %lu must be in [0,%lu) \n",pos, size());

  return JS_OK;
}

// the endian_swap function should be persolaized (for float, int ...) for high performance

short CharBuffer::putShort(short value) {
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  char *pV = reinterpret_cast<char*>(&value);
  buffer.insert(buffer_pos, pV, sizeof(short));
  buffer_pos += sizeof(short);
  return (value);
}

int CharBuffer::putInt(int value) {
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  char *pV = reinterpret_cast<char*>(&value);
  buffer.insert(buffer_pos, pV, sizeof(int));
  buffer_pos += sizeof(int);
  return (value);
}

long CharBuffer::putLong(long value) {
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  char *pV = reinterpret_cast<char*>(&value);
  buffer.insert(buffer_pos, pV, sizeof(long));
  buffer_pos += sizeof(long);
  return (value);
}

float CharBuffer::putFloat(float value) {
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  char *pV = reinterpret_cast<char*>(&value);
  buffer.insert(buffer_pos, pV, sizeof(float));
  buffer_pos += sizeof(float);
  return (value);
}

double CharBuffer::putDouble(double value) {
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  char *pV = reinterpret_cast<char*>(&value);
  buffer.insert(buffer_pos, pV, sizeof(double));
  buffer_pos += sizeof(double);
  return (value);
}

short CharBuffer::getShort() {
  short value = *(reinterpret_cast<short*>(&buffer[buffer_pos]));
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  buffer_pos += sizeof(value);
  return (value);
}

int CharBuffer::getInt() {
  int value = *(reinterpret_cast<int*>(&buffer[buffer_pos]));
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  buffer_pos += sizeof(value);
  return (value);
}

long CharBuffer::getLong() {
  long value = *(reinterpret_cast<long*>(&buffer[buffer_pos]));
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  buffer_pos += sizeof(value);
  return (value);
}

float CharBuffer::getFloat() {
  float value = *(reinterpret_cast<float*>(&buffer[buffer_pos]));
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  buffer_pos += sizeof(value);
  return (value);
}

double CharBuffer::getDouble() {
  double value = *(reinterpret_cast<double*>(&buffer[buffer_pos]));
  if (natOrder != byteOrder) endian_swap(&value, 1, sizeof(value));
  buffer_pos += sizeof(value);
  return (value);
}
}
