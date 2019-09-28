/***************************************************************************
 TraceProperties.cpp -  description
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

#include "xmlreader.h"
#include "PropertyDescription.h"

#include "PSProLogging.h"
#include "TraceProperties.h"

namespace jsIO {
DECLARE_LOGGER(TracePropertiesLog);

const std::string TraceProperties::ENTRY = "entry_";
const std::string TraceProperties::LABEL = "label";
const std::string TraceProperties::DESCRIPTION = "description";
const std::string TraceProperties::FORMAT = "format";
const std::string TraceProperties::COUNT = "elementCount";
const std::string TraceProperties::OFFSET = "byteOffset";

std::vector<PropertyDescription> const& TraceProperties::defaultProperties() {
  static std::vector<PropertyDescription> const _(initDefaultProperties());
  return _;
}
std::vector<PropertyDescription> const& TraceProperties::knownProperties() {
  static std::vector<PropertyDescription> const _(initKnownProperties());
  return _;
}

TraceProperties::~TraceProperties() {
  if (!bset_buffer) delete buffer;
}

TraceProperties::TraceProperties() {
  traceIndex = 0;
  recordLength = 0;
  buffer = new CharBuffer;
  bset_buffer = false;
  buffer_pos = 0;
}

void TraceProperties::CopyClass(const TraceProperties & Other) {
  propList = Other.propList;
  keyMap = Other.keyMap;
  bset_buffer = Other.bset_buffer;
  buffer_pos = Other.buffer_pos;
  traceIndex = Other.traceIndex;
  recordLength = Other.recordLength;
  hdrEntries = Other.hdrEntries;
  *buffer = *(Other.buffer);
}

TraceProperties::TraceProperties(TraceProperties const& _other) {
  buffer = new CharBuffer;
  if (this != &_other) CopyClass(_other);
}

int TraceProperties::getNumProperties() {
  return propList.size();
}

TraceProperties::TraceProperties(int _numProps, PropertyDescription *_traceProps) {
  traceIndex = 0;
  recordLength = 0;
  buffer_pos = 0;
  buffer = new CharBuffer;
  Init(_numProps, _traceProps);
}

int TraceProperties::Init(int _numProps, PropertyDescription *_traceProps) {
  if (_numProps < 0) {
    ERROR_PRINTF(TracePropertiesLog, "Number of trace properties must be positive");
    return JS_USERERROR;
  }
  propList.clear();
  keyMap.clear();

  recordLength = 0;

  for (int i = 0; i < _numProps; i++) {
    if (exists(_traceProps[i].getLabel())) continue;
    propList.push_back(_traceProps[i]);
    keyMap.insert(std::pair<std::string, int>(_traceProps[i].getLabel(), _traceProps[i].getOffset()));
    recordLength += _traceProps[i].getRecordLength();
  }

  if (buffer->size() < recordLength) buffer->resize(recordLength);

  InitHdrEntries();

  return JS_OK;
}

void TraceProperties::setBuffer(CharBuffer *_buffer) {
  delete buffer;
  bset_buffer = true;
  buffer = _buffer;
  buffer_pos = 0;
}

/*
 int TraceProperties::getBuffer(CharBuffer *buf){
 buf = &buffer;
 return buffer->size();
 }*/

void TraceProperties::wrapBuffer(char *_buffer, unsigned long _bufsize) {
  buffer->wrap(_buffer, _bufsize);
  buffer_pos = 0;
}

/*
 int TraceProperties::loadBuffer(std::string filename){
 std::ifstream inheaderfile;
 inheaderfile.open(filename.c_str(), std::ifstream::in);
 if(! inheaderfile.good() ) {
 ERROR_PRINTF(TracePropertiesLog, "Could not open file %s\n", filename.c_str());
 return JS_USERERROR;
 }

 inheaderfile.seekg (0, std::ios::end);
 unsigned long length = inheaderfile.tellg();
 inheaderfile.seekg (0, std::ios::beg);

 int Ntraces = (int)(length/recordLength);
 
 buffer->reserve(length);
 buffer->resize(length);

 inheaderfile.read ((char*)&buffer[0],length);
 inheaderfile.close();

 return Ntraces;

 return JS_OK;
 }
 */

