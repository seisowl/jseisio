/***************************************************************************
 ByteArray.cpp  -  description
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

#include "ByteArray.h"
#include "jsByteOrder.h"

namespace jsIO {
ByteArray::~ByteArray() {
}

ByteArray::ByteArray() {
  buflen = bufInitSize;
  resize_vbuf();
  bufsize = 0;
  bWrap = false;
}

void ByteArray::CopyClass(const ByteArray &Other) {
  bWrap = false;
  buflen = Other.buflen;
  bufsize = Other.bufsize;
  resize_vbuf();
  memcpy(buffer, Other.buffer, bufsize);
}

void ByteArray::wrap(char *_buffer, unsigned long _bufsize) {
  buffer = _buffer;
  bufsize = _bufsize;
  buflen = bufsize;
  bWrap = true;
}

void ByteArray::copyBuffer(char *_buffer, unsigned long _bufsize) {
  bWrap = false;
  bufsize = _bufsize;
  buflen = bufsize + bufInitSize;
  resize_vbuf();
  memcpy(buffer, _buffer, _bufsize);
}

void ByteArray::resize(unsigned long newsize) {
  if(newsize > buflen) {
    long bufsize_old = bufsize;
    bufsize = newsize;
    buflen = newsize + bufInitSize;
    resize_vbuf(bufsize_old);
  } else {
    bufsize = newsize;
    memset(&buffer[bufsize], 0, newsize - bufsize);
  }
}

void ByteArray::reserve(unsigned long newlen) {
  if(newlen > buflen) {
    buflen = newlen;
    resize_vbuf(bufsize);
  }
}

void ByteArray::checkMem(unsigned long sizeneeded) {
  if(sizeneeded > buflen) {
    unsigned long newsize = buflen;
    while(newsize < sizeneeded) {
      newsize += buflen;
    }
    reserve(newsize);
  }
}

int ByteArray::swap_endianness(unsigned long start_pos, int n, int nb) {
  if(start_pos + n * nb >= bufsize) return JS_USERERROR;
  endian_swap((void *) &buffer[start_pos], n, nb);
  return JS_OK;
}

void ByteArray::insert(unsigned long pos, char *val, unsigned long valsize) {
  checkMem(pos + valsize);
  memcpy(&buffer[pos], val, valsize);
}

// void ByteArray::load(unsigned long pos, char *val , unsigned long valsize){
//    memcpy(val, &buffer[pos], valsize);
// }

void ByteArray::resize_vbuf(unsigned long bufsize_old) {
  vbuf.resize(buflen);
  if(bufsize_old > 0 && bWrap) {
    assert(buflen > bufsize_old);
    memcpy(&vbuf[0], buffer, bufsize_old);
  }
  bWrap = false;
  buffer = &vbuf[0];
}

}

