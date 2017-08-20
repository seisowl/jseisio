/** @example reader.cpp
 * A description of the example file, causes the example file to show up in 
 * Examples 
*/

#include <stdio.h>
#include <fstream>

#include "jsFileWriter.h"
#include "jsWriterInput.h"
#include "catalogedHdrEntry.h"

#include <sys/timeb.h>

using namespace jsIO;

int main(int argc, char *argv[])
{
  timeb time0,time1;
  double total;
  ftime(&time0);
 
  // JavaSeis file properties
  // ------------------------

  jsIO::jsWriterInput wrtInput;

//   wrtInput.setFileName("/m/scratch/supertmp/abel/JavaSeis/dataTest.js");
  wrtInput.setFileName("/tmp/abel/dataTest.js");
  wrtInput.setFileDescription("Test data");
  wrtInput.setMapped(true);
  wrtInput.setDiskCacheSize(4*1024*1024); //4MB
  wrtInput.setNumberOfExtents(3);
//   wrtInput.add_secondary_path("/m/scratch/supertmp/abel/JavaSeis/VFtestsSec1/l1/l2/jsFile/");
//    wrtInput.add_secondary_path("/m/scratch/supertmp/abel/JavaSeis/VFtestsSec2/l1/l2/jsFile/");

  // Create data context
  // -------------------

  wrtInput.initData(jsIO::DataType::CUSTOM, jsIO::DataFormat::FLOAT, jsIO::JSIO_LITTLEENDIAN);//DataFormat::FLOAT, COMPRESSED_INT16, SEISPEG
//   wrtInput.setSeispegPolicy(1);//0 = FASTEST, 1 = MAX_COMPRESSION
      
  // Framework definition
  int numDim = 4;
  int NSamples = 501;
  int NOffsets =  197;
  int NXlines =   20;
  int NInlines =  10;
  float off0 = 0;
  float doff = 100;
  float xl0 = 10;
  float dxl = 20;
  float inl0 = 20;
  float dinl = 40;

  wrtInput.initGridDim(numDim);
  wrtInput.initGridAxis(0, jsIO::AxisLabel::TIME, jsIO::Units::SECONDS,jsIO::DataDomain::TIME, NSamples, 0, 1, 0, 0.004);
  wrtInput.initGridAxis(1, jsIO::AxisLabel::OFFSET_BIN, jsIO::Units::M,jsIO::DataDomain::SPACE, NOffsets, 0, 1, off0, doff);
  wrtInput.initGridAxis(2, jsIO::AxisLabel::CROSSLINE, jsIO::Units::M,jsIO::DataDomain::SPACE, NXlines, 0, 1, xl0, dxl);
  wrtInput.initGridAxis(3, jsIO::AxisLabel::INLINE, jsIO::Units::M,jsIO::DataDomain::SPACE, NInlines, 0, 1, inl0, dinl);

  //Add properties/header-words
  wrtInput.addDefaultProperties();
  //     wrtInput.addProperty("ILINE_NO");
  //     wrtInput.addProperty("XLINE_NO");
  wrtInput.addProperty("NEW_HDR", "Header description", "INTEGER", 1);


  //Add geometry
  wrtInput.addSurveyGeom(1,2,3,4,5.5,6.6,7.7,8.8,9.9,10.10);
  wrtInput.addCustomProperty("Stacked", "boolen", "false"); 
  
  // Initalize jsFileWriter object with the defined data context wrtInput
  jsIO::jsFileWriter jsWrtTest;
  int ires;

  printf("init data...\n"); 
  ires = jsWrtTest.Init(&wrtInput);
  if(ires!=JS_OK){
    printf("Error during initalization!\n");
    return 0;
  }   

  // Create dataset (only meta data) using the data context
  // ------------------------------------------------------
  printf("write meta data (xml)...\n");

  ires = jsWrtTest.writeMetaData();
  if(ires!=JS_OK){
    printf("Error writing meta data!\n");
    return 0;
  }   
  printf("write meta data done!\n");
  
  // Write trace and headers data
  // ----------------------------
  printf("write binary data...\n");

  float *frame = jsWrtTest.allocFrameBuf(); // Get directly from data context
  					    //i.e. allocate frame with new float[NSamples*NOffsets];
    					    //the user have to call delete[] afterwards 

  char *hdbuf = jsWrtTest.allocHdrBuf(true);//alloc buffer (with new[] command) and init with SeisSpace standard values.(parameter initVals=true)
    				            //the user have to call delete[] afterwards 

  int traceheaderSize = jsWrtTest.getTraceHeaderSize();

  printf("NSamples=%d, NOffsets=%d\n",NSamples,NOffsets);

  /*  
  FILE *pfile = fopen("/m/scratch/supertmp/abel/JavaSeis/data_seg.strip","rb");
  fread (frame,sizeof(float),NSamples*NOffsets,pfile);
  fclose(pfile);
  */
      
  // init frame with synth values
  for(int i=0;i<NOffsets;i++)
    for(int j=0;j<NSamples;j++)
        frame[i*NSamples+j]=i;
        
      
  //get access to header-words
  jsIO::catalogedHdrEntry itrcTypeHdr = jsWrtTest.getHdrEntry("TRC_TYPE");
  jsIO::catalogedHdrEntry iTimeHdr = jsWrtTest.getHdrEntry("TIME");
  jsIO::catalogedHdrEntry fOffsetHdr = jsWrtTest.getHdrEntry("OFFSET"); 
  jsIO::catalogedHdrEntry iOffsetBinHdr = jsWrtTest.getHdrEntry("OFB_NO"); 
  jsIO::catalogedHdrEntry dCdpXHdr = jsWrtTest.getHdrEntry("CDP_XD");
  jsIO::catalogedHdrEntry dCdpYHdr = jsWrtTest.getHdrEntry("CDP_YD");  
  jsIO::catalogedHdrEntry iInLineHdr = jsWrtTest.getHdrEntry("ILINE_NO"); 
  jsIO::catalogedHdrEntry iXLineHdr = jsWrtTest.getHdrEntry("XLINE_NO"); 

  int trc_type=1;
  int t0 = 0;
  int numLiveTraces;

  jsWrtTest.writeTrace(0,frame);

  //Initalize headers and write frames (data and headers)
  long frameInd=0;
  for(int iInline=0;iInline<NInlines;iInline++){
      printf("iInline=%d\n",iInline);
      for(int iXline=0;iXline<NXlines;iXline++){
//         printf("\t \t iXline=%d\n", iXline);
        for(int iTraces=0;iTraces<NOffsets;iTraces++){
//            if(iInline==0 && iXline<2 && iTraces%2 == 0 ) trc_type = 0; //test non-full frames
//            else trc_type = 1;
  	   itrcTypeHdr.setIntVal(&hdbuf[iTraces*traceheaderSize], trc_type);
    	   iTimeHdr.setIntVal(&hdbuf[iTraces*traceheaderSize], t0);
   	   fOffsetHdr.setFloatVal(&hdbuf[iTraces*traceheaderSize],  off0 + iTraces*doff);
   	   iOffsetBinHdr.setIntVal(&hdbuf[iTraces*traceheaderSize], iTraces);
   	   dCdpXHdr.setDoubleVal(&hdbuf[iTraces*traceheaderSize], xl0 + iXline*dxl);
   	   dCdpYHdr.setDoubleVal(&hdbuf[iTraces*traceheaderSize], inl0 + iInline*dinl);
   	   iInLineHdr.setIntVal(&hdbuf[iTraces*traceheaderSize], iInline);
   	   iXLineHdr.setIntVal(&hdbuf[iTraces*traceheaderSize], iXline);
           
//            printf("\t%d: \tSOU_X=%3.2f, SOU_Y=%3.2f, ILINE_NO=%d, XLINE_NO=%d, Type=%d\n", 
//                   iTraces, xl0 + iXline*dxl, inl0 + iInline*dinl,  iInline, iXline, trc_type);
           
        }
        numLiveTraces = jsWrtTest.leftJustify(frame, hdbuf, NOffsets);
//         printf("numLiveTraces=%d\n", numLiveTraces) ;
        ires = jsWrtTest.writeFrame(frameInd,frame, hdbuf, numLiveTraces);
//         ires = jsWrtTest.writeFrame(frameInd,frame, NULL, numLiveTraces);
        if(ires!=numLiveTraces){
          printf("Error while writing frame # %ld\n", frameInd);
          iXline=NXlines;
          iInline=NInlines;
          break;
        }  
        frameInd++;
      }    
  }
  printf("write binary data done!\n");

  ftime(&time1);
  total = (time1.time-time0.time) +(time1.millitm-time0.millitm)/1.e3;
  std::cout << "\n write time: " << total << " (sec)\n\n";

  delete[] hdbuf;  //was allocated with jsWrtTest.allocHdrBuf()
  delete[] frame;  //was allocated with jsWrtTest.allocFrameBuf()

  return 0;
}
