/***************************************************************************
                          TraceMap.h  -  description
                             -------------------
 * The TraceMap class provides mapping support for the JavaSeis.
 * This class maintains a map of the number of traces for each frame in a
 * dataset.  This class reads the fold for an entire volume but only writes
 * the fold for a single frame.

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

#ifndef TRACEMAP_H
#define TRACEMAP_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <utility> // make_pair
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>

#include "GridDefinition.h"
#include "xmlreader.h"
#include "CharBuffer.h"
#include "IntBuffer.h"
#include "stringfuncs.h"
#include "jsStrDefs.h"
#include "jsDefs.h"

namespace jsIO
{
  class TraceMap {

    public:
      ~TraceMap();
      TraceMap();
      TraceMap(long* _axisLengths,  int _numAxis, JS_BYTEORDER _byteOrder, std::string path, std::string mode);
      int Init(long* _axisLengths,  int _numAxis, JS_BYTEORDER _byteOrder, std::string path, std::string mode);

      int loadVolume(int frameIndex);
      void emptyCache();
      int getFold(const int* position);
      int getFold(int frameIndex);

      int putFold(int* position, int numTraces);
      int putFold(long glbframeIndex, int numTraces);

      void intializeTraceMapOnDisk();
      const int* getTraceMapArray() const;
      void close();
      long getFrameIndex(const int* position) const;

    private:
      bool m_bInit;

      long* m_pAxisLengths;
      int m_numAxis;
      long m_numFrames;
      int m_nVolumeIndex;
      int m_nCurrentVolIndex;

      std::fstream m_mapIO;

      long m_nReadCacheHit;
      long m_nReadCounter;
      long m_nWriteCounter;

      bool m_bSwapByteOrder; //true if _byteOrder != nativeOrder()

      static const int NOTDEFINDEX = -100;
      int* m_pTraceMapArray;

    private:
      void initTraceMapArray();
      void checkVolumeIndex() const;
      void putFold(int* position, int* fold);
      int writeFrame(int frameIndex);
      void writeVolume();
      long getVolumeOffset(int *position) const;
      int assertAllValuesInitialized();

  };
}

#endif


