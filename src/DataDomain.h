/***************************************************************************
 DataDomain.h  -  description
 -------------------
 * This class provides a type-safe representation of domains,
 * such as space, time, and frequency.

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

#ifndef DATADOMAIN_H
#define DATADOMAIN_H

#include <iostream>
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>

namespace jsIO {
/**
 * This class provides a type-safe representation of domains,
 * such as space, time, and frequency.
 */
class DataDomain {
public:
  ~DataDomain() {
  }
  DataDomain() {
  }

  DataDomain(std::string _name, std::string _description);
  DataDomain(std::string _name);

  void Init(std::string _name, std::string _description);
  void Init(std::string _name);

  std::string getName() const {
    return name;
  }
  std::string toString() const {
    return name;
  }
  std::string getDescription() const {
    return description;
  }

private:
  std::string name;
  std::string description;

public:
  static const DataDomain SPACE;
  static const DataDomain TIME;
  static const DataDomain DEPTH;
  static const DataDomain FREQUENCY;
  static const DataDomain WAVENUMBER;
  static const DataDomain SEMBLANCE;
  static const DataDomain VELOCITY;
  static const DataDomain DIP;
  static const DataDomain VSVP;
  static const DataDomain SLOWNESS;
  static const DataDomain ETA;
  static const DataDomain SLOTH;
  static const DataDomain EPSILON;
  static const DataDomain DELTA;
  static const DataDomain ALACRITY;
  static const DataDomain AMPLITUDE;
  static const DataDomain COHERENCE;
  static const DataDomain ENVELOPE;
  static const DataDomain IMPEDANCE;
  static const DataDomain DENSITY;
  static const DataDomain VS;
  static const DataDomain FOLD;
  static const DataDomain INCIDENCE_ANGLE;
  static const DataDomain ROTATION_ANGLE;
  static const DataDomain MODEL_TRANSFORM;
  static const DataDomain FLEX_BINNED;
  static const DataDomain UNDEFINED;
  static const DataDomain UNKNOWN;
};
}

#endif

