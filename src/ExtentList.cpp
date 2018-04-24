/***************************************************************************
 ExtentList.cpp  -  description
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

#include "ExtentList.h"

#include "PSProLogging.h"

namespace jsIO {
DECLARE_LOGGER(ExtentListLog);

const std::string ExtentList::sVirtualFolders = "VirtualFolders";
const std::string ExtentList::sNDIR = "NDIR";
const std::string ExtentList::sDIRtag_simple = "DIR-";
const std::string ExtentList::sDIRtag_ss = "FILESYSTEM-";

const std::string ExtentList::sVersion = "Version";
const std::string ExtentList::sHeader = "Header";
const std::string ExtentList::sType = "Type";
const std::string ExtentList::sPOLICY_ID = "POLICY_ID";
const std::string ExtentList::sGLOBAL_REQUIRED_FREE_SPACE = "GLOBAL_REQUIRED_FREE_SPACE";

const std::string ExtentList::sVFIO_VERSION = "VFIO_VERSION";
const std::string ExtentList::sVFIO_EXTSIZE = "VFIO_EXTSIZE";
const std::string ExtentList::sVFIO_MAXFILE = "VFIO_MAXFILE";
const std::string ExtentList::sVFIO_MAXPOS = "VFIO_MAXPOS";
const std::string ExtentList::sVFIO_EXTNAME = "VFIO_EXTNAME";
const std::string ExtentList::sVFIO_POLICY = "VFIO_POLICY";

const std::string ExtentList::sVersion_val = "2006.2";
const std::string ExtentList::sHeader_val = "VFIO org.javaseis.io.VirtualFolder 2006.2";
const std::string ExtentList::sType_val = "SS";
const std::string ExtentList::sPOLICY_ID_val = "RANDOM";

const std::string ExtentList::sVFIO_VERSION_val = "2006.2";
const std::string ExtentList::sVFIO_POLICY_val = "RANDOM";

ExtentList::ExtentList(std::string _extBaseName, int _numExtents, long _maxFilePosition, long _extentSize,
    VirtualFolders _vFolders) {
  Init(_extBaseName, _numExtents, _maxFilePosition, _extentSize, _vFolders);
}

int ExtentList::Init(std::string _extBaseName, int _numExtents, long _maxFilePosition, long _extentSize,
    VirtualFolders _vFolders) {
  if (_extBaseName.length() == 0) {
    ERROR_PRINTF(ExtentListLog, "Invalid baseName for the extents");
    return JS_USERERROR;
  }
  if (_maxFilePosition <= 0) {
    ERROR_PRINTF(ExtentListLog, "maxFilePosition must be greater than 0");
    return JS_USERERROR;
  }
  if (_extentSize <= 0) {
    ERROR_PRINTF(ExtentListLog, "extentSize must be greater than 0");
    return JS_USERERROR;
  }

  numExtents = _numExtents;
  maxFilePosition = _maxFilePosition;
  extBaseName = _extBaseName;
  vFolders = _vFolders;
  extentSize = _extentSize;

  int numVFolders = vFolders.count();
  int numExtInEachFolder = (int) (numExtents / numVFolders);
  int numExtInFirstFolder = numExtInEachFolder + (numExtents - numExtInEachFolder * numVFolders);

//     printf(" numVFolders=%d, numExtInEachFolder=%d, numExtInFirstFolder=%d\n",numVFolders, numExtInEachFolder, numExtInFirstFolder);

  extents.clear();
  extents.resize(numExtents);
  long startOffset = 0;

  int extInd = 0;
  for (int j = 0; j < numExtInFirstFolder; j++) {
    std::string extName = extBaseName + num2Str(extInd);
    std::string extPath = vFolders[0].getPath() + "/" + extName;
    extents[extInd].Init(extName, extInd, startOffset, extentSize, extPath);
    TRACE_PRINTF(ExtentListLog, "Extent Info: Name=%s, Ind=%d, startOffset=%lu, Size=%lu, Path=%s", extName.c_str(),
        extInd, startOffset, extentSize, extPath.c_str());
    startOffset += extentSize;
    extInd++;
  }

  for (int i = 1; i < numVFolders; i++) {
    for (int j = 0; j < numExtInEachFolder; j++) {
      std::string extName = extBaseName + num2Str(extInd);
      std::string extPath = vFolders[i].getPath() + "/" + extName;
      extents[extInd].Init(extName, extInd, startOffset, extentSize, extPath);
      TRACE_PRINTF(ExtentListLog, "Extent Info: Name=%s, Ind=%d, startOffset=%lu, Size=%lu, Path=%s", extName.c_str(),
          extInd, startOffset, extentSize, extPath.c_str());
      startOffset += extentSize;
      extInd++;
    }
  }

  return JS_OK;
}

void ExtentList::addFolder(std::string _path) {
  vFolders.addFolder(_path);
}

int ExtentList::getExtent(int index, ExtentListEntry *ext) const {
  if (index < 0 || index > numExtents) {
    ERROR_PRINTF(ExtentListLog, "Extent: with an index %d  does not exist", index);
    return JS_USERERROR;
  }
  *ext = extents[index];
  return JS_OK;
}

int ExtentList::createExtents() {
  return 0;
}

int ExtentList::loadExtents() {
  extents.clear();
  int numVFolders = getNumvFolders();
  std::vector<std::string> extNames;
  ExtentListEntry exEntry;

  std::string extName;
  int extIndex;
  long extStartOff;
  std::string path;

  for (int i = 0; i < numVFolders; i++) {
    extNames.clear();
    int ires = vFolders[i].loadExtents(extBaseName, extNames);
    if (ires != JS_OK) {
      return JS_USERERROR;
    }
    for (int j = 0; j < extNames.size(); j++) {
      extName = extNames[j];
      int ipos = extBaseName.length();
      int ilen = extName.length() - extBaseName.length();
      extIndex = atoi(extName.substr(ipos, ilen).c_str());
      extStartOff = 0; //will be initialized after sorting
      path = vFolders[i].getPath() + '/' + extName;
      exEntry.Init(extName, extIndex, extStartOff, extentSize, path);
      extents.push_back(exEntry);
    }
  }
  int numExtentsFound = extents.size();
  if (numExtentsFound == 0) {
    TRACE_PRINTF(ExtentListLog, " WARNING: no %s extents found in Virtual Folders", extBaseName.c_str());
  }

  //sort and init extStartOffsets
  sort(extents.begin(), extents.end(), compare_ExtentListEntry);

  //remove from a list "wrong" extens (for regual case it should do nothing)
  //i.e. if numExtents for TraceFile(s) is 8 but in Virtaul Folders 
  //there was a file named TraceFile15, it will be removed from the list
  //since it does not belong to the data 
  for (int i = 0; i < numExtentsFound; i++) {
    extIndex = extents[i].getIndex();
    if (extIndex >= numExtents) {
      extents.erase(extents.begin() + i);
      i--;
      numExtentsFound--;
    }
  }

  //set start offsets
  for (int i = 0; i < numExtents; i++) {
    extIndex = -1;
    if (i < numExtentsFound) extIndex = extents[i].getIndex(); //normally it must be equal to i, however in cases where
    //	i.e. all frames belonging to tracefile7 are empty tracefile7 does't exist on the disk, so
    //	index of extents[6] is not 6 but 7. In this case create dummy extent with index 6
    if (extIndex != i) {
      extName = extBaseName + num2Str(i);
      path = vFolders[0].getPath() + '/' + extName;
      extName = extName + "_NOT_FOUND"; //dummy extent name
      exEntry.Init(extName, i, extentSize * i, extentSize, path);
      extents.insert(extents.begin() + i, exEntry);
    } else extents[i].setStartOffset(extentSize * extIndex);
  }

  /*
   //set start offsets
   for (int i=0; i<numExtentsFound; i++ ){
   extIndex = extents[i].getIndex();//normally it must be equal to i, however in cases where
   //	i.e. all frames belonging to tracefile7 are empty tracefile7 does't exist on the disk, so
   //	index of extents[6] is not 6 but 7
   extents[i].setStartOffset(extentSize*extIndex);
   }
   */
  if (numExtentsFound > 0) {
    long lastExtentSize = maxFilePosition - extents[numExtentsFound - 1].getStartOffset();
    extents[numExtentsFound - 1].setExtentSize(lastExtentSize);
  }
