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

#ifndef GRIDDEFINITION_H
#define GRIDDEFINITION_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "AxisDefinition.h"

namespace jsIO
{
  class GridDefinition{
    public:
  
      static const int SAMPLE_INDEX = 0;
      static const int TRACE_INDEX  = 1;
      static const int FRAME_INDEX  = 2;
      static const int VOLUME_INDEX = 3;
      static const int HYPERCUBE_INDEX = 4;

      static const int SHOT = 0;
      static const int CDP = 1;
      static const int OFFSET = 2;
      static const int VOLUME = 3;

// private atributes
    private:

      int numDimensions;
      AxisDefinition *axis;

//     bool bInit;
    public:
      ~GridDefinition();
      /** No descriptions */
      GridDefinition();
      GridDefinition(int _numDimensions, AxisDefinition *_axis);
      void Init(int _numDimensions, AxisDefinition *_axis);

      GridDefinition(GridDefinition const& other){CopyClass(other);}; //Copy constructor
      GridDefinition & operator = (const GridDefinition & other)//Assignment operator
      {
        if (this != &other) CopyClass(other);
        return *this;
      } 

      GridDefinition getDefault( int ndim, int* idim ) const;
      int getNumDimensions() const;
      AxisDefinition getAxis(int index) const;
      AxisDefinition* getAxisPtr(int index) const;
      AxisLabel getAxisLabel(int index) const;
      std::string getAxisLabelString(int index) const;
      Units getAxisUnits(int index) const;
      std::string getAxisUnitsString(int index) const;
      DataDomain getAxisDomain(int index) const;
      std::string getAxisDomainString(int index) const;
      long getAxisLength(int index) const;
      long getAxisLogicalOrigin(int index) const;
      long getAxisLogicalDelta(int index) const;
      double getAxisPhysicalOrigin(int index) const;
      double getAxisPhysicalDelta(int index) const;
      long getNumSamplesPerTrace() const;
      long getNumTracesPerFrame() const;
      long getNumFramesPerVolume() const;
      long getNumVolumesPerHypercube() const;
      std::string getIndexName(int index) const;

    private:
      void CopyClass(const GridDefinition & Other);

    protected:

  };
}
 #endif



