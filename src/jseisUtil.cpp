/*
 * jseisUtil.cpp
 *
 */

#include "jseisUtil.h"

#include <string.h>
#include <stdexcept>
#include <assert.h>

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>

namespace jsIO {

string jseisUtil::JS_DIR;
map<string, string> jseisUtil::LABEL2HDR = { { "CROSSLINE", "XLINE_NO" }, { "INLINE", "ILINE_NO" }, { "SAIL_LINE",
    "S_LINE" }, { "TIME", "V_TIME" }, { "DEPTH", "V_DEPTH" }, { "CMP", "CDP" }, { "RECEIVER_LINE", "R_LINE" }, {
    "CHANNEL", "CHAN" }, { "RECEIVER", "REC_SLOC" }, { "OFFSET_BIN", "OFB_NO" } };

jseisUtil::jseisUtil() {
}

jseisUtil::~jseisUtil() {
}

oJseis3D::oJseis3D(const char* fname0, int n1, int n2, int n3, float d1, float d2, float d3, int io1, float o2,
    float o3, int io2, int inc2, int io3, int inc3, DataFormat dataFormat, bool is_depth) :
    _n1(n1), _n2(n2), _n3(n3), _i3(0), _io1(io1), _io2(io2), _io3(io3), _inc2(inc2), _inc3(inc3), _d1(d1), _d2(d2), _d3(
        d3), _o1(io1 * d1), _o2(o2), _o3(o3), _is_depth(is_depth), hdbuf(NULL) {
  string descname;
  string fname = jseisUtil::fullname(fname0, descname);

  jsWriterInput wrtInput;

  wrtInput.setFileName(fname);
  wrtInput.setFileDescription(descname);
  wrtInput.setMapped(true);
  wrtInput.setDiskCacheSize(4 * 1024 * 1024); //4MB
  wrtInput.setNumberOfExtents(4);

//  char* vfolder = getenv("JSEIS_VIRTUAL_FOLDER");
//  if (vfolder) wrtInput.add_secondary_path(vfolder); // WRONG, jseisIO requires exact jsFile path not the mother folder
//   wrtInput.add_secondary_path("/m/scratch/supertmp/abel/JavaSeis/VFtestsSec1/l1/l2/jsFile/");
//   wrtInput.add_secondary_path("/m/scratch/supertmp/abel/JavaSeis/VFtestsSec2/l1/l2/jsFile/");

  // Create data context
  // -------------------
  wrtInput.initData(DataType::CMP, dataFormat); //DataFormat::FLOAT, COMPRESSED_INT16, SEISPEG
//   wrtInput.setSeispegPolicy(1);//0 = FASTEST, 1 = MAX_COMPRESSION

  // Framework definition
  int numDim = 3;
  wrtInput.initGridDim(numDim);
  if (_is_depth) wrtInput.initGridAxis(0, AxisLabel::DEPTH, Units::METERS, DataDomain::DEPTH, _n1, _io1, 1, _o1, _d1);
  else wrtInput.initGridAxis(0, AxisLabel::TIME, Units::MS, DataDomain::TIME, _n1, _io1, 1, _o1, _d1);
  wrtInput.initGridAxis(1, AxisLabel::CROSSLINE, Units::M, DataDomain::SPACE, _n2, _io2, inc2, _o2, _d2);
  wrtInput.initGridAxis(2, AxisLabel::INLINE, Units::M, DataDomain::SPACE, _n3, _io3, inc3, _o3, _d3);

  //Add properties/header-words
  wrtInput.addDefaultProperties();
  //     wrtInput.addProperty("ILINE_NO");
  //     wrtInput.addProperty("XLINE_NO");

  // wrtInput.addSurveyGeom(1, 2, 3, 4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.10);
  wrtInput.addCustomProperty("Stacked", "boolean", "true");

  // Initalize jsFileWriter object with the defined data context wrtInput
  int ires;

  // printf("init data...\n");
  ires = jsWrt.Init(&wrtInput);
  if (ires != JS_OK) {
    printf("Error during initalization!\n");
    return;
  }

  // Create dataset (only meta data) using the data context
  // ------------------------------------------------------
  // printf("write meta data (xml)...\n");

  ires = jsWrt.writeMetaData();
  if (ires != JS_OK) {
    printf("Error writing meta data!\n");
    return;
  }
  // printf("write meta data done!\n");

  // Write trace and headers data
  // ----------------------------
  // printf("write binary data...\n");

  // frame = jsWrt.allocFrameBuf(); // Get directly from data context
  //i.e. allocate frame with new float[NSamples*_n2];
  //the user have to call delete[] afterwards

  hdbuf = jsWrt.allocHdrBuf(true); //alloc buffer (with new[] command) and init with SeisSpace standard values.(parameter initVals=true)
  //the user have to call delete[] afterwards

  traceheaderSize = jsWrt.getTraceHeaderSize();

  //get access to header-words
  itrcTypeHdr = jsWrt.getHdrEntry("TRC_TYPE");
  //iSampleHdr = jsWrt.getHdrEntry("TIME"); //_is_depth ? "DEPTH" : "TIME");
//   fOffsetHdr = jsWrt.getHdrEntry("OFFSET");
//   iOffsetBinHdr = jsWrt.getHdrEntry("OFB_NO");
  dCdpXHdr = jsWrt.getHdrEntry("CDP_XD");
  dCdpYHdr = jsWrt.getHdrEntry("CDP_YD");
  iInLineHdr = jsWrt.getHdrEntry("ILINE_NO");
  iXLineHdr = jsWrt.getHdrEntry("XLINE_NO");
}

oJseis3D::~oJseis3D() {
  delete[] hdbuf;
}

int oJseis3D::write_frame(float* data, int i3) {
  // int trc_type = 1; // live trace
  if (i3 >= _n3) {
    fprintf(stderr, "JS write_frame: iframe %d out of bound(%d)\n", i3, _n3);
    return JS_WARNING;
  }
  for (int i2 = 0; i2 < _n2; i2++) {
    // itrcTypeHdr.setIntVal(&hdbuf[i2 * traceheaderSize], trc_type); // default 1 is OK
    //iSampleHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io1);
    dCdpXHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o2 + i2 * _d2);
    dCdpYHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o3 + i3 * _d3);
    iXLineHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io2 + i2 * _inc2);
    iInLineHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io3 + i3 * _inc3);
  }
  int numLiveTraces = _n2; //jsWrt.leftJustify(data, hdbuf, _n2); // does not needed for all trc_type == 1
  int ires = jsWrt.writeFrame(i3, data, hdbuf, numLiveTraces);
  if (ires != numLiveTraces) {
    fprintf(stderr, "Error while writing frame # %d\n", i3);
    return JS_FATALERROR;
  }

  return JS_OK;
}

