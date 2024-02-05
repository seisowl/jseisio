/***************************************************************************
 CustomProperties.h  -  description
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

#ifndef CUSTOMPROPERTIES_H
#define CUSTOMPROPERTIES_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using std::vector;

#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "stringfuncs.h"
#include "jsDefs.h"

#include <stdexcept>

namespace jsIO {

class SurveyGeometry {
public:
  SurveyGeometry() : minILine(0), maxILine(-1), nILine(1), minXLine(0), maxXLine(-1), nXLine(1), xILine1End(0), yILine1End(0), xILine1Start(
      0), yILine1Start(0), xXLine1End(0), yXLine1End(0), resetOrigin(-1) {
  }
  void setGeom(int i1, int i2, int nIL, int i3, int i4, int nXL, double f1, double f2, double f3, double f4, double f5, double f6,
      int reset) {
    minILine = i1;
    maxILine = i2;
    nILine = nIL;
    minXLine = i3;
    maxXLine = i4;
    nXLine = nXL;
    xILine1End = f1;
    yILine1End = f2;
    xILine1Start = f3;
    yILine1Start = f4;
    xXLine1End = f5;
    yXLine1End = f6;
    resetOrigin = reset;
  }
  void getGeom(int &i1, int &i2, int &nIL, int &i3, int &i4, int &nXL, double &f1, double &f2, double &f3, double &f4, double &f5,
      double &f6, int &reset_origin) {
    i1 = minILine;
    i2 = maxILine;
    nIL = nILine;
    i3 = minXLine;
    i4 = maxXLine;
    nXL = nXLine;
    f1 = xILine1End;
    f2 = yILine1End;
    f3 = xILine1Start;
    f4 = yILine1Start;
    f5 = xXLine1End;
    f6 = yXLine1End;
    reset_origin = resetOrigin;
  }
public:
  int resetOrigin = -1; // unset
  int minILine;
  int maxILine;
  int nILine;
  int minXLine;
  int maxXLine;
  int nXLine;
  double xILine1End;
  double yILine1End;
  double xILine1Start;
  double yILine1Start;
  double xXLine1End;
  double yXLine1End;
};

class AltGrid {
public:
  AltGrid() {
  }
  AltGrid(vector<float> &irregZs, int nzReg, double dzReg, int flagAlt = 0, int nxReg = 0, int nyReg = 0, int nxAlt = 0, int nyAlt = 0,
      double dxReg = 0, double dyReg = 0, double dxAlt = 0, double dyAlt = 0, int ix0Regular = 0, int iy0Regular = 0, int incxRegular = 0,
      int incyRegular = 0, float x0Regular = 0, float y0Regular = 0, float x0Alt = 0, float y0Alt = 0);
  void updateZAlt();
  float zmax();

  vector<float> irregZs;
  bool initialized { };
  int flagAlt { }; // 0: no AltGrid, 1: regular-z, AltGrid, 2: irreg-z AltGrid, -1: regular-z, RegGrid with Alt info, -2: irreg-z, RegGrid with Alt info
  int ix0Regular { }, iy0Regular { }, incxRegular { }, incyRegular { };
  float x0Regular { }, y0Regular { }, x0Alt { }, y0Alt { };
  int nxRegular { }, nyRegular { }, nzRegular { };
  int nxAlt { }, nyAlt { }, nzAlt { };
  double dxRegular { }, dyRegular { }, dzRegular { };
  double dxAlt { }, dyAlt { }, dzAlt { };
};

class CustomProperties {
  struct Property {
    std::string name;
    std::string type;
    std::string value;
  };

public:
  SurveyGeometry survGeom;
  AltGrid altGrid;

public:
  ~CustomProperties();
  /** No descriptions */
  CustomProperties();

  void addProperty(std::string name, std::string type, std::string value);

  int load(std::string &XMLstring);
  int save(std::string &XMLstring);

private:
  std::vector<Property> m_properties;

protected:

};

}

#endif

