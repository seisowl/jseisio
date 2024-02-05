#include <stdio.h>
#include <iostream>

#include <sys/timeb.h>
#include <pthread.h> //for thread-parallel reading 

#include "jsFileReader.h"
#include "TraceProperties.h"
#include "string.h"

using namespace jsIO;

void printDataInfo(const jsFileReader &jsTest);

typedef struct {
  jsFileReader *Object;
  char *rawframe;
  int numLiveTraces;
  float *frame;
  char *headbuf;
  bool active;
  int rank;
  int retval;
  int tid;
} thread_parm_t;

static void *uncompress_ThreadStartup(void *_parm) {
  thread_parm_t *parm = (thread_parm_t *) _parm;
  jsFileReader *pReader = parm->Object;
  char *rawframe = parm->rawframe;
  int numLiveTraces = parm->numLiveTraces;
  float *frame = parm->frame;
  char *headbuf = parm->headbuf;
  int iThread = parm->tid;

  parm->retval = pReader->uncompressRawFrame(rawframe, numLiveTraces, iThread, frame, headbuf);
  return NULL;
}

void readMultiThreaded(const std::string &jsfilename, const int NThreads, const std::string &outfilename);
void readTest(const std::string &jsfilename, const std::string &outfilename);

/*
 This test program reads a dataset in JavaSeis format and writes the float traces in a raw binary file.
 There are two example functions, one for normal (single threaded) read and the second for multithreaded read.
 */
int main(int argc, char *argv[]) {

  //    std::string jsfilename =  "/m/scratch/supertmp/abel/JavaSeis/data.js";
  std::string jsfilename = "";
  std::string outfname = "";
  if(argc < 3) {
    printf("Error: please enter input and output file names\n For example: reader ../test.js testOut.b\n");
    exit(-1);
  }

  jsfilename.assign(argv[1]);
  outfname.assign(argv[2]);

  //normal read test
  readTest(jsfilename, outfname);

  //  multithreded read test
  //    int numThreads = 4;
  //    readMultiThreaded(jsfilename, numThreads, outfname);

  return 0;
}

