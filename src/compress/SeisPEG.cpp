/***************************************************************************
                          SeisPEG.cpp  -  description
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

#include "../IntBuffer.h"
#include "SeisPEG.h"
#include "../PSProLogging.h"

namespace jsIO {
DECLARE_LOGGER(SeisPEGLog);

SeisPEG::~SeisPEG() {
  delete[]m_piHdrInfo;
  if(m_pFtGain != NULL) delete []m_pFtGain;
  if(m_pfWorkBuffer1 != NULL) delete []m_pfWorkBuffer1;
  if(m_pcWorkBuffer2 != NULL) delete []m_pcWorkBuffer2;
  if(m_pcCompressedBuffer != NULL) delete []m_pcCompressedBuffer;
  if(m_pfWorkBlock != NULL) delete []m_pfWorkBlock;
  if(m_pfVecW != NULL) delete []m_pfVecW;
  if(m_pfScratch1 != NULL) delete []m_pfScratch1;
  if(m_pfScratch2 != NULL) delete []m_pfScratch2;
  delete m_hdrIntBuffer;
}


SeisPEG::SeisPEG() {
  // todo: init m_nThreads
  m_bInit = false;
  m_bFtGainExponentWasStored = false;
  m_pFtGain = NULL;  // May remain NULL.
  m_pfWorkBuffer1 = NULL;  // Length of _paddedN1 * _paddedN2.
  m_pcWorkBuffer2 = NULL;   // Large enough to hold a decompressed block.
  m_nWorkBuffer2Size = 0;
  m_pfWorkBlock = NULL;    // Length of m_nVerticalBlockSize*m_nHorizontalBlockSize;
  m_pcCompressedBuffer = NULL;
  m_pfVecW = NULL;     // Length of CACHE_SIZE* _paddedN2.
  m_pfScratch1 = NULL;
  m_pfScratch2 = NULL;
  m_nEnsemblesChecked = 0;
  m_piHdrInfo = new int[LEN_HDR_INFO];
  m_hdrIntBuffer = new IntBuffer;

  m_nTraceLength = 0;
  m_nHdrLength = 0;
  m_nTracesWrittenTotal = 0;
  m_nBytesTotal = 0;
}

/*
 * Constructor used for experimenting with Huffman tables.
 *
 * @param  _huffCount  the Huffman table value count.
 * @param  _n1  the number of trace samples.
 * @param  _n2  the maximum number of traces per frame/ensemble.
 * @param  _distortion  the allowed m_fDistortion.  The value .1 is a good aggressive default.
 * @param  _verticalBlockSize  the vertical block size.  Must be a multiple of 8.
 * @param  _horizontalBlockSize  the horizontal block size.    Must be a multiple of 8.
 * @param  _verticalTransLength  the vertical transform length.  Must be 8 or 16.
 * @param  _horizontalTransLength  the horizontal transform length.  Must be 8 or 16.
 */
SeisPEG::SeisPEG(int *_huffCount, int _n1, int _n2, float _distortion,
                 int _verticalBlockSize, int _horizontalBlockSize,
                 int _verticalTransLength, int _horizontalTransLength) {

  m_bInit = false;
  m_bFtGainExponentWasStored = false;
  m_pFtGain = NULL;  // May remain NULL.
  m_pfWorkBuffer1 = NULL;  // Length of _paddedN1 * _paddedN2.
  m_pfWorkBlock = NULL;    // Length of m_nVerticalBlockSize*m_nHorizontalBlockSize;
  m_pcWorkBuffer2 = NULL;   // Large enough to hold a decompressed block.
  m_nWorkBuffer2Size = 0;
  m_pcCompressedBuffer = NULL;
  m_pfVecW = NULL;     // Length of CACHE_SIZE* _paddedN2.
  m_pfScratch1 = NULL;
  m_pfScratch2 = NULL;
  m_nEnsemblesChecked = 0;
  m_piHdrInfo = new int[LEN_HDR_INFO];
  m_hdrIntBuffer = new IntBuffer;

  m_nTraceLength = 0;
  m_nHdrLength = 0;
  m_nTracesWrittenTotal = 0L;
  m_nBytesTotal = 0L;

  m_blockCompressor.Init(_huffCount);
  float _ftGainExponent = 0.0F;
  Init(_n1, _n2, _distortion, _ftGainExponent, _verticalBlockSize, _horizontalBlockSize,
       _verticalTransLength, _horizontalTransLength, "full");
}




/*

* General purpose constructor.
*
* @param  _n1  the number of trace samples.
* @param  _n2  the maximum number of traces per frame/ensemble.
* @param  _distortion  the allowed m_fDistortion.  The value .1 is good default.
* @param  _verticalBlockSize  the vertical block size.  Must be a multiple of 8.
* @param  _horizontalBlockSize  the horizontal block size.    Must be a multiple of 8.
* @param  _verticalTransLength  the vertical transform length.  Must be 8 or 16.
* @param  _horizontalTransLength  the horizontal transform length.  Must be 8 or 16.
  */
SeisPEG::SeisPEG(int _n1, int _n2, float _distortion,
                 int _verticalBlockSize, int _horizontalBlockSize,
                 int _verticalTransLength, int _horizontalTransLength) {
  m_bInit = false;
  m_bFtGainExponentWasStored = false;
  m_pFtGain = NULL;  // May remain NULL.
  m_pfWorkBuffer1 = NULL;  // Length of _paddedN1 * _paddedN2.
  m_pfWorkBlock = NULL;    // Length of m_nVerticalBlockSize*m_nHorizontalBlockSize;
  m_pcWorkBuffer2 = NULL;   // Large enough to hold a decompressed block.
  m_nWorkBuffer2Size = 0;
  m_pcCompressedBuffer = NULL;
  m_pfVecW = NULL;     // Length of CACHE_SIZE* _paddedN2.
  m_pfScratch1 = NULL;
  m_pfScratch2 = NULL;
  m_nEnsemblesChecked = 0;
  m_piHdrInfo = new int[LEN_HDR_INFO];
  m_hdrIntBuffer = new IntBuffer;

  float _ftGainExponent = 0.0F;
  Init(_n1, _n2, _distortion, _ftGainExponent, _verticalBlockSize, _horizontalBlockSize,
       _verticalTransLength, _horizontalTransLength, "full");

}

/*
                  * The preferred constructor (computes the best block sizes and transform lengths).
                  *
                  * @param  _n1  the number of trace samples.
                  * @param  _n2  the maximum number of traces per frame/ensemble.
                  * @param  _distortion  the allowed m_fDistortion.  The value .1 is good default.
                  * @param  _policy  the compression policy (fastest or maximum compression).
 */
SeisPEG::SeisPEG(int _n1, int _n2, float _distortion, SeisPEG_Policy _policy) {
  Init(_n1, _n2, _distortion, _policy);
}


/*
  * Constructor from existing compressed trace data (for the uncompression case).
  *
  * @param  compressedByteData  compressed trace byte data.
*/
SeisPEG::SeisPEG(char *compressedByteData) {
  m_bInit = false;
  m_hdrIntBuffer = new IntBuffer;
  Init(compressedByteData);
}


int SeisPEG::Init(int _n1, int _n2, float _distortion, SeisPEG_Policy _policy) {
  m_bInit = false;
  m_bFtGainExponentWasStored = false;
  m_pFtGain = NULL;  // May remain NULL.
  m_pfWorkBuffer1 = NULL;  // Length of _paddedN1 * _paddedN2.
  m_pfWorkBlock = NULL;    // Length of m_nVerticalBlockSize*m_nHorizontalBlockSize;
  m_pcWorkBuffer2 = NULL;   // Large enough to hold a decompressed block.
  m_nWorkBuffer2Size = 0;
  m_pcCompressedBuffer = NULL;
  m_pfVecW = NULL;     // Length of CACHE_SIZE* _paddedN2.
  m_pfScratch1 = NULL;
  m_pfScratch2 = NULL;
  m_nEnsemblesChecked = 0;
  m_piHdrInfo = new int[LEN_HDR_INFO];
  m_hdrIntBuffer = new IntBuffer;

  // Determine some reasonable defaults.
  int verticalBlockSize = computeBlockSize(_n1, _policy);
  int verticalTransLength = computeTransLength(verticalBlockSize, _policy);
  int horizontalBlockSize = computeBlockSize(_n2, _policy);
  int horizontalTransLength = computeTransLength(horizontalBlockSize, _policy);

  float ftGainExponent = 0.0f;
  return Init(_n1, _n2, _distortion, ftGainExponent, verticalBlockSize, horizontalBlockSize,
              verticalTransLength, horizontalTransLength, "short");
}

