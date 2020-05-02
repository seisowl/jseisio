/** @example reader.cpp
 * A description of the example file, causes the example file to show up in Examples
 * test multi thread read write cases with new code using OpenMP.
 *
 * -Wl,-no-undefined -std=c++11  -fopenmp -fPIC -g -O3 -ffast-math -msse4.2  -Wno-unused -ftree-vectorize -fopt-info-vec  -Wall
*/

#include <stdio.h>
#include <fstream>
#include <string.h>

#include "jseisUtil.h"
#include "jsFileWriter.h"
#include "catalogedHdrEntry.h"


int main(int argc, char *argv[])
{
	  // step 1: setup parameters
	  std::string fname = "/tmp/junk5d.js";
	  std::string fname2 = "/tmp/junk5d2.js";
	  std::string fname3 = "/tmp/junk5d3.js";
	  std::string fname4 = "/tmp/junk5d4.js";


	  int ndim = 5;
	  int* lengths = new int[5] {11,13,9,7,5};
	  int* logicalOrigins = new int[5] {1,3,5,7,9};
	  int* logicalDeltas = new int[5] {1,2,3,4,5};
	  double* physicalOrigins = new double[5] {10,30,50,70,90};
	  double* physicalDeltas = new double[5] {10,20,30,40,50};
	  std::vector<string> axisHdrs;
	  axisHdrs.push_back("DEPTH");
	  axisHdrs.push_back("TRCAXIS");
	  axisHdrs.push_back("FRMAXIS");
	  axisHdrs.push_back("VOLAXIS");
	  axisHdrs.push_back("HYPAXIS");

	  std::vector<string> extHdrs;
	  extHdrs.push_back("mUTE1");
	  extHdrs.push_back("mUTE2");
	  std::vector<string> extHdrsType;
	  extHdrsType.push_back("float");
	  extHdrsType.push_back("double");


      // step 2: prepare data
      float* volbuf = new float[lengths[0]*lengths[1]*lengths[2]];
      for (int i2 = 0; i2 < lengths[2]; i2++)
    	  for (int i1 = 0; i1 < lengths[1]; i1++)
    		  for (int i0 = 0; i0 < lengths[0]; i0++)
    			  volbuf[i2*lengths[1]*lengths[0] + i1*lengths[0] +i0] = i2*lengths[1]*lengths[0] + i1*lengths[0] +i0;

      // step 3: create js dataset
	  jsIO::oJseisND jsnd(fname, ndim, lengths,logicalOrigins, logicalDeltas, physicalOrigins, physicalDeltas, axisHdrs, jsIO::DataFormat::FLOAT, true,
			  & extHdrs, &extHdrsType);

      // step 4.1 : write frame (iframe, 0, 1) with default header
      size_t iframe = 2;
      size_t ivolume = 0;
      size_t ihyper = 1;
      jsnd.write_frame(&volbuf[iframe*lengths[1]*lengths[0]], iframe, ivolume, ihyper);

      // step 4.2 : write volume with default header
      ivolume = 4;
      ihyper = 1;
      jsnd.write_volume(volbuf, ivolume, ihyper);

      // step 4.2 : write volume with default header
      ivolume = 4;
      ihyper = 1;
      jsnd.write_volume(volbuf, ivolume, ihyper);

      ////////////////////////////////////////////////////////////////////
      // if write out with use input headers /////////////////////////////

      // step 2: create js dataset
      jsIO::oJseisND jsnd2(fname2, ndim, lengths,logicalOrigins, logicalDeltas, physicalOrigins, physicalDeltas, axisHdrs);

      // step 3: prepare header
      jsIO::catalogedHdrEntry itrcTypeHdr = jsnd2.jsWrt.getHdrEntry("TRC_TYPE");
      jsIO::catalogedHdrEntry itrcHdr = jsnd2.jsWrt.getHdrEntry(axisHdrs[1]);
      jsIO::catalogedHdrEntry ifrmHdr = jsnd2.jsWrt.getHdrEntry(axisHdrs[2]);
      jsIO::catalogedHdrEntry ivolHdr = jsnd2.jsWrt.getHdrEntry(axisHdrs[3]);
      jsIO::catalogedHdrEntry ihyperHdr = jsnd2.jsWrt.getHdrEntry(axisHdrs[4]);
      jsIO::catalogedHdrEntry fRecElevHdr = jsnd2.jsWrt.getHdrEntry("REC_ELEV");
      jsIO::catalogedHdrEntry dRecXHdr = jsnd2.jsWrt.getHdrEntry("REC_XD");
      jsIO::catalogedHdrEntry dRecYHdr = jsnd2.jsWrt.getHdrEntry("REC_YD");
      jsIO::catalogedHdrEntry fSouElevHdr = jsnd2.jsWrt.getHdrEntry("SOU_ELEV");
      jsIO::catalogedHdrEntry dSouXHdr = jsnd2.jsWrt.getHdrEntry("SOU_XD");
      jsIO::catalogedHdrEntry dSouYHdr = jsnd2.jsWrt.getHdrEntry("SOU_YD");
      jsIO::catalogedHdrEntry dCdpXHdr = jsnd2.jsWrt.getHdrEntry("CDP_XD");
      jsIO::catalogedHdrEntry dCdpYHdr = jsnd2.jsWrt.getHdrEntry("CDP_YD");
      jsIO::catalogedHdrEntry fAOffsetHdr = jsnd2.jsWrt.getHdrEntry("AOFFSET");
      jsIO::catalogedHdrEntry iInLineHdr = jsnd2.jsWrt.getHdrEntry("ILINE_NO");
      jsIO::catalogedHdrEntry iXLineHdr = jsnd2.jsWrt.getHdrEntry("XLINE_NO");
      jsIO::catalogedHdrEntry iSourceHdr = jsnd2.jsWrt.getHdrEntry("SOURCE");
      jsIO::catalogedHdrEntry iChanHdr = jsnd2.jsWrt.getHdrEntry("CHAN");

      size_t traceheaderSize = jsnd2.jsWrt.getTraceHeaderSize();
      char* hdbuf = new char[traceheaderSize*lengths[1]*lengths[2]];
      memset(hdbuf, 0, traceheaderSize*lengths[1]*lengths[2]);

      // step 4.1 : write frame (iframe, 0, 1) user setup header values
      iframe = 2;
      ivolume = 0;
      ihyper = 1;
      for (int i2 = 0; i2 < lengths[1]; i2++) {
    	size_t offset = (iframe*lengths[1] + i2) * traceheaderSize;
        itrcTypeHdr.setIntVal(&hdbuf[offset], 1); // default 1 is OK
        itrcHdr.setIntVal(&hdbuf[offset], logicalOrigins[1] + i2 * logicalDeltas[1] );
    	ifrmHdr.setIntVal(&hdbuf[offset], logicalOrigins[2] + iframe * logicalDeltas[2] );
        if (ndim > 3) ivolHdr.setIntVal(&hdbuf[offset], logicalOrigins[3]  + ivolume * logicalDeltas[3] );
        if (ndim > 4) ihyperHdr.setIntVal(&hdbuf[offset], logicalOrigins[4] + ihyper * logicalDeltas[4] );
        fRecElevHdr.setFloatVal(&hdbuf[offset], 0);
        dRecXHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[1] + i2 * physicalDeltas[1]);
        dRecYHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[2] + i2 * physicalDeltas[2]);
        fSouElevHdr.setFloatVal(&hdbuf[offset], 0);
        dSouXHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[1] + i2 * physicalDeltas[1]);
        dSouYHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[2] + i2 * physicalDeltas[2]);
        dCdpXHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[1] + i2 * physicalDeltas[1]);
        dCdpYHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[2] + i2 * physicalDeltas[2]);
        fAOffsetHdr.setFloatVal(&hdbuf[offset], 0);
        iXLineHdr.setIntVal(&hdbuf[offset], logicalOrigins[1] + i2 * logicalDeltas[1] );
        iInLineHdr.setIntVal(&hdbuf[offset], logicalOrigins[2] + iframe * logicalDeltas[2] );
        iSourceHdr.setIntVal(&hdbuf[offset], logicalOrigins[1] + iframe * logicalDeltas[1] );
        iChanHdr.setIntVal(&hdbuf[offset], logicalOrigins[2] + i2 * logicalDeltas[2] );
      }
      jsnd2.write_frame(&volbuf[iframe*lengths[1]*lengths[0]], &hdbuf[iframe*lengths[1]*traceheaderSize], iframe, ivolume, ihyper);

      // step 4.2 : write volume with user setup header values
      ivolume = 4;
      ihyper = 1;
      for (iframe = 0; iframe < lengths[2]; iframe++) {
      for (int i2 = 0; i2 < lengths[1]; i2++) {
    	  size_t offset = (iframe*lengths[1] + i2) * traceheaderSize;
          itrcTypeHdr.setIntVal(&hdbuf[offset], 1); // default 1 is OK
          itrcHdr.setIntVal(&hdbuf[offset], logicalOrigins[1] + i2 * logicalDeltas[1] );
          ifrmHdr.setIntVal(&hdbuf[offset], logicalOrigins[2] + iframe * logicalDeltas[2] );
          if (ndim > 3) ivolHdr.setIntVal(&hdbuf[offset], logicalOrigins[3]  + ivolume * logicalDeltas[3] );
          if (ndim > 4) ihyperHdr.setIntVal(&hdbuf[offset], logicalOrigins[4] + ihyper * logicalDeltas[4] );
          fRecElevHdr.setFloatVal(&hdbuf[offset], 0);
          dRecXHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[1] + i2 * physicalDeltas[1]);
          dRecYHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[2] + i2 * physicalDeltas[2]);
          fSouElevHdr.setFloatVal(&hdbuf[offset], 0);
          dSouXHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[1] + i2 * physicalDeltas[1]);
          dSouYHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[2] + i2 * physicalDeltas[2]);
          dCdpXHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[1] + i2 * physicalDeltas[1]);
          dCdpYHdr.setDoubleVal(&hdbuf[offset], physicalOrigins[2] + i2 * physicalDeltas[2]);
          fAOffsetHdr.setFloatVal(&hdbuf[offset], 0);
          iXLineHdr.setIntVal(&hdbuf[offset], logicalOrigins[1] + i2 * logicalDeltas[1] );
          iInLineHdr.setIntVal(&hdbuf[offset], logicalOrigins[2] + iframe * logicalDeltas[2] );
          iSourceHdr.setIntVal(&hdbuf[offset], logicalOrigins[1] + iframe * logicalDeltas[1] );
          iChanHdr.setIntVal(&hdbuf[offset], logicalOrigins[2] + i2 * logicalDeltas[2] );
      }
      }
      jsnd2.write_volume(volbuf, hdbuf, ivolume, ihyper);

      // step 4.3 : write volume and header with frames
	  jsIO::oJseisND jsnd3(fname3, ndim, lengths,logicalOrigins, logicalDeltas, physicalOrigins, physicalDeltas, axisHdrs, jsIO::DataFormat::FLOAT);
      ivolume = 4;
      ihyper = 1;
      jsnd3.write_volume(volbuf, hdbuf, ivolume, ihyper);

      // step 4.4 : write volume and header with whole volume buffer
	  jsIO::oJseisND jsnd4(fname4, ndim, lengths,logicalOrigins, logicalDeltas, physicalOrigins, physicalDeltas, axisHdrs, jsIO::DataFormat::FLOAT);
      ivolume = 4;
      ihyper = 1;
      jsnd4.write_volume_reg(volbuf, hdbuf, ivolume, ihyper);

  return 0;
}
