/***************************************************************************
 VirtualFolders.cpp  -  description
 -------------------
 * This is an abstract base class used to represent a collection of folders
 * that can be used by a JavaSeis dataset.

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

#include "VirtualFolders.h"

#include "PSProLogging.h"

namespace jsIO {
DECLARE_LOGGER(VirtualFoldersLog);

VirtualFolders::~VirtualFolders() {
}

VirtualFolders::VirtualFolders() {
  m_policy = "RANDOM"; //change later if necessary
  m_containsPrecomputedExtents = false;
  m_lastModified = -1;
}

bool VirtualFolders::addFolder(std::string _path) {
  for (int i = 0; i < m_vFolders.size(); i++) {
    if (m_vFolders[i].getPath() == _path) return false;
  }
  m_vFolders.push_back(VirtualFolder(_path));
  TRACE_PRINTF(VirtualFoldersLog, "%s added to the list of Virtual Folders", _path.c_str());
  return true;
}

bool VirtualFolders::removeFolder(std::string _path) {
  for (int i = 0; i < m_vFolders.size(); i++) {
    if (m_vFolders[i].getPath() == _path) {
      m_vFolders.erase(m_vFolders.begin() + i);
      return true;
    }
  }
  return false;
}

// load the list of virtual folders from XML file 
int VirtualFolders::load(std::string _path) {
  std::string VirualFoldersXML = _path + JS_VIRTUAL_FOLDERS_XML;

  //read VirualFoldersXML file into VirualFoldersXMLstring
  std::ifstream ifile(VirualFoldersXML.c_str(), std::ifstream::in);
  if (!ifile.good()) {
    ERROR_PRINTF(VirtualFoldersLog, "Can't open file %s", VirualFoldersXML.c_str());
    return JS_USERERROR;
  }
  // get length of file:
  ifile.seekg(0, std::ios::end);
  long length = ifile.tellg();
  ifile.seekg(0, std::ios::beg);

  char *buffer = new char[length + 1];
  ifile.read(buffer, length);
  ifile.close();
  buffer[length] = '\0';
  std::string VirualFoldersXMLstring(buffer);
  delete[] buffer;
  // ************

  xmlreader reader;

  m_vFolders.clear();

  // Init vFolders
  reader.parse(VirualFoldersXMLstring);
  xmlElement* parSetVirtalFolders = 0;
  parSetVirtalFolders = reader.getBlock("VirtualFolders");
  if (parSetVirtalFolders == 0) {
    ERROR_PRINTF(VirtualFoldersLog, "There is no VirtalFolders part in %s\n", VirualFoldersXML.c_str());
    return JS_USERERROR;
  }

  int NDIR = 0;
  xmlElement* parElement = 0;
  parElement = reader.FirstChildElement(parSetVirtalFolders, "NDIR");
  if (parElement == 0) {
    ERROR_PRINTF(VirtualFoldersLog, "There is no NDIR part in %s\n", VirualFoldersXML.c_str());
    return JS_USERERROR;
  }
  Parameter par;
  reader.load2Parameter(parElement, &par);
  par.valuesAsInts(&NDIR);

//   printf("NDir=%d\n",NDIR);
  TRACE_PRINTF(VirtualFoldersLog, "Number of Virtual Folders = %d", NDIR);

  std::string sDIRtag_simple = "DIR-";
  std::string sDIRtag_ss = "FILESYSTEM-";

  //** find the last 3 subdirectories in jsDataPath, e.g. if jsDataPath=/aaa/bbb/ccc/ddd/eee, then vFolderPathRest=ccc/ddd/eee
  //   vFolderPathRest needed for initalization from SeisSpace generated JavaSeis data, because there in VirualFolders.xml saved 
  //   not the full path but "project" path to which vFolderPathRest must be added to get the full path
  std::string jsDataPathTemp = _path;
  std::string vFolderPathRest = "";
  size_t found;
  found = jsDataPathTemp.find_last_of('/');
  if (found != std::string::npos) vFolderPathRest = jsDataPathTemp.substr(found, jsDataPathTemp.length())
      + vFolderPathRest;
  jsDataPathTemp = jsDataPathTemp.substr(0, found);
  found = jsDataPathTemp.find_last_of('/');
  if (found != std::string::npos) vFolderPathRest = jsDataPathTemp.substr(found, jsDataPathTemp.length())
      + vFolderPathRest;
  jsDataPathTemp = jsDataPathTemp.substr(0, found);
  found = jsDataPathTemp.find_last_of('/');
  if (found != std::string::npos) vFolderPathRest = jsDataPathTemp.substr(found, jsDataPathTemp.length())
      + vFolderPathRest;
  jsDataPathTemp = jsDataPathTemp.substr(0, found);
  found = jsDataPathTemp.find_last_of('/');
  if (found != std::string::npos) vFolderPathRest = jsDataPathTemp.substr(found, jsDataPathTemp.length())
      + vFolderPathRest;
  jsDataPathTemp = jsDataPathTemp.substr(0, found);

  if (NDIR > 0) {
    std::string curVal;
    std::string vFolderPath;
    parElement = reader.FirstChildElement(parSetVirtalFolders);
    int i = 0;
    do {
      reader.load2Parameter(parElement, &par);
      curVal = par.getName();
      if (curVal.substr(0, sDIRtag_simple.length()) == sDIRtag_simple) {
        par.valuesAsStrings(&vFolderPath);
        m_vFolders.push_back(VirtualFolder(vFolderPath));
        TRACE_PRINTF(VirtualFoldersLog, "VirtualFolder[%d]=%s", i, vFolderPath.c_str());
        i++;
      } else if (curVal.substr(0, sDIRtag_ss.length()) == sDIRtag_ss) {
        par.valuesAsStrings(&vFolderPath);
        found = vFolderPath.find(',');
        if (found == std::string::npos) {
          if (vFolderPath[vFolderPath.length() - 1] != '/') vFolderPath.append(1, '/');
          vFolderPath = vFolderPath + vFolderPathRest;
        } else {
          vFolderPath.insert(found, vFolderPathRest);
        }
        // **** when NDIR=1 and Virual Folder path gives as ".", set it to the current directory
        std::string sVF;
        found = vFolderPath.find(vFolderPathRest);
        sVF = vFolderPath.substr(0, found);
        ltrimStr(sVF);
        rtrimStr(sVF);
        if (sVF == ".") vFolderPath = _path;
        // **** 
        m_vFolders.push_back(VirtualFolder(vFolderPath));
        TRACE_PRINTF(VirtualFoldersLog, "VirtualFolder[%d]=%s", i, vFolderPath.c_str());
        i++;
      }
    } while ((parElement = reader.NextSiblingElement(parElement)));
  } else {
    ERROR_PRINTF(VirtualFoldersLog, "Number of Virtual Folders must be greater than 0");
    return JS_USERERROR;
  }
  // ***********
  return JS_OK;
}

//Static delete method to remove the properties file.
bool VirtualFolders::deleteXMLFile(std::string _path) {
  std::string VirualFoldersXML = _path + JS_VIRTUAL_FOLDERS_XML;
  if (remove(VirualFoldersXML.c_str()) != 0) return false;
  else return true;
}

/*
 * Return the list of extent paths so every node will not
 * have to search every virtual folder.  This is extra
 * data and may not be present.
 */