int SeisPEG::Init(char *compressedByteData) {
  m_bFtGainExponentWasStored = false;
  m_pFtGain = NULL;  // May remain NULL.
  m_pfWorkBuffer1 = NULL;  // Length of _paddedN1 * _paddedN2.
  m_pfWorkBlock = NULL;    // Length of m_nVerticalBlockSize*m_nHorizontalBlockSize;
  m_pcWorkBuffer2 = NULL;   // Large enough to hold a decompressed block.
  m_nWorkBuffer2Size = 0;
  m_pcCompressedBuffer = NULL;
  m_pfVecW = NULL;     // Length of CACHE_SIZE* _paddedN2.
  m_pfScratch1 = NULL;
  m_pfScratch2 = NULL;
  m_nEnsemblesChecked = 0;
  m_piHdrInfo = new int[LEN_HDR_INFO];

  if(decodeHdr(compressedByteData, m_piHdrInfo) == JS_USERERROR) {
    ERROR_PRINTF(SeisPEGLog, "Invalid or corrupt SeisPEG file");
    return JS_USERERROR;
  }

  float _distortion = BlockCompressor::intBitsToFloat(m_piHdrInfo[IND_DISTORTION]);
  //     float _distortion = *(reinterpret_cast<float*>((int*)&m_piHdrInfo[IND_DISTORTION])); //Float.intBitsToFloat(_hdrInfo[IND_DISTORTION]);
  int _n1 = m_piHdrInfo[IND_N1];
  int _n2 = m_piHdrInfo[IND_N2];
  int _verticalBlockSize = m_piHdrInfo[IND_VBLOCK_SIZE];
  int _horizontalBlockSize = m_piHdrInfo[IND_HBLOCK_SIZE];
  int _verticalTransLength = m_piHdrInfo[IND_VTRANS_LEN];
  int _horizontalTransLength = m_piHdrInfo[IND_HTRANS_LEN];
  //     float _ftGainExponent = *(reinterpret_cast<float*>((int*)&m_piHdrInfo[IND_FT_GAIN]));//Float.intBitsToFloat(_hdrInfo[IND_FT_GAIN]);
  float _ftGainExponent = BlockCompressor::intBitsToFloat(m_piHdrInfo[IND_FT_GAIN]);

  TRACE_PRINTF(SeisPEGLog, "m_fDistortion=%f, m_n1=%d, m_n2=%d, m_nVerticalBlockSize=%d, \
                  m_nHorizontalBlockSize=%d, m_fFtGainExponent=%f\n m_nVerticalTransLength=%d, m_nHorizontalTransLength=%d",
               _distortion, _n1, _n2, _verticalBlockSize, _horizontalBlockSize, _ftGainExponent, _verticalTransLength,
               _horizontalTransLength);

  return Init(_n1, _n2, _distortion, _ftGainExponent, _verticalBlockSize, _horizontalBlockSize,
              _verticalTransLength, _horizontalTransLength, "existing");
}




/*
   * Does the real work of constructing a SeisPEG.
   *
   * @param  _n1  the number of trace samples.
   * @param  _n2  the maximum number of traces per frame/ensemble.
   * @param  _distortion  the allowed m_fDistortion.  The value .1 is good default.
   * @param  _ftGainExponent  function-of-time gain exponent.  Commonly reset later by the method setGainExponent().
   * @param  _verticalBlockSize  the vertical block size.  Must be a multiple of 8.
   * @param  _horizontalBlockSize  the horizontal block size.    Must be a multiple of 8.
   * @param  _verticalTransLength  the vertical transform length.  Must be 8 or 16.
   * @param  _horizontalTransLength  the horizontal transform length.  Must be 8 or 16.
   * @param  _whichConstructor  for logging and debugging.
 */
int SeisPEG::Init(int _n1, int _n2, float _distortion, float _ftGainExponent,
                  int _verticalBlockSize, int _horizontalBlockSize,
                  int _verticalTransLength, int _horizontalTransLength,
                  std::string _whichConstructor) {
  TRACE_PRINTF(SeisPEGLog, " SeisPEG.Init %s : m_n1=%d, m_n2=%d, _distortion=%f", _whichConstructor.c_str(), _n1, _n2, _distortion);

  if(_n1 < 1  ||  _n2 < 1) {
    ERROR_PRINTF(SeisPEGLog, "The data size must be non-zero");
    return JS_USERERROR;
  }
  if(_distortion <= 0.0f) {
    ERROR_PRINTF(SeisPEGLog, "The m_fDistortion must be positive");
    return JS_USERERROR;
  }

  if(!(checkBlockSize(_verticalBlockSize)  && checkBlockSize(_horizontalBlockSize) &&
       checkTransLength(_verticalTransLength, _verticalBlockSize) && checkTransLength(_horizontalTransLength, _horizontalBlockSize))) {
    ERROR_PRINTF(SeisPEGLog, "Invalid input parameters");
    return JS_USERERROR;
  }

  m_n1 = _n1;
  m_n2 = _n2;
  m_fDistortion = _distortion;
  m_fFtGainExponent = _ftGainExponent;
  m_nVerticalBlockSize = _verticalBlockSize;
  m_nHorizontalBlockSize = _horizontalBlockSize;
  m_nVerticalTransLength = _verticalTransLength;
  m_nHorizontalTransLength = _horizontalTransLength;
  m_nPaddedN1 = computePaddedLength(_n1, _verticalBlockSize);
  m_nPaddedN2 = computePaddedLength(_n2, _horizontalBlockSize);

  // todo !!! multi threaded case
  m_bInit = true;
  return JS_OK;
}



int SeisPEG::computePaddedLength(int nsamples, int blockSize) {
  if(blockSize == 0) {
    ERROR_PRINTF(SeisPEGLog, "Block Size is Invalid");
    return JS_USERERROR;
  }
  // Round up to a multiple of the block size.
  int n = (nsamples / blockSize) * blockSize;
  if(n < nsamples) n += blockSize;
  return n;
}


int SeisPEG::computeBlockSize(int nsamples, SeisPEG_Policy policy) {
  if(nsamples <= 8) return 8;
  if(nsamples <= 16) return 16;
  if(nsamples <= 24) return 24;
  if(nsamples <= 32) return 32;

  // Most of this is based on experimentation.

  if(policy == SEISPEG_POLICY_FASTEST) {

    if(nsamples <= 48) {
      return 24;
    } else if(nsamples <= 64) {
      return 32;
    } else {
      // 64 is the most that we ever do when striving for fastest.
      return 64;
    }
  } else {
    // Maximum compression.
    // Go for the biggest block size that is a multiple of 16;
    int nBlocks = nsamples / 16;
    if(nBlocks * 16 < nsamples) nBlocks++;
    int blockSize = nBlocks * 16;

    // Testing shows that you don't get appreciably better compression ratios
    // with block sizes over 512, and it gets substantially slower.  Here are some
    // comparisons on the Statoil data:
    //   blockSize=256  compressionRatio=25.9 compressionSpeed=22  uncompressionSpeed=40
    //   blockSize=512  compressionRatio=28.9 compressionSpeed=24  uncompressionSpeed=42
    //   blockSize=1024 compressionRatio=29.4 compressionSpeed=18  uncompressionSpeed=33
    // Large blocks also strain memory.
    if(blockSize > 512) blockSize = 512;
    return blockSize;

  }
}


int SeisPEG::computeTransLength(int blockSize, SeisPEG_Policy policy) {
  if(policy == SEISPEG_POLICY_FASTEST) {
    // If we're looking for speed, we always use the fastest transform.
    return 8;
  } else {
    // Going for maximum compression - always try to use 16.
    if((blockSize / 16) * 16 == blockSize) {
      return 16;
    } else {
      // Not a multiple of 16 - forced to use 8.
      return 8;
    }
  }
}

bool SeisPEG::checkBlockSize(int blockSize) {
  // Must always be a multiple of 8.
  if((blockSize / 8) * 8 == blockSize) return true;
  else return false;
}


bool SeisPEG::checkTransLength(int transLength, int blockSize) {
  if(transLength == 8) {
    int nsubBlocks = blockSize / 8;
    if(nsubBlocks * 8 == blockSize) {
      return true;
    }
  } else if(transLength == 16) {
    int nsubBlocks = blockSize / 16;
    if(nsubBlocks * 16 == blockSize) {
      return true;
    }
  }
  //     ERROR_PRINTF(SeisPEGLog, "Invalid transform length of %d for block size of %d",transLength,blockSize);
  return false;
}


