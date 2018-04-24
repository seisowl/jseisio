/***************************************************************************
 Units.h  -  description
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

#include "stringfuncs.h"  
#include "Units.h"

namespace jsIO {
const Units Units::FEET("feet", "Units of feet");
const Units Units::FT = FEET;
const Units Units::METERS("meters", "Units of meters");
const Units Units::M = METERS;
const Units Units::MILLISECONDS("milliseconds", "Units of milliseconds");
const Units Units::MS = MILLISECONDS;
const Units Units::MSEC = MILLISECONDS;
const Units Units::SECONDS("seconds", "Units of seconds");
const Units Units::MICROSEC("microseconds", "Units of microseconds");
const Units Units::HERTZ("hertz", "Units of Hertz");
const Units Units::HZ = HERTZ;
const Units Units::DEGREES("degrees", "Units of degres");
const Units Units::UNDEFINED("Unitless", "Unitless"); //named NULL in java version
const Units Units::UNKNOWN("unknown", "Units unknown");

Units::~Units() {

}

Units::Units() {

}

Units::Units(std::string _name, std::string _description) {
  Init(_name, _description);
}

Units::Units(std::string _name) {
  Init(_name);
}

void Units::Init(std::string _name) {
  name = _name;
  description = name;
  StrToLower(name);
}

void Units::Init(std::string _name, std::string _description) {
  name = _name;
  StrToLower(name);
  description = _description;
}

}