int VirtualFolders::getExtentPathNames(std::string _baseName, std::vector<std::string> &extPaths) {
  std::string traceDataLC, traceHeadersLC;
  std::transform(JS_TRACE_DATA.begin(), JS_TRACE_DATA.end(), traceDataLC.begin(), tolower);
  std::transform(JS_TRACE_HEADERS.begin(), JS_TRACE_HEADERS.end(), traceHeadersLC.begin(), tolower);
  std::transform(_baseName.begin(), _baseName.end(), _baseName.begin(), tolower);

  if (traceDataLC == _baseName) {
    extPaths = m_precomputedTraceExtents;
    return 1;
  } else if (traceHeadersLC == _baseName) {
    extPaths = m_precomputedHeaderExtents;
    return 2;
  } else {
    ERROR_PRINTF(VirtualFoldersLog, "baseName parameter is not valid: %s", _baseName.c_str());
    return JS_USERERROR;
  }
}

/* Insert the precomputed extents.  */
void VirtualFolders::setPreComputedExtents(std::vector<std::string> &_precomputedTraceExtents,
    std::vector<std::string> &_preComputedHeaderExtents) {
  m_precomputedTraceExtents = _precomputedTraceExtents;
  m_precomputedHeaderExtents = _preComputedHeaderExtents;
  m_containsPrecomputedExtents = true;
}

/*
 * Load the exents from disk.  This should only get called from node 0 and
 * allows the node 0 to find all the extents and then share that information
 * with the others, othwerwise when the dataset gets opened all the nodes scan
 * all the secondary folders for extents and this causes IO problems.
 */
int VirtualFolders::findExtents() {
  int numVFolders = m_vFolders.size();

  for (int i = 0; i < numVFolders; i++) {
    int ires = m_vFolders[i].loadExtents(JS_TRACE_DATA, m_precomputedTraceExtents);
    if (ires != JS_OK) {
      return JS_USERERROR;
    }
    ires = m_vFolders[i].loadExtents(JS_TRACE_HEADERS, m_precomputedHeaderExtents);
    if (ires != JS_OK) {
      return JS_USERERROR;
    }

  }
  m_containsPrecomputedExtents = true;

  return JS_OK;
}

