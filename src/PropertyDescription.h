/***************************************************************************
 PropertyDescription.h  -  description
 -------------------
 A description of trace properties, similiar to the header-words.

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

#ifndef PROPERTYDESCRIPTION_H
#define PROPERTYDESCRIPTION_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "jsDefs.h"

namespace jsIO {
class PropertyDescription {
public:
  static const int c_formatStrings_len = 11;
  static const std::string c_formatStrings[11];

  enum HDR_FORMAT {
    HDR_FORMAT_UNDEFINED = 0,
    HDR_FORMAT_BYTE = 1,
    HDR_FORMAT_SHORT = 2,
    HDR_FORMAT_INTEGER = 3,
    HDR_FORMAT_LONG = 4,
    HDR_FORMAT_FLOAT = 5,
    HDR_FORMAT_DOUBLE = 6,
    HDR_FORMAT_COMPLEX = 7,
    HDR_FORMAT_DCOMPLEX = 8,
    HDR_FORMAT_STRING = 9,
    HDR_FORMAT_BYTESTRING = 10
  };

  static const int HDR_OFFSET_UNDEFINED = -1;

// private atributes
private:

  std::string label;
  std::string description;
  int count;
  int format;
  int formatLength;
  int recordLength;
  int offset;

public:
  ~PropertyDescription() {
  }
  PropertyDescription();

  PropertyDescription(std::string _label, std::string _description, int _format, int _count);
  PropertyDescription(std::string _label, std::string _description, std::string _formatstring, int _count);

  static int getFormat(std::string formatString);
  void set(std::string _label, std::string _description, int _format, int _count, int _offset);
  void set(std::string _label, std::string _description, std::string _formatstring, int _count, int _offset);
  void set(std::string _label, std::string _description, std::string _formatstring, int _count);

  int set(std::string _label, std::string _propertyString);

  std::string getLabel() const {
    return (label);
  }
  std::string getDescription() const {
    return (description);
  }
  int getCount() const {
    return (count);
  }
  int getFormat() const {
    return (format);
  }
  std::string getFormatString() const {
    return (c_formatStrings[format]);
  }
  void setFormat(std::string _sformat);
  void setFormat(int _format) {
    format = _format;
  }
  ;

  int getFormatLength() const {
    return (formatLength);
  }
  int getRecordLength() const {
    return (recordLength);
  }
  int getOffset() const {
    return (offset);
  }
  void setOffset(int _offset) {
    offset = _offset;
  }

  std::string toPropertyString();

  void print_info() const;
private:
  void computeLengths();

};
}

#endif