/*
   * Fills a buffer.  Never fear - this method pads with zeros where they are needed
   * for the forward case.
   *
   * @param  _direction  the direction (forward or reverse).
   * @param  _traces  the sample data.
   * @param  _n1  the number of samples.
   * @param  _n2  the number of traces.
   * @param  _paddedN1  the padded length of the sample axis.
   * @param  _paddedN2  the padded length of the trace axis.
   * @param  _workBuffer  a work buffer.
   * @param  _ftGain  a function-of-time gain, or NULL if no gain should be applied.
 */
void SeisPEG::fillBuffer(int _direction, float *_traces, int _n1, int _n2,
                         int _paddedN1, int _paddedN2, float *_workBuffer, float *_ftGain) {
  int index = 0;
  for(int j = 0; j < _n2; j++) {
    if(_direction == FORWARD) {
      //ArrayUtil.arraycopy(traces[j], 0, workBuffer, index, n1);
      memcpy((char *)&_workBuffer[index], (char *)&_traces[j * _n1], _n1 * sizeof(float));
      if(_ftGain != NULL) applyFtGain(_ftGain, _workBuffer, index, _n1);
      // Fill the padding with zeros in the m_n1 direction.
      for(int i = _n1; i < _paddedN1; i++) _workBuffer[index + i] = 0.0f;
    } else {
      //         ArrayUtil.arraycopy(workBuffer, index, traces[j], 0, m_n1);
      memcpy((char *)&_traces[j * _n1], (char *)&_workBuffer[index], _n1 * sizeof(float));
      if(_ftGain != NULL) removeFtGain(_ftGain, &_traces[j * _n1], _n1);
    }
    index += _paddedN1;
  }

  if(_direction == FORWARD) {
    // Fill the padding with zeros in the _n2 direction.
    for(int j = _n2; j < _paddedN2; j++) {
      for(int i = 0; i < _paddedN1; i++) {
        _workBuffer[index + i] = 0.0f;
      }
      index += _paddedN1;
    }
  }
}

/*
   * Sets the gain exponent to use for a function-of-time gain.  Apply this gain is important for
   * raw shot records.  A value between 0.8 and 1.5 is recommended.
   *
   * @param  _ftGainExponent  function-of-time gain exponent.  A value of 0.0 effectively disables the gain.
*/
int SeisPEG::setGainExponent(float _ftGainExponent) {
  if(m_bFtGainExponentWasStored  &&  m_fFtGainExponent != _ftGainExponent) {
    //       ERROR_PRINTF(SeisPEGLog, "Attempt to change m_fFtGainExponent after it has been stored");
    return JS_USERERROR;
  }
  m_fFtGainExponent = _ftGainExponent;
  return JS_OK;
}


/**
   * Manually sets the delta value for quantization (ignoring the m_fDistortion value).
   * This is so that m_fDistortion does not vary from frame to frame.
   *
   * @param  delta  the value to use for delta in all cases.
 */
void SeisPEG::setDelta(float _delta) {
  m_blockCompressor.setDelta(_delta);
}


/*
//    * Returns a buffer of appropriate size for the compressed data.  This
//    * buffer may contain extra space (the size of compressed data varies).

    public CompressedData compressedBufferAlloc() {

    return new CompressedData(this.compressedByteBufferAlloc(), 0);
}
*/


//    * Returns a buffer for compressed data (guessing at the size).
//    *
//    * @return  a buffer for compressed data.

long SeisPEG::compressedByteBufferAllocSize() {
  // This is pure (conservative) guesswork.
  long nbytes;
  if(m_fDistortion > 0.1) {
    // Always at least 2:1 compression?
    nbytes = m_nPaddedN1 * m_nPaddedN2 * 4 / 2;
  } else if(m_fDistortion > 0.01) {
    // Always at least 1:1 compression?
    nbytes = m_nPaddedN1 * m_nPaddedN2 * 4;
  } else {
    // Always at least 1:2 compression?
    nbytes = m_nPaddedN1 * m_nPaddedN2 * 4 * 2;
  }
  return nbytes;
}

int SeisPEG::nbytes4compressedByteBufferAlloc() {
  // This is pure (conservative) guesswork.
  int nbytes;
  if(m_fDistortion > 0.1) {
    // Always at least 2:1 compression?
    nbytes = m_nPaddedN1 * m_nPaddedN2 * 4 / 2;
  } else if(m_fDistortion > 0.01) {
    // Always at least 1:1 compression?
    nbytes = m_nPaddedN1 * m_nPaddedN2 * 4;
  } else {
    // Always at least 1:2 compression?
    nbytes = m_nPaddedN1 * m_nPaddedN2 * 4 * 2;
  }
  return nbytes;
}


/**
   * Applies the lapped orthogonal transform to trace data.  This method exists only
   * for the purpose of showing what the transformed data looks like - it is not used
   * as part of the compression process.
   *
   * @param  _traces  the trace data.
   * @param  _nTraces  the number of live traces.
 */
int SeisPEG::transform(float *_traces, int _nTraces) {
  if(_nTraces > m_n2) {
    ERROR_PRINTF(SeisPEGLog, "The size of the data cannot increase");
    return JS_USERERROR;
  }

  if(m_pfWorkBuffer1 == NULL) m_pfWorkBuffer1 = new float[m_nPaddedN1 * m_nPaddedN2];

  // This method pads with zeros where they are needed.
  fillBuffer(FORWARD, _traces, m_n1, _nTraces, m_nPaddedN1, m_nPaddedN2, m_pfWorkBuffer1, NULL);

  transform2D(m_pfWorkBuffer1);

  // This method pads with zeros where they are needed.
  fillBuffer(REVERSE, _traces, m_n1, _nTraces, m_nPaddedN1, m_nPaddedN2, m_pfWorkBuffer1, NULL);

  return JS_OK;
}


/**
   * Applies the lapped orthogonal transform to trace data.  This method exists only
   * for the purpose of showing what the transformed data looks like - it is not used
   * as part of the compression process.
   *
   * @param  _paddedTraces  the trace data, padded to a multiple of the block size
   *                       in both directions.  These values are changed by this method.
 */
void SeisPEG::transform2D(float *_paddedTraces) {
  // Transform in x1 first.
  x1Transform(FORWARD, _paddedTraces);
  // Transform in time.
  timeTransform(FORWARD, _paddedTraces);
}


/**
   * Checks for the existence of bad amplitudes and complains if they are found.
   *
   * @param  _traces  seismic traces.
   * @param  _nTraces  the number of live traces.
 */
bool SeisPEG::containsBadAmplitude(float *_traces, int _nTraces, int _nSamples) {
  m_nEnsemblesChecked++;
  for(int j = 0; j < _nTraces; j++) {
    float *trace = &_traces[j * m_n1];
    for(int i = 0; i < _nSamples; i++) {
      // This test fails for NaNs as well as dangerously large numbers.
      if(trace[i] < -1.0E15  ||  trace[i] > 1.0E15) {
        TRACE_PRINTF(SeisPEGLog, "Warning : Found uncompressible bad amplitude %g at ensemble %d\n", trace[i], m_nEnsemblesChecked);
        return true;
      }
    }
  }
  return false;
}


/**
   * "Compresses" a frame/ensemble of data that has a bad amplitude.  Actually just
   * copies the input data to the output data, plus inserting a header.
   *
   * @param  traces  the trace data.
   * @param  nTraces  the number of live traces.
   * @param  compressedData  space for the compressed data.  May be NULL.
   * @return  a CompressedData object, which will be the input CompressedData object
   *          if the input is non-NULL and large enough to contain the compressed data,
   *          or a new larger CompressedData object if necessary.
 */
int SeisPEG::badAmplitudeCompress(float *_traces, int _nTraces, CompressedData &_compressedData) {
  // Get an output buffer that is large enough.
  long cData_size = m_n1 * _nTraces * SIZEOF_FLOAT;
  if(_compressedData.getDataLength() < cData_size) {
    _compressedData.allocDataSpace(cData_size);
  } else {
    _compressedData.setDataLength(cData_size);
  }
  char *outputData = _compressedData.getData();

  return badAmplitudeCompress(_traces, _nTraces, outputData);
}


/**
   * "Compresses" a frame/ensemble of data that has a bad amplitude.  Actually just
   * copies the input data to the output data, plus inserting a header.
   *
   * @param  traces  the trace data.
   * @param  nTraces  the number of lives traces.
   * @param  outputData  space for the compressed data.  Must be large enough to hold
   *                     an uncompressed copy of the input data, including non-live traces.
   *                     must be pre-allocated with the size m_n1*_nTraces*sizeof(float)
   * @return  the number of bytes of output data.
 */

