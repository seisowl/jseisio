/***************************************************************************
 IOCachedWriter.cpp
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

#include "IOCachedWriter.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "PSProLogging.h"
#include "FileUtil.h"

namespace jsIO {
DECLARE_LOGGER(IOCachedWriterLog);

IOCachedWriter::IOCachedWriter(const int _fileDescriptor, unsigned long _bufferSize) :
  m_ulBufferSize(_bufferSize), m_ulBufferOffsetBegin(0), m_ulBufferOffsetEnd(0), m_nFileDescriptor(_fileDescriptor), m_pBuffer(
    NULL) {
  m_pBuffer = new unsigned char[_bufferSize];
}

bool IOCachedWriter::setNewFileDescriptor(const int _fileDescriptor) {
  flush();
  m_nFileDescriptor = _fileDescriptor;
  m_ulBufferOffsetBegin = 0;
  m_ulBufferOffsetEnd = 0;
  return true;
}

IOCachedWriter::~IOCachedWriter() {
  if(m_pBuffer != NULL) delete[] m_pBuffer;
}

bool IOCachedWriter::flush() {
  //   if(m_nFileDescriptor<0) return true;
  long writeSize = m_ulBufferOffsetEnd - m_ulBufferOffsetBegin;
  // if (::pwrite(m_nFileDescriptor, m_pBuffer, writeSize, m_ulBufferOffsetBegin) != writeSize) {
  if(wrapIOFull(pwrite, m_nFileDescriptor, m_pBuffer, writeSize, m_ulBufferOffsetBegin) != writeSize) {
    return false;
  }
  return true;
}

bool IOCachedWriter::write(unsigned long _offset, unsigned char *_buffer, unsigned long _bufferLen) {
  long length = _bufferLen;
  long writtenBytes = 0;
  bool firstCheck = true;
  if(m_ulBufferSize == 0 || _bufferLen > m_ulBufferSize) {
    // if (::pwrite(m_nFileDescriptor, _buffer, _bufferLen, _offset) != _bufferLen) {
    if(wrapIOFull(pwrite, m_nFileDescriptor, _buffer, _bufferLen, _offset) != _bufferLen) {
      return false;
    }
    m_ulBufferOffsetBegin = 0;
    m_ulBufferOffsetEnd = 0;
  } else {
    while(length > 0) {
      if(_offset != m_ulBufferOffsetEnd && firstCheck) { // write the buffer to the file
        if(m_ulBufferOffsetEnd - m_ulBufferOffsetBegin > 0) {
          long writeSize = m_ulBufferOffsetEnd - m_ulBufferOffsetBegin;
          // if (::pwrite(m_nFileDescriptor, m_pBuffer, writeSize, m_ulBufferOffsetBegin) != writeSize) {
          if(wrapIOFull(pwrite, m_nFileDescriptor, m_pBuffer, writeSize, m_ulBufferOffsetBegin) != writeSize) {
            return false;
          }
        }
        m_ulBufferOffsetBegin = _offset;
        memcpy(m_pBuffer, _buffer, length);
        m_ulBufferOffsetEnd = m_ulBufferOffsetBegin + length;
        length = 0;
      } else {
        firstCheck = false;
        if(m_ulBufferOffsetEnd + length > m_ulBufferOffsetBegin + m_ulBufferSize) {
          writtenBytes += m_ulBufferOffsetBegin + m_ulBufferSize - m_ulBufferOffsetEnd;
          memcpy(m_pBuffer + (m_ulBufferOffsetEnd - m_ulBufferOffsetBegin), _buffer, writtenBytes);
          // if (::pwrite(m_nFileDescriptor, m_pBuffer, m_ulBufferSize, m_ulBufferOffsetBegin) != m_ulBufferSize) {
          if(wrapIOFull(pwrite, m_nFileDescriptor, m_pBuffer, m_ulBufferSize, m_ulBufferOffsetBegin) != m_ulBufferSize) {
            return false;
          }

          length -= writtenBytes;
          m_ulBufferOffsetBegin += m_ulBufferSize;
          m_ulBufferOffsetEnd = m_ulBufferOffsetBegin;
        } else {
          memcpy(m_pBuffer + (m_ulBufferOffsetEnd - m_ulBufferOffsetBegin), _buffer + writtenBytes, length);
          m_ulBufferOffsetEnd += length;
          length = 0;
        }
      }
    }
  }
  return true;
}

}

