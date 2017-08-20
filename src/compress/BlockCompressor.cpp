
/***************************************************************************
                           BlockCompressor.cpp  -  description
                             -------------------
 * Transformed block compression using run-length coding and Huffman coding.
 *
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
    copyright            : (C) 2012 Fraunhofer ITWM
 ***************************************************************************/

#include "BlockCompressor.h"
#include "../PSProLogging.h"

namespace jsIO
{
  DECLARE_LOGGER(BlockCompressorLog);
  
  union int_float_bits 
  {
    int int_bits;
    float float_bits;
  };
  
  BlockCompressor::~BlockCompressor()
  {
    delete []idata;
    delete []huffchars;
  }

  BlockCompressor::BlockCompressor()
  {
    JS_BYTEORDER bOrder = nativeOrder();
    if(bOrder == LITTLE_ENDIAN) c_littleEndian = true;
    else c_littleEndian = false;

    idata_len=1024;  // Just an initial guess on the size.
    huffchars_len = 1024*4;// Just an initial guess on the size.

    idata = new int[idata_len];
    huffchars = new char[huffchars_len];

    manualDelta = maxFloat;
    Init(HuffCoder::c_huffCount);
  }

  BlockCompressor::BlockCompressor(const int* huffTable)
  {
    JS_BYTEORDER bOrder = nativeOrder();
    if(bOrder == LITTLE_ENDIAN) c_littleEndian = true;
    else c_littleEndian = false;

    idata_len=1024;  // Just an initial guess on the size.
    huffchars_len = 1024*4;// Just an initial guess on the size.

    idata = new int[idata_len];
    huffchars = new char[huffchars_len];

    manualDelta = maxFloat;
    huffCoder.Init(huffTable);
  }

  void BlockCompressor::Init(const int* huffTable)
  {
    huffCoder.Init(huffTable);
  }


  void BlockCompressor::stuffIntInBytes( int ival, char* bvals, int offset)
  {
//     memcpy((char*)&bvals[offset],(char*)&(ival),sizeof(int)); 
// !TRY :which one is faster , memcpy or this
    bvals[offset]   = (char)(ival >> 24);
    bvals[1+offset] = (char)((ival << 8) >> 24);
    bvals[2+offset] = (char)((ival << 16) >> 24);
    bvals[3+offset] = (char)((ival << 24) >> 24);
  }


  void BlockCompressor::stuffInBytes( int ival, char* bvals, int offset)
  {
//     memcpy((char*)&bvals[offset],(char*)&(ival),sizeof(int)); 
    stuffIntInBytes(ival, bvals, offset);
  }

  void BlockCompressor::stuffInBytes( float fval, char* bvals, int offset)
  {
//     memcpy((char*)&bvals[offset],(char*)&(fval),sizeof(float)); 
    int32_t ival = *(reinterpret_cast<int32_t*>((float*)&fval));
    stuffIntInBytes( ival, bvals, offset);
  }


  int BlockCompressor::unsignedByte( char i )
  {
    if ( i < 0 ) {
      return (int)i + 256;
    } else {
      return (int)i;
    }
  }
		  
  float BlockCompressor::intBitsToFloat(int x)
  {
    union int_float_bits bits;
    bits.int_bits = x;
    return bits.float_bits;
  }

  int BlockCompressor::stuffBytesInInt(const char* bvals, int index) 
  {
    int i0 = ((unsignedByte(bvals[index]))) << 24;
    int i1 = ((unsignedByte(bvals[1+index]))) << 16;
    int i2 = ((unsignedByte(bvals[2+index]))) << 8;
    int i3 = unsignedByte(bvals[3+index]);	
    return i0 + i1 + i2 + i3;
//     return *(reinterpret_cast<int*>( (char*)&bvals[index]));
  }

  void BlockCompressor::stuffShortInBytes( short uval, char* bvals, int index) {
// !TRY :which one is faster , memcpy or this
    int ival = (int)uval;
    bvals[index] = (char)((ival << 16) >> 24);
    bvals[1+index] = (char)((ival << 24) >> 24);	
  }


  int BlockCompressor::stuffBytesInShort(const char* bvals, int index) {
    int i0 = (short)(((int)(bvals[index])) << 8);
    int i1 = (short)unsignedByte(bvals[1+index]);
    return (i0 + i1);
//     return *(reinterpret_cast<short*>( (char*)&bvals[index]));
  }

	
  int BlockCompressor::unsignedShort( short i ) {
    if ( i < 0 ) {
      return (int)i + 65536;
    } else {
      return (int)i;
    }
  }


