/***************************************************************************
 ExtentList.h  -  description
 -------------------
 * This class contains a collection of extents for use by the virtual io classes.  This
 * class manages the addition and or discovery of new extents.

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

#ifndef EXTENDLIST_H
#define EXTENDLIST_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <algorithm>

// #include "VirtualFolders.h"
#include "VirtualFolder.h"
#include "VirtualFolders.h"

#include "ExtentListEntry.h"
#include "stringfuncs.h"
#include "xmlreader.h"
#include "jsDefs.h"

namespace jsIO {
class ExtentList {
public:
  ~ExtentList() {
  }
  ExtentList() :
      bInit(false) {
  }
  ExtentList(std::string _extBaseName, int _numExtents, long _maxFilePosition, long _extentSize,
      VirtualFolders _vFolders);
  int Init(std::string _extBaseName, int _numExtents, long _maxFilePosition, long _extentSize,
      VirtualFolders _vFolders);
  int InitFromXML(const std::string &jsDataPath, std::string ExtentManagerXML);

  void setVirtualFolders(VirtualFolders &_vFolders) {
    vFolders = _vFolders;
  }
  VirtualFolders getVirtualFolders() {
    return vFolders;
  }

  inline ExtentListEntry& operator [](int i) {
    return extents[i];
  }
  ;

  std::vector<ExtentListEntry> getExtents() {
    return extents;
  }
  ;
  std::string getExtentBase() {
    return extBaseName;
  }
  ;
  void setExtentBase(std::string _extBaseName) {
    extBaseName = _extBaseName;
  }
  ;
  void addFolder(std::string _path);

  int getExtent(int index, ExtentListEntry *ext) const;

  int createExtents();
  int loadExtents();
  int getExtentIndex(long position) const;

  int saveXML(std::string _path); //save to XML

  int getExtentInfoForFrame(long glbOffset, int &extIndex, long &locOffset) const;
  std::string getExtentPath(int index) const;
  int getNumExtents() const {
    return numExtents;
  }

  long getSumOfExtSizes() const;
  int getNumvFolders() const {
    return vFolders.count();
  }
  ;
  static long computeExtentSize(long fileLength, int numExtents, long frameSize);

public:
  static const std::string sVirtualFolders;
  static const std::string sNDIR;
  static const std::string sDIRtag_simple;
  static const std::string sDIRtag_ss;

  static const std::string sVersion;
  static const std::string sHeader;
  static const std::string sType;
  static const std::string sPOLICY_ID;
  static const std::string sGLOBAL_REQUIRED_FREE_SPACE;

  static const std::string sVFIO_VERSION;
  static const std::string sVFIO_EXTSIZE;
  static const std::string sVFIO_MAXFILE;
  static const std::string sVFIO_MAXPOS;
  static const std::string sVFIO_EXTNAME;
  static const std::string sVFIO_POLICY;

  static const std::string sVersion_val;
  static const std::string sHeader_val;
  static const std::string sType_val;
  static const std::string sPOLICY_ID_val;

  static const std::string sVFIO_VERSION_val;
  static const std::string sVFIO_POLICY_val;

private:
  std::vector<ExtentListEntry> extents;  // The extents
  int numExtents;              // The number of extents in the dataset
  long maxFilePosition;        // The size of the dataset
  std::string extBaseName;         // The basename of the extents (TraceFile, TraceHeaders)
  VirtualFolders vFolders; // The collection of directories where extents reside
  long extentSize;        // The max size of an extent
  bool bInit;
};
}

#endif