string& jseisUtil::js_dir() {
  if (JS_DIR.empty()) {
    const char * dir = getenv("PROMAX_RUNTIME_DIRECTORY");
    if (!dir) {
      char * dir2 = getenv("JSEIS_DEFAULT_DIR");
      JS_DIR = dir2 ? dir2 : "/tmp/js_dir/";
    } else {
      string str = dir;
      // std::cout << "PROMAX_RUNTIME_DIRECTORY: '" << str << "'" << std::endl;
      // typical dir: /js_dir/group@jobdeck/version_number
      size_t pos0 = str.rfind('/', str.size() - 1);
      size_t pos1 = str.rfind('/', pos0 - 1);
      size_t pos2 = str.rfind('@', pos0 - 1);
      JS_DIR = str.substr(0, pos2 == string::npos ? pos1 + 1 : pos2 + 1);
    }
    std::cout << "Setting JS_DIR to " + JS_DIR << std::endl;
  }
  return JS_DIR;
}

string jseisUtil::fullname(const char* fname0, string& descname) {
  string fname = (fname0[0] == '/') ? fname0 : jseisUtil::js_dir() + fname0;
  if (fname.rfind(".js") != fname.length() - 3) fname = fname + ".js";

  descname = fname;
  size_t pos1 = descname.rfind(".js");
  size_t pos0 = descname.rfind('/');
  descname = descname.substr(pos0 + 1, pos1 - pos0 - 1);
  if ((pos0 = descname.rfind('@')) != string::npos) descname = descname.substr(pos0 + 1); // handle groupname for JavaSeis
  pos0 = fname.rfind(descname + ".js");
  std::transform(fname.begin() + pos0, fname.end(), fname.begin() + pos0, ::tolower); // convert file name corresponding to descname to lower case

  return fname;
}

