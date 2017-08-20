
/***************************************************************************
                           CompressedData.cpp  -  description
                             -------------------
 * This class provided a container for a compressed data.

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

  
#include "CompressedData.h"

namespace jsIO
{
  CompressedData::~CompressedData()
  {
    if(compresseddata!=NULL) delete[]compresseddata;
  }

  CompressedData::CompressedData()
  {
    compresseddata=NULL;
  }

  CompressedData::CompressedData(char* _compressedData, unsigned long _dataLength) 
  {
    dataLength = _dataLength;
    compresseddata = new char [dataLength];
    memcpy(compresseddata, _compressedData, dataLength);
  }

  void CompressedData::allocDataSpace(unsigned long nbytes)
  {
    if(compresseddata!=NULL) delete[]compresseddata;
    compresseddata = new char [nbytes];
    dataLength = 0;
  }

  void CompressedData::setDataLength(unsigned long _dataLength) 
  {
    dataLength = _dataLength;
  }

}


