/***************************************************************************
                          CharBuffer.h  -  description
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
#ifndef CHARBUFFER_H
#define CHARBUFFER_H

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "Buffer.h"
#include "jsDefs.h"
#include "jsByteOrder.h"

namespace jsIO
{
  class jsByteOrder;
  class FloatBuffer;
  class ShortBuffer;
  class IntBuffer;
  
  class CharBuffer : public Buffer
  {
    public:
      CharBuffer();
      virtual ~CharBuffer(){}

      unsigned long position(){return buffer_pos;};
      int position(unsigned long _buffer_pos);

      JS_BYTEORDER getByteOrder(){return byteOrder;};
      void setByteOrder(JS_BYTEORDER order){byteOrder=order;};

      void setBuffer(char *_buffer, unsigned long _bufsize){setBufferBase(_buffer,_bufsize); buffer_pos=0;};
      void wrap     (char* _buffer, unsigned long _bufsize){wrapBase(_buffer,_bufsize); buffer_pos=0;};
 
      void asByteBuffer(CharBuffer &bBuf);
      void asFloatBuffer(FloatBuffer &fBuf);
      void asShortBuffer(ShortBuffer &sBuf);
      void asIntBuffer(IntBuffer &iBuf);

      const char* array(){return (const char*)(arrayBase());};

      char get();
      char get(unsigned long index);
      int get(char *dst, int len);
      int get(unsigned long pos, char *dst, int len);

      void put(char ch);
      int put(unsigned long index, char ch);
      void put(char *src, int len);
      int put(unsigned long pos, char *src, int len);


      float getFloat();
      double getDouble();
      int getInt();
      long getLong();
      short getShort();
      float putFloat(float a);
      double putDouble(double a);
      int putInt(int a);
      long putLong(long a);
      short putShort(short a);

// private atributes
    private:
      JS_BYTEORDER byteOrder;
      JS_BYTEORDER natOrder;

      unsigned long buffer_pos;

  };


}
#endif

