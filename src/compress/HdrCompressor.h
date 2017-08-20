
/***************************************************************************
                           HdrCompressor.h  -  description
                             -------------------
 * This class compresses trace headers.  It does so by transposing the headers
 * and then run-length encoding them.  It encodes runs of constant length or
 * ascending or descending values.

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

#ifndef  HDRCOMPRESSOR_H
#define  HDRCOMPRESSOR_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits>

class IntBuffer;

namespace jsIO
{
  class  HdrCompressor{
    public:
      ~ HdrCompressor();
      HdrCompressor();

      static int getOutputBufferSize(int maxHdrLength, int maxFrameSize);

      int compress(int* hdrs, int hdrLength, float* traces, int nTraces, int numSamples, char* encodedBytes, int offset);
      int compress(IntBuffer *hdrIntBuffer, int hdrLength, float* traces, int nTraces, int numSamples, char* encodedBytes, int offset);

      int uncompress(const char* encodedBytes, int offset, int nBytes, int* hdrs, int hdrLength, 
                     float* traces, int nTraces, int numSamples);
      int uncompress(const char* encodedBytes, int offset, int nBytes, IntBuffer *hdrIntBuffer, int hdrLength, 
                     float* traces, int nTraces,  int numSamples);

      int runLengthEncode(int* inValues, long inValues_len, int runSymbolConst, int runSymbolAscend,
                          int runSymbolDescend, int runSymbolDelta, int runSymbolFloats,
                          int endOfData, int* encodedValues);

      int runLengthDecode(int* encodedValues, int* outValues);



    public:

      static const int HDR_LENGTH=10;

// private atributes
    private:
      static int getFirstNonZero(float* trace, int numSamples);
      static void applyRemute(float* trace, int numSamples, int indexFirstNonZero);

      void getUniqueValues(int* inValues, int nInput, int* uniqueValues);
      void getCandidateUniqueValues(int *uniqueValues);
      static bool probablyFloat(int iVal);
    
//     int zip(char *pSource, unsigned long source_len, char* pDest, int offset);
//     int unzip(char *pSource, int offset, unsigned long source_len, char *pDest);
      int zip  (int *encodedValues, int nValues, char* zipInput, char *zipOutput, int offset);
      int unzip(const char *unzipInput, int offset, unsigned long nBytesInput, char *unzipOutput, int *encodedValues);


      int ensureHdrBuffers(int* hdrs, int hdrs_len1, int hdrs_len2, IntBuffer *hdrIntBuffer, int hdrLength);
      void ensureZipBuffers(int* hdrs, int hdrs_len1, int hdrs_len2, IntBuffer *hdrIntBuffer);

      int private_compress(int* hdrs, IntBuffer *hdrIntBuffer, int hdrLength, float* traces,
                           int nTraces, int numSamples, char* encodedBytes, int offset);

      int private_uncompress(const char* encodedBytes, int offset, int nBytes, int* hdrs, IntBuffer *hdrIntBuffer, int hdrLength, 
                             float* traces, int nTraces, int numSamples);

      void HdrCompressorgetUniqueValues(int* inValues, int nInput, int* uniqueValues);
 

    private:
      bool c_littleEndian;

      static const int SIZEOF_INT = 4;
      static const int IND_COOKIE = 0;
      static const int IND_SYM_CONST = 1;        // Run of constant values.
      static const int IND_SYM_ASCEND = 2;       // Run of mono ascending values.
      static const int IND_SYM_DESCEND = 3;      // Run of mono descending values.
      static const int IND_SYM_DELTA = 4;        // Run of delta values.
      static const int IND_SYM_FLOATS = 5;       // Run of float delta values.
      static const int IND_OUT_COUNT = 6;
      static const int IND_HDR_LENGTH= 7;
      static const int IND_NTRACES = 8;
      static const int IND_REMUTE = 9;
      static const int OLD_COOKIE1 = 6821923;
      static const int COOKIE      = 1215649;

  // This is the approximate minimum value that an int will have if it contains positive float bits.
      static const int MIN_POS_FLOAT_BITS =  700000000;

  // This is the approximate maximum value that an int will have if it contains negative float bits.
      static const int MAX_NEG_FLOAT_BITS = -700000000;

  // Compression level of 6 in java.util.zip seems to give compression ratios near the max and
  // it isn't as slow as MAX_COMPRESSION (9).
      static const int BEST_ZIP_LEVEL = 6;

      int* m_piC_candidateValues;
      int* m_piTransposedHdrs;
      int* m_piRunLengthEncodedValues;
      char *m_pcZipWorkBuffer;

      int* m_piSingleHdrWork;
      int* m_piUniqueValues;
      int m_transposedHdrs_length;
      int m_runLengthEncodedValues_length;
      int m_zipWorkBuffer_length;
      int m_singleHdrWork_length;
  };
}



#endif



