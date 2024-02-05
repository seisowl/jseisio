/***************************************************************************
 ExtentListEntry.cpp  -  description
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

#include "ExtentListEntry.h"
#include "PSProLogging.h"

namespace jsIO {
DECLARE_LOGGER(ExtentListEntryLog);

bool compare_ExtentListEntry(ExtentListEntry first, ExtentListEntry second) {
  return (first.getIndex() < second.getIndex());
}
;

ExtentListEntry::ExtentListEntry(std::string &_extName, int _extIndex, long _extStartOff, long _extSize,
                                 std::string &_path) {
  Init(_extName, _extIndex, _extStartOff, _extSize, _path);
}

int ExtentListEntry::Init(std::string &_extName, int _extIndex, long _extStartOff, long _extSize, std::string &_path) {
  std::string strerror = "";
  if(_extName.length() == 0) strerror += "Invalid extent name. ";
  if(_extStartOff < 0) strerror += "Invalid starting offset. ";
  if(_extSize < 0) strerror += "Invalid extent size. ";
  if(_path.length() == 0) strerror += "Invalid extent path. ";

  if(strerror != "") {
    ERROR_PRINTF(ExtentListEntryLog, "%s", strerror.c_str());
    return JS_USERERROR;
  }

  extentName = _extName;
  extentIndex = _extIndex;
  extentStartOffset = _extStartOff;
  extentSize = _extSize;
  extentPath = _path;
  return JS_OK;
}

void ExtentListEntry::print_info() const {
  TRACE_PRINTF(ExtentListEntryLog, "Name=%s, Index=%d, StartOffset=%li, Size=%li, Path=%s", extentName.c_str(),
               extentIndex, extentStartOffset, extentSize, extentPath.c_str());
}

long ExtentListEntry::getExtentSizeOnDisk() const {
  std::string fname = extentPath;
  long fsize = 0;

  FILE *pFile = fopen(fname.c_str(), "rb");
  if(pFile != NULL) {
    fseek(pFile, 0, SEEK_END);
    fsize = ftell(pFile);
    fclose(pFile);
  }
  //  printf("fname=%s, fsize=%ld\n",fname.c_str(),fsize);
  return fsize;
}

}

