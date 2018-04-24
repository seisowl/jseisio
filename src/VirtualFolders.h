/***************************************************************************
 VirtualFolders.h  -  description
 -------------------
 * This class represents a folder that can be used for virtual IO.  

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

#ifndef VIRTUALFOLDERS_H
#define VIRTUALFOLDERS_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include "stringfuncs.h"
#include "xmlreader.h"
#include "jsDefs.h"
#include "jsStrDefs.h"

#include "VirtualFolder.h"

namespace jsIO {
class VirtualFolders {
public:
  ~VirtualFolders();
  /** No descriptions */
  VirtualFolders();

  inline VirtualFolder& operator [](const int& i) {
    return m_vFolders[i];
  }
  ;

  std::vector<VirtualFolder> getFolders() {
    return m_vFolders;
  }
  bool containPrecomputeExtents() {
    return m_containsPrecomputedExtents;
  }
  int count() const {
    return m_vFolders.size();
  }
  void setPreComputedExtents(std::vector<std::string> &_precomputedTraceExtents,
      std::vector<std::string> &_preComputedHeaderExtents);

  int load(std::string _path);
  int save(std::string _path);

  //Must be implemented by extending classes
  std::string getType();

  bool addFolder(std::string _path);
  bool removeFolder(std::string _path);

  static bool deleteXMLFile(std::string _path);
  int getExtentPathNames(std::string _baseName, std::vector<std::string> &extPaths);
  int findExtents();

  int createFolders();
  int removeFoldersContents();

// private atributes
private:
  std::vector<VirtualFolder> m_vFolders;
  std::string m_datasetPath;
  std::string m_policy; //ExtentPolicy m_policy;
  std::vector<std::string> m_precomputedTraceExtents;
  std::vector<std::string> m_precomputedHeaderExtents;
  bool m_containsPrecomputedExtents;
  long m_lastModified;

public:

private:
  int mkdirp(const char *dir, mode_t mode);
  std::string converErrno2Str(int error);
};
}

#endif

