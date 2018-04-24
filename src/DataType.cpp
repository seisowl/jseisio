/***************************************************************************
 DataType.cpp  -  description
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

#include "DataType.h"

namespace jsIO {
const DataType DataType::UNKNOWN("UNKNOWN", "Unknown data");
const DataType DataType::CUSTOM("CUSTOM", "Custom data");
/* CMP data, with typical axes of TIME, OFFSET, CROSSLINE, INLINE. */
const DataType DataType::CMP("CMP", "CMP data");
/* SOURCE data, with typical axes of TIME, CHANNEL, SOURCE, SAIL_LINE. */
const DataType DataType::SOURCE("SOURCE", "Source data");
/* RECEIVER data, with typical axes of TIME, SOURCE, REC_SLOC, SAIL_LINE. */
const DataType DataType::RECEIVER("RECEIVER", "Receiver data");
/* OFFSET data, with typical axes of TIME, CROSSLINE, INLINE, OFFSET_BIN. */
const DataType DataType::OFFSET_BIN("OFFSET_BIN", "Offset-Bin data");
/* STACK data, with typical axes of TIME, CROSSLINE, INLINE, VOLUME. */
const DataType DataType::STACK("STACK", "Stack data");

DataType::DataType(std::string _name, std::string _description) {
  Init(_name, _description);
}

void DataType::Init(std::string _name, std::string _description) {
  name = _name;
  description = _description;
}

DataType DataType::get(std::string _name) {
  if (_name == "CUSTOM") return CUSTOM;
  else if (_name == "CMP") return CMP;
  else if (_name == "SOURCE") return SOURCE;
  else if (_name == "RECEIVER") return RECEIVER;
  else if (_name == "OFFSET_BIN") return OFFSET_BIN;
  else if (_name == "STACK") return STACK;
  return UNKNOWN;
}

}
