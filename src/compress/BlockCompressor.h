/***************************************************************************
                           BlockCompressor.h  -  description
                             -------------------
 * Transformed block compression using run-length coding and Huffman coding.
 
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

#ifndef  BLOCKCOMPRESSOR_H
#define  BLOCKCOMPRESSOR_H

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../jsByteOrder.h"
#include "Transformer.h"
#include "HuffCoder.h"
#include "../jsDefs.h"

namespace jsIO
{
  class  BlockCompressor{
    public:
      ~ BlockCompressor();
      BlockCompressor();
      BlockCompressor(const int* huffTable);

      void Init(const int* huffTable);

      static int runLengthEncode(int* quantdata, int n, char* encodedChars);
      void setDelta(float delta);
     
      int dataEncode(float* data, int nsamps, float distortion, char* encodedData, int index, int outputBufferSize);
      int dataDecode(const char* encodedData, int index, char* workBuffer, int workBufferSize, int nsamps, float* data);

      static void stuffIntInBytes( int ival, char* bvals, int offset);
      static int stuffBytesInInt(const char* bvals, int index);
//    static float stuffBytesInFloat(char* bvals, int index);
      static int stuffBytesInShort(const char* bvals, int index);
      static void stuffShortInBytes( short uval, char* bvals, int index);
      static void stuffInBytes( int ival, char* bvals, int offset);
      static void stuffInBytes( float fval, char* bvals, int offset);
      static float intBitsToFloat(int x);

    public:



// private atributes
    private:

      static int nint(double a) {return (a > 0.0) ? (int)(a+0.5F) : (int)(a-0.5F);} // Faster than Math.round().
      static int unsignedByte( char i );
      static int unsignedShort( short i );
      static float computeDelta( float* x, int n, float distortion );
      static void quantize( float* x, int n, float delta, int* ix );
      static void runLengthDecodeDequant(char* huffchars, int nbytes, float delta, float* quantdata);
    private:
      bool c_littleEndian;
      static const int SIZEOF_INT = 4;
      static const int SIZEOF_CHAR = 1;
      static const int SIZEOF_FLOAT = 4;
      static const float CPDF = 0.26F;               /* quantization factor */
      static const float maxFloat = 1e+038;

      HuffCoder huffCoder;
      int* idata; 
      char* huffchars;
      int idata_len;
      int huffchars_len;
      float manualDelta;

  };
}



#endif



