/***************************************************************************
                          TraceCompressor.h  -  description
                             -------------------
 * The class provides compression support for (trace) data.
 * For 16 or 8 bit compression, the trace is divided into sample windows. The
 * maximum value in each window is used to scale trace samples to 16 or 8-bits.
 * The floating point scalars are stored in the buffer first, followed by the
 * 16 or 8-bit trace samples. Methods are provided to position the buffer for
 * compression or decompression in trace units.

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

#ifndef TRACECOMPRESSOR_H
#define TRACECOMPRESSOR_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Transformer.h"
#include "../DataFormat.h"
#include "../CharBuffer.h"
#include "../ShortBuffer.h"
#include "../FloatBuffer.h"
#include "../jsDefs.h"

namespace jsIO {
class TraceCompressor {
public:
  ~TraceCompressor();
  /** No descriptions */
  TraceCompressor();
  TraceCompressor(DataFormat _traceFormat, int _numSamples, CharBuffer *_bufferByte);
  int Init(DataFormat _traceFormat, int _numSamples, CharBuffer *_bufferByte);

  static int getRecordLength(DataFormat &_traceFormat, int _numSamples);
  int getPosition() const { return(tracePosition);};
  int setPosition(int _tracePosition);
  int packFrame(int _numTraces, const float *_traceData);
  int unpackFrame(int _numTraces, float *_traceData);
  int packTrace(const float *_traceData);
  void packTrace08(const float *_traceData);
  void packTrace16(const float *_traceData);

  int unpackTrace(float *_traceData);
  void unpackTrace08(float *_traceData);
  void unpackTrace16(float *_traceData);

  void updateBuffer(char *_buffer, long _buffersize);
public:

  // private atributes
private:
  static constexpr float RMAXINT2 = 32766.0f ;
  static constexpr float RMAXINT1 = 126.0f ;

  static const int CLIPPING_MAX_INT08 = 127;
  static const int CLIPPING_MIN_INT08 = -128;
  static const int CLIPPING_MAX_INT16 = 32767;
  static const int CLIPPING_MIN_INT16 = -32768;

  static const int WNDWLEN16 = 100;    // Sample window length for COMPRESSED_INT16 format.
  static const int WNDWLEN08 = 25;     // Sample window length for COMPRESSED_INT08 format.

  DataFormat traceFormat;       // Trace data format.
  int numWindows;               // # of windows for scalar computation.
  int numSamples;               // # of samples.
  int numSamplesX;              // # of samples, extended to nearest factor value.
  int bytesPerSample;           // # of bytes per sample.
  int recordLengthInBytes;      // Record length in bytes.
  int recordLengthInShorts;     // Record length in shorts.
  int recordLengthInFloats;     // Record length in floats.
  int scalarsLengthInBytes;     // Scalar length in bytes.
  int scalarsLengthInShorts;    // Scalar length in shorts.
  int buffer16LengthInFloats;   // 16-bit buffer length in floats.
  int buffer08LengthInFloats;   // 8-bit buffer length in float;
  unsigned short *buffer16;     // 16-bit buffer.
  char  *buffer08;              // 8-bit buffer.
  float *scalars;               // Scalar array
  CharBuffer  bufferViewByte;   // Byte view of storage buffer.
  ShortBuffer bufferViewShort;  // Short view of storage buffer.
  FloatBuffer bufferViewFloat;  // Float view of storage buffer.

  short *traceDataShort;
  char  *traceDataByte;
  int tracePosition;           // Buffer position (in trace units).

private:
  void arrayCopyFloatToShort(int _numSamples, const float *traceIn, short *traceOut);
  void arrayCopyFloatToByte(int _numSamples, const float *traceIn, char *traceOut);
  void arrayCopyShortToFloat(int _numSamples, const short *traceIn, float *traceOut);
  void arrayCopyByteToFloat(int _numSamples, const char *traceIn, float *traceOut);

};

void  testTraceCompressor();
}

#endif



