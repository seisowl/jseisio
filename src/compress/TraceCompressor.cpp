/***************************************************************************
                          TraceCompressor.cpp  -  description
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

#include "TraceCompressor.h"
#include "../PSProLogging.h"

namespace jsIO {
DECLARE_LOGGER(TraceCompressorLog);

TraceCompressor::~TraceCompressor() {
  if(buffer16 != NULL) delete[]buffer16;
  if(buffer08 != NULL) delete[]buffer08;
  if(scalars != NULL) delete[]scalars;
  if(traceDataShort != NULL) delete[]traceDataShort;
  if(traceDataByte != NULL) delete[]traceDataByte;
}

TraceCompressor::TraceCompressor() {
  buffer16 = NULL;
  buffer08 = NULL;
  scalars = NULL;
  traceDataShort = NULL;
  traceDataByte = NULL;
}

/**
* Constructs an instace of TraceCompressor.
* @param traceFormat
*   The format type (i.e. DataFormat.INT16, etc).
* @param numSamples
*   The number of samples per trace.
* @param bufferByte
*   The buffer used to store compressed trace data.
 */
TraceCompressor::TraceCompressor(DataFormat _traceFormat, int _numSamples, CharBuffer *_bufferByte) {
  buffer16 = NULL;
  buffer08 = NULL;
  scalars = NULL;
  traceDataShort = NULL;
  traceDataByte = NULL;
  Init(_traceFormat, _numSamples, _bufferByte);
}


int TraceCompressor::Init(DataFormat _traceFormat, int _numSamples, CharBuffer *_bufferByte) {
  int bytesPerSample = 0;
  int remainder = 0;
  // Set initial trace position.
  tracePosition = 0;

  if(buffer16 != NULL) delete[]buffer16;
  if(buffer08 != NULL) delete[]buffer08;
  if(scalars != NULL) delete[]scalars;
  if(traceDataShort != NULL) delete[]traceDataShort;
  if(traceDataByte != NULL) delete[]traceDataByte;

  buffer16 = NULL;
  buffer08 = NULL;
  scalars = NULL;
  traceDataShort = NULL;
  traceDataByte = NULL;

  if(_traceFormat.getName() == "SEISPEG" || _traceFormat.getName() == "FLOAT" ||
      _traceFormat.getName() == "INT16"   || _traceFormat.getName() == "INT08" ||
      _traceFormat.getName() == "COMPRESSED_INT16"   || _traceFormat.getName() == "COMPRESSED_INT08") {
    traceFormat = _traceFormat;
    bytesPerSample = traceFormat.getBytesPerSample();
  } else {
    traceFormat = DataFormat::FLOAT;
    bytesPerSample = traceFormat.getBytesPerSample();
  }

  // Set trace format.
  if(traceFormat.getName() == "SEISPEG") {
    ERROR_PRINTF(TraceCompressorLog, "Cannot use TraceCompressor for SEISPEG format");
    return JS_USERERROR;
  } else if(traceFormat.getName() == "FLOAT") {
    numSamples = _numSamples;
    recordLengthInFloats = numSamples;
    _bufferByte->asFloatBuffer(bufferViewFloat);
  } else if(traceFormat.getName() == "INT16") {
    numSamples = _numSamples;
    recordLengthInShorts = numSamples;
    _bufferByte->asShortBuffer(bufferViewShort);
    traceDataShort = new short[numSamples];
  } else if(traceFormat.getName() == "INT08") {
    numSamples = _numSamples;
    recordLengthInBytes  = numSamples;
    _bufferByte->asByteBuffer(bufferViewByte);
    traceDataByte = new char[numSamples];
  } else if(traceFormat.getName() == "COMPRESSED_INT16") {
    numSamples = _numSamples;
    numWindows = ((numSamples - 1) / WNDWLEN16 + 1);
    remainder = (numSamples % 2);
    numSamplesX = ((remainder == 0) ? numSamples : numSamples + (2 - remainder));
    recordLengthInBytes = (4 * numWindows) + (bytesPerSample * numSamplesX);
    //             printf("\nnumWindows=%d,numSamplesX=%d\n",numWindows,numSamplesX);
    scalars = new float[numWindows];
    buffer16 = new unsigned short[numSamplesX];
    _bufferByte->asByteBuffer(bufferViewByte);
    _bufferByte->asFloatBuffer(bufferViewFloat);
    recordLengthInFloats = (int) recordLengthInBytes / 4;
    recordLengthInShorts = (int) recordLengthInBytes / 2;
    scalarsLengthInShorts = numWindows * 2;
    scalarsLengthInBytes = scalarsLengthInShorts * 2;
    buffer16LengthInFloats = (int) numSamplesX / 2;
  } else if(traceFormat.getName() == "COMPRESSED_INT08") {
    //            printf("COMPRESSED_INT08\n");
    numSamples = _numSamples;
    numWindows = (numSamples + WNDWLEN08 - 1) / WNDWLEN08;
    remainder = (numSamples % 4);
    numSamplesX = ((remainder == 0) ? numSamples : numSamples + (4 - remainder));
    recordLengthInBytes = (4 * numWindows) + (bytesPerSample * numSamplesX);
    //             printf("numWindows=%d, numSamplesX=%d\n",numWindows, numSamplesX);
    scalars = new float[numWindows];
    buffer08 = new char[numSamplesX];
    _bufferByte->asByteBuffer(bufferViewByte);
    _bufferByte->asFloatBuffer(bufferViewFloat);
    recordLengthInFloats = (int) recordLengthInBytes / 4;
    scalarsLengthInBytes = numWindows * 4;
    buffer08LengthInFloats = (int) numSamplesX / 4;
  }

  return JS_OK;
}

