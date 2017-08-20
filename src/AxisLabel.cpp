/***************************************************************************
                          AxisLabel.h  -  description
                             -------------------
 
 * This class encapsulates an axis label.  The primary reason that an axis label is
 * a class, instead of just a String, is to couple a name with a description.
 * A secondary reason is to encourage (but not require) naming convention.
  
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
  
#include "AxisLabel.h"
#include "stringfuncs.h"
 
#include "PSProLogging.h"
 
namespace jsIO
{
  DECLARE_LOGGER(AxisLabelLog);

  const AxisLabel AxisLabel::UNDEFINED("UNDEFINED", "Undefined");
  const AxisLabel AxisLabel::TIME("TIME", "Vertical time");
  const AxisLabel AxisLabel::DEPTH("DEPTH", "Vertical depth");
  const AxisLabel AxisLabel::OFFSET("OFFSET", "Source-receiver offset");
  const AxisLabel AxisLabel::OFFSET_BIN("OFFSET_BIN", "Offset bin #");
  const AxisLabel AxisLabel::CROSSLINE("CROSSLINE", "Crossline #");
  const AxisLabel AxisLabel::INLINE("INLINE", "Inline #");
  const AxisLabel AxisLabel::SOURCE("SOURCE", "Source #");
  const AxisLabel AxisLabel::CMP("CMP", "Common midpoint #");
  const AxisLabel AxisLabel::RECEIVER("RECEIVER", "Receiver #");
  const AxisLabel AxisLabel::RECEIVER_LINE("RECEIVER_LINE", "Receiver line #");
  const AxisLabel AxisLabel::CHANNEL("CHANNEL", "Receiver channel");
  const AxisLabel AxisLabel::SAIL_LINE("SAIL_LINE", "Sail line");
  const AxisLabel AxisLabel::VOLUME("VOLUME", "Volume #");

  AxisLabel::AxisLabel(std::string _name, std::string _description) {
    Init(_name,_description);
  }

  void AxisLabel::Init(std::string _name, std::string _description) {
    if (_name == ""  ||  _description == ""){
      ERROR_PRINTF(AxisLabelLog, "Null input argumentss are not allowed");
      return;
    }  
    name=_name;
    StrToUpper(name);
    description = _description;
  }

  void AxisLabel::getDefault( int ndim, AxisLabel *& axes ) {
    axes = new AxisLabel[ndim];
    axes[0] = AxisLabel( "Sample", "Sample Index");
    if (ndim > 1) axes[1].Init( "Trace", "Trace Index");
    if (ndim > 2) axes[2].Init( "Frame", "Frame Index");
    if (ndim > 3) axes[3].Init( "Volume", "Volume Index");
    if (ndim > 4) axes[4].Init( "Hypercube", "Hypercube Index");
  }
}
