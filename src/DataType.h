/***************************************************************************
                          DataType.h  -  description
                             -------------------
 * The DataType class provides JavaSeis data type enumerations.

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

#ifndef DATATYPE_H
#define DATATYPE_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

namespace jsIO
{
  /**
   * This class provides JavaSeis data type enumerations.
   */
  class DataType{
    public:
      ~DataType(){}
      DataType(){}
      DataType(std::string _name, std::string _description);
      void Init(std::string _name, std::string _description);

      std::string getName() const {return name;}
      std::string toString() const {return name;}
      std::string getDescription() const {return description;}
      static DataType get(std::string _name);

    private:
      std::string name;
      std::string description;

    public:
      static const DataType UNKNOWN;
      static const DataType CUSTOM;
      static const DataType CMP;
      static const DataType SOURCE;
      static const DataType RECEIVER;
      static const DataType OFFSET_BIN;
      static const DataType STACK;
  };
}

#endif