void readTest(const std::string &jsfilename, const std::string &outfilename) {
  jsFileReader jsTest;
  int ierr = jsTest.Init(jsfilename);
  if(ierr != 1) {
    printf("Error in JavaSeis file %s\n", jsfilename.c_str());
    exit(-1);
  }

  // version
  printf("Version: %s\n", jsTest.getVersion().c_str());

  //Custom property
  printf("Stacked: %s\n", jsTest.getCustomProperty("Stacked").c_str());
  //    printf("sourceType: %s\n",jsTest.getCustomProperty("FieldInstruments/sourceType").c_str());

  long Ntr = jsTest.getNtr();
  printf("Total number of traces in %s is %ld\n", jsfilename.c_str(), Ntr);

  // Number of frames
  long numFrames = jsTest.getNFrames();
  printf("Total number of frames in %s is %ld\n", jsfilename.c_str(), numFrames);

  int nSamples = jsTest.getAxisLen(0);
  int nTraces = jsTest.getAxisLen(1);
  int frameLen = nSamples * nTraces;

  int dim = jsTest.getNDim();

  printf("nDim=%d, nSamples=%d, nTracesInFrame=%d, numFrames=%ld, \n", dim, nSamples, nTraces, numFrames);

  //    printDataInfo(jsTest); //print some info

  float *gather = new float[frameLen];

  int traceheaderSize = jsTest.getNumBytesInHeader();
  int frameheaderSize = nTraces * traceheaderSize;
  char *headerBuf = new char[frameheaderSize];

  catalogedHdrEntry hdrSouX = jsTest.getHdrEntry("CDP_XD");
  catalogedHdrEntry hdrSouY = jsTest.getHdrEntry("CDP_YD");
  catalogedHdrEntry hdrRecX = jsTest.getHdrEntry("CDP_XD");
  catalogedHdrEntry hdrRecY = jsTest.getHdrEntry("CDP_YD");
  catalogedHdrEntry hdrIL = jsTest.getHdrEntry("ILINE_NO");
  catalogedHdrEntry hdrXL = jsTest.getHdrEntry("XLINE_NO");
  catalogedHdrEntry hdrType = jsTest.getHdrEntry("TRC_TYPE");
  catalogedHdrEntry hdrOffset = jsTest.getHdrEntry("OFFSET");

  printf("Header hdrSouX details:\n");
  printf("  Name = %s\n", hdrSouX.getName().c_str());
  printf("  Description = %s\n", hdrSouX.getDescription().c_str());
  printf("  Index = %d\n", hdrSouX.getOffset());
  printf("  Dimension = %d\n", hdrSouX.getCount());
  printf("  Bytes = %d\n", hdrSouX.getByteCount());
  printf("  Format = %s\n", hdrSouX.getFormatAsStr().c_str());

  // Read all frames
  printf("\nReading %s\n", jsfilename.c_str());
  long nTracesTotal = 0;
  int nTracesRead;

  float sou_x, sou_y, rec_x, rec_y;
  float fOffset;
  int il, xl;
  int iType;

  std::ofstream outfile(outfilename.c_str(), std::ifstream::out);
  printf("outfile : %s\n", outfilename.c_str());

  timeb time0, time1;
  ftime(&time0);

  for(int i = 0; i < 5/*numFrames*/; i++) {
    nTracesRead = jsTest.readFrame(i, gather, headerBuf);
    //        nTracesRead = jsTest.readFrame(i, gather);
    if(nTracesRead < 0) {
      printf("Error while trying to read frame #%d\n", i);
      break;
    }
    printf("Number of live traces in frame = %d\n", nTracesRead);
    // write the gather outfile
    outfile.write((char *) gather, nSamples * nTracesRead * sizeof(float));

    //print some header info
    for(int j = 0; j < nTracesRead; j++) {
      sou_x = hdrSouX.getDoubleVal(&headerBuf[j * traceheaderSize]);
      sou_y = hdrSouY.getDoubleVal(&headerBuf[j * traceheaderSize]);
      rec_x = hdrRecX.getDoubleVal(&headerBuf[j * traceheaderSize]);
      rec_y = hdrRecY.getDoubleVal(&headerBuf[j * traceheaderSize]);
      il = hdrIL.getIntVal(&headerBuf[j * traceheaderSize]);
      xl = hdrXL.getIntVal(&headerBuf[j * traceheaderSize]);
      iType = hdrType.getIntVal(&headerBuf[j * traceheaderSize]);
      fOffset = hdrOffset.getFloatVal(&headerBuf[j * traceheaderSize]);

      printf(
        "\t%d: \tSOU_X=%3.2f, SOU_Y=%3.2f, REC_X=%3.2f, REC_Y=%3.2f, ILINE_NO=%d, XLINE_NO=%d, Offset=%g, Type=%d\n",
        j, sou_x, sou_y, rec_x, rec_y, il, xl, fOffset, iType);
    }

    nTracesTotal += nTracesRead;
  }
  printf("Read %ld traces\n", nTracesTotal);

  ftime(&time1);
  double total = (time1.time - time0.time) + (time1.millitm - time0.millitm) / 1.e3;
  std::cout << "\n read time: " << total << " (sec)\n\n";

  outfile.close();
  printf("file %s saved\n", outfilename.c_str());

  delete[] gather;
  delete[] headerBuf;
}

void readMultiThreaded(const std::string &jsfilename, const int NThreads, const std::string &outfilename) {
  printf("reading with %d thread\n", NThreads);

  jsFileReader jsTest;
  int ierr = jsTest.Init(jsfilename, NThreads);
  if(ierr != 1) {
    printf("Error in JavaSeis file %s\n", jsfilename.c_str());
    exit(-1);
  }

  long numFrames = jsTest.getNFrames();
  long nTracesTotal = 0;
  int nSamples = jsTest.getAxisLen(0);
  int nTraces = jsTest.getAxisLen(1);

  int frameLen = nSamples * nTraces;
  int frameSize = frameLen * sizeof(float);
  long frameSizeOnDisk = jsTest.getFrameSizeOnDisk();
  int headerSize = jsTest.getNumBytesInHeader();
  float *gather = new float[frameLen * NThreads];
  char *rawframes = new char[frameSize * NThreads];
  char *headerBuf = new char[headerSize * NThreads];

  std::ofstream outfile(outfilename.c_str(), std::ifstream::out);
  printf("outfile : %s\n", outfilename.c_str());

  timeb time0, time1;
  ftime(&time0);

  pthread_t thread[NThreads];
  int rc[NThreads];
  thread_parm_t *parm[NThreads];
  for(int ithread = 0; ithread < NThreads; ithread++) {
    parm[ithread] = new thread_parm_t;
  }
  int *pLiveTraces = new int[NThreads];

  unsigned long pos = 0;
  bool berr = false;
  int numFrameToRead = NThreads;
  int i = 0;
  while(i < numFrames) {
    numFrameToRead = (NThreads < numFrames - i) ? NThreads : numFrames - i; //min of NThreads and the number of the rest frames

    int ires = jsTest.readRawFrames(i, numFrameToRead, rawframes, pLiveTraces);
    if(ires != JS_OK) {
      printf("Error while trying to read frame #%d\n", i);
      break;
    }
    for(int ith = 0; ith < numFrameToRead; ith++) {
      nTracesTotal += pLiveTraces[ith];
      parm[ith]->Object = &jsTest;
      parm[ith]->rawframe = &rawframes[frameSizeOnDisk * ith];
      parm[ith]->numLiveTraces = pLiveTraces[ith];
      parm[ith]->frame = &gather[pos];
      parm[ith]->headbuf = NULL;
      parm[ith]->active = true;
      parm[ith]->tid = ith;
      pos += pLiveTraces[ith] * nSamples;
      rc[ith] = pthread_create(&thread[ith], NULL, uncompress_ThreadStartup, (void *) parm[ith]);
      if(rc[ith]) printf("Failed to create a thread.\n");
    }

    {
      //join threads
      berr = false;
      for(int ith = 0; ith < numFrameToRead; ith++) {
        //                 printf("ith=%d\n",ith);
        rc[ith] = pthread_join(thread[ith], NULL);
        if(rc[ith]) {
          printf("Failed to join to  thread , rc=%d\n", rc[ith]);
        }
        int ires = parm[ith]->retval;
        if(ires < 0) {
          berr = true;
        }
      }
      if(berr) {
        printf("********* Error during uncompression\n");
        break;
      }
    }

    outfile.write((char *) gather, pos * sizeof(float));
    pos = 0;
    i += numFrameToRead;
  }

  ftime(&time1);
  double total = (time1.time - time0.time) + (time1.millitm - time0.millitm) / 1.e3;
  std::cout << "\n read time: " << total << " (sec)\n\n";

  printf("nTracesTotal=%ld\n", nTracesTotal);

  delete[] gather;
  delete[] rawframes;
  delete[] headerBuf;
}