int jseisUtil::save_zxy(const char* fname, float* data, int nz, int nx, int ny, float dz, float dx, float dy, int iz0,
    float x0, float y0, int io2, int inc2, int io3, int inc3, DataFormat dataFormat, bool is_depth) {

  oJseis3D js(fname, nz, nx, ny, dz, dx, dy, iz0, x0, y0, io2, inc2, io3, inc3, dataFormat, is_depth);

  // js.jsWrt.writeTrace(0,data);
  for (int iy = 0; iy < ny; iy++) {
    js.write_frame(data + (size_t) iy * nx * nz, iy);
  }

  std::cout << "Finished writing " << fname << std::endl;

  return JS_OK;
}

bool jseisUtil::file_exists(const char* fname0) {
  string descname;
  string fname = fullname(fname0, descname);

  string metafile = fname + "/FileProperties.xml";
  std::ifstream f(metafile.c_str());
  return f.good();
}

bool jseisUtil::file_valid(const char* fname0) {
  string descname;
  string fname = fullname(fname0, descname);

  jsFileReader js;
  int ierr = js.Init(fname);
  return ierr == 1 ? true : false;
}

int jseisUtil::check_vol(jsFileReader& js, const char* fname0, vector<int>& axisLens, vector<int>& logicOrigs,
    vector<int>& logicDeltas, vector<double>& physOrigs, vector<double>& physDeltas) {
  string descname;
  string fname = fullname(fname0, descname);

  int ierr = js.Init(fname);
  if (ierr != 1) {
    printf("Error in JavaSeis file %s\n", fname.c_str());
    exit(-1);
  }

#if 0
  // version
  printf("Version: %s\n",js.getVersion().c_str());

  //Custom property
  printf("Stacked: %s\n",js.getCustomProperty("Stacked").c_str());
  printf("sourceType: %s\n",js.getCustomProperty("FieldInstruments/sourceType").c_str());

  long Ntr = js.getNtr();
  printf("Total number of traces in %s is %ld\n",fname.c_str(),Ntr);

  // Number of frames
  long numFrames = js.getNFrames();
  printf("Total number of frames in %s is %ld\n", fname.c_str(),numFrames);

  int nSamples = js.getAxisLen(0);
  int nTraces = js.getAxisLen(1);
  int frameSize = nSamples * nTraces;
#endif

  int ndim = js.getNDim();
  axisLens.resize(ndim);
  for (int i = 0; i < ndim; i++)
    axisLens[i] = js.getAxisLen(i);

  physOrigs.resize(ndim);
  physDeltas.resize(ndim);
  logicOrigs.resize(ndim);
  logicDeltas.resize(ndim);

  vector<double> physVals;
  vector<long> logicVals;
  for (int i = 0; i < ndim; i++) {
    js.getAxisPhysicalValues(i, physVals);
    physOrigs[i] = physVals[0];
    physDeltas[i] = axisLens[i] > 1 ? physVals[1] - physVals[0] : 1.0;
    js.getAxisLogicalValues(i, logicVals);
    logicOrigs[i] = logicVals[0];
    logicDeltas[i] = axisLens[i] > 1 ? logicVals[1] - logicVals[0] : 1;
  }

// printf("nDim=%d, nSamples=%d, nTracesInFrame=%d, numFrames=%ld, \n", dim, nSamples,nTraces,numFrames);
  assert(ndim <= 5); // safety, avoid future surprises

  assert(ndim == 3); // only implemented for 3D so far
  return ndim;
}