int TraceProperties::load(std::string &XMLstring) {
  TRACE_PRINTF(TracePropertiesLog, "loading TraceProperties ...");

  xmlreader reader;
  reader.parse(XMLstring);

  xmlElement* parSetTraceProps = 0;
  parSetTraceProps = reader.getBlock("TraceProperties");

  if (parSetTraceProps == 0) {
    ERROR_PRINTF(TracePropertiesLog, "There is no 'TraceProperties' part in XML string\n");
    return JS_USERERROR;
  }

  xmlElement* parSetEntries = 0;
  int NEntries = 0;
  parSetEntries = reader.FirstChildBlock(parSetTraceProps);
  do {
    const char *str = reader.getAttribute(parSetEntries, "name");
    if (strncmp(str, "entry_", 6) == 0) {
      NEntries++;
    }
  } while ((parSetEntries = reader.NextSiblingElement(parSetEntries)));

  if (NEntries == 0) {
    ERROR_PRINTF(TracePropertiesLog, "There is no 'entry' specified in XML string\n");
    return JS_USERERROR;
  }

  int numProps = NEntries;
  TRACE_PRINTF(TracePropertiesLog, "#Properties=%d", numProps);

  recordLength = 0;
  PropertyDescription traceProperty;
  xmlElement* parElement = 0;
  parSetEntries = reader.FirstChildBlock(parSetTraceProps);
  std::string label = "", description = "", sformat = "";
  int count = 0, offset = 0;
  Parameter par;
  std::string curVal;
  for (int i = 0; i < numProps; i++) {
    const char *str = reader.getAttribute(parSetEntries, "name");
    if (strncmp(str, ENTRY.c_str(), 5) == 0) {
      parElement = reader.FirstChildElement(parSetEntries);
      do {
        int ires = reader.load2Parameter(parElement, &par);
        if (ires != JS_OK) {
          ERROR_PRINTF(TracePropertiesLog, "Error in XML string. Invalid header word description");
          return ires;
        }
        curVal = par.getName();
        if (curVal == LABEL) {
          par.valuesAsStrings(&label);
        } else if (curVal == DESCRIPTION) {
          par.valuesAsStrings(&description);
        } else if (curVal == FORMAT) {
          par.valuesAsStrings(&sformat);
        } else if (curVal == COUNT) {
          par.valuesAsInts(&count);
        } else if (curVal == OFFSET) {
          par.valuesAsInts(&offset);
        }
      } while ((parElement = reader.NextSiblingElement(parElement)));
    }
    traceProperty.set(label, description, sformat, count, offset);
//   printf("read properties =%s,%s,%s,%d,%d\n",label.c_str(), description.c_str(), sformat.c_str(), count, offset);
//        my_map.insert(std::pair<std::string, PropertyDescription>(traceProperty.getLabel(),traceProperty));
    propList.push_back(traceProperty);
    keyMap.insert(std::pair<std::string, int>(traceProperty.getLabel(), traceProperty.getOffset()));
    recordLength += traceProperty.getRecordLength();
    parSetEntries = reader.NextSiblingElement(parSetEntries);
  }
//   printf("NEntries=%d\n",NEntries);

  if (buffer->size() < recordLength) buffer->resize(recordLength);

  InitHdrEntries();

  TRACE_PRINTF(TracePropertiesLog, "TraceProperties loaded successfully.");
  return JS_OK;
}

int TraceProperties::save(std::string &XMLstring) {
  PropertyDescription traceProperty;
  XMLstring = "  <parset name=\"TraceProperties\">\n";

  for (int i = 0; i < propList.size(); i++) {
    XMLstring += "    <parset name=\"" + ENTRY + num2Str(i) + "\">\n";
    XMLstring += "      <par name=\"" + LABEL + "\" type=\"string\"> " + propList[i].getLabel() + " </par>\n";
    if (propList[i].getDescription().c_str()[0]=='"') 
      XMLstring += "      <par name=\"" + DESCRIPTION + "\" type=\"string\"> " + propList[i].getDescription() + " </par>\n";
    else
      XMLstring += "      <par name=\"" + DESCRIPTION + "\" type=\"string\"> \"" + propList[i].getDescription() + "\" </par>\n";
    XMLstring += "      <par name=\"" + FORMAT + "\" type=\"string\"> " + propList[i].getFormatString() + " </par>\n";
    XMLstring += "      <par name=\"" + COUNT + "\" type=\"int\"> " + num2Str(propList[i].getCount()) + " </par>\n";
    XMLstring += "      <par name=\"" + OFFSET + "\" type=\"int\"> " + num2Str(propList[i].getOffset()) + " </par>\n";
    XMLstring += "    </parset>\n";
  }
  XMLstring += "  </parset>\n";

  return 1;
}