  /* 
   * Computes the quantization delta.
   *
   * @param  x  transformed samples.
   * @param  n  number of samples.
   * @param  distortion  desired distortion level.
   * @return  quantization delta.
   */
  float BlockCompressor::computeDelta( float* x, int n, float distortion ) {
    if (Transformer::c_integrityTest) {
      return 1.0F;
    }
    double blockVar;
    float delta, quarterDelta, mquarterDelta;
    int i, n2;
    /* Get a low approximation of delta. */
    blockVar = 0.0;
    for ( i=0; i<n; i++ ) blockVar += (double)(x[i]*x[i]);

    blockVar /= (double)n;
    blockVar = sqrt(blockVar);

    delta = distortion * (float)blockVar / CPDF;

    /* Compute delta without the near-zero samples. */
    quarterDelta = delta * 0.25F;
    mquarterDelta = -quarterDelta;
    blockVar = 0.0;
    n2 = 0;
    /* Every 8th sample should be OK for this application. */
    /* Why not use them all - should make very little difference. */
    /* for ( i=0; i<n; i+=8 ) { */
    for ( i=0; i<n; i++ ) {
      if ( x[i] > quarterDelta ) {
        n2++;
        blockVar += (double)(x[i]*x[i]);
      } else if ( x[i] < mquarterDelta ) {
        n2++;
        blockVar += (double)(x[i]*x[i]);
      }
    }

    if ( n2 > 0 ) {
      blockVar /= (double)n2;
      blockVar = sqrt(blockVar);
      delta = distortion * (float)blockVar / CPDF;
    } else {
      delta = 1.0F;
    }

    return delta;
  }


  /* 
   * Applies quantization.
   *
   * @param  transformed samples.
   * @param  number of samples.
   * @param  quantization delta.
   * @param  output quantized samples.
   */
  void BlockCompressor::quantize( float* x, int n, float delta, int* ix ) 
  {
    int i;
    float temp;
    delta = 1.0F / delta;
    for ( i=0; i<n; i++ ) {
      temp = x[i] * delta;
      // Math.round() is dreadfully slow.
      // ix[i] = (int)Math.round( temp );
      // Is is faster to avoid the function call?  Seems like it!
      // ix[i] = (int)nint(temp );
      if (temp > 0.0) {
        ix[i] = (int)(temp + 0.5F);
      } else {
        ix[i] = (int)(temp - 0.5F);
      }
    }

  }


  /* 
   * Applies run length encoding.  There is no provision for the output buffer being
   * too small.
   *
   * @param  quantdata  quantized data (or other integer data).  Must be terminated
   *                    with a non-zero value.
   * @param  n  number of samples.
   * @param  encodedChars  run-length encoded data.
   * @return  number of bytes required to store the encoded data.
   */
  int BlockCompressor::runLengthEncode(int* quantdata, int n, char* encodedChars) 
  {
    int i, nbytes, zrun, nwords, istart;
    short si;
    nwords = n / SIZEOF_INT;
    for (i=nbytes=0; i<nwords;) {
      if ( quantdata[i] == 0 ) {
        /* Begin a run of zeros. */
        istart = i;
        i++;
        /* There is a non-zero dummy value at the end of the buffer. */
        while ( quantdata[i] == 0 ) i++;

        while ( i > istart+65535 ) {
          /* Encode runs of 65535. */
          encodedChars[nbytes++] = 106;
          si = (short)65535;
          stuffShortInBytes(si, encodedChars, nbytes); 
          nbytes += 2;
          istart += 65535;
        }
        zrun = i - istart;
        if (zrun > 255) {
          encodedChars[nbytes] = 106;
          nbytes++;
          si = (short)zrun;
          stuffShortInBytes(si, encodedChars, nbytes);      
          nbytes += 2;
        } else if (zrun > 100) {
          encodedChars[nbytes] = 105;
          nbytes++;
          encodedChars[nbytes] = (char)zrun;
          nbytes++;
        } else {
          encodedChars[nbytes] = (char)zrun;
          nbytes++;
        }
      } else if (quantdata[i] < 75  &&  quantdata[i] > -74) {
        encodedChars[nbytes] = (char)(quantdata[i] + 180);
        nbytes++;
        i++;
      } else {
        /* The data is >= 75 or <= -75. */
        if (quantdata[i] > 0) {
          if (quantdata[i] < 256) {
            encodedChars[nbytes] = 101;
            nbytes++;
            encodedChars[nbytes] = (char)quantdata[i];	
            nbytes++;
          } else if (quantdata[i] < 65536) {
            encodedChars[nbytes] = 103;
            nbytes++;
            si = (short)quantdata[i];
            stuffShortInBytes(si, encodedChars, nbytes);	
            nbytes += 2;
          } else {
            /* It's a huge integer. */
            encodedChars[nbytes] = (char)255;
            nbytes++;
            stuffIntInBytes(quantdata[i], encodedChars, nbytes); 
            nbytes += 4;
          }
        } else {
          /* Less than 0. */
          if (quantdata[i] > -256) {
            encodedChars[nbytes] = 102;
            nbytes++;
            encodedChars[nbytes] = (char)(-quantdata[i]);
            nbytes++;
          } else if (quantdata[i] > -65536) {  
            encodedChars[nbytes] = 104;
            nbytes++;
            si = (short)(-quantdata[i]);
            stuffShortInBytes(si, encodedChars, nbytes);	
            nbytes += 2;
          } else {
            /* It's a huge negative integer. */
            encodedChars[nbytes] = (char)255;
            nbytes++;
            stuffIntInBytes(quantdata[i], encodedChars, nbytes);
            nbytes += 4;
          }
        }
        i++;
      }

    }
	  
    int nInts = nbytes / SIZEOF_INT;
    if ((nInts*SIZEOF_INT) < nbytes) {
      // Be sure that the extra characters contain zeros.
      encodedChars[nbytes] = 0;
      encodedChars[nbytes+1] = 0;
      encodedChars[nbytes+2] = 0;
      nInts++;
    }

    return nbytes;
  }