int SeisPEG::badAmplitudeCompress(float *_traces, int _nTraces, char *outputData) {
  long cData_size = m_n1 * _nTraces * SIZEOF_FLOAT;
  memcpy(outputData, (char *)_traces, cData_size);
  short cookie;
  if(ISNOTZERO(m_fFtGainExponent)) {
    // No reason to mess up compatibility with existing code if the gain isn't used.
    cookie = BAD_AMPLITUDE_COOKIE_V2;
  } else {
    cookie = BAD_AMPLITUDE_COOKIE_V3;
  }

  // Yes, we store a header in the data.  Then we zero the first samples during decompression.
  encodeHdr(outputData, cookie, m_fDistortion, m_fFtGainExponent,
            m_n1, m_n2, m_nVerticalBlockSize, m_nHorizontalBlockSize,
            m_nVerticalTransLength, m_nHorizontalTransLength, cData_size, 0);
  m_bFtGainExponentWasStored = true;


  return cData_size;
}


/**
   * "Uncompresses" a frame/ensemble of data that had a bad amplitude.  Actually just
   * copies the input data to the output data, plus cleaning up where a header was inserted.
   *
   * @param  inData  the input compressed data.
   * @param  inDataLength  the length of the input data.
   * @param  traces  the trace data.
 */
void SeisPEG::badAmplitudeUncompress(const char *inData, int inDataLength, float *traces, int nTraces) {
  //     int nTraces = (inDataLength / SIZEOF_FLOAT) / traces[0].length;
  // Yes, this is probably slow, but who cares - it better not be happening a lot.
  int upI = (LEN_HDR_INFO > m_n1) ? m_n1 : LEN_HDR_INFO;
  for(int j = 0; j < nTraces; j++) {
    float *trace = &traces[j * m_n1];
    memcpy((char *)trace, (const char *)&inData[j * m_n1 * SIZEOF_FLOAT], m_n1 * SIZEOF_FLOAT);
    /*
      float[] trace = traces[j];
      for (int i=0; i<trace.length; i++) {
      int ival = BlockCompressor.stuffBytesInInt(inData, index);
      trace[i] = Float.intBitsToFloat(ival);
      index += SIZEOF_INT;
    }
    */
    if(j == 0) {
      // Don't leave garbage values where the header was.
      for(int i = 0; i < upI; i++)
        trace[i] = 0.0F;
    }
  }

}



/**
   * Compresses a frame/ensemble of traces.  Does not alter the input data.
   *
   * @param  traces  the trace data.
   * @param  nTraces  the number of lives traces.
   * @param  outputData  space for the compressed data.  Must be large enough to hold
   *                     an uncompressed copy of the input data, including non-live traces, i.e.
   *                     it must be pre-allocated with a size nTraces*nSamples*sizeof(float)
   * @return  the number of bytes of output compressed data.
 */
int SeisPEG::compress(float *_traces, int _nTraces, char *_outputData) {

  if(_traces == NULL  ||  _outputData == NULL || _nTraces > m_n2) {
    ERROR_PRINTF(SeisPEGLog, "Invalid input parameters");
    return JS_USERERROR;
  }
  // This isn't a stricly needed policy, but it's smart because we really don't
  // want to recompress with higher distortion.

  if(containsBadAmplitude(_traces, _nTraces, m_n1))
    return badAmplitudeCompress(_traces, _nTraces,  _outputData);

  if(ISNOTZERO(m_fFtGainExponent)) {
    // We want to apply a gain first.
    if(m_pFtGain == NULL)
      computeFtGain(m_n1, m_fFtGainExponent);
  }

  if(m_pfWorkBuffer1 == NULL) {
    m_pfWorkBuffer1 = new float[m_nPaddedN1 * m_nPaddedN2];
  }

  // This method pads with zeros where they are needed.
  fillBuffer(FORWARD, _traces, m_n1, _nTraces, m_nPaddedN1, m_nPaddedN2, m_pfWorkBuffer1, m_pFtGain);
  long outputData_len =  _nTraces * m_n1 * sizeof(float);
  long nbytes = compress2D(m_pfWorkBuffer1, m_fDistortion, m_fFtGainExponent, _outputData, outputData_len);
  m_bFtGainExponentWasStored = true;

  if(nbytes > outputData_len) {  // Should never happen.
    ERROR_PRINTF(SeisPEGLog, "nbytes > outputData.length");
    return JS_USERERROR;
  }

  if(nbytes == 0) {
    // Output buffer is too small.  Compression actually expanded the data!
    return badAmplitudeCompress(_traces, _nTraces, _outputData);
  }

  return nbytes;
}


/**
   * Compresses a frame/ensemble of traces.  Does not alter the input data.
   *
   * @param  traces  the trace data.
   * @param  nTraces  the number of live traces.
   * @param  compressedData  space for the compressed data.  May be NULL.
   * @return  a CompressedData object, which will be the input CompressedData object
   *          if the input is non-NULL and large enough to contain the compressed data,
   *          or a new larger CompressedData object if necessary.
 */
int SeisPEG::compress(float *_traces, int _nTraces, CompressedData &_compressedData) {
  long cData_size = compressedByteBufferAllocSize();
  if(_compressedData.getDataLength() < cData_size) {
    _compressedData.allocDataSpace(cData_size);
  } else {
    _compressedData.setDataLength(cData_size);
  }
  char *outputData = _compressedData.getData();

  return compress(_traces, _nTraces, outputData);
}


/**
   * Convenience method to return the difference between the compressed and uncompressed
   * version of the data.  Does not alter the input data samples.
   *
   * _traces_diff must be pre-allocated with len(traces_diff) == len(traces) == nTraces * nSamples
 */
float SeisPEG::difference(float *_traces, int _nTraces, float *_traces_diff) {
  int cData_size = compressedByteBufferAllocSize();
  char *compressedData = new char[cData_size];

  compress(_traces, _nTraces, compressedData);

  uncompress(compressedData, cData_size, _traces_diff, _nTraces);
  float rms = 0.f;
  for(int j = 0; j < _nTraces; j++) {
    for(int i = 0; i < m_n1; i++) {
      _traces_diff[j * m_n1 + i] -= _traces[j * m_n1 + i];
      rms += _traces_diff[j * m_n1 + i] * _traces_diff[j * m_n1 + i];
    }
  }
  rms = sqrt(rms / (float)(_nTraces * m_n1));

  delete[]compressedData;

  return rms;
}


/*
  * Uncompresses a frame/ensemble of traces.
  *
  * @param  _compressedByteData  the compressed byte data.
  * @param  _compressedDataLength  the length of the compressed data.
  * @param  _traces  the output uncompressed data (must be pre-allocated with the size m_n1*nTraces ).
*/
int SeisPEG::uncompress(const char *_compressedByteData, int _compressedDataLength, float *_traces, int _nTraces) {
  int ires = badAmplitudeData(_compressedByteData);
  if(ires == 1) {
    decodeHdr(_compressedByteData, m_piHdrInfo);
    //       int nBytesTraces = m_piHdrInfo[IND_NBYTES_TRACES];
    badAmplitudeUncompress(_compressedByteData, _compressedDataLength, _traces, _nTraces);
    return JS_OK;
  } else if(ires == JS_USERERROR) {
    return JS_USERERROR;
  }

  if(ISNOTZERO(m_fFtGainExponent)) {
    // We want to apply a gain first.
    if(m_pFtGain == NULL)
      computeFtGain(m_n1, m_fFtGainExponent);
  }

  if(m_pfWorkBuffer1 == NULL) m_pfWorkBuffer1 = new float[m_nPaddedN1 * m_nPaddedN2];

  int iflag = uncompress2D(_compressedByteData, _compressedDataLength, m_pfWorkBuffer1);
  if(iflag != JS_OK) {
    ERROR_PRINTF(SeisPEGLog, "Compressed data is corrupted");
    return JS_USERERROR;
  }

  fillBuffer(REVERSE, _traces, m_n1, _nTraces, m_nPaddedN1, m_nPaddedN2, m_pfWorkBuffer1, m_pFtGain);
  return JS_OK;
}


/*
   * Uncompresses a frame/ensemble of traces.
   *
   * @param  _compressedData  the compressed data.
   * @param  _traces  the output uncompressed data.(must be pre-allocated with the size _n1*_n2
 */
int SeisPEG::uncompress(CompressedData &_compressedData, float *_traces, int _nTraces) {
  const char *inData = _compressedData.getData();
  int inDataLength = _compressedData.getDataLength();
  return uncompress(inData, inDataLength, _traces, _nTraces);
}


/*
   * Performs the transform along the sample (first) axis.
   *
   * @param  _direction  forward or reverse.
   * @param  _paddedTraces  the trace data, padded to a multiple of the block size
   *                       in both directions.
 */
