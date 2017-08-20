/***************************************************************************
    IOCachedReader.cpp
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

#include "IOCachedReader.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "PSProLogging.h"

namespace jsIO
{
  DECLARE_LOGGER(IOCachedReaderLog);

  IOCachedReader::IOCachedReader(const int _fileDescriptor, unsigned long _bufferSize, unsigned long _fileSize )
                                : m_ulDebugReadCounter(0)
                                , m_ulBufferSize(_bufferSize)
                                , m_ulBufferOffset(0)
                                , m_ulFileSize(_fileSize)
                                , m_nFileDescriptor(_fileDescriptor)
                                , m_bufferOffsetInit(false)
                                , m_pBuffer(NULL)

  {
    m_pBuffer = new unsigned char [m_ulBufferSize];
  }
  
  
  IOCachedReader::~IOCachedReader()
  {
    if(m_pBuffer != NULL) delete[] m_pBuffer;
  }

  bool IOCachedReader::setNewFile(const int _fileDescriptor, unsigned long _fileSize)
  {
    m_nFileDescriptor = _fileDescriptor;
    m_ulFileSize = _fileSize;
    m_ulDebugReadCounter=0;
    m_ulBufferOffset=0;
    m_bufferOffsetInit = false;
    return true;
  }


  bool IOCachedReader::read(unsigned long _offset, unsigned char* _buffer, long _bufferSize)
  {
    unsigned long length = _bufferSize;
    unsigned long offset = 0;
    unsigned long readSize = 0;
    if(m_ulBufferSize==0)
    {
      unsigned long  actRead = ::pread(m_nFileDescriptor, _buffer, _bufferSize, _offset );
      if( actRead != _bufferSize )
      {
        return false;
      }
    }
    else
    {
      while( length > 0 )
      {
        if(m_ulFileSize - m_ulBufferOffset == 0 || length > _bufferSize )
        {
          TRACE_VAR4( IOCachedReaderLog, length, m_ulBufferSize, m_ulBufferOffset, _offset );
          return false;
        }
        if( m_bufferOffsetInit == false || _offset  < m_ulBufferOffset || _offset >= m_ulBufferOffset + m_ulBufferSize ) // outside
        {
          m_bufferOffsetInit = true;
          m_ulBufferOffset = _offset;
          m_ulDebugReadCounter++;
          readSize = std::min( m_ulBufferSize, m_ulFileSize - m_ulBufferOffset);
          unsigned long  actRead = ::pread(m_nFileDescriptor, m_pBuffer, readSize, m_ulBufferOffset );
          if( actRead != readSize )
          {
            m_bufferOffsetInit = false;
            return false;
          }
        }
        else if( _offset  >=  m_ulBufferOffset && _offset + length  <  m_ulBufferOffset + m_ulBufferSize ) // inside
        {
          memcpy(_buffer + offset, m_pBuffer + (_offset - m_ulBufferOffset), length );
          length = 0;
        }
        else // right side
        {
          memcpy( _buffer+ offset, m_pBuffer + _offset - m_ulBufferOffset, m_ulBufferOffset + m_ulBufferSize - _offset);
          offset += m_ulBufferOffset + m_ulBufferSize - _offset;
          length = _offset + length - (m_ulBufferOffset + m_ulBufferSize);
          _offset = m_ulBufferOffset + m_ulBufferSize;
        }
      }
    }
    return true;
  }

}

