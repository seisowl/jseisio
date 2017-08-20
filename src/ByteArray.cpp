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

namespace jsIO
{
  ByteArray::~ByteArray(){
    if(!bWrap) delete []buffer;
  }

  ByteArray::ByteArray(){
    buffer = new char[bufInitSize];
    buflen = bufInitSize;
    bufsize = 0;
    bWrap = false;
  }

  void ByteArray::CopyClass(const ByteArray & Other)
  {
    if(!bWrap) delete []buffer;
    bWrap = false;
    buflen = Other.buflen;
    bufsize = Other.bufsize;
    buffer = new char[buflen];
    memcpy(buffer, Other.buffer, bufsize);
  }
 

  void ByteArray::wrap(char *_buffer, unsigned long _bufsize){
    if(!bWrap) delete []buffer;
    buffer = _buffer;
    bufsize = _bufsize;
    buflen = bufsize;
    bWrap = true;
  }

  void ByteArray::setBuffer(char *_buffer, unsigned long _bufsize){
    bWrap = false;
    bufsize = _bufsize;
    buflen = bufsize+bufInitSize;
    delete []buffer;
    buffer =  new char[buflen];
    memcpy(buffer, _buffer, _bufsize);
  }



  void ByteArray::resize(unsigned long newsize){
    if(newsize>buflen){
      char *tmpbuf = new char[bufsize];
      memcpy(tmpbuf,buffer,bufsize);
      delete[]buffer;
      buffer =  new char[newsize+bufInitSize];
      memcpy(buffer,tmpbuf,bufsize);
      memset (&buffer[bufsize], 0, newsize-bufsize);
      delete[]tmpbuf;
      bufsize = newsize;
      buflen = newsize+bufInitSize;
    }else{
      bufsize=newsize;
      memset (&buffer[bufsize], 0, newsize-bufsize);
    }  
  }

  void ByteArray::reserve(unsigned long newlen){
    if(newlen>buflen){
      char *tmpbuf = new char[bufsize];
      memcpy(tmpbuf,buffer,bufsize);
      delete[]buffer;
      buffer =  new char[newlen];
      memcpy(buffer,tmpbuf,bufsize);
      delete[]tmpbuf;
      buflen = newlen;
    }
  }

  void ByteArray::checkMem(unsigned long sizeneeded){
    if(sizeneeded>buflen){
      unsigned long newsize = buflen;
      while(newsize<sizeneeded){
        newsize+=buflen;
      }
      reserve(newsize);
    }
  }

  int ByteArray::swap_endianness(unsigned long start_pos, int n, int nb){
    if(start_pos+n*nb>=bufsize) return JS_USERERROR;
    endian_swap((void*)&buffer[start_pos],n,nb);
    return JS_OK;
  }

  void ByteArray::insert(unsigned long pos, char *val , unsigned long valsize){
    checkMem(pos+valsize);
    memcpy(&buffer[pos], val, valsize);
  }

// void ByteArray::load(unsigned long pos, char *val , unsigned long valsize){
//    memcpy(val, &buffer[pos], valsize);
// }
}