/**
 * Gets specified trace property from storage.
 * @param key
 *    The keyword name of the trace property to be returned.
 * @return The trace property.
 */

bool TraceProperties::getTraceProperty(std::string key, PropertyDescription &property) {
  for (int i = 0; i < propList.size(); i++) {
    if (propList[i].getLabel() == key) {
      property = propList[i];
      return true;
    }
  }
  return false;
}

bool TraceProperties::getTraceProperty(int index, PropertyDescription &property) {
  if (index >= 0 && index < propList.size()) {
    property = propList[index];
    return true;
  }
  return false;
}

/**
 * Sets the buffer position, based on the current trace index and specified property keyword.
 * @param key
 *    The keyword name of the trace property.
 */
void TraceProperties::setBufferPosition(std::string key) {
  int offset = keyMap.find(key)->second;
  buffer_pos = (recordLength * traceIndex) + offset;
  buffer->position(buffer_pos);
}

int TraceProperties::getKeyOffset(std::string key) {
  return keyMap.find(key)->second;
}

//
//    public CharBuffer getBuffer() {
//       return _buffer;
//    }

/*
 void TraceProperties::addTraceProperty(PropertyDescription &property) {
 int offset = property.getOffset();
 int length = property.getRecordLength();
 if(offset == property.HDR_OFFSET_UNDEFINED) {
 property.setOffset(recordLength);
 recordLength += length;
 } else {
 if((offset+length) > recordLength) recordLength = offset+length;
 }
 propList.push_back(property);
 }
 */

/**
 * Gets specified trace property from storage.
 * @param key
 *    The keyword name of the trace property to be returned.
 * @return The trace property.
 */
bool TraceProperties::exists(std::string key) {
  int i;
  for (i = 0; i < propList.size(); i++) {
    if (propList[i].getLabel() == key) break;
  }
  return (i != propList.size());
}

//to check whether a property exist in defaultProperties or knownProperties
int TraceProperties::exists(std::string key, std::vector<PropertyDescription> const&_propList) {
  for (int i = 0; i < _propList.size(); i++) {
    if (_propList[i].getLabel() == key) return i;
  }
  return -1;
}

/**
 * Gets the trace properties currently stored.
 * @return The array of trace properties.
 */
void TraceProperties::getTraceProperties(PropertyDescription *& traceProps) {
  int count = getNumProperties();
  if (count > 0) {
    traceProps = new PropertyDescription[count];
    for (int i = 0; i < count; i++) {
      traceProps[i] = propList[i];
    }
  }
}

short TraceProperties::putShort(std::string key, short value) {
  setBufferPosition(key);
  buffer->putShort(value);
  return (value);
}

int TraceProperties::putInt(std::string key, int value) {
  setBufferPosition(key);
  buffer->putInt(value);
  return (value);
}

long TraceProperties::putLong(std::string key, long value) {
  setBufferPosition(key);
  buffer->putLong(value);
  return (value);
}

float TraceProperties::putFloat(std::string key, float value) {
  setBufferPosition(key);
  buffer->putFloat(value);
  return (value);
}

double TraceProperties::putDouble(std::string key, double value) {
  setBufferPosition(key);
  buffer->putDouble(value);
  return (value);
}

short TraceProperties::getShort(std::string key) {
  setBufferPosition(key);
  return buffer->getShort();
}

int TraceProperties::getInt(std::string key) {
  setBufferPosition(key);
  return buffer->getInt();
}

long TraceProperties::getLong(std::string key) {
  setBufferPosition(key);
  return buffer->getLong();
}

float TraceProperties::getFloat(std::string key) {
  setBufferPosition(key);
  return buffer->getFloat();
}

double TraceProperties::getDouble(std::string key) {
  setBufferPosition(key);
  return buffer->getDouble();
}

void TraceProperties::putShortArray(std::string key, short values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    buffer->putShort(values[i]);
  }
}

void TraceProperties::getShortArray(std::string key, short values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    values[i] = buffer->getShort();
  }
}

