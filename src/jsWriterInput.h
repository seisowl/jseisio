/***************************************************************************
 jsWriterInput.h  -  description
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

#ifndef JSWRITERINPUT_H
#define JSWRITERINPUT_H

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <vector>

#include "DataType.h"
#include "DataFormat.h"
#include "DataDomain.h"
#include "AxisLabel.h"
#include "Units.h"

namespace jsIO {
class GridDefinition;
class DataDefinition;
class TraceProperties;
//   struct SurveyGeometry;
class CustomProperties;

/**
 * This class is for initalization of jsIO::jsFileWriter.
 * Here the user shall define all parameters required for metadata initalization
 * of a dataset in JavaSeis format, like data and framework definition, header-words set etc.
 * For usage examples see examples/testWriter.cpp
 */
class jsWriterInput {
  friend class jsFileWriter;

public:
  jsWriterInput();
  ~jsWriterInput();

  jsWriterInput(jsWriterInput const& _other);
  jsWriterInput & operator =(const jsWriterInput & _other) //Assignment operator
      {
    if (this != &_other) CopyClass(_other);
    return *this;
  }

  /**
   * Set the full name (path included) of the JavaSeis dataset.
   */
  void setFileName(const std::string _filename) {
    jsfilename = _filename;
  }

  void setFileDescription(const std::string _description) {
    description = _description;
  }
  void setMapped(bool _mapped) {
    isMapped = _mapped;
  }

  /**
   * @brief Set cache size used for writing to files.
   * @details Default is 2MB. Set 0 to write directly (not cached).
   */
  void setDiskCacheSize(unsigned long _cacheSize) {
    IOBufferSize = _cacheSize;
  }

  /**
   * @brief Set number of extents to use.
   * @details This number defines to how many parts/extents TraceData and TraceHeader will be splited.
   */
  void setNumberOfExtents(const int _numExtends) {
    NExtends = _numExtends;
  }
  /**
   * @brief Initalize data context
   * @param _dataType data type, e.g. CUSTOM, SOURCE, OFFSET_BIN etc.
   * @param _dataFormat data format, e.g. FLOAT, COMPRESSED_INT16 etc.
   * @param _byteOrder byte order, e.g. JSIO_LITTLEENDIAN. Default is native byte order.
   */
  void initData(const DataType &_dataType, const DataFormat &_dataFormat, JS_BYTEORDER _byteOrder = nativeOrder());

  /**
   * @brief Set seispeg policy
   * @details In case of SeisPEG format set seispeg policy, where  0 = FASTEST, 1 = MAX_COMPRESSION
   */
  void setSeispegPolicy(int _policy) {
    seispegPolicy = _policy;
  }

  ///Initialize data grid dimensions
  void initGridDim(int _numDim);

  /**
   * @brief Initialize grid axes
   * @param _axisInd Axis index (from 0 up to _numDim-1), where 0 is the fastest axis
   * @param  _label Axis Label, e.g. TIME, OFFSET
   * @param _units Axis units, e.g. SECONDS, METERS
   * @param _domain Data domain, e.g. TIME, SPACE
   * @param _length Axis lenght
   * @param _logicalOrigin Axis logical origin
   * @param _logicalDelta Axis logical delta
   * @param _physicalOrigin Axis physical origin
   * @param _physicalDelta Axis physical delta
   * @param _headerName Header-word or Property name conected with this axis physical value.
   *                    This is not a requred parameter.
   * @param _headerBinName Header-word or Property name conected with this axis logical value.
   *                     This is not a requred parameter.
   */
  int initGridAxis(int _axisInd, AxisLabel _label, Units _units, DataDomain _domain, long _length, long _logicalOrigin,
      long _logicalDelta, double _physicalOrigin, double _physicalDelta, std::string _headerName = "",
      std::string _headerBinName = "");

  /**
   * @brief Adds set of default properties/header-words.
   * @details This includes also the properties that are necessary for SeisSpace compatibility
   */
  void addDefaultProperties();
  int addProperty(std::string _label);
  int addProperty(std::string _label, std::string _description, std::string _format, int _count);

  ///Adds secondary path where the data extents may be stored
  void add_secondary_path(std::string _path) {
    virtualFolders.push_back(_path);
  }

  /** @brief Adds survey geometry given by 3 points
   * @details The geometry is not a requied parameter and will be written in CustomProperties part of FileProperties.xml
   */
  void addSurveyGeom(int i1, int i2, int i3, int i4, float f1, float f2, float f3, float f4, float f5, float f6);

  void addCustomProperty(std::string name, std::string type, std::string value);

private:
  void CopyClass(const jsWriterInput & Other);

private:
  int numGridAxis;
  std::string jsfilename;
  std::string description;
  int NExtends;
  bool isMapped;
  unsigned long IOBufferSize; //set 0, to write directly (not cached)
  std::vector<std::string> virtualFolders;
  GridDefinition *gridDef;
  DataDefinition *dataDef;
  TraceProperties *traceProps;
//       SurveyGeometry *geometry;
  int seispegPolicy; //0-Fastest, 1-MaxCompression

  CustomProperties *customProps;
};
}

#endif
