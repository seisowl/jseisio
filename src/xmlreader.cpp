/***************************************************************************
                          xmlreader.cpp -  description
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
  
#include "xmlreader.h"

#include "PSProLogging.h"
 
namespace jsIO
{
  DECLARE_LOGGER(xmlreaderLog);
 
  xmlreader::~xmlreader(){

  }

  xmlreader::xmlreader(){
    bParse = false;
  }
 
  bool xmlreader::parse(std::string &xmlstr){
    bParse = false;
    doc.SetCondenseWhiteSpace(false);
    doc.Parse(xmlstr.c_str());
    if ( doc.Error() ){
      ERROR_PRINTF(xmlreaderLog, "Invalid XML string: %s, %s", doc.Value(), doc.ErrorDesc() );
      return bParse;
    }

    bParse = true;
    return bParse;
  }

  bool xmlreader::parseFile(const char *filename){
    std::ifstream ifile;
    ifile.open(filename, std::ifstream::in);
    if(! ifile.good() ) {
      ERROR_PRINTF(xmlreaderLog, "Could not open file %s\n", filename);
      return -1;
    }
	
    std::string xmlString;
 // get length of file:
    ifile.seekg (0, std::ios::end);
    long length = ifile.tellg();
    ifile.seekg (0, std::ios::beg);
  
    char *buffer = new char [length];
    ifile.read (buffer,length);
    ifile.close();
    xmlString.append(buffer);
    delete[]buffer;

    return parse(xmlString);
  }

  xmlElement* xmlreader::getBlock(std::string blockname){
    if(!bParse){
      ERROR_PRINTF(xmlreaderLog, "You have to parse the document first");
      return NULL;
    }
  
    TiXmlElement* node = 0;
    TiXmlElement* parSetElement = 0;
    TiXmlElement* parBlock = 0;

    node = doc.FirstChildElement( "parset" );
    if(strcmp(node->Attribute("name"), blockname.c_str())==0){
      parBlock = node;
      return parBlock;
    }

    bool  blockFound=false;
    parSetElement = node->FirstChildElement("parset");
    do{
      if(strcmp(parSetElement->Attribute("name"), blockname.c_str())==0){
        parBlock = parSetElement;
        blockFound=true;
      }   
    }while((parSetElement = parSetElement->NextSiblingElement()));
  
    if(!blockFound){
      ERROR_PRINTF(xmlreaderLog, "There is no block named %s in the current xml file",blockname.c_str());
      return NULL;
    }
  
    return parBlock;
  }


//if bequal==true then search string is equal to name otherwise it starts with name
  xmlElement* xmlreader::FirstChildElement(xmlElement *element, std::string name, const bool bequal){
    TiXmlElement* parElement = 0;
    TiXmlElement* parFoundElement = 0;
    int eq=1;
    bool  elementFound=false;
    parElement = element->FirstChildElement("par");
    do{
      if(bequal)
        eq=strcmp(parElement->Attribute("name"), name.c_str());
      else
        eq=strncmp(parElement->Attribute("name"), name.c_str(), name.length());

      if(eq==0){
        parFoundElement = parElement;
        elementFound=true;
      }   
    }while((parElement = parElement->NextSiblingElement()));
  
    if(!elementFound){
      ERROR_PRINTF(xmlreaderLog, "There is no child element named %s in the %s\n",name.c_str(),element->Attribute("name"));
      return parFoundElement;
    }
  
    return parFoundElement;
  }

  xmlElement* xmlreader::FirstChildBlock(xmlElement *element, std::string name, const bool bequal){
    TiXmlElement* parElement = 0;
    TiXmlElement* parFoundElement = 0;
    int eq=1;
    bool  elementFound=false;
    parElement = element->FirstChildElement("parset");
    do{
      if(bequal)
        eq=strcmp(parElement->Attribute("name"), name.c_str());
      else
        eq=strncmp(parElement->Attribute("name"), name.c_str(), name.length());

      if(eq==0){
        parFoundElement = parElement;
        elementFound=true;
      }  
    }while((parElement = parElement->NextSiblingElement()));
  
    if(!elementFound){
      ERROR_PRINTF(xmlreaderLog, "There is no child block named %s in the %s",name.c_str(),element->Attribute("name"));
      return parFoundElement;
    }
  
    return parFoundElement;
  }

  xmlElement * xmlreader::FirstChildBlock(xmlElement *element){
    return element->FirstChildElement("parset");
  }

  xmlElement* xmlreader::FirstChildElement(xmlElement *element){
    return element->FirstChildElement();
  }


  xmlElement* xmlreader::NextSiblingElement(xmlElement *element){
    return element->NextSiblingElement();
  }

  xmlElement* xmlreader::NextSiblingElement(xmlElement *element, const char *str){
    return element->NextSiblingElement(str);
  }

  int xmlreader::load2Parameter(xmlElement *element, Parameter* par){
    std::string name="",value="", type="";
    const char *cStr = element->Attribute("name");
    if(cStr!=NULL) name.append(cStr);
    cStr = element->GetText();
    if(cStr!=NULL) value.append(cStr);
    cStr = element->Attribute("type");
    if(cStr!=NULL) type.append(cStr);
//    printf("name=%s, value=%s, type=%s\n",name.c_str(),value.c_str(),type.c_str());
    int ires = par->Init(name, value, type);
    if(ires!=JS_OK){
      ERROR_PRINTF(xmlreaderLog, "Can't initalize parameter");
    }
    return ires;
  }  

  const char * xmlreader::getAttribute(xmlElement *element, std::string attributeName){
    return element->Attribute(attributeName.c_str());
  }
  
  const char * xmlreader::getText(xmlElement *element)
  {
    return element->GetText();
  }
  
}  