void TraceProperties::putIntArray(std::string key, int values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    buffer->putInt(values[i]);
  }
}

void TraceProperties::getIntArray(std::string key, int values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    values[i] = buffer->getInt();
  }
}

void TraceProperties::putLongArray(std::string key, long values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    buffer->putLong(values[i]);
  }
}

void TraceProperties::getLongArray(std::string key, long values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    values[i] = buffer->getLong();
  }
}

void TraceProperties::putDoubleArray(std::string key, double values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    buffer->putDouble(values[i]);
  }
}

void TraceProperties::getDoubleArray(std::string key, double values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    values[i] = buffer->getDouble();
  }
}

void TraceProperties::putFloatArray(std::string key, float values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    buffer->putFloat(values[i]);
  }
}

void TraceProperties::getFloatArray(std::string key, float values[]) {
  PropertyDescription property;
  getTraceProperty(key, property);
  setBufferPosition(key);
  int count = property.getCount();
  for (int i = 0; i < count; i++) {
    values[i] = buffer->getFloat();
  }
}

int TraceProperties::saveTraceHeader(std::string filename, int firstTrace, int numOfTraces, const bool append) {
  int NTraces = (int) buffer->size() / recordLength;
  if (firstTrace < 0 || numOfTraces < 0 || firstTrace + numOfTraces > NTraces) {
    ERROR_PRINTF(TracePropertiesLog, "Error: illegal parameeters. It must be %d >=0 , %d>0 and %d+%d<=%d.", firstTrace,
        numOfTraces, firstTrace, numOfTraces, NTraces);
    return JS_USERERROR;
  }

  std::string mode;
  if (!append) mode = "wb";
  else mode = "ab+";

  FILE *pFile = fopen(filename.c_str(), mode.c_str());
  if (pFile == NULL) {
    ERROR_PRINTF(TracePropertiesLog, "Error: Can't open file %s.\n", filename.c_str());
    return JS_USERERROR;
  }
  const char *arr = buffer->array();
  if (nativeOrder() != buffer->getByteOrder()) { //swap endianness
    char *tmpbuf = new char[numOfTraces * recordLength];
    memcpy(tmpbuf, &arr[firstTrace * recordLength], numOfTraces * recordLength);
    int hw_count = getNumProperties();
    for (int i = 0; i < numOfTraces; i++) {
      for (int j = 0; j < hw_count; j++) {
        int hcount = propList[j].getCount();
        int hoffset = propList[j].getOffset();
        int hformatLen = propList[j].getFormatLength();
        endian_swap((void*) &tmpbuf[i * recordLength + hoffset], hcount, hformatLen);
      }
    }
    fwrite(tmpbuf, recordLength, numOfTraces, pFile);
    delete[] tmpbuf;
  } else {
    fwrite(&arr[firstTrace * recordLength], recordLength, numOfTraces, pFile);
  }

  fclose(pFile);
  return JS_OK;
}

int TraceProperties::saveTraceHeader(FILE *pFile, int firstTrace, int numOfTraces) {
  int NTraces = (int) buffer->size() / recordLength;
  if (firstTrace < 0 || numOfTraces < 0 || firstTrace + numOfTraces > NTraces) {
    ERROR_PRINTF(TracePropertiesLog, "Error: illegal parameeters. It must be %d >=0 , %d>0 and %d+%d<=%d.", firstTrace,
        numOfTraces, firstTrace, numOfTraces, NTraces);
    return JS_USERERROR;
  }

  if (pFile == NULL || ferror(pFile)) {
    ERROR_PRINTF(TracePropertiesLog, "Invalid file pointer\n");
    return JS_USERERROR;
  }
  const char *arr = buffer->array();
  if (nativeOrder() != buffer->getByteOrder()) { //swap endianness
    char *tmpbuf = new char[numOfTraces * recordLength];
    memcpy(tmpbuf, &arr[firstTrace * recordLength], numOfTraces * recordLength);
    int hw_count = getNumProperties();
    for (int i = 0; i < numOfTraces; i++) {
      for (int j = 0; j < hw_count; j++) {
        int hcount = propList[j].getCount();
        int hoffset = propList[j].getOffset();
        int hformatLen = propList[j].getFormatLength();
        endian_swap((void*) &tmpbuf[i * recordLength + hoffset], hcount, hformatLen);
      }
    }
    fwrite(tmpbuf, recordLength, numOfTraces, pFile);
    delete[] tmpbuf;
  } else {
    fwrite(&arr[firstTrace * recordLength], recordLength, numOfTraces, pFile);
//     hsize+=recordLength*numOfTraces;
//     TRACE_PRINTF(TracePropertiesLog,"hsize=%ld",hsize);
  }

  return JS_OK;
}