int SeisPEG::timeTransform(int _direction, float *_paddedTraces) {
  /*
        if (timeTransformers != NULL) {
        timeTransformThreaded(direction, paddedTraces);
        return;
      }
  */
  int nblocksVertical = m_nPaddedN1 / m_nVerticalBlockSize;
  // The traces must be padded to an even multiple of the block size.
  // todo! this checking can be done also during the initalization
  if(nblocksVertical * m_nVerticalBlockSize != m_nPaddedN1) {
    return JS_USERERROR;
  }

  // An extra two blocks for the LOT.
  if(m_pfScratch1 == NULL) m_pfScratch1 =  new float[m_nPaddedN1 + m_nVerticalBlockSize];

  for(int j = 0; j < m_nPaddedN2; j++) {
    int index = j * m_nPaddedN1;
    if(_direction == FORWARD) {
      m_transformer.lotFwd(_paddedTraces, index, m_nVerticalBlockSize, m_nVerticalTransLength,
                           nblocksVertical, m_pfScratch1);
    } else {
      m_transformer.lotRev(_paddedTraces, index, m_nVerticalBlockSize, m_nVerticalTransLength,
                           nblocksVertical, m_pfScratch1);
    }
  }

  return JS_OK;
}

/*
   * Performs the transform along the trace (second) axis.
   *
   * @param  _direction  forward or reverse.
   * @param  _paddedTraces  the trace data, padded to a multiple of the block size
   *                       in both directions.
 */
int SeisPEG::x1Transform(int _direction, float *_paddedTraces) {
  int nblocksHorizontal = (int)(m_nPaddedN2 / m_nHorizontalBlockSize);
  // The traces must be padded to an even multiple of the block size.
  // todo! this checking can be done also during the initalization
  if(nblocksHorizontal * m_nHorizontalBlockSize != m_nPaddedN2) {
    return JS_USERERROR;
  }

  if(m_pfVecW == NULL) m_pfVecW = new float[CACHE_SIZE * m_nPaddedN2];
  if(m_pfScratch2 == NULL) m_pfScratch2 = new float[m_nPaddedN2 + m_nHorizontalBlockSize];

  int nsamps = m_nPaddedN1;

  for(int i = 0; i < nsamps; i += CACHE_SIZE) {
    int n = nsamps - i;
    if(n > CACHE_SIZE) n = CACHE_SIZE;
    for(int m = 0; m < m_nPaddedN2; m++) {
      int index = m * m_nPaddedN1 + i;
      for(int l = 0; l < n; l++) {
        m_pfVecW[l * m_nPaddedN2 + m] = _paddedTraces[l + index];
      }
    }
    if(_direction == FORWARD) {
      for(int l = 0; l < n; l++)
        m_transformer.lotFwd(&m_pfVecW[l * m_nPaddedN2], 0, m_nHorizontalBlockSize, m_nHorizontalTransLength, nblocksHorizontal, m_pfScratch2);
    } else {
      for(int l = 0; l < n; l++)
        m_transformer.lotRev(&m_pfVecW[l * m_nPaddedN2], 0, m_nHorizontalBlockSize, m_nHorizontalTransLength, nblocksHorizontal, m_pfScratch2);
    }

    for(int m = 0; m < m_nPaddedN2; m++) {
      int index = m * m_nPaddedN1 + i;
      for(int l = 0; l < n; l++) {
        _paddedTraces[l + index] = m_pfVecW[l * m_nPaddedN2 + m];
      }
    }
  }

  return JS_OK;
}


//in java-code codeAllBlocks with direction = FORWARD
int SeisPEG::codeAllBlocks(float *_paddedTraces, int _paddedN1, int _paddedN2,
                           float _distortion, int _verticalBlockSize, int _horizontalBlockSize,
                           char *_encodedData, int _index, int _bufferSize) {
  // This could be threaded, but it would require us to separate all of the work buffers
  // and the instance of BlockCompressor that is used.
  int nblocksVertical = _paddedN1 / _verticalBlockSize;
  int nblocksHorizontal = _paddedN2 / _horizontalBlockSize;

  if(nblocksVertical < 1  ||  nblocksHorizontal < 1) {
    ERROR_PRINTF(SeisPEGLog, "Padded data size is less than 1 block");
    return JS_USERERROR;
  }

  int samplesPerBlock = _verticalBlockSize * _horizontalBlockSize;
  // A column is a vertical series of blocks.
  int samplesPerColumn = samplesPerBlock * nblocksVertical;

  // We add extra space to ensure that dataEncode doesn't walk off the end.
  if(m_pfWorkBlock == NULL)
    m_pfWorkBlock = new float[m_nVerticalBlockSize * m_nHorizontalBlockSize];
  if(m_pcWorkBuffer2 == NULL) {
    // Byte block large enough to hold a block with a compression ratio of !:1.
    m_nWorkBuffer2Size = _verticalBlockSize * _horizontalBlockSize * 4;
    m_pcWorkBuffer2 = new char[m_nWorkBuffer2Size];
  }

  int nbytesTotal = 0;
  int i;

  int encodedDataIndex = _index;
  int workBlockIndex = 0;
  int dataIndex = 0;

  for(int l = 0; l < nblocksHorizontal; l++) {
    for(int k = 0; k < nblocksVertical; k++) {
      dataIndex = l * samplesPerColumn + k * m_nVerticalBlockSize;
      workBlockIndex = 0;
      int nbytes = 0;
      for(int j = 0; j < m_nHorizontalBlockSize; j++) {
        //           if (_direction == FORWARD) {
        for(i = 0; i < m_nVerticalBlockSize; i++)
          m_pfWorkBlock[i + workBlockIndex] = _paddedTraces[i + dataIndex];
        //           }
        dataIndex += _paddedN1;
        workBlockIndex += m_nVerticalBlockSize;
      }

      //         if (_direction == FORWARD)
      //         {
      // Encode the block.
      int nbytesAvailable = _bufferSize - (encodedDataIndex + SIZEOF_INT - _index);
      if(nbytesAvailable < 1) {
        ERROR_PRINTF(SeisPEGLog, "nbytesAvailable < 1");
        return JS_USERERROR;// Certain Overflow!
      }

      nbytes = m_blockCompressor.dataEncode(m_pfWorkBlock, samplesPerBlock,
                                            _distortion, _encodedData, encodedDataIndex + SIZEOF_INT,
                                            nbytesAvailable);
      if(nbytes == 0) {
        ERROR_PRINTF(SeisPEGLog, "nbytesAvailable == 0");
        return JS_USERERROR;// Overflow!
      }

      nbytes += SIZEOF_INT;
      // Stuff the size of the block in front of the data.
      BlockCompressor::stuffIntInBytes(nbytes, _encodedData, encodedDataIndex);
      encodedDataIndex += nbytes;
      //         }
      nbytesTotal += nbytes;
    }
  }

  return nbytesTotal;
}
/*
    * Performs the transform along the trace (second) axis.
    *
    * @param  _paddedTraces  the trace data, padded to a multiple of the block size
    *                       in both directions.
    * @param  _paddedN1  the padded trace length.
    * @param  _paddedN2  the padded traces per frame.
    * @param  _distortion  the allowed m_fDistortion.
    * @param  _verticalBlockSize  the vertical block size.
    * @param  _horizontalBlockSize  the horizontal block size.
    * @param  _encodedData  the output encoded data.
    * @param  _index  the starting index into the encoded data.
    * @param  _bufferSize  the size of the encoded data buffer.
    * @return  JS_USERERROR if the buffer is too small, otherwise the number of bytes required
    *          to hold the encoded data.
   */
