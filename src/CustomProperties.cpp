/***************************************************************************
                          CustomProperties.cpp  -  description
                             -------------------
    copyright            : (C) 2012-2012 Fraunhofer ITWM

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

#include "CustomProperties.h"

#include "PSProLogging.h"
 
namespace jsIO
{
  DECLARE_LOGGER(CustomPropertiesLog);
 
  CustomProperties::~CustomProperties(){

  }

  CustomProperties::CustomProperties(){

  }


  int CustomProperties::load(std::string &XMLstring)
  {
    TRACE_PRINTF(CustomPropertiesLog, "loading CustomProperties ...");

    xmlreader reader;
    reader.parse(XMLstring);

    xmlElement* parSetFileProps=0;
    parSetFileProps=reader.getBlock("CustomProperties");
  
    if(parSetFileProps==0){
      TRACE_PRINTF(CustomPropertiesLog, "Warning: There is no CustomProperties part.");
      return JS_WARNING;
    }  
    Parameter par;
    xmlElement *parElement =0;
    parElement = reader.FirstChildElement(parSetFileProps);
    if(parElement==0){
        TRACE_PRINTF(CustomPropertiesLog, "Warning: There is no Properties part in CustomProperties");
    } else
    do{
        int ires=reader.load2Parameter(parElement, &par);
        if(ires!=JS_OK){
           ERROR_PRINTF(CustomPropertiesLog, "Error in XML string. Invalid CustomProperties");
           return JS_WARNING;
        }
        addProperty(par.getName(), par.getTypeAsString().c_str(), par.getValuesAsString().c_str());
    }while((parElement = reader.NextSiblingElement(parElement,"par")));

    xmlElement* parEntries = 0;
    parEntries = reader.FirstChildBlock(parSetFileProps, "Geometry");
    if(parEntries==0){
      TRACE_PRINTF(CustomPropertiesLog, "Warning: There is no Geometry part in CustomProperties");
      return JS_WARNING;
    } 

    int minILine=0;
    int maxILine=-1;
    int minXLine=0;
    int maxXLine=-1;
    float xILine1End;
    float yILine1End;
    float xILine1Start;
    float yILine1Start;
    float xXLine1End;
    float yXLine1End;
   
    parElement = reader.FirstChildElement(parEntries);
    std::string curVal;
    std::string curValText;
    do{
      int ires=reader.load2Parameter(parElement, &par);
      if(ires!=JS_OK){
        ERROR_PRINTF(CustomPropertiesLog, "error in xml string");
        return ires;
      }
      curVal = par.getName();
      if(curVal=="minILine"){
        par.valuesAsInts(&minILine);
      }else if(curVal=="maxILine"){
        par.valuesAsInts(&maxILine);
      }else if(curVal=="minXLine"){
        par.valuesAsInts(&minXLine);
      }else if(curVal=="maxXLine"){
        par.valuesAsInts(&maxXLine);
      }else if(curVal=="xILine1End"){
        par.valuesAsFloats(&xILine1End);
      }else if(curVal=="yILine1End"){
        par.valuesAsFloats(&yILine1End);
      }else if(curVal=="xILine1Start"){
        par.valuesAsFloats(&xILine1Start);
      }else if(curVal=="yILine1Start"){
        par.valuesAsFloats(&yILine1Start);
      }else if(curVal=="xXLine1End"){
        par.valuesAsFloats(&xXLine1End);
      }else if(curVal=="yXLine1End"){
        par.valuesAsFloats(&yXLine1End);
      }
      parElement=reader.NextSiblingElement(parElement,"par");
// /********* here comes all other properties
    }while(parElement!=NULL);

    survGeom.setGeom(minILine, maxILine, minXLine, maxILine, 
                     xILine1End, yILine1End, xILine1Start, yILine1Start, xXLine1End, yXLine1End);

    TRACE_PRINTF(CustomPropertiesLog, "CustomProperties loaded successfully.");

    return JS_OK;
  }

  int CustomProperties::save(std::string &XMLstring)
  {
    int minILine=0;
    int maxILine=-1;
    int minXLine=0;
    int maxXLine=-1;
    float xILine1End;
    float yILine1End;
    float xILine1Start;
    float yILine1Start;
    float xXLine1End;
    float yXLine1End;

    survGeom.getGeom(minILine, maxILine, minXLine, maxXLine, 
                     xILine1End, yILine1End, xILine1Start, yILine1Start, xXLine1End, yXLine1End);



    XMLstring = "  <parset name=\"CustomProperties\">\n";
    
    for(int i=0;i<m_properties.size();i++)
    {
      XMLstring +="    <par name=\""+m_properties[i].name+"\" type=\""+m_properties[i].type+"\"> "+m_properties[i].value+ " </par>\n";
    }
        
    if(minILine<maxILine)
    {
      XMLstring +="    <parset name=\"Geometry\">\n";
      XMLstring +="      <par name=\"minILine\" type=\"int\"> "+ num2Str(minILine) +" </par>\n";
      XMLstring +="      <par name=\"maxILine\" type=\"int\"> "+ num2Str(maxILine) +" </par>\n";
      XMLstring +="      <par name=\"minXLine\" type=\"int\"> "+ num2Str(minXLine) +" </par>\n";
      XMLstring +="      <par name=\"maxXLine\" type=\"int\"> "+ num2Str(maxXLine) +" </par>\n";
      XMLstring +="      <par name=\"xILine1End\" type=\"float\"> "+ num2Str(xILine1End) +" </par>\n";
      XMLstring +="      <par name=\"yILine1End\" type=\"float\"> "+ num2Str(yILine1End) +" </par>\n";
      XMLstring +="      <par name=\"xILine1Start\" type=\"float\"> "+ num2Str(xILine1Start) +" </par>\n";
      XMLstring +="      <par name=\"yILine1Start\" type=\"float\"> "+ num2Str(yILine1Start) +" </par>\n";
      XMLstring +="      <par name=\"xXLine1End\" type=\"float\"> "+ num2Str(xXLine1End) +" </par>\n";
      XMLstring +="      <par name=\"yXLine1End\" type=\"float\"> "+ num2Str(yXLine1End) +" </par>\n";
      XMLstring +="    </parset>\n";
    }

    XMLstring +="  </parset>\n";

    return JS_OK;
  }
  
  void CustomProperties::addProperty(std::string name, std::string type, std::string value)
  {
    Property prop;
    prop.name = name;
    prop.type = type;
    prop.value = value;
    m_properties.push_back(prop);
  }

}