void TraceProperties::getTraceHeader(long firstTrace, long numOfTraces, char *headbuf) {
  const char *buf = buffer->array();
  memcpy(headbuf, &buf[recordLength * firstTrace], numOfTraces * recordLength);
  if (nativeOrder() != buffer->getByteOrder()) {
    int hw_count = getNumProperties();
    for (int i = 0; i < numOfTraces; i++) {
      for (int j = 0; j < hw_count; j++) {
        int hcount = propList[j].getCount();
        int hoffset = propList[j].getOffset();
        int hformatLen = propList[j].getFormatLength();
        endian_swap((void*) &headbuf[i * recordLength + hoffset], hcount, hformatLen);
      }
    }
  }
}

//swap endiannes of #numOfTraces traces in headbuf
void TraceProperties::swapHeaders(char *headbuf, int numOfTraces) {
  int hw_count = getNumProperties();
  for (int i = 0; i < numOfTraces; i++) {
    for (int j = 0; j < hw_count; j++) {
      int hcount = propList[j].getCount();
      int hoffset = propList[j].getOffset();
      int hformatLen = propList[j].getFormatLength();
      endian_swap((void*) &headbuf[i * recordLength + hoffset], hcount, hformatLen);
    }
  }
}

void TraceProperties::double2ints(double fx, int &ix, int &scalco) {
  int isign = 1;
  if (fx < 0) {
    isign = -1;
    fx = -fx;
  }
  ix = (int) fx;
  double d = fx - ix;
  int m;
  scalco = 1;
  while (ix < INT_MAX && d > 1e-7) {
    d *= 10.;
    m = (int) (d + 1e-8);
    d -= m;
    ix = ix * 10 + m;
    scalco *= 10;
  }
  scalco = -scalco;
  ix = isign * ix;
}

void TraceProperties::copyToBufer(char *_headBuf, int numTraces) {
  buffer->insert(0, _headBuf, numTraces * recordLength);
}

int TraceProperties::addProperty(std::string _label) {
  if (exists(_label)) {
    TRACE_PRINTF(TracePropertiesLog, "A property named %s already exist in the property list", _label.c_str());
    return JS_WARNING;
  }

  int ind = exists(_label, knownProperties());
  if (ind == -1) {
    ERROR_PRINTF(TracePropertiesLog, "A property named %s is unknown", _label.c_str());
    return JS_USERERROR;
  }

  PropertyDescription propD = knownProperties()[ind];
  propD.setOffset(recordLength);
//    propD.print_info();
  propList.push_back(propD);
  keyMap.insert(std::pair<std::string, int>(_label, recordLength));

  recordLength += propD.getRecordLength();

  catalogedHdrEntry cHdrEntry;
  cHdrEntry.Init(propD.getLabel(), propD.getDescription(), propD.getFormat(), propD.getCount(), propD.getOffset());
  cHdrEntry.setByteOrder(buffer->getByteOrder());
  hdrEntries.push_back(cHdrEntry);

  if (buffer->size() < recordLength) buffer->resize(recordLength);

  return JS_OK;
}

int TraceProperties::addProperty(std::string _label, std::string _description, std::string _formatstring, int _count) {
  int format = PropertyDescription::getFormat(_formatstring);
  return addProperty(_label, _description, format, _count);
}

int TraceProperties::addProperty(std::string _label, std::string _description, int _format, int _count) {
  if (exists(_label)) {
    TRACE_PRINTF(TracePropertiesLog, "A property named %s already exist in the property list", _label.c_str());
    return JS_WARNING;
  }

  PropertyDescription propD;
  propD.set(_label, _description, _format, _count, recordLength);
  propList.push_back(propD);
  keyMap.insert(std::pair<std::string, int>(_label, recordLength));

  recordLength += propD.getRecordLength();

  catalogedHdrEntry cHdrEntry;
  cHdrEntry.Init(propD.getLabel(), propD.getDescription(), propD.getFormat(), propD.getCount(), propD.getOffset());
  cHdrEntry.setByteOrder(buffer->getByteOrder());
  hdrEntries.push_back(cHdrEntry);

  if (buffer->size() < recordLength) buffer->resize(recordLength);

  return JS_OK;
}

