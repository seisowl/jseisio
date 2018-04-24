/***************************************************************************
 VirtualFolder.h  -  description
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

#ifndef VIRTUALFOLDER_H
#define VIRTUALFOLDER_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <dirent.h> 
//to get file size
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "jsDefs.h"
#include "stringfuncs.h"

namespace jsIO {
enum Attribute {
  READ_ONLY, READ_WRITE, OVERFLOW_ONLY, RETIRED
};
Attribute str2attrib(std::string &str);
std::string attrib2str(Attribute attrib);

class VirtualFolder {
public:
  ~VirtualFolder();
  /** No descriptions */
  VirtualFolder();
  VirtualFolder(std::string &_path);

  void addElement(std::string item) {
    path += "/" + item;
  }
  ;
  void setPath(std::string _path);
  std::string getPath() {
    return path;
  }
  ;
  Attribute getAttribute() {
    return attrib;
  }
  ;
  void setAttribute(Attribute _attrib) {
    attrib = _attrib;
  }
  std::string toString() {
    return path + "," + attrib2str(attrib);
  }

//    int loadExtents(std::string extBaseName, std::vector<std::string> &extNames, std::vector<long> &extSizes);
  int loadExtents(std::string extBaseName, std::vector<std::string> &extNames);

  bool equals(VirtualFolder &obj);
  int count();

  bool isReadOnly() {
    return attrib == READ_ONLY;
  }
  ;
  bool isOverFlowOnly() {
    return attrib == OVERFLOW_ONLY;
  }
  ;
  bool isRetired() {
    return attrib == RETIRED;
  }
  ;
  bool isWriteable() {
    return attrib == READ_WRITE;
  }
  ;

  bool removeDirectoryContent();

// private atributes
private:
  std::string path;
  Attribute attrib;

public:

protected:

};

}

#endif

