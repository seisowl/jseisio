/***************************************************************************
                          AxisDefinition.h  -  description
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
  
#include "AxisDefinition.h"
 
#include "PSProLogging.h"
 
namespace jsIO
{
  DECLARE_LOGGER(AxisDefinitionLog);

  AxisDefinition::~AxisDefinition()
  {

  }

  AxisDefinition::AxisDefinition()
  {
    bInit=false;
  }


  AxisDefinition::AxisDefinition(AxisLabel _label, Units _units, DataDomain _domain,
                                 long _length, long _logicalOrigin, long _logicalDelta,
                                 double _physicalOrigin, double _physicalDelta,
                                 std::string _headerName, std::string _headerBinName)
  {
    Init(_label, _units, _domain, _length, _logicalOrigin, _logicalDelta,
         _physicalOrigin, _physicalDelta, _headerName, _headerBinName);
  }
  
  void AxisDefinition::Init(AxisLabel _label, Units _units, DataDomain _domain,
                            long _length, long _logicalOrigin, long _logicalDelta,
                            double _physicalOrigin, double _physicalDelta,
                            std::string _headerName, std::string _headerBinName)
  {
    label  = _label;
    units  = _units;
    domain = _domain; 
    length         = _length;
    logicalOrigin  = _logicalOrigin;
    logicalDelta   = _logicalDelta;
    physicalOrigin = _physicalOrigin;
    physicalDelta  = _physicalDelta;
    headerName = _headerName;
    headerBinName = _headerBinName;
    bInit=true;
  }

  void AxisDefinition::getDefault(int ndim, int *idim, AxisDefinition *& axes)
  {
    axes = new AxisDefinition[ndim];
    AxisLabel* labels;
    AxisLabel::getDefault(ndim,labels);
    for (int i=0; i<ndim; i++) {
      axes[i].Init( labels[i], Units::UNDEFINED, DataDomain::UNDEFINED,
                    (long)idim[i], 0, 1, 0.0, 1.0 ); 
    }
  }


  int AxisDefinition::subRange( long* _range, int _rangelen, AxisDefinition *subAxis)  
  {
    if (_rangelen < 3){
      ERROR_PRINTF(AxisDefinitionLog, "Range must have at least 3 elements");
      return JS_USERERROR;
    }
    if (_range[0] < 0 || _range[1] >= length || _range[1] < _range[0] || _range[2] < 1){
      ERROR_PRINTF(AxisDefinitionLog, "Input range does not conform with AxisDefintion range");
      return JS_USERERROR;
    }
    int _length = 1 + (int)( (_range[1]-_range[0])/_range[2] );
    long _logicalOrigin = logicalOrigin + logicalDelta * _range[0];
    long _logicalDelta = _range[2] * logicalDelta;
    double _physicalOrigin = physicalOrigin + physicalDelta * _range[0];
    double _physicalDelta = _range[2] * physicalDelta;
    subAxis->Init( label, units, domain, _length, _logicalOrigin, _logicalDelta,
                   _physicalOrigin, _physicalDelta);
    return JS_OK;
  }

}