int TraceProperties::InitHdrEntries() {
  int N = propList.size();
  hdrEntries.clear();
  hdrEntries.resize(N);

  for (int i = 0; i < N; i++) {
    hdrEntries[i].Init(propList[i].getLabel(), propList[i].getDescription(), propList[i].getFormat(),
        propList[i].getCount(), propList[i].getOffset());
    hdrEntries[i].setByteOrder(buffer->getByteOrder());
  }

  return N;
}

catalogedHdrEntry TraceProperties::getHdrEntry(std::string _name) {
  int N = hdrEntries.size();

  for (int i = 0; i < N; i++) {
    if (hdrEntries[i].getName() == _name) return hdrEntries[i];
  }
  ERROR_PRINTF(TracePropertiesLog, "There is no header named %s", _name.c_str());
  return catalogedHdrEntry();
}

void TraceProperties::addDefaultProperties() {
  for (int i = 0; i < defaultProperties().size(); i++) {
    addProperty(defaultProperties()[i].getLabel());
  }
}

std::vector<PropertyDescription> TraceProperties::initKnownProperties() {
  std::vector<PropertyDescription> props;

  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TRC_TYPE], JS_HEADER_DESC[JSHDR_TRC_TYPE],
          JS_HEADER_TYPES_INT[JSHDR_TRC_TYPE], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_T0], JS_HEADER_DESC[JSHDR_T0], JS_HEADER_TYPES_INT[JSHDR_T0], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TIME], JS_HEADER_DESC[JSHDR_TIME], JS_HEADER_TYPES_INT[JSHDR_TIME], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_OFFSET], JS_HEADER_DESC[JSHDR_OFFSET],
          JS_HEADER_TYPES_INT[JSHDR_OFFSET], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_OFFSET_BIN], JS_HEADER_DESC[JSHDR_OFFSET_BIN],
          JS_HEADER_TYPES_INT[JSHDR_OFFSET_BIN], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_CDPX], JS_HEADER_DESC[JSHDR_CDPX], JS_HEADER_TYPES_INT[JSHDR_CDPX], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_CDPY], JS_HEADER_DESC[JSHDR_CDPY], JS_HEADER_TYPES_INT[JSHDR_CDPY], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_INLINE_NO], JS_HEADER_DESC[JSHDR_INLINE_NO],
          JS_HEADER_TYPES_INT[JSHDR_INLINE_NO], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_XLINE_NO], JS_HEADER_DESC[JSHDR_XLINE_NO],
          JS_HEADER_TYPES_INT[JSHDR_XLINE_NO], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TFULL_E], JS_HEADER_DESC[JSHDR_TFULL_E],
          JS_HEADER_TYPES_INT[JSHDR_TFULL_E], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TLIVE_E], JS_HEADER_DESC[JSHDR_TLIVE_E],
          JS_HEADER_TYPES_INT[JSHDR_TLIVE_E], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TFULL_S], JS_HEADER_DESC[JSHDR_TFULL_S],
          JS_HEADER_TYPES_INT[JSHDR_TFULL_S], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TLIVE_S], JS_HEADER_DESC[JSHDR_TLIVE_S],
          JS_HEADER_TYPES_INT[JSHDR_TLIVE_S], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_LEN_SURG], JS_HEADER_DESC[JSHDR_LEN_SURG],
          JS_HEADER_TYPES_INT[JSHDR_LEN_SURG], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TOT_STAT], JS_HEADER_DESC[JSHDR_TOT_STAT],
          JS_HEADER_TYPES_INT[JSHDR_TOT_STAT], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_NA_STAT], JS_HEADER_DESC[JSHDR_NA_STAT],
          JS_HEADER_TYPES_INT[JSHDR_NA_STAT], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_AMP_NORM], JS_HEADER_DESC[JSHDR_AMP_NORM],
          JS_HEADER_TYPES_INT[JSHDR_AMP_NORM], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TR_FOLD], JS_HEADER_DESC[JSHDR_TR_FOLD],
          JS_HEADER_TYPES_INT[JSHDR_TR_FOLD], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_SKEWSTAT], JS_HEADER_DESC[JSHDR_SKEWSTAT],
          JS_HEADER_TYPES_INT[JSHDR_SKEWSTAT], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_PAD_TRC], JS_HEADER_DESC[JSHDR_PAD_TRC],
          JS_HEADER_TYPES_INT[JSHDR_PAD_TRC], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_NMO_APLD], JS_HEADER_DESC[JSHDR_NMO_APLD],
          JS_HEADER_TYPES_INT[JSHDR_NMO_APLD], 1));

  return props;
}

