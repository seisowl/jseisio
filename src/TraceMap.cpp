/***************************************************************************
 TraceMap.cpp -  description
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

#include "TraceMap.h"
#include "PSProLogging.h"
#include <fcntl.h>

namespace jsIO {
DECLARE_LOGGER(TraceMapLog);

TraceMap::~TraceMap() {
  if (m_pTraceMapArray != NULL) delete[] m_pTraceMapArray;
  if (m_pAxisLengths != NULL) delete[] m_pAxisLengths;
}

TraceMap::TraceMap() {
  m_pTraceMapArray = NULL;
  m_pAxisLengths = NULL;
  m_nCurrentVolIndex = -1;
  m_nReadCacheHit = 0;
  m_nReadCounter = 0;
  m_nWriteCounter = 0;
  m_bInit = false;
}

/*
 * Construct an instance of the TraceMap.
 * @param m_pAxisLengths framework lengths
 * @param byteOrder byteOrder on disk
 * @param path path the the dataset
 * @param mode r - read, w - write
 */
TraceMap::TraceMap(long* _axisLengths, int _numAxis, JS_BYTEORDER _byteOrder, std::string path, std::string mode) {
  m_pTraceMapArray = NULL;
  m_pAxisLengths = NULL;
  m_nCurrentVolIndex = -1;
  Init(_axisLengths, _numAxis, _byteOrder, path, mode);
}

int TraceMap::Init(long* _axisLengths, int _numAxis, JS_BYTEORDER _byteOrder, std::string path, std::string mode) {
  m_nVolumeIndex = NOTDEFINDEX;
  m_nReadCacheHit = 0;
  m_nReadCounter = 0;
  m_nWriteCounter = 0;

  if (_numAxis > 0) {
    m_numAxis = _numAxis;
  } else {
    ERROR_PRINTF(TraceMapLog, "Number of axes must be positive.");
    return JS_USERERROR;
  }

  if (_byteOrder != nativeOrder()) m_bSwapByteOrder = true;
  else m_bSwapByteOrder = false;

  m_pAxisLengths = new long[m_numAxis];

  memcpy(m_pAxisLengths, _axisLengths, m_numAxis * sizeof(long));

  //the internal tracMapArray which holds one volume worth of fold info
  m_numFrames = m_pAxisLengths[m_numAxis - 1];

//     TRACE_PRINTF(TraceMapLog, "m_numAxis=%d, m_numFrames=%ld\n", m_numAxis,m_numFrames);

  m_pTraceMapArray = new int[m_numFrames];

//     std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
  StrToLower(mode);

  if (path[path.length() - 1] != '/') path.append(1, '/');
  if (mode.compare("r") == 0) {
    m_mapIO.open((path + JS_TRACE_MAP).c_str(), std::ifstream::in);
    if (!m_mapIO.good()) {
      ERROR_PRINTF(TraceMapLog, "Unable to open file %s%s", path.c_str(), JS_TRACE_MAP.c_str());
      return JS_USERERROR;
    }
  } else {
    m_mapIO.open((path + JS_TRACE_MAP).c_str(), std::ofstream::out);
    if (!m_mapIO.good()) {
      ERROR_PRINTF(TraceMapLog, "Unable to create file %s%s", path.c_str(), JS_TRACE_MAP.c_str());
      return JS_USERERROR;
    }
  }

  initTraceMapArray();
  m_bInit = true;
  return JS_OK;
}

void TraceMap::checkVolumeIndex() const {
  if (m_nVolumeIndex == NOTDEFINDEX) {
    ERROR_PRINTF(TraceMapLog, "Volume index was not initialized.");
  }
}

/* Returns the fold at the position. */
int TraceMap::getFold(const int* position) {
  // This will load the fold for the entire volume
  long frameIndex = getFrameIndex(position);
  loadVolume(frameIndex);
  return (m_pTraceMapArray[position[m_numAxis - 1]]);
}
/* Returns the fold for the frame with index frameIndex */
int TraceMap::getFold(int frameIndex) {
  // This will load the fold for the entire volume
  loadVolume(frameIndex);
  int pos = frameIndex - (int) (frameIndex / m_numFrames) * m_numFrames;
  return (m_pTraceMapArray[pos]);
}

