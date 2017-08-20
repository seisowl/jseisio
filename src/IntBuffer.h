/***************************************************************************
                          IntBuffer.h  -  description
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
#ifndef INTBUFFER_H
#define INTBUFFER_H

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "Buffer.h"
#include "ByteArray.h"
#include "jsByteOrder.h"

namespace jsIO
{
  class IntBuffer : public Buffer{
    public:
      ~IntBuffer(){}
      IntBuffer();

      JS_BYTEORDER getByteOrder() const {return byteOrder;}
      void setByteOrder(JS_BYTEORDER order){byteOrder=order;}

      unsigned long position() const{return buffer_pos;}
      int position(unsigned long _buffer_pos);

      void setBuffer(int* farray, unsigned long farraylen);
      void wrap(int* farray, unsigned long farraylen);

      size_t size() const{return (size_t)(sizeBase()/SIZEOFINT);}
      size_t capacity() const{return capacityBase()/SIZEOFINT;}
      void resize(size_t sz){resize(sz*SIZEOFINT);}
      void reserve(size_t sz){reserveBase(sz*SIZEOFINT);}
      
      const int* array() const{return (const int*)(arrayBase());}

      int get();
      int get(unsigned long index);
      int  get(int *dst, int len);
      int  get(unsigned long pos, int *dst, int len);

      void put(int value);
      int put(unsigned long index, int value);
      void put(const int *src, int len);
      int put(unsigned long pos, const int *src, int len);

// private atributes
    private:
      JS_BYTEORDER byteOrder;
      JS_BYTEORDER natOrder;
      unsigned long buffer_pos; 
      static const short SIZEOFINT=4;

    public:

  };
}

#endif

