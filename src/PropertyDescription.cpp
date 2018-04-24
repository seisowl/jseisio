/***************************************************************************
 PropertyDescription.cpp -  description
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

#include "PropertyDescription.h"

#include "PSProLogging.h"

namespace jsIO {
DECLARE_LOGGER(PropertyDescriptionLog);

const std::string PropertyDescription::c_formatStrings[] = { "UNKNOWN", "BYTE", "SHORT", "INTEGER", "LONG", "FLOAT",
    "DOUBLE", "COMPLEX", "DCOMPLEX", "STRING", "BYTESTRING" };

PropertyDescription::PropertyDescription() {
  label = "";
  description = "";
  count = 0;
  recordLength = 0;
  offset = HDR_OFFSET_UNDEFINED;

}

PropertyDescription::PropertyDescription(std::string _label, std::string _description, int _format, int _count) {
  set(_label, _description, _format, _count, HDR_OFFSET_UNDEFINED);
}

PropertyDescription::PropertyDescription(std::string _label, std::string _description, std::string _formatstring,
    int _count) {
  int _format = getFormat(_formatstring);
  set(_label, _description, _format, _count, HDR_OFFSET_UNDEFINED);
}

void PropertyDescription::set(std::string _label, std::string _description, std::string _formatstring, int _count) {
  int _format = getFormat(_formatstring);
  set(_label, _description, _format, _count, HDR_OFFSET_UNDEFINED);
}

void PropertyDescription::set(std::string _label, std::string _description, int _format, int _count, int _offset) {
  label = _label;
  description = _description;
  format = _format;
  count = _count;
  formatLength = 0;
  recordLength = 0;
  offset = _offset;
  computeLengths();
}

void PropertyDescription::set(std::string _label, std::string _description, std::string _formatstring, int _count,
    int _offset) {
  int _format = getFormat(_formatstring);
  set(_label, _description, _format, _count, _offset);
}

void PropertyDescription::computeLengths() {
  recordLength = 0;
  formatLength = 0;
  switch (format) {
  case HDR_FORMAT_BYTE:
    formatLength = 1;
    break;
  case HDR_FORMAT_STRING:
    formatLength = 2;
    break;
  case HDR_FORMAT_BYTESTRING:
    formatLength = 1;
    break;
  case HDR_FORMAT_SHORT:
    formatLength = 2;
    break;
  case HDR_FORMAT_INTEGER:
  case HDR_FORMAT_FLOAT:
    formatLength = 4;
    break;
  case HDR_FORMAT_LONG:
  case HDR_FORMAT_DOUBLE:
  case HDR_FORMAT_COMPLEX:
    formatLength = 8;
    break;
  case HDR_FORMAT_DCOMPLEX:
    formatLength = 16;
    break;
  default:
    ;
  }
  recordLength = formatLength * count;
}

int PropertyDescription::getFormat(std::string formatString) {
  for (int i = 0; i < c_formatStrings_len; i++) {
    if (c_formatStrings[i] == formatString) return i;
  }
  return -1;
}

int PropertyDescription::set(std::string _label, std::string _propertyString) {
  std::string str_error = "Could not parse property string: '" + _propertyString + "' for label='" + _label
      + "' (expecting at least 4 values)";
  label = _label;
  size_t found, found_old;
  found = _propertyString.find(';');
  if (found == std::string::npos) {
    ERROR_PRINTF(PropertyDescriptionLog, "%s", str_error.c_str());
    return JS_USERERROR;
  }
  description = _propertyString.substr(0, found - 1);
  found_old = found;

  found = _propertyString.find(';', found_old + 1);
  if (found == std::string::npos) {
    ERROR_PRINTF(PropertyDescriptionLog, "%s", str_error.c_str());
    return JS_USERERROR;
  }
  std::string format_str = _propertyString.substr(found_old + 1, found - 1);
  format = 0;
  for (int i = 1; i < c_formatStrings_len; i++) {
    if (format_str == c_formatStrings[i]) {
      format = i;
      break;
    }
  }

  if (format == 0) {
    ERROR_PRINTF(PropertyDescriptionLog, "Could not match property format: %s", format_str.c_str());
    return JS_USERERROR;
  }

  found = _propertyString.find(';', found_old + 1);
  if (found == std::string::npos) throw str_error;
  count = atoi(_propertyString.substr(found_old + 1, found - 1).c_str());

  found = _propertyString.find(';', found_old + 1);
  if (found == std::string::npos) throw str_error;
  offset = atoi(_propertyString.substr(found_old + 1, found - 1).c_str());

  computeLengths();

  return JS_OK;
}

std::string PropertyDescription::toPropertyString() {
  std::string buffer = description;
  buffer.append(";");
  buffer.append(c_formatStrings[format]);
  buffer.append(";");
  char str[30];
  sprintf(str, "%d;%d", count, offset);
  buffer.append(str);
  return buffer;
}

void PropertyDescription::setFormat(std::string _sformat) {
  format = 0;
  for (int i = 0; i < c_formatStrings_len; i++) {
    if (_sformat.find(c_formatStrings[i]) != std::string::npos) {
      format = i;
      return;
    }
  }
}

void PropertyDescription::print_info() const {
  printf("Label:%s , Description=%s, count=%d, format=%d, formatLenght=%d, recordLength=%d, offset=%d\n", label.c_str(),
      description.c_str(), count, format, formatLength, recordLength, offset);
}

}