void TraceCompressor::updateBuffer(char *_buffer, long _buffersize) {
  bufferViewByte.wrap(_buffer, _buffersize);
  bufferViewByte.asFloatBuffer(bufferViewFloat);
  bufferViewByte.asShortBuffer(bufferViewShort);
  bufferViewByte.position(0);
  bufferViewFloat.position(0);
  bufferViewShort.position(0);
}


/**
 * Gets the record length in bytes.
 * For this purpose SeisPEG is treated the same as 4-byte format.
 * @param traceFormat
 *    The trace format.
 * @param numSamples
 *    The number of samples per trace.
 * @return The record length in bytes.
 */
int TraceCompressor::getRecordLength(DataFormat &_traceFormat, int _numSamples) {
  int length = 0;
  int numWindows = 0;
  int numSamplesX = 0;
  int bytesPerSample = 0;
  int remainder;

  if(_traceFormat.getName() == "COMPRESSED_INT16") {
    numWindows = ((_numSamples - 1) / WNDWLEN16 + 1);
    remainder = _numSamples % 2;
    numSamplesX = ((remainder == 0) ? _numSamples : _numSamples + (2 - remainder));
    bytesPerSample = _traceFormat.getBytesPerSample();
    length = (4 * numWindows) + (bytesPerSample * numSamplesX);
  } else if(_traceFormat.getName() == "COMPRESSED_INT08") {
    numWindows = (_numSamples + WNDWLEN08 - 1) / WNDWLEN08;
    remainder = _numSamples % 4;
    numSamplesX = ((remainder == 0) ? _numSamples : _numSamples + (4 - remainder));
    bytesPerSample = _traceFormat.getBytesPerSample();
    length = (4 * numWindows) + (bytesPerSample * numSamplesX);
  } else {
    bytesPerSample = _traceFormat.getBytesPerSample();
    length = (bytesPerSample * _numSamples);
  }

  //       printf("length=%d, bytesPerSample=%d\n",length, bytesPerSample);

  return length;
}

/**
* Sets the buffer position in trace units.
* @param tracePosition
*    The desired buffer position in trace units.
* @return The buffer position in trace units.
 */
int TraceCompressor::setPosition(int _tracePosition) {
  tracePosition = _tracePosition;

  if(traceFormat.getName() == "SEISPEG") {
    ERROR_PRINTF(TraceCompressorLog, "Cannot use TraceCompressor for SEISPEG format");
    return JS_USERERROR;
  } else if(traceFormat.getName() == "FLOAT") {
    bufferViewFloat.position((size_t)tracePosition * recordLengthInFloats);
  } else if(traceFormat.getName() == "INT16") {
    bufferViewShort.position((size_t)tracePosition * recordLengthInShorts);
  } else if(traceFormat.getName() == "INT08") {
    bufferViewByte.position((size_t)tracePosition * recordLengthInBytes);
  } else if(traceFormat.getName() == "COMPRESSED_INT16") {
    // Short view needed for DataFormat.COMPRESSED_INT16.
    bufferViewByte.position((size_t)tracePosition * recordLengthInBytes);
    bufferViewFloat.position((size_t)tracePosition * recordLengthInFloats);
  } else if(traceFormat.getName() == "COMPRESSED_INT08") {
    bufferViewByte.position((size_t)tracePosition * recordLengthInBytes);
    bufferViewFloat.position((size_t)tracePosition * recordLengthInFloats);
  }

  return(tracePosition);
}