//   * Store the state of this object as a properties file.
int VirtualFolders::save(std::string _path) {
  long globalReqFreeSpace = 0;
  int numVF = m_vFolders.size();
  if (numVF == 0) {
    ERROR_PRINTF(VirtualFoldersLog, "Virtual Folders are not initalized yet");
    return JS_USERERROR;
  }

  size_t found;
  std::string vF = "";

  std::string sFirstVF = m_vFolders[0].getPath();
  if (sFirstVF[sFirstVF.length() - 1] != '/') sFirstVF.append(1, '/');
  if (_path[_path.length() - 1] != '/') _path.append(1, '/');

  if (numVF == 1 && sFirstVF == _path) {
    vF = "  <par name=\"FILESYSTEM-0\" type=\"string\"> .,READ_WRITE </par>\n";
  } else {
    std::string vFolder;
    for (int i = 0; i < numVF; i++) {
      vFolder = m_vFolders[i].getPath();
      found = vFolder.find_last_of('/'); // do this because in SeisSpace as VirtualFolder saved notr the whole folder but only "project folder"
      found = vFolder.find_last_of('/', found - 1);
      found = vFolder.find_last_of('/', found - 1);
      vF += "  <par name=\"FILESYSTEM-" + num2Str(i) + "\" type=\"string\"> " + num2Str(vFolder.substr(0, found))
          + ",READ_WRITE </par>\n";
    }
  }

  std::string VFoldersXML =
      "<parset name=\"VirtualFolders\">\n\
        <par name=\"NDIR\" type=\"int\">" + num2Str(numVF) + "</par>\n" + vF
          + "\
        <par name=\"Version\" type=\"string\"> 2006.2 </par>\n\
        <par name=\"Header\" type=\"string\"> \"VFIO org.javaseis.io.VirtualFolder 2006.2\" </par>\n\
        <par name=\"Type\" type=\"string\"> SS </par>\n\
        <par name=\"POLICY_ID\" type=\"string\"> RANDOM </par>\n\
        <par name=\"GLOBAL_REQUIRED_FREE_SPACE\" type=\"long\"> "
          + num2Str(globalReqFreeSpace) + " </par>\n\
        </parset>\n";

  std::string fpath = _path + JS_VIRTUAL_FOLDERS_XML;
  FILE *pfile = fopen(fpath.c_str(), "w");
  if (pfile == NULL) {
    ERROR_PRINTF(VirtualFoldersLog, "Can't open file %s.\n", fpath.c_str());
    return JS_USERERROR;
  }

  fprintf(pfile, "%s", VFoldersXML.c_str());
  fclose(pfile);
  return JS_OK;
}

int VirtualFolders::createFolders() {
  for (int i = 0; i < m_vFolders.size(); i++) {
    int ires = mkdirp(m_vFolders[i].getPath().c_str(), 0777);
    TRACE_PRINTF(VirtualFoldersLog, "Create direcory %s", m_vFolders[i].getPath().c_str());
    if (ires != JS_OK) {
      TRACE_PRINTF(VirtualFoldersLog, "Can't create direcory %s", m_vFolders[i].getPath().c_str());
      return ires;
    }
  }
  return JS_OK;
}

int VirtualFolders::removeFoldersContents() {
  for (int i = 0; i < m_vFolders.size(); i++) {
    TRACE_PRINTF(VirtualFoldersLog, "Remove contents of directory %s", m_vFolders[i].getPath().c_str());
    bool ires = m_vFolders[i].removeDirectoryContent();
    if (ires != true) {
      TRACE_PRINTF(VirtualFoldersLog, "Can't remove contents of direcory %s", m_vFolders[i].getPath().c_str());
      return ires;
    }
  }
  return JS_OK;
}

//recursive mkdir
int VirtualFolders::mkdirp(const char *dir, mode_t mode) {
  char dir_cpy[1024];
  strcpy(dir_cpy, dir);
  char *p = dir_cpy + 1;
  while (*p) {
    if (*p == '/') {
      *((char*) p) = '\0';
      ::mkdir(dir_cpy, mode);
      *((char*) p) = '/';
    }
    p++;

  }
  if (*(p - 1) != '/') if (::mkdir(dir_cpy, mode) == -1 && errno != EEXIST) {
    ERROR_PRINTF(VirtualFoldersLog, "Can't create directory. %s.\n", converErrno2Str(errno).c_str());
    return JS_USERERROR;
  }
  return JS_OK;
}

std::string VirtualFolders::converErrno2Str(int error) {
  switch (error) {
  case EACCES:
    return "The parent directory does not allow write permission to the process, or one of the directories in pathname did not allow search permission.";
  case EEXIST:
    return "pathname already exists (not necessarily as a directory).  This includes the case where pathname is a symbolic link, dangling or not.";
  case EFAULT:
    return "pathname points outside your accessible address space.";
  case ELOOP:
    return "  Too many symbolic links were encountered in resolving pathname.";
  case EMLINK:
    return " The number of links to the parent directory would exceed LINK_MAX.";
  case ENAMETOOLONG:
    return " pathname was too long.";
  case ENOENT:
    return "A directory component in pathname does not exist or is a dangling symbolic link.";
  case ENOMEM:
    return "Insufficient kernel memory was available.";
  case ENOSPC:
    return "The device containing pathname has no room for the new directory or The new directory cannot be created because the user's disk quota is exhausted.";
  case ENOTDIR:
    return "A component used as a directory in pathname is not, in fact, a directory.";
  case EPERM:
    return "  The file system containing pathname does not support the creation of directories.";
  case EROFS:
    return " pathname refers to a file on a read-only file system.";
  }
  return "Unknown error";
}
}
