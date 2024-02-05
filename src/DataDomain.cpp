/***************************************************************************
 DataDomain.h  -  description
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
#include "stringfuncs.h"
#include "DataDomain.h"

namespace jsIO {
const DataDomain DataDomain::SPACE("space", "Space in units of distance");
const DataDomain DataDomain::TIME("time", "Time in seconds or milliseconds");
const DataDomain DataDomain::DEPTH("depth", "Vertical distance");
const DataDomain DataDomain::FREQUENCY("frequency", "Fourier dual of time");
const DataDomain DataDomain::WAVENUMBER("wavenumber", "Fourier dual of space");
const DataDomain DataDomain::SEMBLANCE("semblance", "Normalized coherency measurement");
const DataDomain DataDomain::VELOCITY("velocity", "Velocity in units of space over time");
const DataDomain DataDomain::DIP("dip", "Dip in units of time over space");
const DataDomain DataDomain::VSVP("vsvp", "Ratio of shear velocity to pressure velocity");
const DataDomain DataDomain::SLOWNESS("slowness", "Reciprocol velocity");
const DataDomain DataDomain::ETA("eta", "Anisotropic Thompsen parameter eta");
const DataDomain DataDomain::SLOTH("sloth", "Squared reciprocol velocity");
const DataDomain DataDomain::EPSILON("epsilon", "Anisotropic Thompsen parameter epsilon");
const DataDomain DataDomain::DELTA("delta", "Anisotropic Thompsen parameter delta");
const DataDomain DataDomain::ALACRITY("alacrity", "Squared velocity");
const DataDomain DataDomain::AMPLITUDE("amplitude", "Amplitude domain");
const DataDomain DataDomain::COHERENCE("coherence", "Non-semblance measurement of coherency");
const DataDomain DataDomain::ENVELOPE("envelope", "Analytic envelope of a waveform");
const DataDomain DataDomain::IMPEDANCE("impedance", "Rock property impedance in units of density*velocity");
const DataDomain DataDomain::DENSITY("density", "Density in units of mass per volume");
const DataDomain DataDomain::VS("vs", "Shear velocity");
const DataDomain DataDomain::FOLD("fold", "Number of independent data that have been summed for a result");
const DataDomain DataDomain::INCIDENCE_ANGLE("incidence angle",
    "Angle at which a ray (or normal to a wavefront) strikes a surface");
const DataDomain DataDomain::ROTATION_ANGLE("rotation angle", "Angle of rotation of two traces");
const DataDomain DataDomain::MODEL_TRANSFORM("model transform", "Model-based transform (ProMAX)");
const DataDomain DataDomain::FLEX_BINNED("flex binned", "Flex-binned data (ProMAX)");
const DataDomain DataDomain::UNDEFINED("null", "Unspecified domain"); //named NULL in java version
const DataDomain DataDomain::UNKNOWN("unknown", "Unknown domain");

DataDomain::DataDomain(std::string _name, std::string _description) {
  Init(_name, _description);
}

DataDomain::DataDomain(std::string _name) {
  Init(_name);
}

void DataDomain::Init(std::string _name) {
  name = _name;
  description = _name;
  StrToLower(name);
}

void DataDomain::Init(std::string _name, std::string _description) {
  name = _name;
  description = _description;
  StrToLower(name);
}

}
