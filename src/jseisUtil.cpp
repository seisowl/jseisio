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

  printf("fname0 = %s, n1=%d, n2=%d, n3=%d, d1=%f, d2=%f, d3=%f, io1=%d,  o2=%f, o3=%f, io2=%d, inc2=%d, io3=%d, inc3=%d, "
  	  "format=%s, is_depth=%d\n", fname0,n1, n2,n3,d1,d2,d3,io1,o2,o3,io2,inc2,io3,inc3,dataFormat.getName().c_str(), is_depth);

  jsWrt.setFileName(fname0);
  jsWrt.initDataType("CMP", "FLOAT", true, 4);
  jsWrt.initGridDim(3);
  if (_is_depth) jsWrt.initGridAxis(0, "DEPTH", "METERS", "DEPTH", _n1, _io1, 1, _o1, _d1);
  else jsWrt.initGridAxis(0, "TIME", "MILLISECONDS", "TIME", _n1, _io1, 1, _o1, _d1);
  jsWrt.initGridAxis(1, "CROSSLINE", "METERS", "SPACE", _n2, _io2, inc2, _o2, _d2);
  jsWrt.initGridAxis(2, "INLINE", "METERS", "SPACE", _n3, _io3, inc3, _o3, _d3);
  jsWrt.addCustomProperty("Stacked", "boolean", "false");
  jsWrt.addSurveyGeom(_io3, _io3 + (_n3-1)*inc3, _io2, _io2 + (_n2-1)*inc2,	_o2+(_n2-1)*_d2, _o3, _o2, _o3, _o2, _o3 + (_n3-1)*d3);
//
//  int initGridAxis(int _axisInd, std::string axislabel,  std::string units,  std::string domain,
//                         long length, long logicalOrigin, long logicalDelta,
//                         double physicalOrigin, double physicalDelta);
//  void addSurveyGeom(int minILine, int maxILine, int minXLine, int maxXLine,
//      		  float xILine1End, float yILine1End, float xILine1Start, float yILine1Start, float xXLine1End, float yXLine1End);
//  // Initalize jsFileWriter object with the defined data context wrtInput
  int ires;

  // printf("write meta data (xml)...\n");

  ires = jsWrt.writeMetaData();
  if (ires != JS_OK) {
    printf("Error writing meta data!\n");
    return;
  }

  hdbuf = jsWrt.allocHdrBuf(true); //alloc buffer (with new[] command) and init with SeisSpace standard values.(parameter initVals=true)
  //the user have to call delete[] afterwards

  traceheaderSize = jsWrt.getTraceHeaderSize();

  //get access to header-words
  itrcTypeHdr = jsWrt.getHdrEntry("TRC_TYPE");
  //iSampleHdr = jsWrt.getHdrEntry("TIME"); //_is_depth ? "DEPTH" : "TIME");
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
  // printf("JS write_frame: iframe %d out of (%d)\n", i3, _n3);
  if (i3 >= _n3) {
    fprintf(stderr, "JS write_frame: iframe %d out of bound(%d)\n", i3, _n3);
    return JS_WARNING;
  }
  for (int i2 = 0; i2 < _n2; i2++) {
    itrcTypeHdr.setIntVal(&hdbuf[i2 * traceheaderSize], 1); // default 1 is OK
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


	jsWrt.setFileName(fname0);
	jsWrt.initDataType("SOURCE", "FLOAT", true, 4);
	jsWrt.initGridDim(3);
    jsWrt.initGridAxis(0, "TIME", "MILLISECONDS", "TIME", _n1, _io1, 1, _o1, _d1);
	jsWrt.initGridAxis(1, "CHANNEL", "METERS", "SPACE", _n2, _io2, inc2, _o2, _d2);
	jsWrt.initGridAxis(2, "SOURCE", "METERS", "SPACE", _n3, _io3, inc3, _o3, _d3);
	jsWrt.addCustomProperty("Stacked", "boolean", "false");
	jsWrt.addSurveyGeom(_io3, _io3 + (_n3-1)*inc3, _io2, _io2 + (_n2-1)*inc2,	_o2+(_n2-1)*_d2, _o3, _o2, _o3, _o2, _o3 + (_n3-1)*d3);

   //Add properties/header-words
	//jsWrt.addDefaultProperties();
	jsWrt.addProperty("SOURCE", "Source number", "INTEGER", 1);
	jsWrt.addProperty("CHAN", "Recording channel number", "INTEGER", 1);
	jsWrt.addProperty("AOFFSET", "Absolute value of offset", "FLOAT", 1);
	jsWrt.addProperty("SOU_ELEV", "Source elevation", "FLOAT", 1);
	jsWrt.addProperty("REC_ELEV", "Receiver elevation", "FLOAT", 1);
	jsWrt.addProperty("SOU_XD", "Source x-coordinate", "DOUBLE", 1);
	jsWrt.addProperty("SOU_YD", "Source y-coordinate", "DOUBLE", 1);
	jsWrt.addProperty("REC_XD", "Receiver x-coordinate", "DOUBLE", 1);
	jsWrt.addProperty("REC_YD", "Receiver y-coordinate", "DOUBLE", 1);

  // Initalize jsFileWriter object with the defined data context wrtInput
  int ires;

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

// we need fill in the header buffer before??
int oJseisShots::write_frame(int iframe, float* data, char* hdr, int ntrc) {
  return jsWrt.writeFrame(iframe, data, hdr, ntrc);
}

}
