/***************************************************************************
                          VirtualFolder.cpp  -  description
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
  
#include "VirtualFolder.h"

#include "PSProLogging.h"
 
 
namespace jsIO
{
  DECLARE_LOGGER(VirtualFolderLog);

  VirtualFolder::~VirtualFolder(){
  }

  VirtualFolder::VirtualFolder(){
    path = ".";
    attrib = READ_WRITE ;
  }


  VirtualFolder::VirtualFolder(std::string &_path) {
    if(_path.length() < 1){
      ERROR_PRINTF(VirtualFolderLog, "A valid path must be specified");
      return;
    }
    size_t found=_path.find(",");
    if (found!=std::string::npos){
      path =  _path.substr(0, found-1);
      std::string sAttr=_path.substr(found+1);
      attrib = str2attrib(sAttr);
    }else{
      path = _path;
      attrib = READ_WRITE ;
    }  

  //If we get a trailing seperatorChar get rid of it
    if(path[path.length()-1] == '/')
      path = path.substr(0, path.length()-1) ;
  }

  void VirtualFolder::setPath(std::string _path)
  {
    path = _path;
    if(path[path.length()-1] == '/')  path = path.substr(0, path.length()-1) ;
  }
  /*
  * Currently equals is only looking at the path information
  * and not other attributes such as RO.
  */
  bool VirtualFolder::equals(VirtualFolder &obj) {
    if(path == obj.getPath())return true ;
    return false ;
  }
  
  /* Helper method to see if the folder is empty */
  int VirtualFolder::count() {
    int i=0;
    DIR *d;
//    struct dirent *dir;
    d = opendir(path.c_str());
    if (d){
//       while ((dir = readdir(d)) != NULL){
      while (readdir(d) != NULL){
        i++;//printf("%s\n", dir->d_name);
      }
      closedir(d);
    }
    return i;
  }
/*
  int VirtualFolder::loadExtents(std::string extBaseName, std::vector<std::string> &extNames, std::vector<long> &extSizes){
  DIR *pDIR;
  struct dirent *entry;
  struct stat filestatus;
  int n=0;
  if( pDIR=opendir(path.c_str()) ){
  while(entry = readdir(pDIR)){
  std::string ename(entry->d_name);
  if(ename.substr(0, extBaseName.length())== extBaseName && //file name begin with extBaseName
  ename.substr(extBaseName.length(), ename.length()).find('.')==std::string::npos) //file name has no extension
  {
  std::string filepath = path+'/'+ename;
  stat( filepath.c_str(), &filestatus );
  long esize = filestatus.st_size;
  extNames.push_back(ename);
  extSizes.push_back(esize);
  n++;
//                          printf("%s , size=%d bytes\n",filepath.c_str(),esize); 
}
}
  closedir(pDIR);
}
  return n;
}
*/
  int VirtualFolder::loadExtents(std::string extBaseName, std::vector<std::string> &extNames){
    DIR *pDIR;
    struct dirent *entry;
    int n=0;
    if((pDIR=opendir(path.c_str())) ){
      while((entry = readdir(pDIR))){
        std::string ename(entry->d_name);
        if(ename.substr(0, extBaseName.length())== extBaseName && //file name begin with extBaseName
           ename.substr(extBaseName.length(), ename.length()).find('.')==std::string::npos) //file name has no extension
        {
          extNames.push_back(ename);
          n++;
//                          printf("%s , size=%d bytes\n",filepath.c_str(),esize); 
        }
      }
      closedir(pDIR);
    }else{
      ERROR_PRINTF(VirtualFolderLog, "Can't open directory : %s",path.c_str());
      return JS_USERERROR;
    }
    return JS_OK;
  }

  bool VirtualFolder::removeDirectoryContent() {
    std::string dpath=path;
    if(dpath[dpath.length()-1]!='/') dpath.append(1,'/');
    DIR *pdir = NULL; 
    pdir = opendir (dpath.c_str());
    struct dirent *pent = NULL;
    if (pdir == NULL) { 
      return false; 
    }
    char file[1024];

    int counter = 1; // use this to skip the first TWO which can cause an infinite loop 
    pent = readdir (pdir);
    while (pent != NULL) { // while there is still something in the directory to list
//       TRACE_PRINTF(VirtualFolderLog, "file : %s", pent->d_name);
      if (strcmp(pent->d_name, ".")!=0 && 
          strcmp(pent->d_name, "..")!=0 &&
          counter > 2)
      {
        memset(file, 0, 1024);
        strcat(file, dpath.c_str());
        strcat(file, pent->d_name); 
        int res = remove(file);
        if(res!=0)
        {
//           ERROR_PRINTF(VirtualFolderLog, "Can't remove file : %s", file);
          closedir (pdir);
          return false;
        }
      }
      counter++;
      pent = readdir (pdir);
    }

    closedir (pdir); // close the directory
    return true;
  }


  std::string attrib2str(Attribute attrib){
    if (attrib==READ_ONLY) return "READ_ONLY";
    else if(attrib == OVERFLOW_ONLY) return "OVERFLOW_ONLY";
    else if(attrib == RETIRED) return "RETIRED";
    else return "READ_WRITE";
  }

  Attribute str2attrib(std::string &str){
    if(str=="READ_ONLY") return READ_ONLY;
    else if(str=="OVERFLOW_ONLY") return OVERFLOW_ONLY;
    else if(str=="RETIRED") return RETIRED;
    else return READ_WRITE;
  }
  
}




