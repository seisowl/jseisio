/** @example reader.cpp
 * A description of the example file, causes the example file to show up in Examples
 * test multi thread read write cases with new code using OpenMP.
 *
 * -Wl,-no-undefined -std=c++11  -fopenmp -fPIC -g -O3 -ffast-math -msse4.2  -Wno-unused -ftree-vectorize -fopt-info-vec  -Wall
*/

#include <stdio.h>
#include <fstream>

#include "jsFileWriter.h"
#include "jsWriterInput.h"
#include "catalogedHdrEntry.h"
#include "jsFileReader.h"

#include <sys/timeb.h>
#include <omp.h>

using namespace jsIO;

#define LOG4CPLUS_DISABLE_DEBUG
#define LOG4CPLUS_DISABLE_TRACE

void updating(jsIO::jsFileWriter * jsWrtTest, int seed) {

	   int dim = jsWrtTest->getNDim();
	   int NSamples = jsWrtTest->getAxisLen(0);
	   int NOffsets  = jsWrtTest->getAxisLen(1);
	   int NXlines = dim>2 ? jsWrtTest->getAxisLen(2) : 0;
	   int NInlines = dim>3 ? jsWrtTest->getAxisLen(3) : 0;
	   double off0 = jsWrtTest->getAxisPhysicalOrigin(1);
	   double doff = jsWrtTest->getAxisPhysicalDelta(1);
	   int xl0 = jsWrtTest->getAxisLogicalOrigin(2);
	   int dxl = jsWrtTest->getAxisLogicalDelta(2);
	   int inl0 = jsWrtTest->getAxisLogicalOrigin(3);
	   int dinl = jsWrtTest->getAxisLogicalOrigin(4);

	   // mulit thread replace/update the values
		#pragma omp parallel num_threads(4)
		{
		 float *frame = jsWrtTest->allocFrameBuf(); // Get directly from data context
								//i.e. allocate frame with new float[NSamples*NOffsets];
								//the user have to call delete[] afterwards

		 char *hdbuf = jsWrtTest->allocHdrBuf(true);//alloc buffer (with new[] command) and init with SeisSpace standard values.(parameter initVals=true)
									//the user have to call delete[] afterwards
		 int trc_type=1;
		 int t0 = 0;

		 // jsWrtTest.writeTrace(0,frame);

		 //Initalize headers and write frames (data and headers)

		 //get access to header-words
		  jsIO::catalogedHdrEntry itrcTypeHdr = jsWrtTest->getHdrEntry("TRC_TYPE");
		  jsIO::catalogedHdrEntry iTimeHdr = jsWrtTest->getHdrEntry("TIME");
		  jsIO::catalogedHdrEntry fOffsetHdr = jsWrtTest->getHdrEntry("OFFSET");
		  jsIO::catalogedHdrEntry iOffsetBinHdr = jsWrtTest->getHdrEntry("OFB_NO");
		  jsIO::catalogedHdrEntry dCdpXHdr = jsWrtTest->getHdrEntry("CDP_XD");
		  jsIO::catalogedHdrEntry dCdpYHdr = jsWrtTest->getHdrEntry("CDP_YD");
		  jsIO::catalogedHdrEntry iInLineHdr = jsWrtTest->getHdrEntry("ILINE_NO");
		  jsIO::catalogedHdrEntry iXLineHdr = jsWrtTest->getHdrEntry("XLINE_NO");

		  int traceheaderSize = jsWrtTest->getTraceHeaderSize();

		  #pragma omp for
		  for(int iInline=0;iInline<NInlines;iInline++){
		//	 printf("iInline=%d\n",iInline);
			 for(int iXline=0;iXline<NXlines;iXline++){
		 //       printf("\t \t iXline=%d\n", iXline);
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

				  // init frame with synth values
				  for(int j=0;j<NSamples;j++)
					 frame[iTraces*NSamples+j]= seed + j + (iInline*NXlines + iXline)*NOffsets+ iTraces;
			   }

			   int numLiveTraces = jsWrtTest->leftJustify(frame, hdbuf, NOffsets);
			   long frameInd=iInline*NXlines + iXline;
			   int ires1 = jsWrtTest->writeFrame(frameInd,frame, hdbuf, numLiveTraces);
		//         ires = jsWrtTest.writeFrame(frameInd,frame, NULL, numLiveTraces);
			   if(ires1!=numLiveTraces){
				 printf("Error while writing frame # %ld\n", frameInd);
				 iXline=NXlines;
				 iInline=NInlines;
				 break;
			   }
			 }
		 }

		 delete[] hdbuf;  //was allocated with jsWrtTest.allocHdrBuf()
		 delete[] frame;  //was allocated with jsWrtTest.allocFrameBuf()

		}
}