/**
* Packs frame data into compression buffer.
* @param traceData
*    The array of frame data to compress.
 */
int TraceCompressor::packFrame(int _numTraces, const float *_traceData) {

  if(traceFormat.getName() == "SEISPEG") {
    ERROR_PRINTF(TraceCompressorLog, "Cannot use TraceCompressor for SEISPEG format");
    return JS_USERERROR;
  } else if(traceFormat.getName() == "FLOAT" ||
            traceFormat.getName() == "INT16" ||
            traceFormat.getName() == "INT08") {
    for(int i = 0; i < _numTraces; i++) {
      setPosition(i);
      packTrace(&_traceData[(size_t)i * numSamples]);
    }
  } else if(traceFormat.getName() == "COMPRESSED_INT16") {
    for(int i = 0; i < _numTraces; i++) {
      setPosition(i);
      packTrace16(&_traceData[(size_t)i * numSamples]);
    }
  } else if(traceFormat.getName() == "COMPRESSED_INT08") {
    for(int i = 0; i < _numTraces; i++) {
      setPosition(i);
      packTrace08(&_traceData[(size_t)i * numSamples]);
    }
  }

  return JS_OK;
}


/**
* Unpacks frame data from compression buffer.
* @param traceData
*    The array to contain decompressed frame data.
 */
int TraceCompressor::unpackFrame(int _numTraces, float *_traceData) {
  //      printf( "recordLengthInBytes=%d, recordLengthInFloats=%d\n",recordLengthInBytes,recordLengthInFloats);
  if(traceFormat.getName() == "SEISPEG") {
    ERROR_PRINTF(TraceCompressorLog, "Cannot use TraceCompressor for SEISPEG format");
    return JS_USERERROR;
  } else if(traceFormat.getName() == "FLOAT" ||
            traceFormat.getName() == "INT16" ||
            traceFormat.getName() == "INT08") {
    for(int i = 0; i < _numTraces; i++) {
      setPosition(i);
      unpackTrace(&_traceData[(size_t)i * numSamples]);
    }
  } else if(traceFormat.getName() == "COMPRESSED_INT16") {
    for(int i = 0; i < _numTraces; i++) {
      setPosition(i);
      unpackTrace16(&_traceData[(size_t)i * numSamples]);
    }
  } else if(traceFormat.getName() == "COMPRESSED_INT08") {
    for(int i = 0; i < _numTraces; i++) {
      setPosition(i);
      unpackTrace08(&_traceData[(size_t)i * numSamples]);
    }
  }

  return JS_OK;
}


/**
* Packs trace data into DataFormat.FLOAT,INT16,INT08 buffer.
* @param traceData
*    The array of trace data to compress.
 */
int TraceCompressor::packTrace(const float *_traceData) {
  if(traceFormat.getName() == "SEISPEG") {
    ERROR_PRINTF(TraceCompressorLog, "Cannot use TraceCompressor for SEISPEG format");
    return JS_USERERROR;
  } else if(traceFormat.getName() == "FLOAT") {
    bufferViewFloat.put(_traceData, numSamples);
  } else if(traceFormat.getName() == "INT16") {
    arrayCopyFloatToShort(numSamples, _traceData, traceDataShort);
    bufferViewShort.put(traceDataShort, numSamples);
  } else if(traceFormat.getName() == "INT08") {
    arrayCopyFloatToByte(numSamples, _traceData, traceDataByte);
    bufferViewByte.put(traceDataByte, numSamples);
  }
  return JS_OK;
}

/**
* Packs trace data into DataFormat.COMPRESSED_INT08 buffer.
* @param traceData
*    The array of trace data to compress.
 */
