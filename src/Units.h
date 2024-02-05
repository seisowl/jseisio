/***************************************************************************
                          Units.h  -  description
                             -------------------
 * This class provides a type-safe representation of units.

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

#ifndef UNITS_H
#define UNITS_H

#include <iostream>
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>

namespace jsIO {
/**
 * This class is for type-safe representation of units.
 */

class Units {
public:

  static const Units FEET;
  static const Units FT;
  static const Units METERS;
  static const Units M;
  static const Units MILLISECONDS;
  static const Units MS;
  static const Units MSEC;
  static const Units SECONDS;
  static const Units MICROSEC;
  static const Units HERTZ;
  static const Units HZ;
  static const Units DEGREES;
  static const Units UNDEFINED;//named NULL in java version
  static const Units UNKNOWN;

  // private atributes
private:

public:
  ~Units();
  /** No descriptions */
  Units();

  Units(std::string _name, std::string _description);
  Units(std::string _name);

  void Init(std::string _name, std::string _description);
  void Init(std::string _name);

  std::string getName() const {return name;}
  std::string toString()  const {return name;}
  std::string getDescription()  const {return description;}

  static Units get(std::string _name);

private:
  std::string name;
  std::string description;

protected:

};

}


#endif