  /* 
   * Manually sets the delta value for quantization (ignoring the distortion value).
   * This is so that distortion does not vary from frame to frame.
   *
   * @param  delta  the value to use for delta in all cases.
   */
  void BlockCompressor::setDelta(float delta) 
  {
    manualDelta = delta;
  }


  /* 
   * Run-length decoding and dequantization (both steps applied at once for
   * performance gain.  NOTE: the output must not overlay the input.
   *
   * @param  huffchars  Huffman decoded data (needs run-length decoding).
   * @param  nbytes  number of input bytes.
   * @param  delta  quantization delta.
   * @param  quantdata  output decoded and dequantized data.
   */
  void BlockCompressor::runLengthDecodeDequant(char* _huffchars, int nbytes, float delta, float* quantdata) 
  {
    int i, j, iend, ival;
//     short si;
    for ( i=j=0; i<nbytes; ) {
      // Function calls in tight loops are hurting performance.
      // int ihuffchar = unsignedByte(huffchars[i]);
      int ihuffchar = (int)_huffchars[i];
      if (ihuffchar < 0) ihuffchar += 256;

      if ( ihuffchar > 0  &&  ihuffchar < 101 ) {
        iend = j + ihuffchar;
        for ( ; j<iend; j++ ) quantdata[j] = 0.0F;
        i++;
      } else if ( ihuffchar == 105 ) {
        i++;
        // ival = unsignedByte(huffchars[i]);
        ival = (int)_huffchars[i];
        if (ival < 0) ival += 256;
        iend = j + ival;
        for ( ; j<iend; j++ ) quantdata[j] = 0.0F;
        i++;
      } else if ( ihuffchar > 106  &&  ihuffchar < 255 ) {
        // quantdata[j++] = ((float)unsignedByte(huffchars[i++]) - 180.0F) * delta;
        quantdata[j++] = ((float)ihuffchar - 180.0F) * delta;
        i++;
      } else {
        if ( _huffchars[i] == 101 ) {
          i++;
          // ival = unsignedByte(huffchars[i]);
          ival = (int)_huffchars[i];
          if (ival < 0) ival += 256;
          quantdata[j++] = (float)ival * delta;
          i++;
        } else if ( _huffchars[i] == 102 ) {
          i++;
          // ival = unsignedByte(huffchars[i]);
          ival = (int)_huffchars[i];
          if (ival < 0) ival += 256;
          quantdata[j++] = (-(float)ival) * delta;
          i++;
        } else if ( _huffchars[i] == 103 ) {
          i++;
          // ival = unsignedShort(stuffBytesInShort(huffchars, i));
          ival = stuffBytesInShort(_huffchars, i);
          if (ival < 0) ival += 65536;
          quantdata[j++] = (float)ival * delta;
          i += 2;
        } else if ( _huffchars[i] == 104 ) {
          i++;
          // ival = unsignedShort(stuffBytesInShort(huffchars, i));
          ival = stuffBytesInShort(_huffchars, i);
          if (ival < 0) ival += 65536;
          quantdata[j++] = (-(float)ival) * delta;
          i += 2;
        } else if ( _huffchars[i] == 106 ) {
          i++;
          // ival = unsignedShort(stuffBytesInShort(huffchars, i));
          ival = stuffBytesInShort(_huffchars, i);
          if (ival < 0) ival += 65536;
          iend = j + ival;
          for ( ; j<iend; j++ ) quantdata[j] = 0.0F;
          i += 2;
        } else if ( ihuffchar == 255 ) {
          i++;
          ival = stuffBytesInInt(_huffchars, i);
          quantdata[j++] = ((float)ival) * delta;
          i += 4;
        } else {
          TRACE_PRINTF(BlockCompressorLog, "Warning: HuffTableDecode: bad character encountered at element %d",(int)_huffchars[i]);
          // Just punt - don't know what else to do.  This may actually happen,
          // because data gets corrupted.
          // TODO: Perhaps we should zero the data.
          return;
        }
      }
    }
    return;
  }