//in java-code codeAllBlocks with direction = REVERSE
int SeisPEG::decodeAllBlocks(float *_paddedTraces, int _paddedN1, int _paddedN2,
                             float _distortion, int _verticalBlockSize, int _horizontalBlockSize,
                             const char *_encodedData, int _index, int _bufferSize) {

  // This could be threaded, but it would require us to separate all of the work buffers
  // and the instance of BlockCompressor that is used.
  int nblocksVertical = _paddedN1 / _verticalBlockSize;
  int nblocksHorizontal = _paddedN2 / _horizontalBlockSize;

  if(nblocksVertical < 1  ||  nblocksHorizontal < 1) {
    ERROR_PRINTF(SeisPEGLog, "Padded data size is less than 1 block");
    return JS_USERERROR;
  }

  int samplesPerBlock = _verticalBlockSize * _horizontalBlockSize;
  // A column is a vertical series of blocks.
  int samplesPerColumn = samplesPerBlock * nblocksVertical;

  // We add extra space to ensure that dataEncode doesn't walk off the end.
  if(m_pfWorkBlock == NULL)
    m_pfWorkBlock = new float[m_nVerticalBlockSize * m_nHorizontalBlockSize];
  if(m_pcWorkBuffer2 == NULL) {
    // Byte block large enough to hold a block with a compression ratio of !:1.
    m_nWorkBuffer2Size = _verticalBlockSize * _horizontalBlockSize * 4;
    m_pcWorkBuffer2 = new char[m_nWorkBuffer2Size];
  }

  int nbytesTotal = 0;
  int i;

  int encodedDataIndex = _index;
  int workBlockIndex = 0;
  int dataIndex = 0;

  for(int l = 0; l < nblocksHorizontal; l++) {
    for(int k = 0; k < nblocksVertical; k++) {
      dataIndex = l * samplesPerColumn + k * m_nVerticalBlockSize;
      workBlockIndex = 0;
      int nbytes = 0;
      //         if (_direction == REVERSE)
      //         {
      nbytes = BlockCompressor::stuffBytesInInt(_encodedData, encodedDataIndex);
      if((encodedDataIndex - _index) + nbytes > _bufferSize) {
        ERROR_PRINTF(SeisPEGLog, "encodedDataIndex-index)+nbytes > bufferSize");
        return JS_USERERROR;// Overflow!
      }
      int ierr = -1;
      while(ierr != JS_OK) {
        ierr = m_blockCompressor.dataDecode(_encodedData, SIZEOF_INT + encodedDataIndex,
                                            m_pcWorkBuffer2, m_nWorkBuffer2Size,
                                            samplesPerBlock, m_pfWorkBlock);
        if(ierr != JS_OK) {
          // Buffer is too small!
          m_nWorkBuffer2Size *= 2;
          delete[]m_pcWorkBuffer2;
          m_pcWorkBuffer2 = new char[m_nWorkBuffer2Size];
        }
      }
      encodedDataIndex += nbytes;
      //         }

      for(int j = 0; j < m_nHorizontalBlockSize; j++) {
        for(i = 0; i < m_nVerticalBlockSize; i++)
          _paddedTraces[i + dataIndex] = m_pfWorkBlock[i + workBlockIndex];
        dataIndex += _paddedN1;
        workBlockIndex += m_nVerticalBlockSize;
      }

      nbytesTotal += nbytes;
    }
  }

  return nbytesTotal;
}



/**
   * Stores values in the header of the encoded data.
   *
   * @param  _encodedData  the encoded data.
   * @param  _hdrInfo  an array of header data.
   * @return  the length of the stored header (same as returned from encodeHdr()).
 */
int SeisPEG::updateHdr(char *_encodedData, int *_hdrInfo) {
  short _cookie = (short)_hdrInfo[IND_COOKIE];
  float _distortion = BlockCompressor::intBitsToFloat(_hdrInfo[IND_DISTORTION]);
  int _n1 = _hdrInfo[IND_N1];
  int _n2 = _hdrInfo[IND_N2];
  int _verticalBlockSize = _hdrInfo[IND_VBLOCK_SIZE];
  int _horizontalBlockSize = _hdrInfo[IND_HBLOCK_SIZE];
  int _verticalTransLength = _hdrInfo[IND_VTRANS_LEN];
  int _horizontalTransLength = _hdrInfo[IND_HTRANS_LEN];
  int _nBytesTraces = _hdrInfo[IND_NBYTES_TRACES];
  int _nBytesHdrs = _hdrInfo[IND_NBYTES_HDRS];
  float _ftGainExponent =  BlockCompressor::intBitsToFloat(_hdrInfo[IND_FT_GAIN]);

  return encodeHdr(_encodedData, _cookie, _distortion, _ftGainExponent, _n1,
                   _n2, _verticalBlockSize, _horizontalBlockSize,
                   _verticalTransLength, _horizontalTransLength,
                   _nBytesTraces, _nBytesHdrs);
}

/*
   * Stores values in the header of the encoded data.
   *
   * @param  _encodedData  the encoded data.
   * @param  _cookie  the cookie to use (different for bad-amplitude data).
   * @param  _distortion  the allowed m_fDistortion.
   * @param  _ftGainExponent  the function-of-time gain exponent, or 0.0 if none was applied.
   * @param  _n1  the number of samples.
   * @param  _n2  the number of traces.
   * @param  _verticalBlockSize  the vertical block size.
   * @param  _horizontalBlockSize  the horizontal block size.
   * @param  _verticalTransLength  the vertical transform length.  Must be 8 or 16.
   * @param  _horizontalTransLength  the horizontal transform length.  Must be 8 or 16.
   * @param  _nBytesTraces  the number of bytes used to store the traces (if traces and headers
   *                       are compressed together), otherwise 0;
   * @param  _nBytesHdrs  the number of bytes used to store the headers (if traces and headers
   *                     are compressed together), otherwise 0;
   * @return  the length of the stored header.
 */