void printDataInfo(const jsFileReader &jsTest) {
  // Size of frame etc.
  int dim = jsTest.getNDim();
  std::vector<std::string> labels(dim), units(dim);
  std::vector<std::vector<double> > physicalValues(dim, std::vector<double>(0));
  std::vector<std::vector<long> > logicalValues(dim, std::vector<long>(0));
  std::vector<int> lengths(dim);
  jsTest.getAxisLabels(labels);
  jsTest.getAxisUnits(units);

  for(int i = 0; i < dim; i++) {
    lengths[i] = jsTest.getAxisPhysicalValues(i, physicalValues[i]);
    lengths[i] = jsTest.getAxisLogicalValues(i, logicalValues[i]);
  }

  // Logging
  printf("\n\nParadigm Reader Properties:\n");
  printf("------------------------------------------\n");

  printf("DsNuserLabel=%s\n", jsTest.getDescriptiveName().c_str());   //Descriptive Name
  printf("DsNsampleEncoding=%s\n", jsTest.getTraceFormatName().c_str());  //Data Encoding Format
  printf("DsNwordByteOrder=%s\n", jsTest.getByteOrderAsString().c_str());       //Byte Order
  printf("\tDaNisRegular=%s\n\n", jsTest.isRegular() ? "TRUE" : "FALSE");
  printf("DsNaxisOrder =");
  for(int i = 0; i < dim; i++) {
    printf(" %s", labels[i].c_str());
  }
  printf("\n");
  for(int i = 0; i < dim; i++) {
    printf("Dimension %i\n", i);
    printf("\tDaNDataType=%s,\n", labels[i].c_str());    //Axis label
    printf("\tDaNpgUnit=%s\n", units[i].c_str());     //Units
    printf("\tDaNaxisNumMax=%d\n", jsTest.getAxisLen(i)); //Max length
    printf("\tDaNaxisFirst=%4.2f\n", physicalValues[i][0]);                        //Minimum physical axis value
    printf("\tDaNaxisLast =%4.2f\n", physicalValues[i][lengths[i] - 1]);             //Maximum physical axis value
    printf("\tDaNaxisStep =%4.2f\n\n", physicalValues[i][1] - physicalValues[i][0]); //Physical axis delta (needs calculating)

    printf("\tDaNaxisNumFirst=%ld\n", logicalValues[i][0]);            //Minimum logical axis value
    printf("\tDaNaxisNumStep =%ld\n", logicalValues[i][1] - logicalValues[i][0]); //Logical axis delta (needs calculating)
    printf("\n");
  }

  printf("Sample Values\n");
  printf("\tDaNDataType=%s,\n", labels[0].c_str());     //Axis label
  printf("\tDaNpgUnit  =%s\n", units[0].c_str());      //Units
  printf("\tDaNminVal  =%4.2f\n", physicalValues[0][0]);            //Minimum physical axis value
  printf("\tDaNmaxVal  =%4.2f\n", physicalValues[0][lengths[0] - 1]); //Maximum physical axis value
  printf("\n");

  printf("------------------------------------------\n\n");
}
