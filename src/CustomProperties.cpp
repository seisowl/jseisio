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

#include "FloatBuffer.h"
#include "CustomProperties.h"
#include "xmlreader.h"

#include "PSProLogging.h"

namespace jsIO {
DECLARE_LOGGER(CustomPropertiesLog);

CustomProperties::~CustomProperties() {

}

CustomProperties::CustomProperties() {

}

int CustomProperties::load(std::string &XMLstring) {
  TRACE_PRINTF(CustomPropertiesLog, "loading CustomProperties ...");

  xmlreader reader;
  reader.parse(XMLstring);

  xmlElement *parSetFileProps = 0;
  parSetFileProps = reader.getBlock("CustomProperties");

  if(parSetFileProps == 0) {
    TRACE_PRINTF(CustomPropertiesLog, "Warning: There is no CustomProperties part.");
    return JS_WARNING;
  }
  Parameter par;
  xmlElement *parElement = 0;
  parElement = reader.FirstChildElement(parSetFileProps);
  if(parElement == 0) {
    TRACE_PRINTF(CustomPropertiesLog, "Warning: There is no Properties part in CustomProperties");
  } else do {
    int ires = reader.load2Parameter(parElement, &par);
    if(ires != JS_OK) {
      ERROR_PRINTF(CustomPropertiesLog, "Error in XML string. Invalid CustomProperties");
      return JS_WARNING;
    }
    addProperty(par.getName(), par.getTypeAsString().c_str(), par.getValuesAsString().c_str());
  } while((parElement = reader.NextSiblingElement(parElement, "par")));

  xmlElement *parEntries = 0;

  { // Geometry
    parEntries = reader.FirstChildBlock(parSetFileProps, "Geometry");
    if(parEntries == 0) {
      TRACE_PRINTF(CustomPropertiesLog, "Warning: There is no Geometry part in CustomProperties");
      return JS_WARNING;
    }

    int minILine = 0;
    int maxILine = -1;
    int nIL = 0;
    int minXLine = 0;
    int maxXLine = -1;
    int nXL = 0;
    double xILine1End;
    double yILine1End;
    double xILine1Start;
    double yILine1Start;
    double xXLine1End;
    double yXLine1End;
    int resetOrigin = -1; // default value, means unset

    parElement = reader.FirstChildElement(parEntries);
    std::string curVal;
    std::string curValText;
    do {
      int ires = reader.load2Parameter(parElement, &par);
      if(ires != JS_OK) {
        ERROR_PRINTF(CustomPropertiesLog, "error in xml string");
        return ires;
      }
      curVal = par.getName();
      if(curVal == "resetOrigin") {
        par.valuesAsInts(&resetOrigin);
      } else if(curVal == "minILine") {
        par.valuesAsInts(&minILine);
      } else if(curVal == "maxILine") {
        par.valuesAsInts(&maxILine);
      } else if(curVal == "nILines") {
        par.valuesAsInts(&nIL);
      } else if(curVal == "minXLine") {
        par.valuesAsInts(&minXLine);
      } else if(curVal == "maxXLine") {
        par.valuesAsInts(&maxXLine);
      } else if(curVal == "nXLines") {
        par.valuesAsInts(&nXL);
      } else if(curVal == "xILine1End") {
        par.valuesAsDoubles(&xILine1End);
      } else if(curVal == "yILine1End") {
        par.valuesAsDoubles(&yILine1End);
      } else if(curVal == "xILine1Start") {
        par.valuesAsDoubles(&xILine1Start);
      } else if(curVal == "yILine1Start") {
        par.valuesAsDoubles(&yILine1Start);
      } else if(curVal == "xXLine1End") {
        par.valuesAsDoubles(&xXLine1End);
      } else if(curVal == "yXLine1End") {
        par.valuesAsDoubles(&yXLine1End);
      }
      parElement = reader.NextSiblingElement(parElement, "par");
      // /********* here comes all other properties
    } while(parElement != NULL);

    survGeom.setGeom(minILine, maxILine, nIL, minXLine, maxXLine, nXL, xILine1End, yILine1End, xILine1Start, yILine1Start, xXLine1End,
                     yXLine1End, resetOrigin);
  }

  { // AltGrid
    parEntries = reader.FirstChildBlock(parSetFileProps, "AltGrid");
    if(parEntries != 0) {

      parElement = reader.FirstChildElement(parEntries);
      std::string curVal;
      std::string curValText;
      do {
        int ires = reader.load2Parameter(parElement, &par);
        if(ires != JS_OK) {
          ERROR_PRINTF(CustomPropertiesLog, "error in xml string");
          return ires;
        }
        curVal = par.getName();
        if(curVal == "flagAlt") {
          par.valuesAsInts(&(altGrid.flagAlt));
        } else if(curVal == "ix0Regular") {
          par.valuesAsInts(&(altGrid.ix0Regular));
        } else if(curVal == "iy0Regular") {
          par.valuesAsInts(&(altGrid.iy0Regular));
        } else if(curVal == "incxRegular") {
          par.valuesAsInts(&(altGrid.incxRegular));
        } else if(curVal == "incyRegular") {
          par.valuesAsInts(&(altGrid.incyRegular));
        } else if(curVal == "x0Regular") {
          par.valuesAsFloats(&(altGrid.x0Regular));
        } else if(curVal == "y0Regular") {
          par.valuesAsFloats(&(altGrid.y0Regular));
        } else if(curVal == "x0Alt") {
          par.valuesAsFloats(&(altGrid.x0Alt));
        } else if(curVal == "y0Alt") {
          par.valuesAsFloats(&(altGrid.y0Alt));
        } else if(curVal == "nxRegular") {
          par.valuesAsInts(&(altGrid.nxRegular));
        } else if(curVal == "nyRegular") {
          par.valuesAsInts(&(altGrid.nyRegular));
        } else if(curVal == "nzRegular") {
          par.valuesAsInts(&(altGrid.nzRegular));
        } else if(curVal == "nxAlt") {
          par.valuesAsInts(&(altGrid.nxAlt));
        } else if(curVal == "nyAlt") {
          par.valuesAsInts(&(altGrid.nyAlt));
        } else if(curVal == "nzAlt") {
          par.valuesAsInts(&(altGrid.nzAlt));
        } else if(curVal == "dxRegular") {
          par.valuesAsDoubles(&(altGrid.dxRegular));
        } else if(curVal == "dyRegular") {
          par.valuesAsDoubles(&(altGrid.dyRegular));
        } else if(curVal == "dzRegular") {
          par.valuesAsDoubles(&(altGrid.dzRegular));
        } else if(curVal == "dxAlt") {
          par.valuesAsDoubles(&(altGrid.dxAlt));
        } else if(curVal == "dyAlt") {
          par.valuesAsDoubles(&(altGrid.dyAlt));
        } else if(curVal == "dzAlt") {
          par.valuesAsDoubles(&(altGrid.dzAlt));
        } else if(curVal == "irregZs") {
          string str;
          par.valuesAsStrings(&str);
          altGrid.irregZs = str2floats(str);
        }
        parElement = reader.NextSiblingElement(parElement, "par");
        // /********* here comes all other properties
      } while(parElement != NULL);
      altGrid.updateZAlt();
    }
  }

  TRACE_PRINTF(CustomPropertiesLog, "CustomProperties loaded successfully.");

  return JS_OK;
}

int CustomProperties::save(std::string &XMLstring) {
  int minILine = 0;
  int maxILine = -1;
  int nIL = 1;
  int minXLine = 0;
  int maxXLine = -1;
  int nXL = 1;
  double xILine1End;
  double yILine1End;
  double xILine1Start;
  double yILine1Start;
  double xXLine1End;
  double yXLine1End;
  int resetOrigin;

  survGeom.getGeom(minILine, maxILine, nIL, minXLine, maxXLine, nXL, xILine1End, yILine1End, xILine1Start, yILine1Start, xXLine1End,
                   yXLine1End, resetOrigin);

  if(m_properties.size() == 0) {
    XMLstring = "";
    return JS_OK;
  }

  XMLstring = "  <parset name=\"CustomProperties\">\n";

  for(int i = 0; i < m_properties.size(); i++) {
    XMLstring += "    <par name=\"" + m_properties[i].name + "\" type=\"" + m_properties[i].type + "\"> " + m_properties[i].value
        + " </par>\n";
  }

  if(minILine <= maxILine) {
    XMLstring += "    <parset name=\"Geometry\">\n";
    if(resetOrigin >= 0) XMLstring += "      <par name=\"resetOrigin\" type=\"int\"> " + num2Str(resetOrigin) + " </par>\n";
    XMLstring += "      <par name=\"minILine\" type=\"int\"> " + num2Str(minILine) + " </par>\n";
    XMLstring += "      <par name=\"maxILine\" type=\"int\"> " + num2Str(maxILine) + " </par>\n";
    XMLstring += "      <par name=\"nILines\" type=\"int\"> " + num2Str(nIL) + " </par>\n";
    XMLstring += "      <par name=\"minXLine\" type=\"int\"> " + num2Str(minXLine) + " </par>\n";
    XMLstring += "      <par name=\"maxXLine\" type=\"int\"> " + num2Str(maxXLine) + " </par>\n";
    XMLstring += "      <par name=\"nXLines\" type=\"int\"> " + num2Str(nXL) + " </par>\n";
    XMLstring += "      <par name=\"xILine1End\" type=\"double\"> " + num2Str(xILine1End) + " </par>\n";
    XMLstring += "      <par name=\"yILine1End\" type=\"double\"> " + num2Str(yILine1End) + " </par>\n";
    XMLstring += "      <par name=\"xILine1Start\" type=\"double\"> " + num2Str(xILine1Start) + " </par>\n";
    XMLstring += "      <par name=\"yILine1Start\" type=\"double\"> " + num2Str(yILine1Start) + " </par>\n";
    XMLstring += "      <par name=\"xXLine1End\" type=\"double\"> " + num2Str(xXLine1End) + " </par>\n";
    XMLstring += "      <par name=\"yXLine1End\" type=\"double\"> " + num2Str(yXLine1End) + " </par>\n";
    XMLstring += "    </parset>\n";
  }

  if(altGrid.flagAlt) {
    XMLstring += "    <parset name=\"AltGrid\">\n";
    // Start GPT
    XMLstring += "      <par name=\"flagAlt\" type=\"int\"> " + num2Str(altGrid.flagAlt) + " </par>\n";
    XMLstring += "      <par name=\"ix0Regular\" type=\"int\"> " + num2Str(altGrid.ix0Regular) + " </par>\n";
    XMLstring += "      <par name=\"iy0Regular\" type=\"int\"> " + num2Str(altGrid.iy0Regular) + " </par>\n";
    XMLstring += "      <par name=\"incxRegular\" type=\"int\"> " + num2Str(altGrid.incxRegular) + " </par>\n";
    XMLstring += "      <par name=\"incyRegular\" type=\"int\"> " + num2Str(altGrid.incyRegular) + " </par>\n";

    XMLstring += "      <par name=\"x0Regular\" type=\"float\"> " + num2Str(altGrid.x0Regular) + " </par>\n";
    XMLstring += "      <par name=\"y0Regular\" type=\"float\"> " + num2Str(altGrid.y0Regular) + " </par>\n";
    XMLstring += "      <par name=\"x0Alt\" type=\"float\"> " + num2Str(altGrid.x0Alt) + " </par>\n";
    XMLstring += "      <par name=\"y0Alt\" type=\"float\"> " + num2Str(altGrid.y0Alt) + " </par>\n";

    XMLstring += "      <par name=\"nxRegular\" type=\"int\"> " + num2Str(altGrid.nxRegular) + " </par>\n";
    XMLstring += "      <par name=\"nyRegular\" type=\"int\"> " + num2Str(altGrid.nyRegular) + " </par>\n";
    XMLstring += "      <par name=\"nzRegular\" type=\"int\"> " + num2Str(altGrid.nzRegular) + " </par>\n";

    XMLstring += "      <par name=\"nxAlt\" type=\"int\"> " + num2Str(altGrid.nxAlt) + " </par>\n";
    XMLstring += "      <par name=\"nyAlt\" type=\"int\"> " + num2Str(altGrid.nyAlt) + " </par>\n";
    XMLstring += "      <par name=\"nzAlt\" type=\"int\"> " + num2Str(altGrid.nzAlt) + " </par>\n";

    XMLstring += "      <par name=\"dxRegular\" type=\"double\"> " + num2Str(altGrid.dxRegular) + " </par>\n";
    XMLstring += "      <par name=\"dyRegular\" type=\"double\"> " + num2Str(altGrid.dyRegular) + " </par>\n";
    XMLstring += "      <par name=\"dzRegular\" type=\"double\"> " + num2Str(altGrid.dzRegular) + " </par>\n";

    XMLstring += "      <par name=\"dxAlt\" type=\"double\"> " + num2Str(altGrid.dxAlt) + " </par>\n";
    XMLstring += "      <par name=\"dyAlt\" type=\"double\"> " + num2Str(altGrid.dyAlt) + " </par>\n";
    XMLstring += "      <par name=\"dzAlt\" type=\"double\"> " + num2Str(altGrid.dzAlt) + " </par>\n";
    // end GPT
    if(!altGrid.irregZs.empty()) XMLstring += "      <par name=\"irregZs\" type=\"string\"> " + floats2str(altGrid.irregZs) + " </par>\n";
    XMLstring += "    </parset>\n";
  }

  XMLstring += "  </parset>\n";

  return JS_OK;
}

void CustomProperties::addProperty(std::string name, std::string type, std::string value) {
  Property prop;
  prop.name = name;
  prop.type = type;
  prop.value = value;
  m_properties.push_back(prop);
}

AltGrid::AltGrid(vector<float> &irregZs, int nzReg, double dzReg, int flagAlt, int nxReg, int nyReg, int nxAlt, int nyAlt, double dxReg,
    double dyReg, double dxAlt, double dyAlt, int ix0Regular, int iy0Regular, int incxRegular, int incyRegular, float x0Regular,
    float y0Regular, float x0Alt, float y0Alt) : irregZs(irregZs), nzRegular(nzReg), dzRegular(dzReg), flagAlt(flagAlt), nxRegular(nxReg), nyRegular(
    nyReg), nxAlt(nxAlt), nyAlt(nyAlt), dxRegular(dxReg), dyRegular(dyReg), dxAlt(dxAlt), dyAlt(dyAlt), ix0Regular(ix0Regular), iy0Regular(
    iy0Regular), incxRegular(incxRegular), incyRegular(incyRegular), x0Regular(x0Regular), y0Regular(y0Regular), x0Alt(x0Alt), y0Alt(y0Alt) {
  initialized = true;
  updateZAlt();
}

void AltGrid::updateZAlt() {
  if(!irregZs.empty()) {
    assert(flagAlt == 2 || flagAlt == -2);
    assert(irregZs.size() > 1L);
    nzAlt = irregZs.size();
    dzAlt = irregZs[1] - irregZs[0];
  }
}

float AltGrid::zmax() {
  return (flagAlt > 1) ? irregZs[irregZs.size() - 1] : (flagAlt == 1) ? (nzAlt - 1) * dzAlt : (nzRegular - 1) * dzRegular;
}

}

