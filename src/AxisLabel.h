/***************************************************************************
 AxisLabel.h  -  description
 -------------------
 * This class encapsulates an axis label.  The primary reason that an axis label is
 * a class, instead of just a String, is to couple a name with a description.
 * A secondary reason is to encourage (but not require) naming convention.

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

#ifndef AXISLABEL_H
#define AXISLABEL_H

#include <iostream>
#include <map>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
using std::string;
using std::map;

namespace jsIO {
/**
 * This class is for representation of an axis label.
 */
class AxisLabel {
public:
  static map<string, string> LABEL2HDR;
public:
  ~AxisLabel() {
  }
  AxisLabel() {
  }
  AxisLabel(std::string _name, std::string _description);

  void Init(std::string _name, std::string _description);

  std::string getName() const {
    return name;
  }
  std::string toString() const {
    return name;
  }
  std::string getDescription() const {
    return description;
  }

  static void getDefault(int ndim, AxisLabel *&axes);

public:

  static const AxisLabel UNDEFINED;
  static const AxisLabel TIME;
  static const AxisLabel DEPTH;
  static const AxisLabel OFFSET;
  static const AxisLabel OFFSET_BIN;
  static const AxisLabel CROSSLINE;
  static const AxisLabel INLINE;
  static const AxisLabel SOURCE;
  static const AxisLabel CMP;
  static const AxisLabel RECEIVER;
  static const AxisLabel RECEIVER_LINE;
  static const AxisLabel CHANNEL;
  static const AxisLabel SAIL_LINE;
  static const AxisLabel VOLUME;

  // private atributes
private:
  std::string name;
  std::string description;

private:

protected:

};

}

#endif