void TraceCompressor::packTrace08(const float *_traceData) {
  float scalar, valueAbs, valueMax;
  char b;
  int k1 = 0;
  int k2 = 0;
  for(int i = 0; i < numWindows; i++) {
    // Set sample range.
    k1 = k2;
    k2 = k1 + WNDWLEN08;
    if(k2 > numSamples) k2 = numSamples;
    // Find maximum absolute value in window.
    valueMax = 0.0f;
    for(int k = k1; k < k2; k++) {
      valueAbs = fabs(_traceData[k]);
      if(valueAbs > valueMax) valueMax = valueAbs;
    }
    // Set scale factor.
    scalar = 0;
    if(valueMax > 0.0) {
      scalar = RMAXINT1 / valueMax;
    }
    scalars[i] = scalar;
    // Scale float values to byte (8-bit) values.
    for(int k = k1; k < k2; k++) {
      b = (char)((short)(127.5 + (scalar * _traceData[k])));
      buffer08[k] = (char)(b & 0xFF);
    }
  }

  // Put scalars into "float" view buffer.
  bufferViewFloat.put(scalars, numWindows);
  // Update position of "byte" view buffer.
  bufferViewByte.position(bufferViewByte.position() + scalarsLengthInBytes);
  // Put DataFormat.COMPRESSED_INT08 samples into "short" view buffer.
  bufferViewByte.put(buffer08, numSamplesX);
  //       bufferViewByte.put((char*)buffer08, numSamples);//*sizeof(char)==1

  // Update position of "float" view buffer.
  bufferViewFloat.position(bufferViewFloat.position() + buffer08LengthInFloats);
}


/**
* Packs trace data into DataFormat.COMPRESSED_INT16 buffer.
* @param traceData
*    The array of trace data to compress.
 */
void TraceCompressor::packTrace16(const float *_traceData) {
  float scalar, value, valueMax;
  int k1 = 0;
  int k2 = 0;

  for(int i = 0; i < numWindows; i++) {
    // Set sample range.
    k1 = k2;
    k2 = k1 + WNDWLEN16;
    if(k2 > numSamples) k2 = numSamples;
    // Find maximum absolute value in window.
    valueMax = 0.0f;
    for(int k = k1; k < k2; k++) {
      value = fabs(_traceData[k]);
      if(value > valueMax) valueMax = value;
    }
    // Set scale factor.
    scalar = 0;
    if(valueMax > 0.0) {
      scalar = RMAXINT2 / valueMax;
    }
    scalars[i] = scalar;
    // Scale float values to short (16-bit) values.
    for(int k = k1; k < k2; k++) {
      buffer16[k] = (unsigned short)(32767.5 + scalar * _traceData[k]);
    }
  }

  // Put scalars into "float" view buffer.
  bufferViewFloat.put(scalars, numWindows);
  // Update position of "byte" view buffer.
  bufferViewByte.position(bufferViewByte.position() + scalarsLengthInBytes);

  // Put DataFormat.COMPRESSED_INT16 samples into "byte" view buffer.
  bufferViewByte.put((char *)buffer16, numSamples * sizeof(short));
  /*
      for(int k = 0; k < numSamples; k++) {
      bufferViewByte.putShort(buffer16[k]);
    }
  */
  // Update position of "float" view buffer.
  bufferViewFloat.position(bufferViewFloat.position() + buffer16LengthInFloats);

}

/**
* Unpacks trace data from DataFormat.FLOAT,INT16,INT08 buffer.
* @param traceData
*    The array to contain decompressed trace data.
 */
int TraceCompressor::unpackTrace(float *_traceData) {
  if(traceFormat.getName() == "SEISPEG") {
    ERROR_PRINTF(TraceCompressorLog, "Cannot use TraceCompressor for SEISPEG format");
    return JS_USERERROR;
  } else if(traceFormat.getName() == "FLOAT") {
    bufferViewFloat.get(_traceData, numSamples);
  } else if(traceFormat.getName() == "INT16") {
    bufferViewShort.get(traceDataShort, numSamples);
    arrayCopyShortToFloat(numSamples, traceDataShort, _traceData);
  } else if(traceFormat.getName() == "INT08") {
    bufferViewByte.get(traceDataByte, numSamples);
    arrayCopyByteToFloat(numSamples, traceDataByte, _traceData);
  }
  return JS_OK;
}


/**
* Unpacks trace data from DataFormat.COMPRESSED_INT08 buffer.
* @param traceData
*    The array to contain decompressed trace data.
 */