/*
 * Causes the cache to be invalidated so that any calls to
 * get/put fold will then be forced to do a read.
 */
void TraceMap::emptyCache() {
  m_nVolumeIndex = NOTDEFINDEX;
}

/** Sets the fold at the logical position. */
int TraceMap::putFold(int* position, int numTraces) {
  m_nWriteCounter++;
  // insert the fold value into the array
  int framelocIndex = position[m_numAxis - 1];
  m_pTraceMapArray[framelocIndex] = numTraces;
  long oldMapFilePosition = getVolumeOffset(position); //4 * (m_pAxisLengths[GridDefinition::FRAME_INDEX] * volIndex + frameIndex);
  m_mapIO.seekp(oldMapFilePosition);
  long newMapFilePosition = m_mapIO.tellp();
  if (newMapFilePosition != oldMapFilePosition) {
    ERROR_PRINTF(TraceMapLog, "Unable to seek to file offset %lu", oldMapFilePosition);
    return JS_USERERROR;
  }

  if (m_bSwapByteOrder) endian_swap((void*) &numTraces, 1, sizeof(int));
  m_mapIO.write((const char*) &numTraces, sizeof(int)); //m_mapIO.write(mapBuffer.array(),sizeof(int));
  return JS_OK;
}

int TraceMap::putFold(long glbframeIndex, int numTraces) {
  m_nWriteCounter++;
  // insert the fold value into the array
  long oldMapFilePosition = glbframeIndex * sizeof(int);
  m_mapIO.seekp(oldMapFilePosition);
  long newMapFilePosition = m_mapIO.tellp();
  if (newMapFilePosition != oldMapFilePosition) {
    ERROR_PRINTF(TraceMapLog, "Unable to seek to file offset %lu", oldMapFilePosition);
    return JS_USERERROR;
  }

  if (m_bSwapByteOrder) endian_swap((void*) &numTraces, 1, sizeof(int));
  m_mapIO.write((const char*) &numTraces, sizeof(int));
  return JS_OK;
}

/**
 * Sets the fold for an entire volume.  This does not attempt to merge
 * the fold values for frames within this volume.  The typical use for
 * this method is during initialization of the foldmap.  (Use the method
 * above to only write the fold for a single frame when writting to the
 * dataset in parallel.
 **/
void TraceMap::putFold(int* position, int* fold) {
  // This will index to the volume and load it from disk
  long frInd = getFrameIndex(position);
  loadVolume(frInd);
  checkVolumeIndex();

  // insert the fold value into the array
  for (int i = 0; i < m_numFrames; i++)
    m_pTraceMapArray[i] = fold[i];

  writeVolume();
}

/**
 * Re-initializes the trace map array.
 */
void TraceMap::initTraceMapArray() {
  for (int i = 0; i < m_numFrames; i++)
    m_pTraceMapArray[i] = NOTDEFINDEX;
}

/**
 * Initialize the trace map on disk by setting all values in the map to zero
 * This is not thread safe or parallel IO safe. The caller should make sure
 * that this is only done by a single node.
 *
 * @throws SeisException
 */
void TraceMap::intializeTraceMapOnDisk() {
  for (int frameIndex = 0; frameIndex < m_numFrames; frameIndex++) {
    m_pTraceMapArray[frameIndex] = 0;
  }

  int totalNumVols = 1;
  for (int i = 3; i < m_numAxis; i++)
    totalNumVols *= m_pAxisLengths[i];

  m_mapIO.seekp(std::ios_base::beg);
  if (m_bSwapByteOrder) endian_swap((void*) m_pTraceMapArray, m_numFrames, sizeof(int));

  for (int i = 0; i < totalNumVols; i++) {
    m_mapIO.write((const char*) m_pTraceMapArray, m_numFrames * sizeof(int));
  }

  m_mapIO.flush();

  initTraceMapArray();
}

/**
 * Write a frame of fold to disk.  This assumes that the m_nVolumeIndex
 * has already been set and that the fold has been put into the
 * array.  This is not intended to be called externally.
 */
