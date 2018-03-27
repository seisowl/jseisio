/***************************************************************************
                          ShortBuffer.h  -  description
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
#ifndef SHORTBUFFER_H
#define SHORTBUFFER_H

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "Buffer.h"
#include "jsByteOrder.h"
#include "jsDefs.h"

namespace jsIO
{
  class ShortBuffer : public Buffer{
    public:
      ShortBuffer();
      virtual ~ShortBuffer(){}

      JS_BYTEORDER getByteOrder() const{return byteOrder;};
      void setByteOrder(JS_BYTEORDER order){byteOrder=order;};

      unsigned long position() const{return buffer_pos;};
      int position(unsigned long _buffer_pos);

      void setBuffer(short* sarray, unsigned long sarraylen){setBufferBase((char*)sarray,sarraylen*SIZEOFSHORT);buffer_pos=0;};
      void wrap(short* sarray, unsigned long sarraylen){wrapBase((char*)sarray,sarraylen*SIZEOFSHORT);buffer_pos=0;};

      size_t size() const{return (size_t)(sizeBase()/SIZEOFSHORT);};
      size_t capacity() const{return capacityBase()/SIZEOFSHORT;};
      void resize(size_t sz){resize(sz*SIZEOFSHORT);};
      void reserve(size_t sz){reserveBase(sz*SIZEOFSHORT);};

      const short* array() const{return (const short*)(arrayBase());};
 
      short get();
      short get(unsigned long index);
      int get(short *dst, int len);
      int get(unsigned long pos, short *dst, int len);

      void put(short value);
      int put(unsigned long index, short value);
      void put(const short *src, int len);
      int put(unsigned long pos, const short *src, int len);

    private:
      JS_BYTEORDER byteOrder;
      JS_BYTEORDER natOrder;

      unsigned long buffer_pos; 

      static const short SIZEOFSHORT=2;

    public:

  };
}

#endif