int readVerifyTest(const std::string &jsfilename, int value)
{
   // open file
   jsFileReader jsReadTest;
   int ierr = jsReadTest.Init(jsfilename);
   if(ierr!=1){
       printf("Error in JavaSeis file %s\n", jsfilename.c_str());
       exit(-1);
   }

    // version
//   printf("Version: %s\n",jsReadTest.getVersion().c_str());
   printf("Description : %s\n",jsReadTest.getDescriptiveName().c_str());

   //Custom property
//   printf("Stacked: %s\n",jsReadTest.getCustomProperty("Stacked").c_str());
   //    printf("sourceType: %s\n",jsTest.getCustomProperty("FieldInstruments/sourceType").c_str());

   long Ntr = jsReadTest.getNtr();
   printf("Total number of traces in %s is %ld\n",jsfilename.c_str(),Ntr);

   // Number of frames
   long numFrames = jsReadTest.getNFrames();
   printf("Total number of frames in %s is %ld\n",jsfilename.c_str(),numFrames);

   int dim = jsReadTest.getNDim();
   int nSamples = jsReadTest.getAxisLen(0);
   int nTraces  = jsReadTest.getAxisLen(1);
   int nframes = dim>2 ? jsReadTest.getAxisLen(2) : 0;
   int nvolumes = dim>3 ? jsReadTest.getAxisLen(3) : 0;
   printf("nDim=%d, nSamples=%d, nTracesInFrame=%d, nFrameInVolume=%d, nVolumeInHypercube=%d, numFrames=%ld, \n",
		   dim,nSamples,nTraces,nframes,nvolumes,numFrames);

   std::vector<std::string> axis;
   jsReadTest.getAxisLabels(axis);
   for (int i=0; i <dim; i++)
	   printf("Axis %d  %s\n", i+1,  axis[i].c_str());

// 1 way to get all header name
   std::vector<std::string> hdrEs;
   int N = jsReadTest.getHdrEntries(hdrEs);
   for(int i=0;i<N;i++){
   	   printf("Header %d  %s \n", i+1,  hdrEs[i].c_str());
   }

// another way to using catalogedHdrEntry
   N = jsReadTest.getNumHeaderWords();
   std::vector<jsIO::catalogedHdrEntry, std::allocator<jsIO::catalogedHdrEntry> > hdrEntries = jsReadTest.getHdrEntries();
   for(int i=0;i<N;i++){
	   printf("Header %d  %s  offset=%d \n", i+1,  hdrEntries[i].getName().c_str(), hdrEntries[i].getOffset());
   }
   catalogedHdrEntry hdrSouX = jsReadTest.getHdrEntry("CDP_XD");
   printf("Header hdrSouX details:\n");
   printf("  Name = %s\n", hdrSouX.getName().c_str());
   printf("  Description = %s\n", hdrSouX.getDescription().c_str());
   printf("  Index = %d\n", hdrSouX.getOffset());
   printf("  Dimension = %d\n", hdrSouX.getCount());
   printf("  Bytes = %d\n", hdrSouX.getByteCount());
   printf("  Format = %s\n", hdrSouX.getFormatAsStr().c_str());


   // Read all frames
   printf("\nReading %s\n",jsfilename.c_str());
   long nTracesTotal=0;

   int frameLen = nSamples*nTraces;
   int traceheaderSize = jsReadTest.getNumBytesInHeader();
   int frameheaderSize  =  nTraces*traceheaderSize;

   timeb time0,time1;
   ftime(&time0);

   #pragma omp parallel num_threads(3)
  {
	   jsFileReader jsReadTestTh;
	   jsReadTestTh.Init(jsfilename);
	   char *headerBuf = new char[frameheaderSize];
	   float *gather = new float[frameLen];

   for (int i=omp_get_thread_num(); i<numFrames; i+=omp_get_num_threads())
   {
	   printf("Frame number=%d\n",i);
	   int nTracesRead = jsReadTestTh.readFrame(i, gather, headerBuf);
       if (nTracesRead < 0) {
           printf("Error while trying to read frame #%d\n",i);
           break;
       }
       printf("Number of live traces in frame = %d\n", nTracesRead);

       //print some header info
       float sou_x, sou_y, rec_x, rec_y;
       float fOffset;
       int il, xl;
       int iType;
       for (int j=0; j<nTracesRead; j++)
       {
	   sou_x = jsReadTestTh.getDoubleHdrVal("CDP_XD", &headerBuf[j*traceheaderSize]);
	   sou_x = jsReadTestTh.getDoubleHdrVal("CDP_XD", &headerBuf[j*traceheaderSize]);
	   sou_y = jsReadTestTh.getDoubleHdrVal("CDP_YD", &headerBuf[j*traceheaderSize]);
	   rec_x = jsReadTestTh.getDoubleHdrVal("CDP_XD", &headerBuf[j*traceheaderSize]);
	   rec_y = jsReadTestTh.getDoubleHdrVal("CDP_YD", &headerBuf[j*traceheaderSize]);
	   il = jsReadTestTh.getIntHdrVal("ILINE_NO", &headerBuf[j*traceheaderSize]);
	   xl = jsReadTestTh.getIntHdrVal("XLINE_NO", &headerBuf[j*traceheaderSize]);
	   iType =  jsReadTestTh.getIntHdrVal("TRC_TYPE", &headerBuf[j*traceheaderSize]);
	   fOffset =  jsReadTestTh.getFloatHdrVal("OFFSET", &headerBuf[j*traceheaderSize]);

	   //printf("\t%d: \tSOU_X=%3.2f, SOU_Y=%3.2f, REC_X=%3.2f, REC_Y=%3.2f, ILINE_NO=%d, XLINE_NO=%d, Offset=%g, Type=%d\n",
       //           j, sou_x, sou_y, rec_x, rec_y, il, xl, fOffset, iType);

	   for(int k=0;k<nSamples;k++)
	      if (gather[j*nSamples+k] != k + (il*nframes + xl)*nTraces+ j + value) {
	           printf("Error trace data #%d\n",k);
	           exit(-1);
	      };
       }
 
       nTracesTotal +=nTracesRead;
   }
   delete[] gather;
   delete[] headerBuf;
  }

   printf("Read %ld traces and verified!\n", nTracesTotal);

   ftime(&time1);
   double total = (time1.time-time0.time) +(time1.millitm-time0.millitm)/1.e3;
   std::cout << "\n read time: " << total << " (sec)\n\n";

   return 0;
}