int SeisPEG::encodeHdr(char *_encodedData, short _cookie, float _distortion, float _ftGainExponent,
                       int _n1, int _n2, int _verticalBlockSize, int _horizontalBlockSize,
                       int _verticalTransLength, int _horizontalTransLength,
                       int _nBytesTraces, int _nBytesHdrs) {
  int encodedDataIndex = 0;
  BlockCompressor::stuffShortInBytes(_cookie, _encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_SHORT;
  BlockCompressor::stuffInBytes(_distortion, _encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  BlockCompressor::stuffIntInBytes(_n1, _encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  BlockCompressor::stuffIntInBytes(_n2, _encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  // Assume that block sizes will never exceed 32767.
  if(_verticalBlockSize > 32767) {
    ERROR_PRINTF(SeisPEGLog, "m_nVerticalBlockSize > Short.MAX_VALUE");
    return JS_USERERROR;
  }
  BlockCompressor::stuffShortInBytes((short)_verticalBlockSize, _encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_SHORT;
  if(_horizontalBlockSize > 32767) {
    ERROR_PRINTF(SeisPEGLog, "m_nHorizontalBlockSize > Short.MAX_VALUE");
    return JS_USERERROR;
  }
  BlockCompressor::stuffShortInBytes((short)_horizontalBlockSize, _encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_SHORT;
  // Assume that the transform length will never exceed 127.
  if(_verticalTransLength > 127) {
    ERROR_PRINTF(SeisPEGLog, "_verticalTransLength > Char.MAX_VALUE");
    return JS_USERERROR;
  }
  _encodedData[encodedDataIndex] = (char)_verticalTransLength;
  encodedDataIndex += SIZEOF_CHAR;
  if(_horizontalTransLength > 127) {
    ERROR_PRINTF(SeisPEGLog, "_horizontalTransLength > Char.MAX_VALUE");
    return JS_USERERROR;
  }
  _encodedData[encodedDataIndex] = (char)_horizontalTransLength;
  encodedDataIndex += SIZEOF_CHAR;
  BlockCompressor::stuffIntInBytes(_nBytesTraces, _encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  BlockCompressor::stuffIntInBytes(_nBytesHdrs, _encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  if(_cookie == COOKIE_V3  ||  _cookie == BAD_AMPLITUDE_COOKIE_V3) {
    BlockCompressor::stuffInBytes(_ftGainExponent, _encodedData, encodedDataIndex);
    encodedDataIndex += SIZEOF_INT;
  }

  return encodedDataIndex;
}

bool SeisPEG::checkDataIntegrity(char *_encodedData) {

  int *m_piHdrInfo = new int[LEN_HDR_INFO];
  int ires = decodeHdr(_encodedData, m_piHdrInfo);
  delete[]m_piHdrInfo;
  if(ires < 0) return false;
  return true;
}


/*
  * Returns true if the encoded data contained bad amplitudes, otherwise false.
  *
  * @param  _encodedData  the encoded data.
  * @return  true if the encoded data contained bad amplitudes, otherwise false.
*/
int SeisPEG::badAmplitudeData(const char *_encodedData)  {

  int cookie = BlockCompressor::stuffBytesInShort(_encodedData, 0);
  if(cookie == COOKIE_V2  ||  cookie == COOKIE_V3) {
    return 0;//false;
  } else if(cookie == BAD_AMPLITUDE_COOKIE_V2  ||  cookie == BAD_AMPLITUDE_COOKIE_V3) {
    return 1;//true;
  } else {
    ERROR_PRINTF(SeisPEGLog, "Sorry - you are trying to uncompress data from an unsupported unreleased version of SeisPEG. cookie=%d", cookie);
    return JS_USERERROR;
  }
}


/*
  * Fetches the compressed data header information.
  *
  * @param  _encodedData  the encoded data.
  * @param  _hdrInfo  an array of resulting values.  Inspect code for details.
  * @return  the number of bytes in the encoded header.
*/
int SeisPEG::decodeHdr(const char *_encodedData, int *_hdrInfo) {

  int encodedDataIndex = 0;
  int cookie = BlockCompressor::stuffBytesInShort(_encodedData, encodedDataIndex);
  //     printf("cookie=%d\n",cookie);
  if(cookie != COOKIE_V2  &&  cookie != BAD_AMPLITUDE_COOKIE_V2
      &&  cookie != COOKIE_V3  &&  cookie != BAD_AMPLITUDE_COOKIE_V3) {
    ERROR_PRINTF(SeisPEGLog, "Sorry - you are trying to uncompress data from an unsupported unreleased version of SeisPEG");
    return JS_USERERROR;
  }

  encodedDataIndex += SIZEOF_SHORT;
  int _idistortion = BlockCompressor::stuffBytesInInt(_encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  int _n1 = BlockCompressor::stuffBytesInInt(_encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  int _n2 = BlockCompressor::stuffBytesInInt(_encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  int _verticalBlockSize = BlockCompressor::stuffBytesInShort(_encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_SHORT;
  int _horizontalBlockSize = BlockCompressor::stuffBytesInShort(_encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_SHORT;
  int _verticalTransLength = (int)_encodedData[encodedDataIndex];
  encodedDataIndex += SIZEOF_CHAR;
  int _horizontalTransLength = (int)_encodedData[encodedDataIndex];
  encodedDataIndex += SIZEOF_CHAR;
  int _nBytesTraces = BlockCompressor::stuffBytesInInt(_encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  int _nBytesHdrs = BlockCompressor::stuffBytesInInt(_encodedData, encodedDataIndex);
  encodedDataIndex += SIZEOF_INT;
  int _iftGainExponent = 0;
  if(cookie == COOKIE_V3  ||  cookie == BAD_AMPLITUDE_COOKIE_V3) {
    _iftGainExponent = BlockCompressor::stuffBytesInInt(_encodedData, encodedDataIndex);
    encodedDataIndex += SIZEOF_INT;
  }

  _hdrInfo[IND_COOKIE] = cookie;
  _hdrInfo[IND_DISTORTION] = _idistortion;
  _hdrInfo[IND_N1] = _n1;
  _hdrInfo[IND_N2] = _n2;
  _hdrInfo[IND_VBLOCK_SIZE] = _verticalBlockSize;
  _hdrInfo[IND_HBLOCK_SIZE] = _horizontalBlockSize;
  _hdrInfo[IND_VTRANS_LEN] = _verticalTransLength;
  _hdrInfo[IND_HTRANS_LEN] = _horizontalTransLength;
  _hdrInfo[IND_NBYTES_TRACES] = _nBytesTraces;
  _hdrInfo[IND_NBYTES_HDRS] = _nBytesHdrs;
  _hdrInfo[IND_FT_GAIN] = _iftGainExponent;

  for(int i = IND_N1; i < IND_HTRANS_LEN; i++) {
    if(_hdrInfo[i] < 1) {
      ERROR_PRINTF(SeisPEGLog, "Invalid header - data corrupted?");
      return JS_USERERROR;
    }
  }

  return encodedDataIndex;
}


/*
  * Performs the forward compression - for testing.
  *
  * @param  _paddedTraces  the trace data, padded to a multiple of the block size
  *                       in both directions.
  * @param  _n1  number of samples per trace.
  * @param  _n2  number of traces per frame.
  * @param  _paddedN1  the padded length of the sample axis.
  * @param  _paddedN2  the padded length of the trace axis.
  * @param  _distortion  the allowed m_fDistortion.
  * @param  _verticalBlockSize  the vertical block size.  Must be a multiple of 8.
  * @param  _horizontalBlockSize  the horizontal block size.    Must be a multiple of 8.
  * @param  _verticalTransLength  the vertical transform length.  Must be 8 or 16.
  * @param  _horizontalTransLength  the horizontal transform length.  Must be 8 or 16.
  * @param  _encodedData  the output encoded data.
  * @param  _outputBufferSize  the size of the encoded data buffer.
  * @return  0 if the buffer is too small, otherwise the number of bytes required
  *          to hold the encoded data.
*/
int SeisPEG::compress2D(float *_paddedTraces, int _n1, int _n2,
                        int _paddedN1, int _paddedN2, float _distortion, float _ftGainExponent,
                        int _verticalBlockSize, int _horizontalBlockSize,
                        int _verticalTransLength, int _horizontalTransLength,
                        char *_encodedData, int _outputBufferSize) {
  if(_paddedN1 == 0  ||  _paddedN2 == 0  ||  _verticalBlockSize == 0
      ||  _horizontalBlockSize == 0  ||  _verticalTransLength == 0
      ||  _horizontalBlockSize == 0) {
    ERROR_PRINTF(SeisPEGLog, "Invalid args");
    return JS_USERERROR;
  }

  m_n1 = _n1;
  m_n2 = _n2;
  m_nPaddedN1 = _paddedN1;
  m_nPaddedN2 = _paddedN2;
  m_nVerticalBlockSize = _verticalBlockSize;
  m_nHorizontalBlockSize = _horizontalBlockSize;
  m_nVerticalTransLength = _verticalTransLength;
  m_nHorizontalTransLength = _horizontalTransLength;

  return compress2D(_paddedTraces, _distortion, _ftGainExponent, _encodedData, _outputBufferSize);

}



int SeisPEG::computeFtGain(int _samplesPerTrace, float _ftGainExponent) {
  if(m_pFtGain != NULL) delete []m_pFtGain;
  m_pFtGain = new float[_samplesPerTrace];
  double sum = 0.0;
  for(int i = 0; i < _samplesPerTrace; i++) {
    // We can just assume a nominal sample interval of 4.0.
    // double time = ((double)i * (double)sampleInterval) / 1000.0;
    double time = ((double)i * 4.0) / 1000.0;
    // Add a small number to stabilize.
    m_pFtGain[i] = (float)pow(time, m_fFtGainExponent) + 0.001F;
    sum += (double)m_pFtGain[i];
  }

  // Now we divide by the mean, to make the amplitudes more similar to the original.
  float averageGain = (float)(sum / (double)_samplesPerTrace);
  for(int i = 0; i < _samplesPerTrace; i++) {
    m_pFtGain[i] /= averageGain;
  }
  m_nFtGain_len = _samplesPerTrace;
  return m_nFtGain_len;
}


void SeisPEG::applyFtGain(float *_ftGain, float *_buffer, int _index, int _n1) {
  for(int i = 0; i < _n1; i++)
    _buffer[_index + i] *= _ftGain[i];
}


void SeisPEG::removeFtGain(float *_ftGain, float *_trace, int _tracelen) {
  for(int i = 0; i < _tracelen; i++)
    _trace[i] /= _ftGain[i];
}


/*
  * Performs the forward compression.
  *
  * @param  _paddedTraces  the trace data, padded to a multiple of the block size
  *                       in both directions.
  * @param  _distortion  the allowed m_fDistortion.
  * @param  _encodedData  the output encoded data.
  * @param  _outputBufferSize  the size of the encoded data buffer.
  * @return  JS_USERERROR if the buffer is too small, otherwise the number of bytes required
  *          to hold the encoded data.
*/
int SeisPEG::compress2D(float *_paddedTraces, float _distortion, float _ftGainExponent,
                        char *_encodedData, int _outputBufferSize) {
  // Transform in x1 first.
  x1Transform(FORWARD, _paddedTraces);

  // Transform in time.
  timeTransform(FORWARD, _paddedTraces);

  // Encode the header.
  short cookie;
  if(ISNOTZERO(_ftGainExponent)) {
    // No reason to mess up compatibility with existing code if the gain isn't used.
    cookie = COOKIE_V2;
  } else {
    cookie = COOKIE_V3;
  }
  int nbytesHdr = encodeHdr(_encodedData, cookie, _distortion, _ftGainExponent,
                            m_n1, m_n2, m_nVerticalBlockSize, m_nHorizontalBlockSize,
                            m_nVerticalTransLength, m_nHorizontalTransLength, 0, 0);


  // Encode each transform block individually.
  int nbytesData = codeAllBlocks(_paddedTraces, m_nPaddedN1, m_nPaddedN2,
                                 _distortion, m_nVerticalBlockSize,
                                 m_nHorizontalBlockSize, _encodedData, nbytesHdr,
                                 _outputBufferSize - nbytesHdr);

  if(nbytesData == JS_USERERROR) {
    // We don't have enough room in the output buffer.  Punt.
    ERROR_PRINTF(SeisPEGLog, "There is no enough room in the output buffer");
    return JS_USERERROR;
  }

  int nBytesTotal = nbytesHdr + nbytesData;

  // Update the header.
  decodeHdr(_encodedData, m_piHdrInfo);
  m_piHdrInfo[IND_NBYTES_TRACES] = nBytesTotal;
  m_piHdrInfo[IND_NBYTES_HDRS] = 0;
  updateHdr(_encodedData, m_piHdrInfo);

  return nBytesTotal;
}


/*
  * Performs the reverse uncompression.
  *
  * @param  _encodedData  the input encoded data.
  * @param  _inputBufferSize  the size of the encoded data buffer.
  * @param  _paddedTraces  the output trace data, padded to a multiple of the block size
  *                       in both directions.
  * @return  JS_USERERROR if the data appears to be corrupted, otherwise JS_OK.
*/
int SeisPEG::uncompress2D(const char *_encodedData, int _inputBufferSize, float *_paddedTraces) {
  // Decode the hdr.
  int nbytesHdr = decodeHdr(_encodedData, m_piHdrInfo);

  m_n1 = m_piHdrInfo[IND_N1];
  m_n2 = m_piHdrInfo[IND_N2];
  m_nVerticalBlockSize = m_piHdrInfo[IND_VBLOCK_SIZE];
  m_nHorizontalBlockSize = m_piHdrInfo[IND_HBLOCK_SIZE];
  m_nVerticalTransLength = m_piHdrInfo[IND_VTRANS_LEN];
  m_nHorizontalTransLength = m_piHdrInfo[IND_HTRANS_LEN];

  m_nPaddedN1 = computePaddedLength(m_n1, m_nVerticalBlockSize);
  m_nPaddedN2 = computePaddedLength(m_n2, m_nHorizontalBlockSize);

  // Decode each transform block individually.
  // We don't care about the size of the output buffer, since it's
  // actually an input buffer.

  float _distortion = 0.0F; /*NOT USED*/

  int nbytes = decodeAllBlocks(_paddedTraces, m_nPaddedN1, m_nPaddedN2,
                               _distortion, m_nVerticalBlockSize,
                               m_nHorizontalBlockSize, _encodedData, nbytesHdr,
                               _inputBufferSize - nbytesHdr);

  if(nbytes == JS_USERERROR) return JS_USERERROR;

  // Transform in time.
  timeTransform(REVERSE, _paddedTraces);

  // Transform in x1 last.
  x1Transform(REVERSE, _paddedTraces);

  return JS_OK;
}


/*
   * Returns the appropriate number of bytes for the output buffer for compressed
   * trace headers.
   *
   * @param  _maxHdrLength  the maximum length of any header (number of ints).
   * @param  _maxTracesPerFrame  the maximum number of trace per frame.
   * @return  the appropriate number of bytes for the output buffer for compressed
   *          trace headers.
 */
int SeisPEG::getOutputHdrBufferSize(int _maxHdrLength, int _maxTracesPerFrame) {
  return HdrCompressor::getOutputBufferSize(_maxHdrLength, _maxTracesPerFrame);
}


/*
   * Compresses a 2D array of trace headers.
   *
   * @param  _hdrs  the trace headers.
  * @param  _hdrLength  the length of each header (may be less than the array length).
   * @param  _nTraces  the number of live trace headers.
   * @param  _encodedBytes  the output compressed headers.  To determine the necessary
   *                        size use getOutputBufferSize().
   * @return  the number of bytes used to encode the data.
 */
int SeisPEG::compressHdrs(int *_hdrs, int _hdrLength, int _nTraces, char *_encodedBytes) {
  return m_hdrCompressor.compress(_hdrs, _hdrLength, NULL, _nTraces, 0, _encodedBytes, 0);
}


/*
   * Convenience method for JavaSeis format.
   * Compresses a frame/ensemble of traces and trace headers.  Does not alter the input data.
   *
   * @param  _traces  the trace data.
   * @param  _nTraces  the number of lives traces.
   * @param  _hdrIntBuffer  an IntBuffer that contains the trace headers.
   * @param  _hdrLength  the length of each header.
   * @param  _outputData  space for the compressed data.  Must be large enough to hold
   *                     an uncompressed copy of the input data, including non-live traces,
   *                     and the space required for headers (use HdrCompressor.getOutputBufferSize()).
   * @return  the number of bytes of output compressed data.
 */
int SeisPEG::compress(float *_traces, int _nTraces, IntBuffer *_hdrIntBuffer, int _hdrLength, char *_outputData) {
  int nBytesTraces = compress(_traces, _nTraces, _outputData);
  int nBytesHdrs = m_hdrCompressor.compress(_hdrIntBuffer, _hdrLength, _traces, _nTraces, m_n1, _outputData, nBytesTraces);

  // Update the header.
  decodeHdr(_outputData, m_piHdrInfo);
  m_piHdrInfo[IND_NBYTES_TRACES] = nBytesTraces;
  m_piHdrInfo[IND_NBYTES_HDRS] = nBytesHdrs;
  updateHdr(_outputData, m_piHdrInfo);
  return nBytesTraces + nBytesHdrs;
}


/*
  * Uncompresses a 2D array of trace headers.
  *
  * @param  _encodedBytes  the input compressed headers (from the compress() method).
  * @param  _nBytes  the number of input compressed bytes.
  * @param  _hdrs  the output trace headers.
  * @return  the number of live output trace headers.
*/
int SeisPEG::uncompressHdrs(const char *_encodedBytes, int _nBytes, int *_hdrs, int _hdrLength) {
  decodeHdr(_encodedBytes, m_piHdrInfo);
  uncompress(_encodedBytes, _nBytes, NULL, 0);
  int nBytesTraces = m_piHdrInfo[IND_NBYTES_TRACES];
  int nBytesHdrs = m_piHdrInfo[IND_NBYTES_HDRS];
  return m_hdrCompressor.uncompress(_encodedBytes, nBytesTraces, nBytesHdrs, _hdrs, _hdrLength,  NULL, m_n2, m_n1);
}


/*
  * Convenience method for JavaSeis format.
  * Uncompresses a frame/ensemble of traces and trace headers.
  *
  * @param  _compressedByteData  the compressed byte data.
  * @param  _compressedDataLength  the length of the compressed data.
  * @param  _traces  the output uncompressed data.
  * @param  _hdrIntBuffer  the output trace headers.
  * @return  the number of live output trace headers.
*/
int SeisPEG::uncompress(const char *_compressedByteData, int _compressedDataLength,
                        float *_traces, int _nTraces, IntBuffer *_hdrIntBuffer, int _hdrLength) {

  decodeHdr(_compressedByteData, m_piHdrInfo);
  uncompress(_compressedByteData, _compressedDataLength, _traces, _nTraces);
  int nBytesTraces = m_piHdrInfo[IND_NBYTES_TRACES];
  int nBytesHdrs = m_piHdrInfo[IND_NBYTES_HDRS];
  return m_hdrCompressor.uncompress(_compressedByteData, nBytesTraces, nBytesHdrs,
                                    _hdrIntBuffer, _hdrLength, _traces, _nTraces, m_n1);
}

int SeisPEG::uncompress(const char *_compressedByteData, int _compressedDataLength,
                        float *_traces, int _nTraces, int *_hdrIntBufArray, int _hdrLength) {

  decodeHdr(_compressedByteData, m_piHdrInfo);
  uncompress(_compressedByteData, _compressedDataLength, _traces, _nTraces);
  int nBytesTraces = m_piHdrInfo[IND_NBYTES_TRACES];
  int nBytesHdrs = m_piHdrInfo[IND_NBYTES_HDRS];
  return m_hdrCompressor.uncompress(_compressedByteData, nBytesTraces, nBytesHdrs,
                                    _hdrIntBufArray, _hdrLength, _traces, _nTraces, m_n1);
}


/*
  * Updates compression statistics that can be used to compute compression ratios.
  * This method accumulates the values that are passed to it.
  *
  * @param  nTracesWritten  the number of traces just written.
  * @param  m_nTraceLength  the trace length (number of floats).
  * @param  m_nHdrLength  the trace header length (number of ints).
  * @param  nBytes  the number of bytes used to compress the traces and headers.
*/
void SeisPEG::updateStatistics(int _nTracesWritten, int _traceLength, int _hdrLength, int _nBytes) {
  m_nTraceLength = _traceLength;
  m_nHdrLength = _hdrLength;
  m_nTracesWrittenTotal += _nTracesWritten;
  m_nBytesTotal += _nBytes;
}

long SeisPEG::countTracesWritten() {
  return m_nTracesWrittenTotal;
}

double SeisPEG::getCompressionRatio() {
  return (double)(m_nTraceLength * m_nTracesWrittenTotal * 4 + m_nHdrLength * m_nTracesWrittenTotal * 4) /
         (double)m_nBytesTotal;
}

}