int jseisUtil::load_vol(const char* fname0, vector<float>& data, vector<int>& axisLens, vector<int>& logicOrigs,
    vector<int>& logicDeltas, vector<double>& physOrigs, vector<double>& physDeltas) {
  jsFileReader js;
  int ndim = check_vol(js, fname0, axisLens, logicOrigs, logicDeltas, physOrigs, physDeltas);

  int n1 = axisLens[0];
  int n2 = axisLens[1];
  int n3 = axisLens[2];
  printf("CDataFormats::read_vol: resizing data to %dx%dx%d\n", n1, n2, n3);
  data.resize((size_t) n1 * n2 * n3);

  read_vol(js, &data[0], n1, n2, n3, logicOrigs[1], logicDeltas[1]);

  return ndim;
}

int jseisUtil::load_vol(const char* fname0, float* data, size_t bufsize, vector<int>& axisLens, vector<int>& logicOrigs,
    vector<int>& logicDeltas, vector<double>& physOrigs, vector<double>& physDeltas) {
  jsFileReader js;
  int ndim = check_vol(js, fname0, axisLens, logicOrigs, logicDeltas, physOrigs, physDeltas);

  int n1 = axisLens[0];
  int n2 = axisLens[1];
  int n3 = axisLens[2];
  assert(bufsize >= (size_t ) n1 * n2 * n3);

  read_vol(js, data, n1, n2, n3, logicOrigs[1], logicDeltas[1]);

  return ndim;
}

int jseisUtil::read_vol(jsFileReader& js, float* data, int n1, int n2, int n3, int io2, int inc2) {
  vector<string> axisHdrs;
  getAxisHdrs(js, axisHdrs);
  catalogedHdrEntry frameHdr = js.getHdrEntry(axisHdrs[1]);

  int nbhdr = js.getNumBytesInHeader();
  size_t sizeHdrs = (size_t) n2 * nbhdr;
  size_t size2d = (size_t) n1 * n2;

  vector<float> buf(size2d);
  char * hdbuf = NULL;
  for (int i = 0; i < n3; i++) {
    int n2live = js.readFrame(i, &data[size2d * i]);

    if (n2live < n2) { // handle non-full frames and put into the right positions in array
      memcpy(&buf[0], &data[size2d * i], sizeof(float) * n1 * n2live);
      memset(&data[size2d * i], 0, sizeof(float) * size2d);
      if (hdbuf == NULL) hdbuf = new char[sizeHdrs];
      js.readFrameHeader(i, hdbuf);
      for (int ir = 0; ir < n2live; ir++) {
        int i2 = (int) nearbyintf((frameHdr.getLongVal(hdbuf + (size_t) ir * nbhdr) - io2) / (float) inc2);
        memcpy(&data[size2d * i + (size_t) i2 * n1], &buf[(size_t) ir * n1], sizeof(float) * n1);
      }
    }
  }
  delete[] hdbuf;

  return JS_OK;
}

int jseisUtil::getAxisHdrs(jsFileReader& js, vector<string>& axisHdrs) {
  int iret = JS_OK;
  int ndim = js.getNDim();
  vector<string> axisLabels;
  iret = js.getAxisLabels(axisLabels);

  axisHdrs.resize(ndim);
  for (int i = 0; i < ndim; i++) {
    string label = axisLabels[i];
    axisHdrs[i] = LABEL2HDR.count(label) ? LABEL2HDR[label] : label;
  }

  return iret;
}

