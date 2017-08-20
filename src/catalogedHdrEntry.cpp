/***************************************************************************
                          catalogedHdrEntry.cpp  -  description
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

#include <limits>
#include <string>
#include <string.h>

#include "PropertyDescription.h"
#include "PSProLogging.h"
#include "catalogedHdrEntry.h"

namespace jsIO
{
  DECLARE_LOGGER(catalogedHdrEntryLog);
  
  catalogedHdrEntry::catalogedHdrEntry()
  {
    offset = -1;
    byteOrder = JSIO_LITTLEENDIAN;
    natOrder = nativeOrder();
  }
 
  void catalogedHdrEntry::Init(std::string _name, std::string _description, int _format, int _count,  int _offset)
  {
    name = _name;
    description = _description;
    format = _format;
    count = _count;
    offset = _offset;
  }
 
  float catalogedHdrEntry::getFloatVal(char * headerBuf)
  {
    if(format==PropertyDescription::HDR_FORMAT_FLOAT){
      if(byteOrder!=natOrder) endian_swap (&headerBuf[offset], 1, sizeof(float));
      return *(reinterpret_cast<float*>( &headerBuf[offset]));
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to read non-float header as a float");
      return std::numeric_limits<float>::max();
    }  
  }

  double catalogedHdrEntry::getDoubleVal(char * headerBuf)
  {
    if(format==PropertyDescription::HDR_FORMAT_DOUBLE){
      if(byteOrder!=natOrder) endian_swap (&headerBuf[offset], 1, sizeof(double));
      return *(reinterpret_cast<double*>( &headerBuf[offset]));
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to read non-double header as a double");
      return std::numeric_limits<double>::max();
    }  
  }

  long catalogedHdrEntry::getLongVal(char * headerBuf)
  {
    if(format==PropertyDescription::HDR_FORMAT_LONG){
      if(byteOrder!=natOrder) endian_swap (&headerBuf[offset], 1, sizeof(long));
      return *(reinterpret_cast<long*>( &headerBuf[offset]));
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to read non-long header as a long");
      return std::numeric_limits<long>::max();
    }  
  }

  int catalogedHdrEntry::getIntVal(char * headerBuf)
  {
    if(format==PropertyDescription::HDR_FORMAT_INTEGER){
      if(byteOrder!=natOrder) endian_swap (&headerBuf[offset], 1, sizeof(int));
      return *(reinterpret_cast<int*>( &headerBuf[offset]));
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to read non-int header as an int %d", format);
      return std::numeric_limits<int>::max();
    }  
  }

  short catalogedHdrEntry::getShortVal(char * headerBuf)
  {
    if(format==PropertyDescription::HDR_FORMAT_SHORT){
      if(byteOrder!=natOrder) endian_swap (&headerBuf[offset], 1, sizeof(short));
      return *(reinterpret_cast<short*>( &headerBuf[offset]));
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to read non-short header as a short");
      return std::numeric_limits<short>::max();
    }  
  }


  int catalogedHdrEntry::setFloatVal(char * headerBuf, float val)
  {
    if(format==PropertyDescription::HDR_FORMAT_FLOAT){
      if(byteOrder!=natOrder) endian_swap (&val, 1, sizeof(float));
      char *pV=reinterpret_cast<char*>(&val);
      memcpy(&headerBuf[offset], pV, sizeof(float));
      return JS_OK;
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to write float value in a non-float header %s", name.c_str());
      return JS_USERERROR;
    }  
  }

  int catalogedHdrEntry::setDoubleVal(char * headerBuf, double val)
  {
    if(format==PropertyDescription::HDR_FORMAT_DOUBLE){
      if(byteOrder!=natOrder) endian_swap (&val, 1, sizeof(double));
      char *pV=reinterpret_cast<char*>(&val);
      memcpy(&headerBuf[offset], pV, sizeof(double));
      return JS_OK;
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to write double value in a non-double header %s", name.c_str());
      return JS_USERERROR;
    }  
  }

  int catalogedHdrEntry::setLongVal(char * headerBuf, long val)
  {
    if(format==PropertyDescription::HDR_FORMAT_LONG){
      if(byteOrder!=natOrder) endian_swap (&val, 1, sizeof(long));
      char *pV=reinterpret_cast<char*>(&val);
      memcpy(&headerBuf[offset], pV, sizeof(long));
      return JS_OK;
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to write long value in a non-long header %s", name.c_str());
      return JS_USERERROR;
    }  
  }

  int catalogedHdrEntry::setIntVal(char * headerBuf, int val)
  {
    if(format==PropertyDescription::HDR_FORMAT_INTEGER){
      if(byteOrder!=natOrder) endian_swap (&val, 1, sizeof(int));
      char *pV=reinterpret_cast<char*>(&val);
      memcpy(&headerBuf[offset], pV, sizeof(int));
      return JS_OK;
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to write int value in a non-int header %s", name.c_str());
      return JS_USERERROR;
    }  
  }

  int catalogedHdrEntry::setShortVal(char * headerBuf, short val)
  {
    if(format==PropertyDescription::HDR_FORMAT_SHORT){
      if(byteOrder!=natOrder) endian_swap (&val, 1, sizeof(short));
      char *pV=reinterpret_cast<char*>(&val);
      memcpy(&headerBuf[offset], pV, sizeof(short));
      return JS_OK;
    }  
    else{
      ERROR_PRINTF( catalogedHdrEntryLog, "You are trying to write short value in a non-short header %s", name.c_str());;
      return JS_USERERROR;
    }
  }

  int catalogedHdrEntry::getByteCount()
  {
    switch(format)
    {
      case PropertyDescription::HDR_FORMAT_BYTE:
        return 1;
      case PropertyDescription::HDR_FORMAT_SHORT:
        return 2;
      case PropertyDescription::HDR_FORMAT_INTEGER:
        return 4;
      case PropertyDescription::HDR_FORMAT_LONG:
        return 8;
      case PropertyDescription::HDR_FORMAT_FLOAT:
        return 4;
      case PropertyDescription::HDR_FORMAT_DOUBLE:
        return 8;
    }
    return 0;
  }

  std::string catalogedHdrEntry::getFormatAsStr()
  {
    switch(format)
    {
      case PropertyDescription::HDR_FORMAT_BYTE:
        return "char";
      case PropertyDescription::HDR_FORMAT_SHORT:
        return "short";
      case PropertyDescription::HDR_FORMAT_INTEGER:
        return "int32";
      case PropertyDescription::HDR_FORMAT_LONG:
        return "int64";
      case PropertyDescription::HDR_FORMAT_FLOAT:
        return "float";
      case PropertyDescription::HDR_FORMAT_DOUBLE:
        return "double";
    }
    return "undefined";
  }
}