int TraceMap::writeFrame(int frameIndex) {
  m_nWriteCounter++;
  checkVolumeIndex();

  long oldMapFilePosition = 4 * (m_pAxisLengths[GridDefinition::FRAME_INDEX] * m_nVolumeIndex + frameIndex);

  m_mapIO.seekp(oldMapFilePosition);
  long newMapFilePosition = m_mapIO.tellp();
  if (newMapFilePosition != oldMapFilePosition) {
    ERROR_PRINTF(TraceMapLog, "Unable to seek to file offset %lu", oldMapFilePosition);
    return JS_USERERROR;
  }
  m_mapIO.write((const char*) &m_pTraceMapArray[frameIndex], sizeof(int));
  return JS_OK;
}

/**
 * Write an entire volume of fold to disk.  This assumes that the m_nVolumeIndex
 * has already been set and that the fold has been put into the
 * array.  This is not intended to be called externally.
 */
void TraceMap::writeVolume() {
  checkVolumeIndex();
  m_mapIO.write((const char*) m_pTraceMapArray, m_numFrames * sizeof(int));
}

/*
 * Based on the position find the correct volume and load the
 * data.  We use the position array so that we can use this to
 * index to the correct position in 3, 4 and 5D datasets.
 */
int TraceMap::loadVolume(int frameIndex) {
  int volIndex = (int) (frameIndex / m_numFrames);
  if (m_nCurrentVolIndex == volIndex) {
    //if we already have the volume loaded don't load it again
    m_nReadCacheHit++;
    return JS_OK;
  } else {
    m_nReadCounter++;
    long oldMapFilePosition = volIndex * m_numFrames * sizeof(int); //getVolumeOffset(frameIndex);

    m_mapIO.seekg(oldMapFilePosition);
    long newMapFilePosition = m_mapIO.tellg();
    if (newMapFilePosition != oldMapFilePosition) {
      ERROR_PRINTF(TraceMapLog, "Unable to seek to file offset %lu", oldMapFilePosition);
      return JS_USERERROR;
    }
    m_mapIO.read((char*) m_pTraceMapArray, m_numFrames * sizeof(int));
    if (!m_mapIO.good()) {
      // No data present. (This happens during initilization)
      initTraceMapArray();
    } else {
      if (m_bSwapByteOrder) endian_swap((void*) m_pTraceMapArray, m_numFrames, sizeof(int));
    }
  }
  m_nCurrentVolIndex = volIndex;

  return JS_OK;
}

long TraceMap::getVolumeOffset(int *position) const {
  int frameIndex = getFrameIndex(position);
  int volIndex = (int) (frameIndex / m_numFrames);
  return volIndex * m_numFrames * sizeof(int);
}

long TraceMap::getFrameIndex(const int* position) const {
  long frIndex = 0;
  for (int i = 0; i < m_numAxis; i++) {
    int volsize = 1;
    for (int j = m_numAxis - i - 1; j >= 1; j--) {
      volsize *= m_pAxisLengths[j];
    }
    frIndex += position[i] * volsize;
  }
  return frIndex;
}

/** Return the tracemap for the current volume */
const int* TraceMap::getTraceMapArray() const {
  return (const int*) m_pTraceMapArray;
}

/**
 * Asserts that all trace map values have been initialized (i.e. > 0). Used
 * for testing.
 */
int TraceMap::assertAllValuesInitialized() {

  for (int frameIndex = 0; frameIndex < m_numFrames; frameIndex++) {
    if (m_pTraceMapArray[frameIndex] < 1) {
      ERROR_PRINTF(TraceMapLog, "Invalid trace map value (%d) found at frame index %d", m_pTraceMapArray[frameIndex],
          frameIndex);
      return JS_USERERROR;
    }
  }
  return JS_OK;
}

void TraceMap::close() {
//    TRACE_PRINTF(TraceMapLog, "TraceMap reads = %li, read cached hits %li, TraceMap writes = %li",m_nReadCounter,m_nReadCacheHit,m_nWriteCounter) ;
  m_mapIO.close();
}

}

