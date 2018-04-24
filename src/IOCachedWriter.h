/***************************************************************************
 IOCachedWriter.h
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

#ifndef IOCACHEDWRITER_H
#define IOCACHEDWRITER_H

namespace jsIO {
// This class helps developer to optimize IO writing routine
class IOCachedWriter {
public:
  IOCachedWriter(const int _fileDescriptor, unsigned long _bufferSize);
  ~IOCachedWriter();

  /*
   * if the internal buffer is full or the  @param _unitOffset is not equal the last internal offset.
   * The internal buffer will written to the disk.
   * return false if not success
   */
  bool write(unsigned long _offset, unsigned char* _buffer, unsigned long _bufferLen);

  bool setNewFileDescriptor(const int _fileDescriptor);

  bool flush();

private:
  unsigned long m_ulBufferSize;
  unsigned long m_ulBufferOffsetBegin;
  unsigned long m_ulBufferOffsetEnd;
  unsigned long m_ulFileSize;
  int m_nFileDescriptor;
  unsigned char *m_pBuffer;
};

}

#endif /* IOWRITERHELPER_H_ */