void TraceCompressor::unpackTrace08(float *_traceData) {
  float scalar;
  char b;
  int firstByte;
  int k1 = 0;
  int k2 = 0;

  // Get scalars from "float" view buffer.
  bufferViewFloat.get(scalars, numWindows);
  // Update position of "byte" view buffer.
  bufferViewByte.position(bufferViewByte.position() + scalarsLengthInBytes);
  // Get DataFormat.COMPRESSED_INT08 samples values from "byte" view buffer.
  bufferViewByte.get(buffer08, numSamplesX);
  //       printf("unpackTrace08 2 bufferViewByte Pos=%d\n",bufferViewByte.position());

  for(int i = 0; i < numWindows; i++) {
    // Set sample range.
    k1 = k2;
    k2 = k1 + WNDWLEN08;
    if(k2 > numSamples) k2 = numSamples;
    // Set inverse scale factor.
    scalar = 0;
    if(scalars[i] > 0.0) {
      scalar = 1.0f / scalars[i];
    }
    // Scale byte (8-bit) values to float values.
    for(int k = k1; k < k2; k++) {
      b = buffer08[k];
      firstByte = (0x000000FF & ((int)b));
      _traceData[k] = (scalar * (firstByte - CLIPPING_MAX_INT08));
    }
  }
}


/**
* Unpacks trace data from DataFormat.COMPRESSED_INT16 buffer.
* @param traceData
*    The array to contain decompressed trace data.
 */
void TraceCompressor::unpackTrace16(float *_traceData) {
  float scalar;
  int k1 = 0;
  int k2 = 0;

  // Get scalars from "float" view buffer.
  bufferViewFloat.get(scalars, numWindows);
  // Update position of "byte" view buffer.
  bufferViewByte.position(bufferViewByte.position() + scalarsLengthInBytes);
  // Get DataFormat.COMPRESSED_INT16 samples values from "byte" view buffer.
  //       bufferViewByte.get((char*)buffer16, buffer16LengthInFloats*sizeof(float));
  for(int k = 0; k < numSamples; k++) {
    buffer16[k] = bufferViewByte.getShort();
  }

  for(int i = 0; i < numWindows; i++) {
    // Set sample range.
    k1 = k2;
    k2 = k1 + WNDWLEN16;
    if(k2 > numSamples) k2 = numSamples;
    // Set inverse scale factor.
    scalar = 0;
    if(scalars[i] > 0.0) {
      scalar = 1.0f / scalars[i];
    }
    // Scale short (16-bit) values to float values.
    for(int k = k1; k < k2; k++) {
      int sval = (int)buffer16[k];
      _traceData[k] = (scalar * (sval - CLIPPING_MAX_INT16));
    }
  }
}


/**
* Copies trac edata from byte array to float array.
* @param numSamples
*    The number of samples in the arrays.
* @param traceIn
*    The input byte array.
* @param traceOut
*    The output float array.
 */
void TraceCompressor::arrayCopyFloatToShort(int _numSamples, const float *traceIn, short *traceOut) {
  float value;
  for(int i = 0; i < _numSamples; i++) {
    value = traceIn[i];
    value = fmin(value, (float)CLIPPING_MAX_INT16);
    value = fmax(value, (float)CLIPPING_MIN_INT16);
    traceOut[i] = (short)value;
  }
}

/**
* Copies trace data from float array to byte array (clipping if necessary).
* @param numSamples
*    The number of samples in the arrays.
* @param traceIn
*    The input float array.
* @param traceOut
*    The output char array.
 */
void TraceCompressor::arrayCopyFloatToByte(int _numSamples, const float *traceIn, char *traceOut) {
  float value;
  for(int i = 0; i < _numSamples; i++) {
    value = traceIn[i];
    value = fmin(value, (float)CLIPPING_MAX_INT08);
    value = fmax(value, (float)CLIPPING_MIN_INT08);
    traceOut[i] = (char)value;
  }
}


/**
* Copies trace data from short array to float array.
* @param numSamples
*    The number of samples in the arrays.
* @param traceIn
*    The input short array.
* @param traceOut
*    The output float array.
 */
void TraceCompressor::arrayCopyShortToFloat(int _numSamples, const short *traceIn, float *traceOut) {
  for(int i = 0; i < _numSamples; i++) {
    traceOut[i] = (float)traceIn[i];
  }
}

/**
* Copies trace data from byte array to float array.
* @param numSamples
*    The number of samples in the arrays.
* @param traceIn
*    The input byte array.
* @param traceOut
*    The output float array.
 */
void TraceCompressor::arrayCopyByteToFloat(int _numSamples, const char *traceIn, float *traceOut) {
  for(int i = 0; i < _numSamples; i++) {
    traceOut[i] = (float)traceIn[i];
  }
}




