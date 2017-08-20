/***************************************************************************
                          SeisPEG.h  -  description
                             -------------------
* the class for SeisPEG compression methods.

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

#ifndef SEISPEG_H
#define SEISPEG_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
 
#include "CompressedData.h"
#include "Transformer.h"
#include "BlockCompressor.h"
#include "HdrCompressor.h"
#include "../jsDefs.h"

namespace jsIO
{
  enum SeisPEG_Policy {SEISPEG_POLICY_FASTEST, SEISPEG_POLICY_MAX_COMPRESSION};

  class SeisPEG{
    public:
      ~SeisPEG();
      SeisPEG();
      SeisPEG(int* _huffCount, int _n1, int _n2, float _distortion,
              int _verticalBlockSize, int _horizontalBlockSize,
              int _verticalTransLength, int _horizontalTransLength);

      SeisPEG(int _n1, int _n2, float _distortion,
              int _verticalBlockSize, int _horizontalBlockSize,
              int _verticalTransLength, int _horizontalTransLength);

      SeisPEG(int _n1, int _n2, float _distortion, SeisPEG_Policy _policy);
      SeisPEG(char* compressedByteData);

      int Init(int _n1, int _n2, float _distortion, SeisPEG_Policy _policy);
          
      int Init(char* compressedByteData);
      int Init(int _n1, int _n2, float _distortion, float _ftGainExponent,
               int _verticalBlockSize, int _horizontalBlockSize,
               int _verticalTransLength, int _horizontalTransLength, std::string _whichConstructor);

      float getDistortion() const {return m_fDistortion;}; 
      int getSamplesPerTrace() const {return m_n1;};
      int getMaxTracesPerFrame() const {return m_n2;};
      int getVerticalBlockSize() const {return m_nVerticalBlockSize;};
      int getHorizontalBlockSize() const {return m_nHorizontalBlockSize;};
      int getVerticalTransLength() const {return m_nVerticalTransLength;};
      int getHorizontalTransLength() const {return m_nHorizontalTransLength;}
      int uncompressedBufferAllocSize() const {return m_n2*m_n1;};

      int setGainExponent(float _ftGainExponent);
      void setDelta(float _delta);
      long compressedByteBufferAllocSize();
 

      int compress(float* _traces, int _nTraces, CompressedData &_compressedData);
      int compress(float* _traces, int _nTraces, char* _outputData);
      int compress(float* _traces, int _nTraces, IntBuffer *_hdrIntBuffer, int _hdrLength, char* outputData);

      int uncompress(CompressedData &compressedData, float* traces, int nTraces);
      int uncompress(const char* compressedByteData, int compressedDataLength, float* traces, int nTraces);
      int uncompress(const char* compressedByteData, int compressedDataLength, float* traces, int nTraces, 
                     IntBuffer *hdrIntBuffer, int _hdrLength);
      int uncompress(const char* compressedByteData, int compressedDataLength, float* traces, int nTraces, 
                     int *hdrIntBufArray, int _hdrLength);

      int uncompressHdrs(const char* encodedBytes, int nBytes, int* hdrs, int _hdrLength);
      void updateStatistics(int _nTracesWritten, int _traceLength, int _hdrLength, int _nBytes);
      
      
      static int getOutputHdrBufferSize(int maxHdrLength, int maxTracesPerFrame);
      static int computePaddedLength(int nsamples, int blockSize);
      static int computeBlockSize(int nsamples, SeisPEG_Policy policy);
      static int computeTransLength(int blockSize, SeisPEG_Policy policy);
      static bool checkBlockSize(int blockSize);
      static bool checkTransLength(int transLength, int blockSize);
      static void fillBuffer(int _direction, float* _traces, int _n1, int _n2,
                             int _paddedN1, int _paddedN2, float* _workBuffer, float* _ftGain);
    
      static void applyFtGain(float* m_pFtGain, float* buffer, int index, int _n1);
      static void removeFtGain(float* m_pFtGain, float* trace, int tracelen) ;
      
      
      long countTracesWritten();
      double getCompressionRatio();

    public:
      static const int LEN_HDR_INFO = 11;

    private:

      int nbytes4compressedByteBufferAlloc();
      void transform2D(float* paddedTraces);
      bool containsBadAmplitude(float* traces, int nTraces, int nSamples);
      int computeFtGain(int _samplesPerTrace, float _ftGainExponent);

      int codeAllBlocks(float* _paddedTraces, int _paddedN1, int _paddedN2,
                        float _distortion, int _verticalBlockSize, int _horizontalBlockSize,
                        char* _encodedData, int _index, int _bufferSize);

      int decodeAllBlocks(float* _paddedTraces, int _paddedN1, int _paddedN2,
                          float _distortion, int _verticalBlockSize, int _horizontalBlockSize,
                          const char* _encodedData, int _index, int _bufferSize);

      int timeTransform(int direction, float* paddedTraces);
      int x1Transform(int direction, float* paddedTraces);

      int transform(float* traces, int nTraces);
      int badAmplitudeCompress(float* _traces, int _nTraces, CompressedData &_compressedData);
      int badAmplitudeCompress(float* _traces, int _nTraces, char *_outputData);
      void badAmplitudeUncompress(const char* _inData, int _inDataLength, float* _traces, int _nTraces);

      int compress2D(float* _paddedTraces, float _distortion, float _ftGainExponent,
                     char* _encodedData, int _outputBufferSize);
      int compress2D(float* _paddedTraces, int _n1, int _n2, int _paddedN1, int _paddedN2, 
                     float _distortion, float _ftGainExponent, int _verticalBlockSize, int _horizontalBlockSize,
                     int _verticalTransLength, int _horizontalTransLength, char* _encodedData, int _outputBufferSize);
      int uncompress2D(const char* encodedData, int inputBufferSize, float* paddedTraces);
      int compressHdrs(int* hdrs, int m_nHdrLength, int nTraces, char* encodedBytes);

      float difference(float* _traces, int _nTraces, float *_traces_diff);


      int updateHdr(char* encodedData, int* m_piHdrInfo);
      int encodeHdr(char* _encodedData, short _cookie, float _distortion, float _ftGainExponent,
                    int _n1, int _n2, int _verticalBlockSize, int _horizontalBlockSize,
                    int _verticalTransLength, int _horizontalTransLength,
                    int _nBytesTraces, int _nBytesHdrs);
      bool checkDataIntegrity(char* encodedData);
      int badAmplitudeData(const char* encodedData);
      int decodeHdr(const char* encodedData, int* m_piHdrInfo);



// private atributes
    private:
      int m_n1;
      int m_n2;
      float m_fDistortion;
      float m_fFtGainExponent;
      bool m_bFtGainExponentWasStored;
      float* m_pFtGain;
      int m_nFtGain_len;
      int m_nVerticalBlockSize;
      int m_nHorizontalBlockSize;
      int m_nVerticalTransLength;
      int m_nHorizontalTransLength;
      int m_nPaddedN1;
      int m_nPaddedN2;
      float* m_pfWorkBuffer1;  // Length of _paddedN1 * _paddedN2.
      char* m_pcWorkBuffer2;   // Large enough to hold a decompressed block.
      int m_nWorkBuffer2Size;
      float* m_pfWorkBlock;    // Length of m_nVerticalBlockSize*m_nHorizontalBlockSize;
      char* m_pcCompressedBuffer;
      Transformer m_transformer;
      BlockCompressor m_blockCompressor;
      HdrCompressor m_hdrCompressor;

      IntBuffer *m_hdrIntBuffer;

      float* m_pfVecW;     // Length of CACHE_SIZE* _paddedN2.
      float* m_pfScratch1;
      float* m_pfScratch2;
      int m_nEnsemblesChecked;
      int* m_piHdrInfo;

      int m_nTraceLength;
      int m_nHdrLength;
      long m_nTracesWrittenTotal;
      long m_nBytesTotal;

      bool m_bInit;

      int m_nThreads;

      static const int FORWARD =  1;
      static const int REVERSE = -1;
 
      static const int IND_COOKIE = 0;
      static const int IND_DISTORTION = 1;
      static const int IND_N1 = 2;
      static const int IND_N2 = 3;
      static const int IND_VBLOCK_SIZE = 4;
      static const int IND_HBLOCK_SIZE = 5;
      static const int IND_VTRANS_LEN = 6;
      static const int IND_HTRANS_LEN = 7;
      static const int IND_NBYTES_TRACES = 8;
      static const int IND_NBYTES_HDRS = 9;
      static const int IND_FT_GAIN = 10;


      static const int CACHE_SIZE = 32;

  // The first version did not have a cookie.
      static const short COOKIE_V2 = 30607;  // Small enough to fit in a short.
      static const short COOKIE_V3 = 30744;  // Small enough to fit in a short.
      static const short BAD_AMPLITUDE_COOKIE_V2 = 29899;
      static const short BAD_AMPLITUDE_COOKIE_V3 = 29941;

      static const int SIZEOF_INT = 4;
      static const int SIZEOF_FLOAT = 4;
      static const int SIZEOF_SHORT = 2;
      static const int SIZEOF_CHAR = 1;

    protected:

  };

}

#endif