int writeTestbyWriter(const std::string &jsfilename)
{
	  timeb time0,time1;
	  double total;
	  ftime(&time0);

	  // Framework definition
	  int numDim = 4;
	  int NSamples = 501;
	  int NOffsets =  59;
	  int NXlines =   13;
	  int NInlines =  5;
	  float off0 = 0;
	  float doff = 100;
	  int xl0 = 10;
	  int dxl = 20;
	  int inl0 = 20;
	  int dinl = 40;

/* we have 3 way to create dataset:
 1. using jsWriterInput
 2. using jsFileWriter to setup datacontent
 3. copy from jsFileReader, the data is also copied.
*/
	 //   jsIO::jsWriterInput wrtInput;
	 //	  wrtInput.setFileName(jsfilename);
	 //	  wrtInput.setFileDescription("Test data");
	 //	  wrtInput.setMapped(true);
	 //	  wrtInput.setDiskCacheSize(4*1024*1024); //4MB
	 //	  wrtInput.setNumberOfExtents(3);
	 //   wrtInput.add_secondary_path("/m/scratch/supertmp/abel/JavaSeis/VFtestsSec1/l1/l2/jsFile/");
	 //   wrtInput.add_secondary_path("/m/scratch/supertmp/abel/JavaSeis/VFtestsSec2/l1/l2/jsFile/");

      //  wrtInput.initData(jsIO::DataType::CUSTOM, jsIO::DataFormat::FLOAT, jsIO::JSIO_LITTLEENDIAN);//DataFormat::FLOAT, COMPRESSED_INT16, SEISPEG
	  //  wrtInput.initData(jsIO::DataType::CUSTOM, jsIO::DataFormat::COMPRESSED_INT16, jsIO::JSIO_LITTLEENDIAN);//DataFormat::FLOAT, COMPRESSED_INT16, SEISPEG
	  //  wrtInput.initData(jsIO::DataType::CUSTOM, jsIO::DataFormat::SEISPEG, jsIO::JSIO_LITTLEENDIAN);//DataFormat::FLOAT, COMPRESSED_INT16, SEISPEG
	  //  wrtInput.setSeispegPolicy(1);//0 = FASTEST, 1 = MAX_COMPRESSION
	  //  wrtInput.initGridDim(numDim);
	  //  wrtInput.initGridAxis(0, jsIO::AxisLabel::TIME, jsIO::Units::SECONDS,jsIO::DataDomain::TIME, NSamples, 0, 1, 0, 4);
	  //  wrtInput.initGridAxis(1, jsIO::AxisLabel::OFFSET_BIN, jsIO::Units::M,jsIO::DataDomain::SPACE, NOffsets, 0, 1, off0, doff);
	  //  wrtInput.initGridAxis(2, jsIO::AxisLabel::CROSSLINE, jsIO::Units::M,jsIO::DataDomain::SPACE, NXlines, 0, 1, xl0, dxl);
	  //  wrtInput.initGridAxis(3, jsIO::AxisLabel::INLINE, jsIO::Units::M,jsIO::DataDomain::SPACE, NInlines, 0, 1, inl0, dinl);
	  //       Add properties/header-words
	  //  wrtInput.addDefaultProperties();
	  //  wrtInput.addProperty("NEW_HDR", "Header description", "INTEGER", 1);
	  //      Initalize jsFileWriter object with the defined data context wrtInput
	  //Add geometry
	  //  wrtInput.addSurveyGeom(1,2,3,4,5.5,6.6,7.7,8.8,9.9,10.10);
	  //  wrtInput.addCustomProperty("Stacked", "boolen", "false");

	  //  int ires = jsWrtTest.Init(&wrtInput);

	  jsIO::jsFileWriter jsWrtTest;
	  printf("init data...\n");
	  jsWrtTest.setFileName(jsfilename);
	  jsWrtTest.initDataType("CUSTOM", "FLOAT", true, 4, "/tmp/second");
	  jsWrtTest.initGridDim(numDim);
	  jsWrtTest.initGridAxis(0, "TIME", "SECONDS","TIME", NSamples, 0, 1, 0, 4);
	  jsWrtTest.initGridAxis(1, "OFFSET_BIN", "METERS", "SPACE", NOffsets, 0, 1, off0, doff);
	  jsWrtTest.initGridAxis(2, "CROSSLINE", "METERS", "SPACE", NXlines, 0, 1, xl0, dxl);
	  jsWrtTest.initGridAxis(3, "INLINE", "METERS", "SPACE", NInlines, 0, 1, inl0, dinl);
	  jsWrtTest.addProperty("NEW_HDR", "Header description", "INTEGER", 1);
	  jsWrtTest.addSurveyGeom(inl0,inl0 + (NInlines-1)*dinl,xl0, xl0 + (NXlines-1)*dxl,
			  5.5,6.6,7.7,8.8,9.9,10.10);
	  jsWrtTest.addCustomProperty("Stacked", "boolean", "false");

	  // Create dataset (only meta data) using the data context
	  // ------------------------------------------------------
	  printf("write meta data (xml)...\n");
	  int ires = jsWrtTest.writeMetaData();
	  if(ires!=JS_OK){
	    printf("Error writing meta data!\n");
	    return 0;
	  }
	  printf("write meta data done!\n");

	  // ----------------------------
	  printf("write binary data...\n");

	  updating(&jsWrtTest, 0);

      printf("write binary data done!\n");

	  ftime(&time1);
	  total = (time1.time-time0.time) +(time1.millitm-time0.millitm)/1.e3;
	  std::cout << "\n Write time: " << total << " (sec)\n\n";

	  return 0;
}