//   if(numExtents>0) maxFilePosition = extents[numExtents-1].getStartOffset()+extents[numExtents-1].getExtentSize();

  TRACE_PRINTF(ExtentListLog, "List of %s Extents . Found: %d, Max:%d", extBaseName.c_str(), numExtentsFound,
      numExtents);

  for (int i = 0; i < numExtents; i++) {
    extents[i].print_info();
  }

  return JS_OK;
}

/*
 * The position in the global virtual file is converted to the
 * appropriate extent based on extentSize.  This implies that
 * all extents are the same size and the extents are in order.
 */
int ExtentList::getExtentIndex(long position) const {
  if (position < 0 || position > maxFilePosition) {
    ERROR_PRINTF(ExtentListLog, "Requested position %li is invalid", position);
    return JS_USERERROR;
  }
  int index = 0;
  while (position > extents[index].getStartOffset() && index < numExtents) {
    index++;
  }
  index--;
  return index;
}

// The the path based on the extent index 
std::string ExtentList::getExtentPath(int index) const {
  if (index < 0 || index >= numExtents) return "";
  return extents[index].getPath();
}

int ExtentList::InitFromXML(const std::string &jsDataPath, std::string ExtentManagerXML) {

  int ires;
  if (vFolders.count() == 0) {
    TRACE_PRINTF(ExtentListLog, "VirtualFolders are not set. Trying to load.");
    ires = vFolders.load(jsDataPath);
    if (ires < 0) {
      ERROR_PRINTF(ExtentListLog, "Unable to load VirtualFolders from  %s\n", jsDataPath.c_str());
      return JS_USERERROR;
    }
  }

  xmlreader reader;
  Parameter par;

  ExtentManagerXML = jsDataPath + ExtentManagerXML;

//read ExtentManagerXML file into ExtentManagerXMLstring
  std::ifstream ifile;
  ifile.open(ExtentManagerXML.c_str(), std::ifstream::in);
  if (!ifile.good()) {
    ERROR_PRINTF(ExtentListLog, "Can't open file %s", ExtentManagerXML.c_str());
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
  std::string ExtentManagerXMLstring(buffer);
//   printf("length=%d\n*********\n%s\n*********\n",length,buffer);
  delete[] buffer;
  // ************

  // Init Extents
  reader.clear();
  reader.parse(ExtentManagerXMLstring);
  xmlElement* parSetExtentManager = 0;
  parSetExtentManager = reader.getBlock("ExtentManager");
  if (parSetExtentManager == 0) {
    ERROR_PRINTF(ExtentListLog, "There is no ExtentManager part in %s", ExtentManagerXML.c_str());
    return JS_USERERROR;
  }

//   int numExt=0;
  xmlElement* parNumExt = 0;
  xmlElement* parExtSize = 0;
  xmlElement* parExtBaseName = 0;
  xmlElement* parMaxFilePos = 0;

  parNumExt = reader.FirstChildElement(parSetExtentManager, sVFIO_MAXFILE);
  parExtSize = reader.FirstChildElement(parSetExtentManager, sVFIO_EXTSIZE);
  parExtBaseName = reader.FirstChildElement(parSetExtentManager, sVFIO_EXTNAME);
  parMaxFilePos = reader.FirstChildElement(parSetExtentManager, sVFIO_MAXPOS);

  if (parNumExt == 0 || parExtSize == 0 || parExtBaseName == 0 || parMaxFilePos == 0) {
    ERROR_PRINTF(ExtentListLog, "Error in XML file %s. Some of these %s, %s, %s, %s tags are missing.",
        ExtentManagerXML.c_str(), sVFIO_EXTSIZE.c_str(), sVFIO_MAXFILE.c_str(), sVFIO_EXTNAME.c_str(),
        sVFIO_MAXPOS.c_str());
    return JS_USERERROR;
  }

  reader.load2Parameter(parNumExt, &par);
//   par.valuesAsInts(&numExt);
  par.valuesAsInts(&numExtents);

  reader.load2Parameter(parMaxFilePos, &par);
  par.valuesAsLongs(&maxFilePosition);
//   printf("maxFilePosition=%ld\n", maxFilePosition);

  reader.load2Parameter(parExtSize, &par);
  par.valuesAsLongs(&extentSize);

  reader.load2Parameter(parExtBaseName, &par);
  par.valuesAsStrings(&extBaseName);
  ires = loadExtents();
  if (ires < 0) {
    ERROR_PRINTF(ExtentListLog, "Can't initalize %s extents", extBaseName.c_str());
    return JS_USERERROR;
  }
  /*
   int nReadedExtents = loadExtents();
   if(numExt!=nReadedExtents){
   ERROR_PRINTF(ExtentListLog, "number of extents in Virual Folders : %d must be equal to %d", nReadedExtents,numExt);
   return JS_USERERROR;
   }
   */
  // ************
  return JS_OK;
}

long ExtentList::getSumOfExtSizes() const {
  long sz = 0;
  for (int i = 0; i < numExtents; i++)
    sz += extents[i].getExtentSize();

  return sz;
}

long ExtentList::computeExtentSize(long fileLength, int numExtents, long frameSize) {
  if (fileLength <= 0 || numExtents <= 0 || frameSize <= 0) {
    ERROR_PRINTF(ExtentListLog, "invalid input parameters");
    return JS_USERERROR;
  }

  long extentCreateSize = fileLength / (long) numExtents;
  if (fileLength % numExtents != 0) extentCreateSize++;
  long fact = extentCreateSize / frameSize;
  if (extentCreateSize % frameSize != 0) fact = fact + 1;
  extentCreateSize = frameSize * fact;
  return extentCreateSize;
}

//for the given glbOffset corresponding to a certain frame return extIndex and locOffset
//where that frame could be read
int ExtentList::getExtentInfoForFrame(long glbOffset, int &extIndex, long &locOffset) const {
  if (glbOffset < 0 || glbOffset > maxFilePosition) {
    ERROR_PRINTF(ExtentListLog, "invalid input parameter %ld, %ld", glbOffset, maxFilePosition);
    return JS_USERERROR;
  }
  extIndex = (int) (glbOffset / extentSize);
  locOffset = glbOffset - extIndex * extentSize;
  return JS_OK;
}

//save to XML file
int ExtentList::saveXML(std::string _path) {
  std::string fpath = _path + extBaseName + ".xml";
  FILE *pfile = fopen(fpath.c_str(), "w");
  if (pfile == NULL) {
    ERROR_PRINTF(ExtentListLog, "Can't open file %s.\n", fpath.c_str());
    return JS_USERERROR;
  }

  std::string VExtManXML =
      "<parset name=\"ExtentManager\">\n\
        <par name=\"VFIO_VERSION\" type=\"string\"> 2006.2 </par>\n\
        <par name=\"VFIO_EXTSIZE\" type=\"long\"> "
          + num2Str(extentSize) + " </par>\n\
        <par name=\"VFIO_MAXFILE\" type=\"int\"> " + num2Str(numExtents)
          + " </par>\n\
        <par name=\"VFIO_MAXPOS\" type=\"long\"> " + num2Str(maxFilePosition)
          + " </par>\n\
        <par name=\"VFIO_EXTNAME\" type=\"string\"> " + extBaseName
          + " </par>\n\
        <par name=\"VFIO_POLICY\" type=\"string\"> RANDOM </par>\n\
        </parset>\n";

  fprintf(pfile, "%s", VExtManXML.c_str());
  fclose(pfile);
  return JS_OK;
}

}

