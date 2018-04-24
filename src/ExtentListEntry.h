/***************************************************************************
 ExtentListEntry.h  -  description
 -------------------
 * This class defines the properties of an individual extent in a virtual file.
 * Each extent has a name, a directory where the extent is located, a starting
 * "global" offset in the virtual file and a size in bytes.

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

#ifndef EXTENDLISTENTRY_H
#define EXTENDLISTENTRY_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "jsDefs.h"

namespace jsIO {
class ExtentListEntry {
public:
  ~ExtentListEntry() {
  }
  ExtentListEntry() {
  }
  ExtentListEntry(std::string &_extName, int _extIndex, long _extStartOff, long _extSize, std::string &_path);
  int Init(std::string &_extName, int _extIndex, long _extStartOff, long _extSize, std::string &_path);

  std::string getPath() const {
    return extentPath;
  }
  int getIndex() const {
    return extentIndex;
  }
  long getStartOffset() const {
    return extentStartOffset;
  }
  void setStartOffset(long _extentStartOffset) {
    extentStartOffset = _extentStartOffset;
  }

  long getExtentSize() const {
    return extentSize;
  }
  void setExtentSize(long _extentSize) {
    extentSize = _extentSize;
  }

  long getExtentSizeOnDisk() const;
  void print_info() const;

private:
  std::string extentName; // Name of this extent
  std::string extentPath; // Path to the extent
  int extentIndex;	// The index of this extent
  long extentStartOffset; // Start offset of this extent
  long extentSize;	// Size in bytes of this extent

public:

protected:

};

bool compare_ExtentListEntry(ExtentListEntry first, ExtentListEntry second);
}

#endif

