/***************************************************************************
 TraceProperties.h  -  description
 -------------------
 The TraceProperites class provides trace properties support for
 JavaSeis datasets.

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

#ifndef TRACEPROPERTIES_H
#define TRACEPROPERTIES_H

#include <map>
#include <utility> // make_pair
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
//

#include "CharBuffer.h"
#include "jsDefs.h"
#include "jsStrDefs.h"
#include "catalogedHdrEntry.h"

namespace jsIO {
class PropertyDescription;

class TraceProperties {
public:
  static const std::string ENTRY;
  static const std::string LABEL;
  static const std::string DESCRIPTION;
  static const std::string FORMAT;
  static const std::string COUNT;
  static const std::string OFFSET;

  static std::vector<PropertyDescription> const &defaultProperties();
  static std::vector<PropertyDescription> const &knownProperties();

public:
  ~TraceProperties();
  TraceProperties();
  TraceProperties(int _numProps, PropertyDescription *_traceProps);

  int Init(int _numProps, PropertyDescription *_traceProps);

  TraceProperties(TraceProperties const &_other);
  TraceProperties &operator =(const TraceProperties &_other) { //Assignment operator
    if(this != &_other) CopyClass(_other);
    return *this;
  }

  void reserveBuffer(long bsize) {
    buffer->reserve(bsize);
  }
  ;
  void resizeBuffer(long bsize) {
    buffer->resize(bsize);
  }
  ;

  void setBuffer(CharBuffer *_buffer);
  void wrapBuffer(char *_buffer, unsigned long _bufsize);

  void copyToBufer(char *_headBuf, int numTraces);

  //    int getBuffer(CharBuffer *buf); // get the pointer to buffer and return the buffer size. Be careful with it!

  //    int loadBuffer(std::string filename);

  int getRecordLength() {
    return recordLength;
  }
  int getNumProperties();

  void addDefaultProperties();
  int addProperty(std::string _label);
  int addProperty(std::string _label, std::string _description, std::string _format, int _count);
  int addProperty(std::string _label, std::string _description, int _format, int _count);

  int load(std::string &XMLstring);
  int save(std::string &XMLstring);

  int saveTraceHeader(std::string filename, int firstTrace, int numOfTraces, const bool append = false);
  int saveTraceHeader(FILE *pFile, int firstTrace, int numOfTraces);

  bool getTraceProperty(std::string key, PropertyDescription &property);
  bool getTraceProperty(int index, PropertyDescription &property);

  unsigned long getBufferPos() {
    return buffer_pos;
  }

  unsigned long getBufferSize() {
    return buffer->size();
  }

  unsigned long getBufferCapacity() {
    return buffer->capacity();
  }

  //headbuf must be pre-allocated with the size = numOfTraces*recordLength
  void getTraceHeader(long firstTrace, long numOfTraces, char *headbuf);

  //headbuf must be filled with the headers of numOfTraces traces
  void swapHeaders(char *headbuf, int numOfTraces);

  catalogedHdrEntry getHdrEntry(std::string _name, bool must_exist = false);
  std::vector<catalogedHdrEntry> getHdrEntries() {
    return hdrEntries;
  }

  //    void setBufferPosition(PropertyDescription &property);
  void setTraceIndex(int _traceIndex) {
    traceIndex = _traceIndex;
  }
  ;
  //    void addTraceProperty(PropertyDescription &property);

  bool exists(std::string key);
  int exists(std::string key, std::vector<PropertyDescription> const &_propList);

  void getTraceProperties(PropertyDescription *&traceProps);

  int getKeyOffset(std::string key); //needed for jsFileWriter:leftJustify

  short putShort(std::string key, short value);
  short getShort(std::string key);
  int putInt(std::string key, int value);
  int getInt(std::string key);
  long putLong(std::string key, long value);
  long getLong(std::string key);
  float putFloat(std::string key, float value);
  float getFloat(std::string key);
  double putDouble(std::string key, double value);
  double getDouble(std::string key);

  void putShortArray(std::string key, short values[]);
  void getShortArray(std::string key, short values[]);
  void putIntArray(std::string key, int values[]);
  void getIntArray(std::string key, int values[]);
  void putLongArray(std::string key, long values[]);
  void getLongArray(std::string key, long values[]);
  void putFloatArray(std::string key, float values[]);
  void getFloatArray(std::string key, float values[]);
  void putDoubleArray(std::string key, double values[]);
  void getDoubleArray(std::string key, double values[]);

  template<typename T> void putValue(std::string key, T value) {
    setBufferPosition(key);
    buffer->put(value);
  }

  template<typename T> void getValue(std::string key, T &value) {
    setBufferPosition(key);
    buffer->get(value);
  }

private:
  int InitHdrEntries();
  void setBufferPosition(std::string key);
  void double2ints(double fx, int &ix, int &scalco);

  void CopyClass(const TraceProperties &Other);

  static std::vector<PropertyDescription> initKnownProperties();
  static std::vector<PropertyDescription> initDefaultProperties();

private:
  std::vector<PropertyDescription> propList;
  std::map<std::string, int> keyMap;

  //this will be initalized automatically and contains "catalogedHdrEntry-equivalent"  elements from propList
  std::vector<catalogedHdrEntry> hdrEntries;

  CharBuffer *buffer;
  bool bset_buffer; //default: false. true if buffer was "set"-ed setBuffer function
  //in that case "delete buffer" will not be called in destructor.

  unsigned long buffer_pos;

  unsigned long traceIndex; //index of currnet trace;
  int recordLength; //the lenght (in bytes) of the current header-words

};
}

#endif

