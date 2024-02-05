/***************************************************************************
 IOCachedReader.h
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

#ifndef IOCACHEDREADER_H
#define IOCACHEDREADER_H

namespace jsIO {
class IOCachedReader {
public:
  IOCachedReader(const int _fileDescriptor, unsigned long _bufferSize, unsigned long _fileSize);
  ~IOCachedReader();
  bool read(unsigned long _offset, unsigned char *_buffer, long _bufferSize);
  bool setNewFile(const int _fileDescriptor, unsigned long _fileSize);

  // When enabled, the IOCachedReader will make the system disk cache not cache again what it has read itself
  //     static void enableFileUncache(bool _enable);

  unsigned long m_ulDebugReadCounter {};
private:
  unsigned long m_ulBufferSize {};
  unsigned long m_ulUnitSize {};
  unsigned long m_ulBufferOffset {};
  unsigned long m_ulFileSize {};
  int m_nFileDescriptor {};
  bool m_bufferOffsetInit {};
  unsigned char *m_pBuffer {};
  //     static bool s_uncacheEnabled;
  //     FileUncache m_fUncache;
};
}

#endif /* IOREADERHELPER_H_ */
