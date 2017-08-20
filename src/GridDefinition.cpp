/***************************************************************************
                          GridDefinition.h  -  description
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
  
#include "GridDefinition.h"
#include "PSProLogging.h"
 
namespace jsIO
{
  DECLARE_LOGGER(GridDefinitionLog);

  GridDefinition::~GridDefinition()
  {
    if(axis!=NULL) delete [] axis;
  }

  GridDefinition::GridDefinition()
  {
    axis = NULL;
  }

  GridDefinition::GridDefinition(int _numDimensions, AxisDefinition *_axis)
  {
    Init(_numDimensions, _axis);
  }
 
  void GridDefinition::Init(int _numDimensions, AxisDefinition *_axis)
  {
    if(_numDimensions < 1) {
      ERROR_PRINTF(GridDefinitionLog, "Number of dimensions must be greater than 0.");
    }
    numDimensions = _numDimensions;
    axis = new AxisDefinition[numDimensions];
    for(int i=0;i<numDimensions;i++){
      axis[i] = _axis[i];
    }
  }


  void GridDefinition::CopyClass(const GridDefinition & Other)
  {
    numDimensions=Other.numDimensions;
    if(axis!=NULL) delete [] axis;

    axis = new AxisDefinition[numDimensions];
    for(int i=0;i<numDimensions;i++){
      axis[i] = Other.axis[i];
    }
  }


  GridDefinition GridDefinition::getDefault( int ndim, int* idim ) const
  {
    AxisDefinition * _axes;
    AxisDefinition::getDefault(ndim, idim, _axes);
    GridDefinition gd( ndim, _axes);
    delete[]_axes;
    return gd;
  }

  int GridDefinition::getNumDimensions() const
  {
    return(numDimensions);
  }


  AxisDefinition GridDefinition::getAxis(int index) const
  {
    return(axis[index]);
  }

  AxisDefinition* GridDefinition::getAxisPtr(int index) const
  {
    return(&axis[index]);
  }

  AxisLabel GridDefinition::getAxisLabel(int index) const
  {
    return(axis[index].getLabel());
  }


  std::string GridDefinition::getAxisLabelString(int index) const
  {
    return(axis[index].getLabel().toString());
  }


  Units GridDefinition::getAxisUnits(int index) const
  {
    return(axis[index].getUnits());
  }


  std::string GridDefinition::getAxisUnitsString(int index) const
  {
    return(axis[index].getUnits().toString());
  }


  DataDomain GridDefinition::getAxisDomain(int index) const
  {
    return(axis[index].getDomain());
  }


  std::string GridDefinition::getAxisDomainString(int index) const
  {
    return(axis[index].getDomain().toString());
  }


  long GridDefinition::getAxisLength(int index) const
  {
    return(axis[index].getLength());
  }


  long GridDefinition::getAxisLogicalOrigin(int index) const
  {
    return(axis[index].getLogicalOrigin());
  }


  long GridDefinition::getAxisLogicalDelta(int index) const
  {
    return(axis[index].getLogicalDelta());
  }


  double GridDefinition::getAxisPhysicalOrigin(int index) const
  {
    return(axis[index].getPhysicalOrigin());
  }


  double GridDefinition::getAxisPhysicalDelta(int index) const
  {
    return(axis[index].getPhysicalDelta());
  }


  long GridDefinition::getNumSamplesPerTrace() const
  {
    return(axis[SAMPLE_INDEX].getLength());
  }


  long GridDefinition::getNumTracesPerFrame() const
  {
    return(axis[TRACE_INDEX].getLength());
  }


  long GridDefinition::getNumFramesPerVolume() const
  {
    return(axis[FRAME_INDEX].getLength());
  }


  long GridDefinition::getNumVolumesPerHypercube() const
  {
    return(axis[VOLUME_INDEX].getLength());
  }

  std::string GridDefinition::getIndexName(int index) const 
  {
    switch (index) {
      case SAMPLE_INDEX:
        return "Sample" ;
      case TRACE_INDEX:
        return "Trace" ;
      case FRAME_INDEX:
        return "Frame" ;
      case VOLUME_INDEX:
        return "Volume" ;
      case HYPERCUBE_INDEX:
        return "Hypercube";
    }
    return "" ;  //never get here
  }
}


