/***************************************************************************
 Parameter.h  -  description
 -------------------
 * A parameter - a named value (or array of values) with a type and 
 * (optional) units.
 * A parameter's type may be set explicitly or by by setting its value(s).
 * A parameter value set as one type may be got as another type, provided
 * that the implied conversion is supported. For example, any float may be 
 * converted to a String, but only some Strings may be converted to floats.
 * Getting a parameter value never changes the intrinsic parameter type.
 * A ParameterConvertException is thrown when a conversion fails.

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

#ifndef PARAMETER_H
#define PARAMETER_H

#include <iostream>
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "stringfuncs.h"
#include "jsDefs.h"

namespace jsIO {
class Parameter {
public:
  ~Parameter();
  /** No descriptions */
  Parameter();
  int Init(std::string &_name, std::string &_units, std::string &_values, int _type);
  int Init(std::string &_name, std::string &_values, int _type);
  int Init(std::string &_name, std::string &_values, std::string _stype);

  std::string getName() const {
    return name;
  }
  void setName(std::string _name) {
    name = _name;
  }
  ;
  int getType() const {
    return type;
  }
  ;
  int getNValues() const {
    return N_values;
  }
  ;
  bool setValues(std::string &_values, int _type);

  bool valuesAsBooleans(bool*) const;
  bool valuesAsInts(int*) const;
  bool valuesAsLongs(long*) const;
  bool valuesAsFloats(float*) const;
  bool valuesAsDoubles(double*) const;
  bool valuesAsStrings(std::string*) const;

  std::string getValuesAsString() const;
  std::string getTypeAsString() const;

  std::string saveAsXML() const;

public:

  static const int UNDEFINED = 0;
  static const int BOOLEAN = 1;
  static const int INT = 2;
  static const int LONG = 3;
  static const int FLOAT = 4;
  static const int DOUBLE = 5;
  static const int STRING = 6;

private:

  std::string name;
  std::string units;
  int type;
  int N_values;
  bool *bvalues;
  int *ivalues;
  long *lvalues;
  float *fvalues;
  double *dvalues;
  std::string *svalues;

private:
  void setTypeAsString(std::string stype);

};
}

#endif

