/***************************************************************************
 DataFormat.h  -  description
 -------------------
 * The DataFormat class provides JavaSeis data format enumerations.

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

#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

namespace jsIO {
/**
 * This class provides JavaSeis data format enumerations.
 */
class DataFormat {
public:
  ~DataFormat() {
  }
  DataFormat() {
  }
  DataFormat(std::string _name, std::string _description, int _bytesPerSample);
  void Init(std::string _name, std::string _description, int _bytesPerSample);

  std::string getName() const {
    return name;
  }
  std::string toString() const {
    return name;
  }
  std::string getDescription() const {
    return description;
  }
  int getBytesPerSample() const {
    return bytesPerSample;
  }
  static DataFormat get(std::string _name);

private:
  std::string name;
  std::string description;
  int bytesPerSample  = sizeof(float);

public:
  static const DataFormat FLOAT;
  static const DataFormat INT16;
  static const DataFormat INT08;
  static const DataFormat COMPRESSED_INT16;
  static const DataFormat COMPRESSED_INT08;
  static const DataFormat SEISPEG;
};
}

#endif