int writeTestbyCopy(const std::string &injsfilename, const std::string &outjsfilename, int update)
{
   timeb time0,time1;
   double total;
   ftime(&time0);

   jsFileReader jsReadTest;

   int ierr = jsReadTest.Init(injsfilename);
   if(ierr!=1){
       printf("Error in JavaSeis file %s\n", injsfilename.c_str());
       exit(-1);
   }
   jsIO::jsFileWriter jsWrtTest;
   jsWrtTest.setFileName(outjsfilename);
   printf("init data...\n");
   jsWrtTest.Init(&jsReadTest);
   printf("write meta data (xml), and copy data ...\n");
   int ires = jsWrtTest.writeMetaData(2);
   printf("write meta data and copy data done!\n");

   if (!update) return 0;

   printf("update binary data...\n");

   updating(&jsWrtTest, 10);

   printf("write binary data done!\n");
   ftime(&time1);
   total = (time1.time-time0.time) +(time1.millitm-time0.millitm)/1.e3;
   std::cout << "\n Write time: " << total << " (sec)\n\n";

   return 0;
}

int writeTestbyUpdate(const std::string &injsfilename)
{
   timeb time0,time1;
   double total;
   ftime(&time0);

   jsFileReader jsReadTest;

   int ierr = jsReadTest.Init(injsfilename);
   if(ierr!=1){
       printf("Error in JavaSeis file %s\n", injsfilename.c_str());
       exit(-1);
   }
   jsIO::jsFileWriter jsWrtTest;
   jsWrtTest.setFileName(injsfilename);
   printf("init data...\n");
   jsWrtTest.Init(&jsReadTest);
   jsWrtTest.Initialize();

   //printf("write meta data (xml), and copy data ...\n");
   //int ires = jsWrtTest.writeMetaData(2);
   //printf("write meta data and copy data done!\n");

   // if (!update) return 0;

   printf("update binary data, no create...\n");

   updating(&jsWrtTest, 100);

   printf("write binary data done!\n");
   ftime(&time1);
   total = (time1.time-time0.time) +(time1.millitm-time0.millitm)/1.e3;
   std::cout << "\n Write time: " << total << " (sec)\n\n";

   return 0;
}

