/***************************************************************************
 ByteArray.h  -  description
 -------------------
 this class should be similar to std::vector<char>
 the only reason to create it is to be able to contol (wrap function) the buffer where the elements saved

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
#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
using std::vector;

#include "jsDefs.h"

namespace jsIO {
class ByteArray {
public:
  ~ByteArray();
  ByteArray();

  ByteArray(ByteArray const &other) {
    CopyClass(other);
  } //Copy constructor

  ByteArray &operator =(const ByteArray &other) { //Assignment operator
    if(this != &other) CopyClass(other);  //AssignClass(other);
    return *this;
  }
  const char *getBuffer() const {
    return (const char *) buffer;
  }

  unsigned long size() const {
    return bufsize;
  }

  unsigned long capacity() const {
    return buflen;
  }

  void copyBuffer(char *_buffer, unsigned long _bufsize);
  void wrap(char *_buffer, unsigned long _bufsize);
  void resize(unsigned long sz);
  void reserve(unsigned long sz);
  void insert(unsigned long pos, char *val, unsigned long valsize);
  //    void load(unsigned long pos, char *val , unsigned long valsize);

  inline char &operator [](const unsigned long &i) {
    return buffer[i];
  }

  int swap_endianness(unsigned long start_pos, int n, int nb);

private:
  vector<char> vbuf;
  char *buffer;
  unsigned long buflen;
  unsigned long bufsize;
  bool bWrap;

  static const int bufInitSize = 256;
private:
  void checkMem(unsigned long sizeneeded);
  void CopyClass(const ByteArray &Other);
  void resize_vbuf(unsigned long bufsize_old = 0);
};
}

#endif

