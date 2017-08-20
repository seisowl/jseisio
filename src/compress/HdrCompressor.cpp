
/***************************************************************************
                           HdrCompressor.cpp  -  description
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

#include "../PSProLogging.h"
#include "BlockCompressor.h"
#include "../IntBuffer.h"
#include "miniz.h"

#include "HdrCompressor.h"


 
namespace jsIO
{
  DECLARE_LOGGER(HdrCompressorLog);


  HdrCompressor::~HdrCompressor(){
    delete[] m_piC_candidateValues;
    delete[] m_piUniqueValues;
    if(m_piTransposedHdrs != NULL) delete[]m_piTransposedHdrs;
    if(m_piRunLengthEncodedValues != NULL) delete[]m_piRunLengthEncodedValues;
    if(m_pcZipWorkBuffer != NULL) delete[]m_pcZipWorkBuffer;
    if(m_piSingleHdrWork != NULL) delete[]m_piSingleHdrWork;
  }

  HdrCompressor::HdrCompressor(){
    JS_BYTEORDER bOrder = nativeOrder();
    if(bOrder == LITTLE_ENDIAN) c_littleEndian = true;
    else c_littleEndian = false;

    m_piC_candidateValues = new int[256];
    getCandidateUniqueValues(m_piC_candidateValues);
    m_piUniqueValues = new int[5];

    m_piTransposedHdrs = NULL;
    m_piRunLengthEncodedValues = NULL;
    m_pcZipWorkBuffer = NULL;
    m_piSingleHdrWork = NULL;
    m_transposedHdrs_length = 0;
    m_runLengthEncodedValues_length = 0;
    m_zipWorkBuffer_length = 0;
    m_singleHdrWork_length = 0;


  }


  /* 
   * Returns the appropriate number of bytes for the output buffer for compressed
   * trace headers.
   *
   * @param  maxHdrLength  the maximum length of any header (number of ints).
   * @param  maxFrameSize  the maximum number of trace headers per frame/ensemble.
   * @return  the appropriate number of bytes for the output buffer for compressed
   *          trace headers.
   */
  int HdrCompressor::getOutputBufferSize(int maxHdrLength, int maxFrameSize) 
  {
    if (maxHdrLength < 1  ||  maxFrameSize < 1){
      ERROR_PRINTF(HdrCompressorLog,"Header length of headers per frame is invalid");
      return -1;
    }

    // We add 1 to each header to ensure space for the remute index.
    return ((maxHdrLength+1) * maxFrameSize + HDR_LENGTH) * SIZEOF_INT;
  }

  /* 
   * Ensures that the internal header work buffers are created and of appropriate size.
   *
   * @param  hdrs some trace headers , or null if hdrIntBuffer is used.- 2dim array saved as a 1d
   * @param  hdrs_len1 length of hdrs's first(fastest) dimenstion
   * @param  hdrs_len2 length of hdrs's second dimenstion  
   * @param  hdrIntBuffer  an IntBuffer of headers, or null if hdrs is used.
   * @param  hdrLength  the length of each header (may be less than the array length).
   */
  int HdrCompressor::ensureHdrBuffers(int* hdrs, int hdrs_len1, int hdrs_len2, IntBuffer *hdrIntBuffer, int hdrLength) 
  {
    if (hdrLength < 1){
      ERROR_PRINTF(HdrCompressorLog,"Header length is nonsensical");
      return JS_USERERROR;
    }

    int lenHdr0 = 0; 
    int nIntsMin;
    // We add 1 to each header to ensure space for the remute index.
    if (hdrs != NULL) {
//        lenHdr0 = hdrs[0].length;
//        nIntsMin = hdrs.length * (hdrs[0].length+1) + HDR_LENGTH;
      lenHdr0 = hdrs_len1;
      nIntsMin = hdrs_len2 * (hdrs_len1+1) + HDR_LENGTH;
    } else {
      int nTraces = hdrIntBuffer->capacity() / hdrLength;
      nIntsMin = hdrIntBuffer->capacity() + HDR_LENGTH + nTraces;
    }

    nIntsMin += 1024;  // Throw in some extra for good measure.

    int lenHdr = std::max(lenHdr0, hdrLength);
    lenHdr += 128;  // Throw in some extra for good measure.

    if (m_piSingleHdrWork == NULL  ||  m_singleHdrWork_length < lenHdr){
      if(m_singleHdrWork_length>0) delete[]m_piSingleHdrWork;
      m_piSingleHdrWork = new int[lenHdr];
      m_singleHdrWork_length = lenHdr;
    }   

    if (m_piTransposedHdrs == NULL  ||  m_transposedHdrs_length < nIntsMin){
      if(m_transposedHdrs_length>0) delete[]m_piTransposedHdrs;
      m_piTransposedHdrs = new int[nIntsMin];
      m_transposedHdrs_length = nIntsMin;
    }  

    return JS_OK;
  }


  /* 
   * Ensures that the internal Zip work buffers are created and of appropriate size.
   *
   * @param  hdrs some trace headers , or null if hdrIntBuffer is used.- 2dim array saved as a 1d
   * @param  hdrs_len1 length of hdrs's first(fastest) dimenstion
   * @param  hdrs_len2 length of hdrs's second dimenstion  
   * @param  hdrIntBuffer  an IntBuffer of headers, or null if hdrs is used.
   */
  void HdrCompressor::ensureZipBuffers(int* hdrs, int hdrs_len1, int hdrs_len2,  IntBuffer *hdrIntBuffer) 
  {
    int nIntsMin;
    // We add 1 to each header to ensure space for the remute index.
    if (hdrs != NULL) {
//       nIntsMin = hdrs.length * (hdrs[0].length+1) + HDR_LENGTH;
      nIntsMin = hdrs_len2 * (hdrs_len1+1) + HDR_LENGTH;
    } else {
      // We have no choice but to double the capacity, because we don't
      // know how many traces there really are.
      nIntsMin = hdrIntBuffer->capacity()*2 + HDR_LENGTH;
    }

    nIntsMin += 1024;  // Throw in some extra for good measure.

    if (m_piRunLengthEncodedValues == NULL  ||  m_runLengthEncodedValues_length < nIntsMin){
      if(m_runLengthEncodedValues_length>0) delete[]m_piRunLengthEncodedValues;
      m_piRunLengthEncodedValues = new int[nIntsMin];
      m_runLengthEncodedValues_length = nIntsMin;
    } 

    if (m_pcZipWorkBuffer == NULL  ||  m_zipWorkBuffer_length < nIntsMin*SIZEOF_INT){
      if(m_zipWorkBuffer_length>0) delete[]m_pcZipWorkBuffer; 
      m_pcZipWorkBuffer = new char[nIntsMin*SIZEOF_INT];
      m_zipWorkBuffer_length = nIntsMin*SIZEOF_INT;
    }

  }


  /**
   * Compresses a 2D array of trace headers.
   *
   * @param  hdrs  the trace headers.
   * @param  hdrLength  the length of each header (may be less than the array length).
   * @param  traces  the trace samples used to determine the mute/remute, or null if no
   *                 mute/remute is desired.
   * @param  nTraces  the number of live trace headers.
   * @param  encodedBytes  the output compressed headers.  To determine the necessary
   *                        size use getOutputBufferSize().
   * @param  offset  the byte offset to begin in the output compressed headers.
   * @return  the number of bytes used to encode the data.
   */
  int HdrCompressor::compress(int* hdrs, int hdrLength, float* traces, int nTraces, int numSamples,
                              char* encodedBytes, int offset) 
  {
    return private_compress(hdrs, NULL, hdrLength, traces, nTraces, numSamples, encodedBytes, offset);
  }


  /**
   * Compresses a 2D array of trace headers.
   *
   * @param  hdrIntBuffer  an IntBuffer that contains the trace headers.
   * @param  hdrLength  the length of each header.
   * @param  traces  the trace samples used to determine the mute/remute, or null if no
   *                 mute/remute is desired.
   * @param  nTraces  the number of live trace headers.
   * @param  encodedBytes  the output compressed headers.  To determine the necessary
   *                        size use getOutputBufferSize().
   * @param  offset  the byte offset to begin in the output compressed headers.
   * @return  the number of bytes used to encode the data.
   */
  int HdrCompressor::compress(IntBuffer* hdrIntBuffer, int hdrLength, float* traces, int nTraces, int numSamples,
                              char* encodedBytes, int offset)
  {
    return private_compress(NULL, hdrIntBuffer, hdrLength, traces, nTraces, numSamples, encodedBytes, offset);
  }


  /**
   * Compresses a 2D array of trace headers.
   *
   * @param  hdrs  the trace headers, or null if hdrIntBuffer is used.
   * @param  hdrIntBuffer  an IntBuffer that contains the trace headers, or null if hdrs is used.
   * @param  hdrLength  the length of each header.
   * @param  traces  the trace samples used to determine the mute/remute, or null if no
   *                 mute/remute is desired.
   * @param  nTraces  the number of live trace headers.
   * @param  encodedBytes  the output compressed headers.  To determine the necessary
   *                        size use getOutputBufferSize().
   * @param  offset  the byte offset to begin in the output compressed headers.
   * @return  the number of bytes used to encode the data.
   */
  int HdrCompressor::private_compress(int* hdrs, IntBuffer* hdrIntBuffer, int hdrLength,
                                      float* traces, int nTraces, int numSamples, char* encodedBytes, int offset) 
  {
    ensureZipBuffers(hdrs, hdrLength, nTraces, hdrIntBuffer);
    ensureHdrBuffers(hdrs, hdrLength, nTraces, hdrIntBuffer, hdrLength);


    // Transpose the header values as we copy them into place.
    int count = 0;
    if (hdrs != NULL) {
      for (int i=0; i<hdrLength; i++) {
        for (int j=0; j<nTraces; j++) {
          m_piTransposedHdrs[count] = hdrs[j*hdrLength+i];
          count++;
        }
      }
    } else {
      for (int j=0; j<nTraces; j++) {
        hdrIntBuffer->position(j * hdrLength);
        hdrIntBuffer->get(m_piSingleHdrWork, hdrLength);
        for (int i=0; i<hdrLength; i++) {
          int dest = j + i * nTraces;
          m_piTransposedHdrs[dest] = m_piSingleHdrWork[i];
        }
      }
      count = hdrLength * nTraces;
    }

    if (traces != NULL) {
      // Save the index of the first non-zero trace for remuting.
      for (int j=0; j<nTraces; j++) {
        m_piTransposedHdrs[count] = getFirstNonZero(&traces[j*numSamples], numSamples);
        count++;
      }
      m_piRunLengthEncodedValues[IND_REMUTE] = 1;
    } else {
      m_piRunLengthEncodedValues[IND_REMUTE] = 0;
    }

    // We get 3 values that don't occur in the data.  We use these values
    // as flags in the data.
    getUniqueValues(m_piTransposedHdrs, count, m_piUniqueValues);
    int endOfData = m_piUniqueValues[0];
    int runSymbolConst = m_piUniqueValues[0];  // We can use the first value for two purposes.
    int runSymbolAscend = m_piUniqueValues[1];
    int runSymbolDescend = m_piUniqueValues[2];
    int runSymbolDelta = m_piUniqueValues[3];
    int runSymbolFloats = m_piUniqueValues[4];

    m_piTransposedHdrs[count] = endOfData;  // This is why we add 1 (or more) to the length.

    m_piRunLengthEncodedValues[IND_HDR_LENGTH] = hdrLength;
    m_piRunLengthEncodedValues[IND_NTRACES] = nTraces;

    // Run-length encode the header values.
    int nInts = runLengthEncode(m_piTransposedHdrs, count, runSymbolConst, runSymbolAscend,
                                runSymbolDescend, runSymbolDelta, runSymbolFloats,
                                endOfData, m_piRunLengthEncodedValues);

    // Zip the run-length encoded values.
    // Tests show that java.util.zip coding is more effective on trace headers than
    // HuffCoder alone is, although java.util.zip is quite a bit slower than HuffCoder.
    return zip(m_piRunLengthEncodedValues, nInts, m_pcZipWorkBuffer, encodedBytes, offset);
//     return zip((char *)m_piRunLengthEncodedValues, nInts*SIZEOF_INT, encodedBytes, offset);
  }


  /**
   * Returns the index of the first non-zero trace sample.
   *
   * @param  trace  a seismic trace.
   * @return  the index of the first non-zero trace sample.
   */
  int HdrCompressor::getFirstNonZero(float* trace, int numSamples)
  {
    for (int i=0; i<numSamples; i++) {
      if (ISNOTZERO(trace[i])) return i;
    }
    return numSamples;  // All zeros.
  }


  /**
   * Applies a remute to a seismictrace.
   *
   * @param  trace  a seismic trace.
   * @param  indexFirstNonZero  the index of the first non-zero sample.
   */
  void HdrCompressor::applyRemute(float* trace, int numSamples, int indexFirstNonZero) 
  {
    for (int i=0; i<std::min(indexFirstNonZero,numSamples); i++) trace[i] = 0.0F;
  }

 
  /**
   * Uncompresses a 2D array of trace headers.
   *
   * @param  encodedBytes  the input compressed headers (from the compress() method).
   * @param  offset  the byte offset to begin in the input compressed headers.
   * @param  nBytes  the number of input compressed bytes.
   * @param  hdrs  the output trace headers.
   * @param  traces  the trace samples apply remute, or null if no remute is desired.
   * @return  the number of live output trace headers.
   */
  int HdrCompressor::uncompress(const char* encodedBytes, int offset, int nBytes,
                                int* hdrs, int hdrLength, float* traces, int nTraces, int numSamples)
  {
    return private_uncompress(encodedBytes, offset, nBytes,
                              hdrs, NULL, hdrLength, traces, nTraces, numSamples);
  }
  
  
    


  /*
  * Uncompresses a 2D array of trace headers.
  *
  * @param  encodedBytes  the input compressed headers (from the compress() method).
  * @param  offset  the byte offset to begin in the input compressed headers.
  * @param  nBytes  the number of input compressed bytes.
  * @param  hdrIntBuffer  the output trace headers.
  * @param  traces  the trace samples apply remute, or null if no remute is desired.
  * @return  the number of live output trace headers.
  */
  int HdrCompressor::uncompress(const char* encodedBytes, int offset, int nBytes,
                                IntBuffer *hdrIntBuffer, int hdrLength, float* traces, int nTraces, int numSamples)
  {
    return private_uncompress(encodedBytes, offset, nBytes,
                              NULL, hdrIntBuffer,  hdrLength, traces, nTraces, numSamples);
  }


  /* 
   * Uncompresses a 2D array of trace headers.
   *
   * @param  encodedBytes  the input compressed headers (from the compress() method).
   * @param  offset  the byte offset to begin in the input compressed headers.
   * @param  nBytes  the number of input compressed bytes.
   * @param  hdrs  the output trace headers, or null if hdrIntBuffer is used.
   * @param  hdrIntBuffer  the output compressed headers, or null if hdrs is used.
   * @param  traces  the trace samples apply remute, or null if no remute is desired.
   * @return  the number of live output trace headers.
   */

  int HdrCompressor::private_uncompress(const char* encodedBytes, int offset, int nBytes,
                                        int* hdrs, IntBuffer *hdrIntBuffer, int _hdrLength, float* traces, int _nTraces, int numSamples)
  {
    ensureZipBuffers(hdrs, _hdrLength, _nTraces, hdrIntBuffer);
    int ires = unzip(encodedBytes, offset, (unsigned long)nBytes, m_pcZipWorkBuffer, m_piRunLengthEncodedValues);
    if(ires==JS_USERERROR){
      ERROR_PRINTF(HdrCompressorLog,"Failed to unzip header");
      return JS_USERERROR;
    }

    if (m_piRunLengthEncodedValues[IND_COOKIE] != COOKIE){
      ERROR_PRINTF(HdrCompressorLog,"Compressed data is corrupted or from an unsupported version");
      return JS_USERERROR;
    } 
    int hdrLength = m_piRunLengthEncodedValues[IND_HDR_LENGTH];
    int nTraces = m_piRunLengthEncodedValues[IND_NTRACES];
    int iRemute = m_piRunLengthEncodedValues[IND_REMUTE];

    if (hdrLength < 1  ||  nTraces < 1){
      ERROR_PRINTF(HdrCompressorLog,"Header length and/or trace count is invalid");
      return JS_USERERROR;
    } 
    if (iRemute != 0  &&  iRemute != 1){
      ERROR_PRINTF(HdrCompressorLog,"Remute flag is invalid");
      return JS_USERERROR;
    }
    ensureHdrBuffers(hdrs, hdrLength, nTraces, hdrIntBuffer, hdrLength);

    // Decode the header values.
    runLengthDecode(m_piRunLengthEncodedValues, m_piTransposedHdrs);

    // Transpose the header values back as we copy them into place.
    int count = 0;
    if (hdrs != NULL) {
      count = 0;
      for (int i=0; i<hdrLength; i++) {
        for (int j=0; j<nTraces; j++) {
          hdrs[j*hdrLength+i] = m_piTransposedHdrs[count];
          count++;
        }
      }
    } else {
      for (int j=0; j<nTraces; j++) {
        for (int i=0; i<hdrLength; i++) {
          int src = j + i * nTraces;
          m_piSingleHdrWork[i] = m_piTransposedHdrs[src];
        }
        hdrIntBuffer->position(j * hdrLength);
        hdrIntBuffer->put(m_piSingleHdrWork, hdrLength);
      }
      count = hdrLength * nTraces;
    }

    if (traces != NULL  &&  iRemute == 1) {
      // Apply the remute.
      for (int j=0; j<nTraces; j++) {
        applyRemute(&traces[j*numSamples],numSamples,  m_piTransposedHdrs[count]);
        count++;
      }
    }

    return nTraces;
  }


  /**
   * Finds unique values that do not occur in the input data.  This algorithm depends on the
   * fact that the data has fewer values than the range of ints.
   *
   * @param  inValues  input values.
   * @param  nInput  number of input values.
   * @param  uniqueValues  output unique values.
   */
  void HdrCompressor::getUniqueValues(int* inValues, int nInput, int* uniqueValues) 
  {
    int outCount = 0;
    int candidateCount = 0;

    while (true) {
      int candidateValue;
      if (candidateCount < 256/*m_piC_candidateValues.length*/) {
        // Try one of the specially computed values.
        candidateValue = m_piC_candidateValues[candidateCount];
      } else {
        // First candidate is just a big negative integer.
        candidateValue = std::numeric_limits<int>::min() + candidateCount;
      }
      candidateCount++;

      int n = 0;
      for (n=0; n<nInput; n++) {
        if (inValues[n] == candidateValue) {
          // Oops.  This value occurs in the data.
          break;
        }
      }

      if (n == nInput) {
        // The candidate value was not found in the data.
        uniqueValues[outCount] = candidateValue;
        outCount++;
        if (outCount == 5/*uniqueValues.length*/) return;  // Got 'em all.
      }
    }

  }


  // Returns candidates for unique values that have a lot of zeros in them.
  void HdrCompressor::getCandidateUniqueValues(int *uniqueValues) 
  {
    char bVals[4];
    bVals[1] = 0;
    bVals[2] = 0;
    bVals[3] = 0;
    //int[] uniqueValues = new int[256]; 
    //len of uniqueValues is 256
    int lenOfuniqueValues = 256;

    int count = 0;

    // Make some special ones that will compress well in a Huffman encoder.
    // These are values that occur a lot in byte data that is presented to the
    // Huffman coder.

    bVals[0] = (char)65;
    uniqueValues[count] = *(reinterpret_cast<int*>( (char*)bVals));
    count++;

    bVals[0] = (char)1;
    uniqueValues[count] = *(reinterpret_cast<int*>( (char*)bVals));
    count++;

    bVals[0] = (char)74;
    uniqueValues[count] = *(reinterpret_cast<int*>( (char*)bVals));//BlockCompressor::stuffBytesInInt(bVals, 0);
    count++;

    bVals[0] = (char)68;
    uniqueValues[count] = *(reinterpret_cast<int*>( (char*)bVals));
    count++;

    for (int i=-128; i<=127; i++) {
      if (count < lenOfuniqueValues) {
        bVals[0] = (char)i;
        uniqueValues[count] = *(reinterpret_cast<int*>( (char*)bVals));
      }
    }
  }


  /* 
  * Returns true if the input integer value is probably a non-zero float, otherwise false.
  *
  * @param  iVal  an integer value.
  * @return  true if the input integer value is probably a non-zero float, otherwise false.
  */
  bool HdrCompressor::probablyFloat(int iVal) 
  {
    void *piVal= &iVal; // to supress gcc warning "to dereferencing type-punned pointer will break strict-aliasing rules"
    if (iVal > MIN_POS_FLOAT_BITS  ||  iVal < MAX_NEG_FLOAT_BITS) {
//       if (isnan(*(reinterpret_cast<float*>( (int*)&iVal)))) {
      if (isnan(*(reinterpret_cast<float*>((int*)piVal)))) {
        return false;
      } else {
        return true;
      }
    } else {
      return false;
    }
  }


  /* 
  * Run-length encodes an array of integers.
  *
  * @param  inValues  input array.  Must be terminated by the endOfData value.
  * @param  runSymbolConst  value used to represent constant runs.  Must not occur in the data.
  * @param  runSymbolAscend  value used to represent ascending runs.  Must not occur in the data.
  * @param  runSymbolDescend  value used to represent descending runs.  Must not occur in the data.
  * @param  runSymbolDelta  value used to represent non-unity delta runs.  Must not occur in the data.
  * @param  runSymbolFloats  value used to represent floats delta runs.  Must not occur in the data.
  * @param  endOfData  value used to terminate the input.  Must not occur in the data.
  * @param  encodedValues  output encoded values.  Must be large enough to hold the input plus header.
  * @return  the number of output values.
  */
  // This algorithm will never grow the size, except the header.
  int HdrCompressor::runLengthEncode(int* inValues,  long inValues_len, int runSymbolConst, int runSymbolAscend,
                                     int runSymbolDescend, int runSymbolDelta, int runSymbolFloats,
                                     int endOfData, int* encodedValues) 
  {
    
    if (inValues_len < 1) return 0;
    std::string strerror="";

    if (runSymbolConst == runSymbolAscend)
      strerror="IllegalArgumentException: runSymbolConst == runSymbolAscend";
    else if (runSymbolConst == runSymbolDescend)
      strerror="IllegalArgumentException: runSymbolConst == runSymbolDescend";
    else if (runSymbolConst == runSymbolDelta)
      strerror="IllegalArgumentException: runSymbolConst == runSymbolDelta";
    else if (runSymbolConst == runSymbolFloats)
      strerror="IllegalArgumentException: runSymbolConst == runSymbolFloats";

    if(strerror!=""){
      ERROR_PRINTF(HdrCompressorLog,strerror.c_str());
      return JS_USERERROR;
    }
    
    // Store the header.
    // TODO: Store the dimensions of the input data?
    encodedValues[IND_COOKIE] = COOKIE;
    encodedValues[IND_SYM_CONST] = runSymbolConst;
    encodedValues[IND_SYM_ASCEND] = runSymbolAscend;
    encodedValues[IND_SYM_DESCEND] = runSymbolDescend;
    encodedValues[IND_SYM_DELTA] = runSymbolDelta;
    encodedValues[IND_SYM_FLOATS] = runSymbolFloats;
    int outCount = HDR_LENGTH;

    int inCount = 0;

    while (true) {
      int firstVal = inValues[inCount];
      int delta = inValues[inCount+1] - firstVal;
      float deltaFloat = 0.0f;

      int runSymbol = runSymbolConst;
      int i = inCount + 1;
      int runLength = 1;
      while (inValues[i] != endOfData  &&  inValues[i] == firstVal) {
        runLength++;
        i++;
      }

      // It takes 3 values to store a run, so we don't bother unless the
      // run is longer than 3.

      if (runLength < 3) {
        // Constant didn't work.  Check for mono increasing.
        runSymbol = runSymbolAscend;
        i = inCount + 1;
        runLength = 1;
        while (i != inValues_len  &&  inValues[i] == firstVal+runLength) {
          runLength++;
          i++;
        }
      }

      if (runLength < 3) {
        // Constant and increasing didn't work.  Check for mono decreasing.
        runSymbol = runSymbolDescend;
        i = inCount + 1;
        runLength = 1;
        while (i != inValues_len  &&  inValues[i] == firstVal-runLength) {
          runLength++;
          i++;
        }
      }

      if (runLength < 3) {
        // Constant and mono didn't work.  Try a non-unity delta.
        runSymbol = runSymbolDelta;
        i = inCount + 1;
        runLength = 1;
        while (i != inValues_len  &&  inValues[i] == firstVal+runLength*delta) {
          runLength++;
          i++;
        }
        if (runLength < 5) runLength = 0;  // Disqualified - it takes 4 to store a delta run.
      }

      if (runLength < 3) {
        // Nothing else has worked.  Try a run of floats.
        // The logic here ensures that Nans are not used.
        if (probablyFloat(firstVal)
            &&  probablyFloat(inValues[inCount+1])) 
        {
          void *pfirstVal= &firstVal;
          float firstValFloat = *(reinterpret_cast<float*>((int*)pfirstVal));// Float.intBitsToFloat(firstVal);
          deltaFloat = *(reinterpret_cast<float*>((int*)&inValues[inCount+1]))-firstValFloat; //Float.intBitsToFloat(inValues[inCount+1]) - firstValFloat;
          runSymbol = runSymbolFloats;
          i = inCount + 1;
          runLength = 1;
          // We require the exact equal bit pattern on output (not just almost equal).
          // This will not work on large floats, but it's the best that we can to.
          float fa=firstValFloat+((float)runLength)*deltaFloat;
          void *pfa= &fa;
          int  ia=*(reinterpret_cast<int*>((float*)pfa)); 
          while (inValues[i] != endOfData  &&  probablyFloat(inValues[i])
                 &&  inValues[i] == ia/*Float.floatToIntBits(firstValFloat+((float)runLength)*deltaFloat)*/) 
          {
            runLength++;
            i++;
            fa=firstValFloat+((float)runLength)*deltaFloat;
            pfa= &fa;
            ia=*(reinterpret_cast<int*>((float*)pfa));
          }
          if (runLength < 5) runLength = 0;  // Disqualified - it takes 4 to store a delta run.
        }
      }

      if (runLength > 3) {
        // We got some kind of run.
        if (runSymbol == runSymbolDelta) {
          encodedValues[outCount] = runSymbol;
          encodedValues[outCount+1] = runLength;
          encodedValues[outCount+2] = firstVal;
          encodedValues[outCount+3] = delta;
          inCount += runLength;
          outCount += 4;
        } else if (runSymbol == runSymbolFloats) {
          encodedValues[outCount] = runSymbol;
          encodedValues[outCount+1] = runLength;
          encodedValues[outCount+2] = firstVal;
          void *pdeltaFloat= &deltaFloat; 
          encodedValues[outCount+3] = *(reinterpret_cast<int*>((float*)pdeltaFloat));//Float.floatToIntBits(deltaFloat);
          inCount += runLength;
          outCount += 4;
        } else {
          encodedValues[outCount] = runSymbol;
          encodedValues[outCount+1] = runLength;
          encodedValues[outCount+2] = firstVal;
          inCount += runLength;
          outCount += 3;
        }
      } else {
        // No run of any kind was found.
        encodedValues[outCount] = inValues[inCount];
        inCount++;
        outCount++;
      }

      if (inValues[inCount] == endOfData) {
        // Yes, the output length includes the header.
        encodedValues[IND_OUT_COUNT] = outCount;
        return outCount;
      }

    }  // End of endless loop.

//     return JS_OK;
  }


  /**
   * Decodes run-length encoded integers.
   *
   * @param  encodedValues  input encoded values.
   * @param  outValues  output decoded values.
   * @return  the number of output values.
   */
  int HdrCompressor::runLengthDecode(int* encodedValues, int* outValues) 
  {
    if (encodedValues[IND_COOKIE] == OLD_COOKIE1){
      ERROR_PRINTF(HdrCompressorLog,"Compressed data is from an unsupported version");
      return JS_USERERROR;
    }  
    if (encodedValues[IND_COOKIE] != COOKIE){
      ERROR_PRINTF(HdrCompressorLog,"Input encoded values have invalid header (wrong endianness?) %d != %d", encodedValues[IND_COOKIE], COOKIE);
      return JS_USERERROR;
    }

    int runSymbolConst = encodedValues[IND_SYM_CONST];
    int runSymbolAscend = encodedValues[IND_SYM_ASCEND];
    int runSymbolDescend = encodedValues[IND_SYM_DESCEND];
    int runSymbolDelta = encodedValues[IND_SYM_DELTA];
    int runSymbolFloats = encodedValues[IND_SYM_FLOATS];
    int nInput = encodedValues[IND_OUT_COUNT];

    int outCount = 0;
    int delta = 0;
    float deltaFloat = 0.0F;
    float firstValFloat = 0.0F;
    for (int inCount=HDR_LENGTH; inCount<nInput;) {
      if (encodedValues[inCount] == runSymbolConst
          ||  encodedValues[inCount] == runSymbolAscend
          ||  encodedValues[inCount] == runSymbolDescend
          ||  encodedValues[inCount] == runSymbolDelta
          ||  encodedValues[inCount] == runSymbolFloats) {

        int runLength = encodedValues[inCount+1];
        int firstVal = encodedValues[inCount+2];
        if (encodedValues[inCount] == runSymbolDelta)
          delta = encodedValues[inCount+3];
        if (encodedValues[inCount] == runSymbolFloats) {
          deltaFloat = *(reinterpret_cast<float*>((int*)&encodedValues[inCount+3])); //Float.intBitsToFloat(encodedValues[inCount+3]);
          void *pfirstVal= &firstVal; 
          firstValFloat = *(reinterpret_cast<float*>((int*)pfirstVal));//Float.intBitsToFloat(firstVal);
        }

        for (int i=0; i<runLength; i++) {
          if (encodedValues[inCount] == runSymbolConst) {
            outValues[outCount] = firstVal;
          } else if (encodedValues[inCount] == runSymbolAscend) {
            outValues[outCount] = firstVal + i;
          } else if (encodedValues[inCount] == runSymbolDescend) {
            outValues[outCount] = firstVal - i;
          } else if (encodedValues[inCount] == runSymbolDelta) {
            outValues[outCount] = firstVal + i*delta;
          } else {
            float fa = firstValFloat + ((float)i)*deltaFloat;
            outValues[outCount] = *(reinterpret_cast<float*>((int*)&fa));//Float.floatToIntBits(firstValFloat + ((float)i)*deltaFloat);
          }
          outCount++;
        }

        if (encodedValues[inCount] == runSymbolDelta
            ||  encodedValues[inCount] == runSymbolFloats) {
          inCount += 4;
            } else {
              inCount += 3;
            }

          } else {
            outValues[outCount] = encodedValues[inCount];
            outCount++;
            inCount++;
          }
    }

    return outCount;
  }


  /**
   * Applies zip compression from miniz.c
   *
   * @param  pSource  source array
   * @param  source_len  source length
   * @param  pDest  the output zipped bytes.
   * @param  offset  the byte offset to begin in the output data.
   * @return  the number of bytes in the compressed data.
   */
  int HdrCompressor::zip(int *encodedValues, int nValues, char* zipInput, char *zipOutput, int offset)
  {
    int index = 0;
    for (int i=0; i<nValues; i++) {
      BlockCompressor::stuffIntInBytes(encodedValues[i], zipInput, index);
      index += SIZEOF_INT;
    }

    unsigned long  nBytes=m_zipWorkBuffer_length; //length of output buffer
    int ires = mz_compress2((unsigned char*)&zipOutput[offset], &nBytes, (unsigned char*)zipInput, nValues*SIZEOF_INT, BEST_ZIP_LEVEL);
 
    if(ires!=0){
      ERROR_PRINTF(HdrCompressorLog,"mz_compress2 failed");
      return JS_USERERROR;
    }  
    return nBytes;
  }


//    * Applies zip decompression  from miniz.c
  int HdrCompressor::unzip(const char *unzipInput, int offset, unsigned long nBytesInput, char *unzipOutput, int *encodedValues)
  {
    unsigned long nBytesOutput=m_zipWorkBuffer_length; //length of output buffer
    int ires = mz_uncompress((unsigned char*)unzipOutput, &nBytesOutput, (const unsigned char*)&unzipInput[offset], nBytesInput);
    if(ires!=0){
      ERROR_PRINTF(HdrCompressorLog,"mz_uncompress failed");
      return JS_USERERROR;
    }  
 
    int nInts = nBytesOutput / SIZEOF_INT;
    if (nInts*SIZEOF_INT != nBytesOutput){
      ERROR_PRINTF(HdrCompressorLog,"Unzip returned an odd number of bytes");
      return JS_USERERROR;
    } 

    int index = 0;
    for (int i=0; i<nInts; i++) {
      encodedValues[i] = BlockCompressor::stuffBytesInInt(unzipOutput, index);
      index += SIZEOF_INT;
    }

    return nInts;
  }
  
}