std::vector<PropertyDescription> TraceProperties::initDefaultProperties() {
  std::vector<PropertyDescription> props;
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TRC_TYPE], JS_HEADER_DESC[JSHDR_TRC_TYPE],
          JS_HEADER_TYPES_INT[JSHDR_TRC_TYPE], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_T0], JS_HEADER_DESC[JSHDR_T0], JS_HEADER_TYPES_INT[JSHDR_T0], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TIME], JS_HEADER_DESC[JSHDR_TIME], JS_HEADER_TYPES_INT[JSHDR_TIME], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_OFFSET], JS_HEADER_DESC[JSHDR_OFFSET],
          JS_HEADER_TYPES_INT[JSHDR_OFFSET], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_OFFSET_BIN], JS_HEADER_DESC[JSHDR_OFFSET_BIN],
          JS_HEADER_TYPES_INT[JSHDR_OFFSET_BIN], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_CDPX], JS_HEADER_DESC[JSHDR_CDPX], JS_HEADER_TYPES_INT[JSHDR_CDPX], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_CDPY], JS_HEADER_DESC[JSHDR_CDPY], JS_HEADER_TYPES_INT[JSHDR_CDPY], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_INLINE_NO], JS_HEADER_DESC[JSHDR_INLINE_NO],
          JS_HEADER_TYPES_INT[JSHDR_INLINE_NO], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_XLINE_NO], JS_HEADER_DESC[JSHDR_XLINE_NO],
          JS_HEADER_TYPES_INT[JSHDR_XLINE_NO], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TFULL_E], JS_HEADER_DESC[JSHDR_TFULL_E],
          JS_HEADER_TYPES_INT[JSHDR_TFULL_E], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TLIVE_E], JS_HEADER_DESC[JSHDR_TLIVE_E],
          JS_HEADER_TYPES_INT[JSHDR_TLIVE_E], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TFULL_S], JS_HEADER_DESC[JSHDR_TFULL_S],
          JS_HEADER_TYPES_INT[JSHDR_TFULL_S], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TLIVE_S], JS_HEADER_DESC[JSHDR_TLIVE_S],
          JS_HEADER_TYPES_INT[JSHDR_TLIVE_S], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_LEN_SURG], JS_HEADER_DESC[JSHDR_LEN_SURG],
          JS_HEADER_TYPES_INT[JSHDR_LEN_SURG], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TOT_STAT], JS_HEADER_DESC[JSHDR_TOT_STAT],
          JS_HEADER_TYPES_INT[JSHDR_TOT_STAT], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_NA_STAT], JS_HEADER_DESC[JSHDR_NA_STAT],
          JS_HEADER_TYPES_INT[JSHDR_NA_STAT], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_AMP_NORM], JS_HEADER_DESC[JSHDR_AMP_NORM],
          JS_HEADER_TYPES_INT[JSHDR_AMP_NORM], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_TR_FOLD], JS_HEADER_DESC[JSHDR_TR_FOLD],
          JS_HEADER_TYPES_INT[JSHDR_TR_FOLD], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_SKEWSTAT], JS_HEADER_DESC[JSHDR_SKEWSTAT],
          JS_HEADER_TYPES_INT[JSHDR_SKEWSTAT], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_PAD_TRC], JS_HEADER_DESC[JSHDR_PAD_TRC],
          JS_HEADER_TYPES_INT[JSHDR_PAD_TRC], 1));
  props.push_back(
      PropertyDescription(JS_HEADER_NAMES[JSHDR_NMO_APLD], JS_HEADER_DESC[JSHDR_NMO_APLD],
          JS_HEADER_TYPES_INT[JSHDR_NMO_APLD], 1));

  return props;
}
}