int writeTestbyCopyHeader(const std::string &injsfilename, const std::string &outjsfilename)
{
   timeb time0,time1;
   double total;
   ftime(&time0);

   jsFileReader jsReadTest;

   int ierr = jsReadTest.Init(injsfilename);
   if(ierr!=1){
       printf("Error in JavaSeis file %s\n", injsfilename.c_str());
       exit(-1);
   }

   jsIO::jsFileWriter jsWrtTest;
   jsWrtTest.setFileName(outjsfilename);
   printf("init data...\n");
   jsWrtTest.Init(&jsReadTest);


       int dim = jsReadTest.getNDim();
   	   int NSamples = jsReadTest.getAxisLen(0);
   	   int NOffsets  = jsReadTest.getAxisLen(1);
   	   int NXlines = dim>2 ? jsReadTest.getAxisLen(2) : 0;
   	   int NInlines = dim>3 ? jsReadTest.getAxisLen(3) : 0;
   	   double off0 = jsReadTest.getAxisPhysicalOrigin(1);
   	   double doff = jsReadTest.getAxisPhysicalDelta(1);
   	   int xl0 = jsReadTest.getAxisLogicalOrigin(2);
   	   int dxl = jsReadTest.getAxisLogicalDelta(2);
   	   int inl0 = jsReadTest.getAxisLogicalOrigin(3);
   	   int dinl = jsReadTest.getAxisLogicalOrigin(4);
       printf("input length : (%d,%d,%d,%d) \n", NSamples, NOffsets, NXlines, NInlines);

   	   NXlines /=2;
   	   dxl *= 2;

   	   int frameLen = NSamples*NOffsets;
   	   int traceheaderSize = jsReadTest.getNumBytesInHeader();
   	   int frameheaderSize  =  NOffsets*traceheaderSize;
   	   char *headerBuf = new char[frameheaderSize];
   	   float *gather = new float[frameLen];

       // need update 3D axis because it might be decimated, not same as reader
   	jsWrtTest.updateGridAxis(1, NOffsets, 1, NOffsets, off0, doff);
   	jsWrtTest.updateGridAxis(2, NXlines, xl0, dxl,jsReadTest.getAxisPhysicalOrigin(2), jsReadTest.getAxisPhysicalDelta(2));
       // create a new dataset on disk
   	printf("output length : (%d,%d,%d,%d) \n", NSamples, NOffsets, NXlines, NInlines);
   	jsWrtTest.writeMetaData();


       printf("Create only 1 volume out of %d \n", NInlines);
       int frame_offset = 0; //13*2
       for (int i = 0; i < NXlines; i++) {
    	   jsReadTest.readFrame(2*i+frame_offset, gather, headerBuf);
           int numLiveTraces = jsWrtTest.leftJustify(gather, headerBuf, NOffsets);
		   int ires1 = jsWrtTest.writeFrame(i, gather, headerBuf, numLiveTraces);
       }

       printf("write binary data done!\n");
       ftime(&time1);
       total = (time1.time-time0.time) +(time1.millitm-time0.millitm)/1.e3;
       std::cout << "\n Write time: " << total << " (sec)\n\n";

   return 0;
}

