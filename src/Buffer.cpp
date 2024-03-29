/***************************************************************************
 Buffer.cpp  -  description
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

#include "Buffer.h"

namespace jsIO {
void Buffer::copyBufferBase(char *_buffer, unsigned long _bufsize) {
  buffer.copyBuffer(_buffer, _bufsize);
}

void Buffer::wrapBase(char *_buffer, unsigned long _bufsize) {
  buffer.wrap(_buffer, _bufsize);
}

void Buffer::insert(unsigned long pos, char *val, unsigned long valsize) {
  buffer.insert(pos, val, valsize);
}

/*
 void Buffer::load(unsigned long pos, char *val , unsigned long valsize)
 {
 buffer.load(pos,val,valsize);
 }
 */

}

