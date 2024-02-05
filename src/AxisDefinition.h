/***************************************************************************
 AxisDefinition.h  -  description
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

#ifndef AXISDEFINITION_H
#define AXISDEFINITION_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "AxisLabel.h"
#include "Units.h"
#include "DataDomain.h"
#include "jsDefs.h"

namespace jsIO {
class AxisDefinition {
public:

  // private atributes
private:

  AxisLabel label; // the label of the axis.
  Units units; //the physical units along the axis.
  DataDomain domain; //the physical domain of the axis.
  long length; // the length of the axis (number of samples).
  long logicalOrigin; //the logical origin of the axis (e.g. the first inline number).
  long logicalDelta; //the logical increment along the axis.
  double physicalOrigin; //the physical origin of the axis (i.e. the coordinate of the first sample)
  double physicalDelta; // the physical increment between samples.

  std::string headerName; //these two string can be used to set the corresponding (to axis) header words in TraceHeader file(s)
  std::string headerBinName;

  bool bInit;
public:
  ~AxisDefinition();
  /** No descriptions */
  AxisDefinition();

  AxisDefinition(AxisLabel _label, Units _units, DataDomain _domain, long _length, long _logicalOrigin,
                 long _logicalDelta, double _physicalOrigin, double _physicalDelta, std::string _headerName = "",
                 std::string _headerBinName = "");

  void Init(AxisLabel _label, Units _units, DataDomain _domain, long _length, long _logicalOrigin, long _logicalDelta,
            double _physicalOrigin, double _physicalDelta, std::string _headerName = "", std::string _headerBinName = "");

  static void getDefault(int ndim, int *idim, AxisDefinition *&axes);

  int subRange(long *_range, int _rangelen, AxisDefinition *subAxis);

  AxisLabel getLabel() const {
    return (label);
  }
  Units getUnits() const {
    return (units);
  }
  DataDomain getDomain() const {
    return (domain);
  }
  long getLength() const {
    return (length);
  }
  long getLogicalOrigin() const {
    return (logicalOrigin);
  }
  long getLogicalDelta() const {
    return (logicalDelta);
  }
  double getPhysicalOrigin() const {
    return (physicalOrigin);
  }
  double getPhysicalDelta() const {
    return (physicalDelta);
  }
  std::string getHeaderName() const {
    return (headerName);
  }
  std::string getHeaderBinName() const {
    return (headerBinName);
  }

private:

protected:

};

}

#endif