int main(int argc, char *argv[])
{

  writeTestbyWriter("/tmp/primary/area/proj/sub/dataTest.js");

  readVerifyTest("/tmp/primary/area/proj/sub/dataTest.js", 0);

  printf("\n\n write test done!\n\n");

  writeTestbyCopy("/tmp/primary/area/proj/sub/dataTest.js", "/tmp/primary/area/proj/sub/dataTest2.js", 0);

  readVerifyTest("/tmp/primary/area/proj/sub/dataTest2.js", 0);

  printf("\n\n copy test done!\n\n");

  writeTestbyCopy("/tmp/primary/area/proj/sub/dataTest.js", "/tmp/primary/area/proj/sub/sk@dataTest3.VID", 1);
  writeTestbyCopy("/tmp/primary/area/proj/sub/dataTest.js", "/tmp/primary/area/proj/sub/sk@dataTest3.vid", 1);

  readVerifyTest("/tmp/primary/area/proj/sub/sk@dataTest3.VID", 10);

  printf("\n\n copy and write test done!\n\n");

  writeTestbyUpdate("/tmp/primary/area/proj/sub/dataTest2.js");

  readVerifyTest("/tmp/primary/area/proj/sub/dataTest2.js", 100);

  printf("\n\n overwrite test done!\n");

  writeTestbyCopyHeader("/tmp/primary/area/proj/sub/dataTest.js", "/tmp/primary/area/proj/sub/dataTestSV.js");

  readVerifyTest("/tmp/primary/area/proj/sub/dataTestSV.js", 0);

  return 0;
}
