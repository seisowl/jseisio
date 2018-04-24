/***************************************************************************
 xmlreader.h  -  description
 -------------------
 wrapper of XML readerr

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

#ifndef XMLREADER_H
#define XMLREADER_H

#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include "xmlParser/tinyxml.h"

// #include "stringfuncs.h"
#include "Parameter.h"

namespace jsIO {
typedef class TiXmlElement xmlElement;

class xmlreader {

public:

public:
  ~xmlreader();
  /** No descriptions */
  xmlreader();
  xmlElement* getBlock(std::string blockname);
  xmlElement * FirstChildElement(xmlElement *element, std::string name, const bool bequal = true);
  xmlElement * FirstChildBlock(xmlElement *element, std::string name, const bool bequal = true);

  xmlElement * FirstChildBlock(xmlElement *element);

  xmlElement * NextSiblingElement(xmlElement *element);
  xmlElement * NextSiblingElement(xmlElement *element, const char *str);
  xmlElement * FirstChildElement(xmlElement *element);

  const char * getAttribute(xmlElement *element, std::string attributeName);

  const char * getText(xmlElement *element);

  int load2Parameter(xmlElement *element, Parameter* par);

  void clear() {
    doc.Clear();
  }
  ;

  bool parseFile(const char *filename);
  bool parse(std::string &xmlstr);

private:
  TiXmlDocument doc;
  bool bParse;
};
}

#endif