//  testfuction
void testTraceCompressor() {
  // Construct a 2D frame of traces, compress/decompress and check values.
  int numSamples = 251;
  int numTraces = 60;

  TraceCompressor traceCompressor;

  DataFormat traceFormat = DataFormat::FLOAT;
  int recordLength = traceCompressor.getRecordLength(traceFormat, numSamples);
  CharBuffer bufferByte;
  bufferByte.resize((size_t)recordLength * numTraces);
  float *origTraceData = new float[(size_t)numTraces * numSamples];
  float *testTraceData = new float[(size_t)numTraces * numSamples];
  //       float value;
  double omega1;
  double omega2;
  double t, pi = M_PI;

  // Generate "original" frame data for testing.
  omega1 = pi / (double)(numSamples - 1);
  for(int i = 0; i < numTraces; i++) {
    omega2 = (double)(i + 10) * omega1;
    for(int j = 0; j < numSamples; j++) {
      t = (double)j;
      origTraceData[(size_t)i * numSamples + j] = (float)(cos(omega1 * t) * sin(omega2 * t));
      testTraceData[(size_t)i * numSamples + j] = origTraceData[(size_t)i * numSamples + j];
    }
  }

  printf("TraceCompressor: Synthetic data generated for DataFormat.FLOAT\n");
  printf("TraceCompressor: Test DataFormat.COMPRESSED_INT08...\n");

  // Create the compressor, for testing DataFormat.COMPRESSED_INT08.
  traceFormat = DataFormat::COMPRESSED_INT08;
  traceCompressor.Init(traceFormat, numSamples, &bufferByte);

  // Pack the frame data from the "original" array.
  traceCompressor.packFrame(numTraces, origTraceData);
  printf("TraceCompressor: Synthetic data packed.\n");

  // Zero out the "test" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      testTraceData[(size_t)i * numSamples + j] = 0.0f;
    }
  }

  // Unpack the frame data into the "test" array.
  traceCompressor.unpackFrame(numTraces, testTraceData);
  printf("TraceCompressor: Synthetic data unpacked.\n");

  // Validate the "test" array against the "original" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      if(fabsf(testTraceData[(size_t)i * numSamples + j] - origTraceData[(size_t)i * numSamples + j]) > 0.025f) {
        printf("Error validating compression for trace %d sample=%d  packed[trace][sample]=%g  unpacked[trace][sample]=%g\n",
               i, j, origTraceData[(size_t)i * numSamples + j], testTraceData[(size_t)i * numSamples + j]);
        //                  printf("%g, ",origTraceData[(size_t)i*numSamples+j]-testTraceData[(size_t)i*numSamples+j]);
      }
    }
  }

  printf("TraceCompressor: Synthetic data validated.\n");

  bufferByte.position(0);

  printf("TraceCompressor: Test DataFormat.COMPRESSED_INT16...\n");

  // Create the compressor, for testing DataFormat.COMPRESSED_INT16.
  traceFormat = DataFormat::COMPRESSED_INT16;
  traceCompressor.Init(traceFormat, numSamples, &bufferByte);

  // Pack the frame data from the "original" array.
  traceCompressor.packFrame(numTraces, origTraceData);
  printf("TraceCompressor: Synthetic data packed.\n");

  // Zero out the "test" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      testTraceData[(size_t)i * numSamples + j] = 0.0f;
    }
  }

  // Unpack the frame data into the "test" array.
  traceCompressor.unpackFrame(numTraces, testTraceData);
  printf("TraceCompressor: Synthetic data unpacked.\n");

  // Validate the "test" array against the "original" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      if(fabsf(testTraceData[(size_t)i * numSamples + j] - origTraceData[(size_t)i * numSamples + j]) > 0.001f) {
        printf("Error validating compression for trace %d sample=%d  packed[trace][sample]=%g  unpacked[trace][sample]=%g\n",
               i, j, origTraceData[(size_t)i * numSamples + j], testTraceData[(size_t)i * numSamples + j]);
      }
    }
  }

  printf("TraceCompressor: Synthetic data validated.\n");
  bufferByte.position(0);
  printf("TraceCompressor: Test DataFormat.FLOAT...\n");

  // Create the compressor, for testing DataFormat.FLOAT.
  traceFormat = DataFormat::FLOAT;
  traceCompressor.Init(traceFormat, numSamples, &bufferByte);

  // Pack the frame data from the "original" array.
  traceCompressor.packFrame(numTraces, origTraceData);
  printf("TraceCompressor: Synthetic data packed.\n");

  // Zero out the "test" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      testTraceData[(size_t)i * numSamples + j] = 0.0f;
    }
  }

  // Unpack the frame data into the "test" array.
  traceCompressor.unpackFrame(numTraces, testTraceData);
  printf("TraceCompressor: Synthetic data unpacked.\n");

  // Validate the "test" array against the "original" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      if(fabs(testTraceData[(size_t)i * numSamples + j] - origTraceData[(size_t)i * numSamples + j]) > 0.001) {
        printf("Error validating compression for trace %d sample=%d  packed[trace][sample]=%g  unpacked[trace][sample]=%g\n",
               i, j, origTraceData[(size_t)i * numSamples + j], testTraceData[(size_t)i * numSamples + j]);
      }
    }
  }

  printf("TraceCompressor: Synthetic data validated.\n");
  bufferByte.position(0);

  // Generate "original" frame data for testing.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      origTraceData[(size_t)i * numSamples + j] = (float)(j * 0.1);
      testTraceData[(size_t)i * numSamples + j] = origTraceData[(size_t)i * numSamples + j];
    }
  }

  printf("TraceCompressor: Synthetic data generated for DataFormat.INT08.\n");
  printf("TraceCompressor: Test DataFormat.INT08...\n");

  // Create the compressor, for testing DataFormat.INT08.
  traceFormat = DataFormat::INT08;
  traceCompressor.Init(traceFormat, numSamples, &bufferByte);

  // Pack the frame data from the "original" array.
  traceCompressor.packFrame(numTraces, origTraceData);
  printf("TraceCompressor: Synthetic data packed.\n");

  // Zero out the "test" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      testTraceData[(size_t)i * numSamples + j] = 0.0f;
    }
  }

  // Unpack the frame data into the "test" array.
  traceCompressor.unpackFrame(numTraces, testTraceData);
  printf("TraceCompressor: Synthetic data unpacked.\n");

  // Validate the "test" array against the "original" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      if(fabs(testTraceData[(size_t)i * numSamples + j] - origTraceData[(size_t)i * numSamples + j]) > 1) {
        printf("Error validating compression for trace %d sample=%d  packed[trace][sample]=%g  unpacked[trace][sample]=%g\n",
               i, j, origTraceData[(size_t)i * numSamples + j], testTraceData[(size_t)i * numSamples + j]);
      }
    }
  }

  printf("TraceCompressor: Synthetic data validated.\n");
  bufferByte.position(0);

  // Generate "original" frame data for testing.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      origTraceData[(size_t)i * numSamples + j] = (float)(j * 25.6);
      testTraceData[(size_t)i * numSamples + j] = origTraceData[(size_t)i * numSamples + j];
    }
  }
  printf("TraceCompressor: Synthetic data generated for DataFormat.INT16.\n");

  printf("TraceCompressor: Test DataFormat.INT16...\n");

  // Create the compressor, for testing DataFormat.INT16.
  traceFormat = DataFormat::INT16;
  traceCompressor.Init(traceFormat, numSamples, &bufferByte);

  // Pack the frame data from the "original" array.
  traceCompressor.packFrame(numTraces, origTraceData);
  printf("TraceCompressor: Synthetic data packed.\n");

  // Zero out the "test" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      testTraceData[(size_t)i * numSamples + j] = 0.0f;
    }
  }

  // Unpack the frame data into the "test" array.
  traceCompressor.unpackFrame(numTraces, testTraceData);
  printf("TraceCompressor: Synthetic data unpacked.\n");

  // Validate the "test" array against the "original" array.
  for(int i = 0; i < numTraces; i++) {
    for(int j = 0; j < numSamples; j++) {
      if(fabs(testTraceData[(size_t)i * numSamples + j] - origTraceData[(size_t)i * numSamples + j]) > 1) {
        printf("Error validating compression for trace %d sample=%d  packed[trace][sample]=%g  unpacked[trace][sample]=%g\n",
               i, j, origTraceData[(size_t)i * numSamples + j], testTraceData[(size_t)i * numSamples + j]);
      }
    }
  }

  printf("TraceCompressor: Synthetic data validated.\n");

  delete [] origTraceData;
  delete [] testTraceData;
}
}

