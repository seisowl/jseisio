
/***************************************************************************
                           HuffCoder.h  -  description
                             -------------------
* This class does Huffman coding/decoding.

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

#ifndef  HUFFCODER_H
#define  HUFFCODER_H

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace jsIO
{
  class HuffNode {
    public:
      int info;
      HuffNode *left;
      HuffNode *right;
    public:
//    ~HuffNode();
      HuffNode(){
        left = NULL;
        right = NULL;
        info = -1;
      };
  };

  class  HuffCoder{
    public:
      ~HuffCoder();
      HuffCoder();
      HuffCoder(const int* huffTable);
      void Init(const int* huffTable);

      int huffEncode(char* runLengthEncodedData, int ninputBytes,char* huffEncodedData, int index, int outputBufferSize);
      int huffEncode(int* runLengthEncodedData, int ninputInts,char* huffEncodedData, int index, int outputBufferSize);
      int huffDecode(const char* huffEncodedData, int index, char* cHout, int outputBufferSize);

      void getHuffTable(int *huffTable);
      void printTable(int* count);

    public:
      static const int c_huffCount[257]; //size = MAXVALUE+1
      static const int c_hdrHuffCount[257];
      static const int c_hdrHuffCountImproved[257];

// private atributes
    private:
      HuffNode *globalRoot;
      int  * huffCode;
      int  * huffLen;
      char * bVals;



      static const int MAXVALUE = 256;
      static const int MAXVALUEP1 = 257;  

    private:
      void deleteTree(HuffNode *rootNode);
      HuffNode* addTree(HuffNode *x, int v, int l, int info);
      HuffNode* buildHuffmanTree();
      void downHeap(int* count, int* heap, int N, int k);
      void stuffInBytes( int ival, char* bvals, int offset);

      void huffTableMake(int* count, int* heap, int* parent, int* huffCode, int* huffLen);

  };

}


#endif