  /* 
   * Quantizes and encodes a body of data.
   *
   * @param  data  transformed sample values.
   * @param  nsamps  number of sample values.
   * @param  distortion  the distortion level.
   * @param  encodedData  output encoded data.
   * @param  index  starting index in the output encoded data.
   * @param  outputBufferSize  the size of the output buffer.
   * @return  number of output bytes, or 0 if the output buffer is too small.
   */
  int BlockCompressor::dataEncode(float* data, int nsamps, float distortion, char* encodedData, int index, int outputBufferSize) 
  {
    int i;
    int nbytesHdrExpected = SIZEOF_INT * 2;
    
    if (outputBufferSize < nbytesHdrExpected+1 ) return 0;

    float delta;
    if ( ISNOTZERO(manualDelta - maxFloat)) {
      // Delta was set manually.
      delta = manualDelta;
    } else {
      delta = computeDelta( data, nsamps, distortion );
    }

    /// Need extra samples to set nNonZero.
    if(idata_len<nsamps+2){
      delete[]idata;
      idata_len = nsamps+2;
      idata = new int[idata_len];
    }
//     while (idata.length < nsamps+2){
//       idata = new int[_idata.length*2];
//     }
 
    quantize(data, nsamps, delta, idata);

    for ( i=nsamps-1; i>=0; i-- ) if(idata[i] != 0) break;
    int nNonZero = i + 1;
    // This must be a multiple of 2.
    if ( (nNonZero >> 1) << 1 < nNonZero ) nNonZero++;
   
    /* Load delta. */
    // int idelta = (int)delta;
//     int idelta = Float.floatToIntBits(delta);
//     stuffIntInBytes(idelta, encodedData, index);
    stuffInBytes(delta, encodedData, index);

    int nbytesHdr = SIZEOF_INT;
    index += SIZEOF_INT;
	  
    /* Load the number of non-zero values. */
    stuffIntInBytes(nNonZero, encodedData, index);
    nbytesHdr += SIZEOF_INT;
    index += SIZEOF_INT;
	  
    /* Run-length encode. */
    /* The input must be terminated with a non-zero value first. */
    idata[nNonZero] = 1;
    /* Note that we don't even try to encode the zeroed part of the data. */
    int nbytes = nNonZero * SIZEOF_INT;

    // We need extra space because compression may actually be negative.
    if(huffchars_len<nbytes*2){
      delete[]huffchars;
      huffchars_len = nbytes*2;
      huffchars = new char[huffchars_len];
    }
//     while (_huffchars.length < nbytes*2)
//       _huffchars = new byte[_huffchars.length*2];


    nbytes = runLengthEncode(idata, nbytes, huffchars);

    /* Sanity check. */
    if (nbytesHdr != nbytesHdrExpected){
      if(nbytes != 0){
        ERROR_PRINTF(BlockCompressorLog, "Error: header mismatch!");
        return JS_USERERROR;
      }  
    } 

    /* Huffman encode. */
    nbytes = huffCoder.huffEncode(huffchars, nbytes, encodedData, index,
                                  outputBufferSize-nbytesHdr );	 

    if (nbytes == 0) return 0;  /* Overflow! */

    return nbytes + nbytesHdr;
  }


  /* 
   * Decodes and dequantizes a body of data
   *
   * @param  encodedData  encoded byte data.
   * @param  index  index into the encoded data.
   * @param  workBuffer  work buffer.
   * @param  workBufferSize  size of the work buffer.
   * @param  nsamps  number of output samples.
   * @param  data  output decoded data.
   */
  int BlockCompressor::dataDecode(const char* encodedData, int index, char* workBuffer,
                                  int workBufferSize, int nsamps, float* data) 
  {
    
    /* Unload delta. */
    float delta;
    int idelta = stuffBytesInInt(encodedData, index);
    delta = intBitsToFloat(idelta);
//     delta = stuffBytesInFloat(encodedData, index);

    index += SIZEOF_FLOAT;
	
    /* Unload the number of non-zero samples. */
    int nNonZero = stuffBytesInInt(encodedData, index);
    index += SIZEOF_INT;
		
    /* Huffman decode. */
    int nbytes = huffCoder.huffDecode(encodedData, index, workBuffer, workBufferSize);
		
    if (nbytes == -1) return JS_WARNING;

    /* Run-length decode and dequantize. */
    runLengthDecodeDequant(workBuffer, nbytes, delta, data);
	  
    /* Fill in the zeros. */
    /* We know that nNonZero is a multiple of 2. */
    for (int i=0; i<nsamps-nNonZero; i++) data[i+nNonZero] = 0.0F;

    return JS_OK;
  }


}



