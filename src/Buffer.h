/***************************************************************************
                          Buffer.h  -  description
                             -------------------
 The base class for Buffer, FloatBuffer, etc..

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
#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "ByteArray.h"

namespace jsIO
{
  class Buffer{
    public:
      ~Buffer(){}
      Buffer(){}
 
      virtual unsigned long size() const {return sizeBase();};
      virtual unsigned long capacity() const {return capacityBase();};

      virtual void resize(size_t sz){resizeBase(sz);};
      virtual void reserve(size_t sz){reserveBase(sz);};

      void insert(unsigned long pos, char *val , unsigned long valsize); //ByteArray.insert
//    void load(unsigned long pos, char *val , unsigned long valsize);//ByteArray.load
   
    protected:
      ByteArray buffer;

    protected:
      void setBufferBase(char *_buffer, unsigned long _bufsize);
      void wrapBase     (char* _buffer, unsigned long _bufsize);
      unsigned long sizeBase()const{return buffer.size();};
      unsigned long capacityBase()const{return buffer.capacity();};
      void resizeBase(size_t sz){buffer.resize(sz);};
      void reserveBase(size_t sz){buffer.reserve(sz);};
      const char* arrayBase() const {return buffer.getBuffer();};

  };

}

#endif