oJseisShots::oJseisShots(const char* fname0, bool recreate, int n1, int n2, int n3, float dt_seconds, float d2,
    float d3, int io1, float o2, float o3, int io2, int inc2, int io3, int inc3, jsIO::DataFormat dataFormat) :
    _n1(n1), _n2(n2), _n3(n3), _i3(0), _io1(io1), _io2(io2), _io3(io3), _inc2(inc2), _inc3(inc3), _d1(
        dt_seconds * 1000), _d2(d2), _d3(d3), _o1(io1 * _d1), _o2(o2), _o3(o3) {
  string descname;
  string fname = jseisUtil::fullname(fname0, descname);

  jsWriterInput wrtInput;

  wrtInput.setFileName(fname);
  wrtInput.setFileDescription(descname);
  wrtInput.setMapped(true); // segfault when set to false
  wrtInput.setDiskCacheSize(4 * 1024 * 1024); //4MB
  wrtInput.setNumberOfExtents(4);

  // Create data context
  // -------------------
  wrtInput.initData(DataType::SOURCE, dataFormat); //DataFormat::FLOAT, COMPRESSED_INT16, SEISPEG

//   wrtInput.setSeispegPolicy(1);//0 = FASTEST, 1 = MAX_COMPRESSION

  // Framework definition
  int numDim = 3;
  wrtInput.initGridDim(numDim);
  wrtInput.initGridAxis(0, AxisLabel::TIME, Units::MS, DataDomain::TIME, _n1, _io1, 1, _o1, _d1);
  wrtInput.initGridAxis(1, AxisLabel::CHANNEL, Units::UNDEFINED, DataDomain::UNDEFINED, _n2, _io2, inc2, _o2, _d2);
  wrtInput.initGridAxis(2, AxisLabel::SOURCE, Units::UNDEFINED, DataDomain::UNDEFINED, _n3, _io3, inc3, _o3, _d3);

  //Add properties/header-words
  wrtInput.addDefaultProperties();
  wrtInput.addProperty("SOURCE", "Source number", "INTEGER", 1);
  wrtInput.addProperty("CHAN", "Recording channel number", "INTEGER", 1);
  wrtInput.addProperty("AOFFSET", "Absolute value of offset", "FLOAT", 1);
  wrtInput.addProperty("SOU_ELEV", "Source elevation", "FLOAT", 1);
  wrtInput.addProperty("REC_ELEV", "Receiver elevation", "FLOAT", 1);
  wrtInput.addProperty("SOU_XD", "Source x-coordinate", "DOUBLE", 1);
  wrtInput.addProperty("SOU_YD", "Source y-coordinate", "DOUBLE", 1);
  wrtInput.addProperty("REC_XD", "Receiver x-coordinate", "DOUBLE", 1);
  wrtInput.addProperty("REC_YD", "Receiver y-coordinate", "DOUBLE", 1);

  // wrtInput.addSurveyGeom(1, 2, 3, 4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.10);
  wrtInput.addCustomProperty("Stacked", "boolean", "false");

  // Initalize jsFileWriter object with the defined data context wrtInput
  int ires;

  // printf("init data...\n");
  ires = jsWrt.Init(&wrtInput);
  if (ires != JS_OK) {
    printf("Error during initalization!\n");
    return;
  }

  // Create dataset (only meta data) using the data context
  // ------------------------------------------------------
  // printf("write meta data (xml)...\n");

  if (recreate || !jseisUtil::file_exists(fname0)) { // TODO: if data exists and overwrite==false, check the data contexts and headers are the same!!
    ires = jsWrt.writeMetaData();
    if (ires != JS_OK) {
      printf("Error writing meta data!\n");
      return;
    }
  }
  // printf("write meta data done!\n");

  traceheaderSize = jsWrt.getTraceHeaderSize();

  iSourceHdr = jsWrt.getHdrEntry("SOURCE");
  iChanHdr = jsWrt.getHdrEntry("CHAN");
  dSouXHdr = jsWrt.getHdrEntry("SOU_XD");
  dSouYHdr = jsWrt.getHdrEntry("SOU_YD");
  fSouElevHdr = jsWrt.getHdrEntry("SOU_ELEV");
  dRecXHdr = jsWrt.getHdrEntry("REC_XD");
  dRecYHdr = jsWrt.getHdrEntry("REC_YD");
  fRecElevHdr = jsWrt.getHdrEntry("REC_ELEV");
  fAOffsetHdr = jsWrt.getHdrEntry("AOFFSET");
  dCdpXHdr = jsWrt.getHdrEntry("CDP_XD");
  dCdpYHdr = jsWrt.getHdrEntry("CDP_YD");
}

oJseisShots::~oJseisShots() {
}

int oJseisShots::write_frame(int iframe, float* data, char* hdr, int ntrc) {
  return jsWrt.writeFrame(iframe, data, hdr, ntrc);
}

}
