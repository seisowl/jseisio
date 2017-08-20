/***************************************************************************
                          DataFormat.cpp  -  description
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
  
#include "DataFormat.h"

namespace jsIO
{
  const DataFormat DataFormat::FLOAT("FLOAT", "32-bit Float",   4);
  const DataFormat DataFormat::INT16("INT16", "16-bit Integer", 2);
  const DataFormat DataFormat::INT08("INT08", "08-bit Integer", 1);
  const DataFormat DataFormat::COMPRESSED_INT16("COMPRESSED_INT16", "Compressed 16-bit Integer", 2);
  const DataFormat DataFormat::COMPRESSED_INT08("COMPRESSED_INT08", "Compressed 08-bit Integer", 1);
  const DataFormat DataFormat::SEISPEG("SeisPEG Compressed", "SeisPEG 2D Compression", 4);

  DataFormat::DataFormat(std::string _name, std::string _description, int _bytesPerSample) {
    Init(_name,_description,_bytesPerSample);
  }

  void  DataFormat::Init(std::string _name, std::string _description, int _bytesPerSample){
    name = _name;
    description = _description;
    bytesPerSample =_bytesPerSample;
  }

  DataFormat DataFormat::get(std::string _name) {
    if(_name==INT16.getName()) return INT16;
    else if(_name==INT08.getName()) return INT08;
    else if(_name==COMPRESSED_INT16.getName()) return COMPRESSED_INT16;
    else if(_name==COMPRESSED_INT08.getName()) return COMPRESSED_INT08;
    else if(_name=="\""+SEISPEG.getName()+"\"") return SEISPEG;
    return FLOAT;
  } 
}


