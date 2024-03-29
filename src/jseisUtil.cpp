/*
 * jseisUtil.cpp
 *
 */

#include "jseisUtil.h"
#include "stringfuncs.h"
#include "Assertion.h"

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

jseisUtil::jseisUtil() {
}

jseisUtil::~jseisUtil() {
}

oJseis3D::oJseis3D(const char *fname0, int n1, int n2, int n3, float d1, float d2, float d3, int io1, float o2, float o3, int io2, int inc2,
    int io3, int inc3, DataFormat dataFormat, bool is_depth, float t0) : _n1(n1), _n2(n2), _n3(n3), _i3(0), _io1(io1), _io2(io2), _io3(io3), _inc2(
    inc2), _inc3(inc3), _d1(d1), _d2(d2), _d3(d3), t0(t0), _o1(io1 * d1), _o2(o2), _o3(o3), _is_depth(is_depth), hdbuf(NULL) {
  string descname;
  string fname = jseisUtil::fullname(fname0, descname);
  // fprintf(stderr,
  //         "fname = %s, n1=%d, n2=%d, n3=%d, d1=%f, d2=%f, d3=%f, io1=%d,  o2=%f, o3=%f, io2=%d, inc2=%d, io3=%d, inc3=%d, "
  //         "format=%s, is_depth=%d\n", fname.c_str(), n1, n2, n3, d1, d2, d3, io1, o2, o3, io2, inc2, io3, inc3,
  //         dataFormat.getName().c_str(), is_depth);

  jsWrt.setFileName(fname);
  jsWrt.setFileDescription(descname);
  const char *jseis_nparts = getenv("JSEIS_EXTENTS");
  int nparts = jseis_nparts ? atoi(jseis_nparts) : 1;
  jsWrt.initDataType("CMP", "FLOAT", true, nparts);
  jsWrt.initGridDim(3);
  if(_is_depth) jsWrt.initGridAxis(0, "DEPTH", "METERS", "DEPTH", _n1, _io1, 1, _o1, _d1);
  else jsWrt.initGridAxis(0, "TIME", "MILLISECONDS", "TIME", _n1, _io1, 1, _o1, _d1);
  jsWrt.initGridAxis(1, "XLINE_NO", "METERS", "SPACE", _n2, _io2, inc2, _o2, _d2);
  jsWrt.initGridAxis(2, "ILINE_NO", "METERS", "SPACE", _n3, _io3, inc3, _o3, _d3);
  jsWrt.addCustomProperty("Stacked", "boolean", "false");
  jsWrt.addSurveyGeom(_io3, _io3 + (_n3 - 1) * inc3, _n3, _io2, _io2 + (_n2 - 1) * inc2, _n2, _o2 + (_n2 - 1) * _d2, _o3, _o2, _o3, _o2,
                      _o3 + (_n3 - 1) * d3, 1);
  //
  //  int initGridAxis(int _axisInd, std::string axislabel,  std::string units,  std::string domain,
  //                         long length, long logicalOrigin, long logicalDelta,
  //                         double physicalOrigin, double physicalDelta);
  //  // Initalize jsFileWriter object with the defined data context wrtInput
  int ires;

  // printf("write meta data (xml)...\n");
  ires = jsWrt.writeMetaData();
  if(ires != JS_OK) {
    fprintf(stderr, "Error writing meta data!\n");
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
  T0Hdr = jsWrt.getHdrEntry("T0");
}

int oJseis3D::write_frame(float *data, int i3) {
  // int trc_type = 1; // live trace
  // printf("JS write_frame: iframe %d out of (%d)\n", i3, _n3);
  if(i3 >= _n3) {
    fprintf(stderr, "JS write_frame: iframe %d out of bound(%d)\n", i3, _n3);
    return JS_WARNING;
  }
  for(int i2 = 0; i2 < _n2; i2++) {
    itrcTypeHdr.setIntVal(&hdbuf[i2 * traceheaderSize], 1); // default 1 is OK
    //iSampleHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io1);
    dCdpXHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o2 + i2 * _d2);
    dCdpYHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o3 + i3 * _d3);
    iXLineHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io2 + i2 * _inc2);
    iInLineHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io3 + i3 * _inc3);
    T0Hdr.setFloatVal(&hdbuf[i2 * traceheaderSize], t0);
  }
  int numLiveTraces = _n2; //jsWrt.leftJustify(data, hdbuf, _n2); // does not needed for all trc_type == 1
  int ires = jsWrt.writeFrame(i3, data, hdbuf, numLiveTraces);
  if(ires != numLiveTraces) {
    fprintf(stderr, "Error while writing frame # %d\n", i3);
    return JS_FATALERROR;
  }

  return JS_OK;
}

oJseis3D::~oJseis3D() {
  delete[] hdbuf;
}

oJseisND::~oJseisND() {
  Close();
}

void oJseisND::Close() {
  jsWrt.Close();
  if(hdbuf != NULL) delete[] hdbuf;
  hdbuf = NULL;
}

float* oJseisND::allocFrameBuf() {
  return jsWrt.allocFrameBuf();
}

char* oJseisND::allocHdrBuf(bool initVals) {
  return jsWrt.allocHdrBuf(initVals);
}

//constructor used by slave after the master have created the output file already
oJseisND::oJseisND(string fname0) {
  string descname;
  string fname = jseisUtil::fullname(fname0.c_str(), descname);
  jsIO::jsFileReader jsRead;
  int ierr = jsRead.Init(fname);
  if(ierr != 1) {
    printf("Error in JavaSeis file %s\n", fname.c_str());
    exit(-1);
  }
  jsWrt.setFileName(fname);
  jsWrt.Init(&jsRead);
  jsWrt.Initialize();

  hdbuf = jsWrt.allocHdrBuf(true); //alloc buffer (with new[] command) and init with SeisSpace standard values.(parameter initVals=true)
  traceheaderSize = jsWrt.getTraceHeaderSize();

  _ndim = jsWrt.getNDim();
  _n1 = jsWrt.getAxisLen(0);
  _n2 = jsWrt.getAxisLen(1);
  _n3 = jsWrt.getAxisLen(2);
  _io1 = jsWrt.getAxisLogicalOrigin(0);
  _io2 = jsWrt.getAxisLogicalOrigin(1);
  _io3 = jsWrt.getAxisLogicalOrigin(2);
  _inc1 = jsWrt.getAxisLogicalDelta(0);
  _inc2 = jsWrt.getAxisLogicalDelta(1);
  _inc3 = jsWrt.getAxisLogicalDelta(0);
  _o1 = jsWrt.getAxisPhysicalOrigin(0);
  _o2 = jsWrt.getAxisPhysicalOrigin(1);
  _o3 = jsWrt.getAxisPhysicalOrigin(2);
  _d1 = jsWrt.getAxisPhysicalDelta(0);
  _d2 = jsWrt.getAxisPhysicalDelta(1);
  _d3 = jsWrt.getAxisPhysicalDelta(2);
  if(_ndim > 3) {
    _n4 = jsWrt.getAxisLen(3);
    _io4 = jsWrt.getAxisLogicalOrigin(3);
    _inc4 = jsWrt.getAxisLogicalDelta(3);
    _o4 = jsWrt.getAxisPhysicalOrigin(3);
    _d4 = jsWrt.getAxisPhysicalDelta(3);
  }
  if(_ndim > 4) {
    _n5 = jsWrt.getAxisLen(4);
    _io5 = jsWrt.getAxisLogicalOrigin(4);
    _inc5 = jsWrt.getAxisLogicalDelta(4);
    _o5 = jsWrt.getAxisPhysicalOrigin(4);
    _d5 = jsWrt.getAxisPhysicalDelta(4);
  }

  //get access to header-words
  axisHdr1 = jsWrt.getAxisHdrEntry(0).getName();
  itrcHdr = jsWrt.getAxisHdrEntry(1);
  axisHdr2 = itrcHdr.getName();
  ifrmHdr = jsWrt.getAxisHdrEntry(2);
  axisHdr3 = ifrmHdr.getName();
  if(_ndim > 3) {
    ivolHdr = jsWrt.getAxisHdrEntry(3);
    axisHdr4 = ivolHdr.getName();
  }
  if(_ndim > 4) {
    ihyperHdr = jsWrt.getAxisHdrEntry(4);
    axisHdr5 = ihyperHdr.getName();
  }

  itrcTypeHdr = jsWrt.getHdrEntry("TRC_TYPE");
  fRecElevHdr = jsWrt.getHdrEntry("REC_ELEV");
  dRecXHdr = jsWrt.getHdrEntry("REC_XD");
  dRecYHdr = jsWrt.getHdrEntry("REC_YD");
  fSouElevHdr = jsWrt.getHdrEntry("SOU_ELEV");
  dSouXHdr = jsWrt.getHdrEntry("SOU_XD");
  dSouYHdr = jsWrt.getHdrEntry("SOU_YD");
  dCdpXHdr = jsWrt.getHdrEntry("CDP_XD");
  dCdpYHdr = jsWrt.getHdrEntry("CDP_YD");
  fAOffsetHdr = jsWrt.getHdrEntry("AOFFSET");
  iInLineHdr = jsWrt.getHdrEntry("ILINE_NO");
  iXLineHdr = jsWrt.getHdrEntry("XLINE_NO");
  iSourceHdr = jsWrt.getHdrEntry("SOURCE");
  iChanHdr = jsWrt.getHdrEntry("CHAN");

  jsRead.Close();
}

// c++11 feature: delegating constructor
oJseisND::oJseisND(string fname0, vector<int> lengths, vector<int> logicalOrigins, vector<int> logicalDeltas,
    vector<double> physicalOrigins, vector<double> physicalDeltas, std::vector<string> axisHdrs, DataFormat dataFormat, bool is_depth,
    std::vector<string> *extendHdrNames, std::vector<string> *extendHdrTypes) : oJseisND(fname0, lengths.size(), &lengths[0],
                                                                                         &logicalOrigins[0], &logicalDeltas[0],
                                                                                         &physicalOrigins[0], &physicalDeltas[0], axisHdrs,
                                                                                         dataFormat, is_depth, extendHdrNames,
                                                                                         extendHdrTypes) {
}
oJseisND::oJseisND(string fname0, int ndim, int *lengths, int *logicalOrigins, int *logicalDeltas, double *physicalOrigins,
    double *physicalDeltas, std::vector<string> axisHdrs, DataFormat dataFormat, bool is_depth, std::vector<string> *extendHdrNames,
    std::vector<string> *extendHdrTypes) {

  string descname;
  string fname = jseisUtil::fullname(fname0.c_str(), descname);
  // fprintf(stderr, "fname = %s, ndim=%d, format=%s, is_depth=%d\n", fname.c_str(), ndim, dataFormat.getName().c_str(), is_depth);

  if(ndim < 3 || ndim > 5) {
    fprintf(stderr, "Error dimensions, must be 3, 4, or 5!\n");
    return;
  }

  if(!(extendHdrNames == NULL || (extendHdrNames != NULL && extendHdrTypes != NULL && (*extendHdrNames).size() == (*extendHdrTypes).size()))) {
    fprintf(stderr, "Extend header vectors not consistent !\n");
    return;
  }

  _ndim = ndim;
  _n1 = lengths[0];
  _n2 = lengths[1];
  _n3 = lengths[2];
  _io1 = logicalOrigins[0];
  _io2 = logicalOrigins[1];
  _io3 = logicalOrigins[2];
  _inc1 = logicalDeltas[0];
  _inc2 = logicalDeltas[1];
  _inc3 = logicalDeltas[2];
  _o1 = physicalOrigins[0];
  _o2 = physicalOrigins[1];
  _o3 = physicalOrigins[2];
  _d1 = physicalDeltas[0];
  _d2 = physicalDeltas[1];
  _d3 = physicalDeltas[2];
  if(_ndim > 3) {
    _n4 = lengths[3];
    _io4 = logicalOrigins[3];
    _inc4 = logicalDeltas[3];
    _o4 = physicalOrigins[3];
    _d4 = physicalDeltas[3];
  }
  if(_ndim > 4) {
    _n5 = lengths[4];
    _io5 = logicalOrigins[4];
    _inc5 = logicalDeltas[4];
    _o5 = physicalOrigins[4];
    _d5 = physicalDeltas[4];
  }

  int nExts = ((long)_n5 * _n4 * _n3 * _n2 * _n1 - 1) / 2048 / 1024 / 1024 + 1;  // 2GB per extend

  jsWrt.setFileName(fname);
  jsWrt.initDataType("CUSTOM", dataFormat.getName().c_str(), true, nExts);
  jsWrt.setFileDescription(descname);
  jsWrt.initGridDim(_ndim);

  axisHdr1 = axisHdrs[0];
  StrToUpper(axisHdr1);
  jsWrt.addProperty(axisHdr1, axisHdr1, "INTEGER", 1);
  axisHdr2 = axisHdrs[1];
  StrToUpper(axisHdr2);
  if(AxisLabel::LABEL2HDR.count(axisHdr2)) axisHdr2 = AxisLabel::LABEL2HDR[axisHdr2];
  jsWrt.addProperty(axisHdr2, axisHdr2, "INTEGER", 1);
  axisHdr3 = axisHdrs[2];
  StrToUpper(axisHdr3);
  if(AxisLabel::LABEL2HDR.count(axisHdr3)) axisHdr3 = AxisLabel::LABEL2HDR[axisHdr3];
  jsWrt.addProperty(axisHdr3, axisHdr3, "INTEGER", 1);
  if(_ndim > 3) {
    axisHdr4 = axisHdrs[3];
    StrToUpper(axisHdr4);
    if(AxisLabel::LABEL2HDR.count(axisHdr4)) axisHdr4 = AxisLabel::LABEL2HDR[axisHdr4];
    jsWrt.addProperty(axisHdr4, axisHdr4, "INTEGER", 1);
  }
  if(_ndim > 4) {
    axisHdr5 = axisHdrs[4];
    StrToUpper(axisHdr5);
    if(AxisLabel::LABEL2HDR.count(axisHdr5)) axisHdr5 = AxisLabel::LABEL2HDR[axisHdr5];
    jsWrt.addProperty(axisHdr5, axisHdr5, "INTEGER", 1);
  }

  // extend headers
  if(extendHdrNames != NULL && (*extendHdrNames).size() > 0) {
    int nhs = (*extendHdrNames).size();
    for(int i = 0; i < nhs; i++) {
      StrToUpper((*extendHdrNames)[i]);
      StrToUpper((*extendHdrTypes)[i]);
      fprintf(stderr, "Addding Header: %s, %s\n", (*extendHdrNames)[i].c_str(), (*extendHdrTypes)[i].c_str());
      jsWrt.addProperty((*extendHdrNames)[i], (*extendHdrNames)[i], (*extendHdrTypes)[i], 1);
    }
  }

  if(is_depth) jsWrt.initGridAxis(0, axisHdr1, "METERS", "DEPTH", _n1, _io1, _inc1, _o1, _d1);
  else jsWrt.initGridAxis(0, axisHdr1, "MILLISECONDS", "TIME", _n1, _io1, _inc1, _o1, _d1);
  jsWrt.initGridAxis(1, axisHdr2, "METERS", "SPACE", _n2, _io2, _inc2, _o2, _d2);
  jsWrt.initGridAxis(2, axisHdr3, "METERS", "SPACE", _n3, _io3, _inc3, _o3, _d3);
  if(_ndim > 3) jsWrt.initGridAxis(3, axisHdr4, "METERS", "SPACE", _n4, _io4, _inc4, _o4, _d4);
  if(_ndim > 4) jsWrt.initGridAxis(4, axisHdr5, "METERS", "SPACE", _n5, _io5, _inc5, _o5, _d5);
  jsWrt.addCustomProperty("Stacked", "boolean", "false");
  jsWrt.addSurveyGeom(_io3, _io3 + (_n3 - 1) * _inc3, _n3, _io2, _io2 + (_n2 - 1) * _inc2, _n2, _o2 + (_n2 - 1) * _d2, _o3, _o2, _o3, _o2,
                      _o3 + (_n3 - 1) * _d3, 1);

  // Initalize jsFileWriter object with the defined data context wrtInput
  int ires = jsWrt.writeMetaData();
  if(ires != JS_OK) {
    fprintf(stderr, "Error writing meta data!\n");
    return;
  }
  hdbuf = jsWrt.allocHdrBuf(true); //alloc buffer (with new[] command) and init with SeisSpace standard values.(parameter initVals=true)
  traceheaderSize = jsWrt.getTraceHeaderSize();
  assertion(hdbuf, "hdbuf=%p, traceheaderSize=%d", hdbuf, traceheaderSize);

  //get access to header-words
  itrcHdr = jsWrt.getHdrEntry(axisHdr2);
  ifrmHdr = jsWrt.getHdrEntry(axisHdr3);
  if(_ndim > 3) ivolHdr = jsWrt.getHdrEntry(axisHdr4);
  if(_ndim > 4) ihyperHdr = jsWrt.getHdrEntry(axisHdr5);

  itrcTypeHdr = jsWrt.getHdrEntry("TRC_TYPE");
  fRecElevHdr = jsWrt.getHdrEntry("REC_ELEV");
  dRecXHdr = jsWrt.getHdrEntry("REC_XD");
  dRecYHdr = jsWrt.getHdrEntry("REC_YD");
  fSouElevHdr = jsWrt.getHdrEntry("SOU_ELEV");
  dSouXHdr = jsWrt.getHdrEntry("SOU_XD");
  dSouYHdr = jsWrt.getHdrEntry("SOU_YD");
  dCdpXHdr = jsWrt.getHdrEntry("CDP_XD");
  dCdpYHdr = jsWrt.getHdrEntry("CDP_YD");
  fAOffsetHdr = jsWrt.getHdrEntry("AOFFSET");
  iInLineHdr = jsWrt.getHdrEntry("ILINE_NO");
  iXLineHdr = jsWrt.getHdrEntry("XLINE_NO");
  iSourceHdr = jsWrt.getHdrEntry("SOURCE");
  iChanHdr = jsWrt.getHdrEntry("CHAN");
}

// write header only
int oJseisND::write_frameHeader(char *hdr, int iframe, int ivolume, int ihypercube) { // index start from 0

  if(iframe >= _n3 || (_ndim > 3 && ivolume >= _n4) || (_ndim > 4 && ihypercube >= _n5)) {
    fprintf(stderr, "JS write_frame:(%d %d %d) out of bound(%d %d %d)\n", iframe, ivolume, ihypercube, _n3, _n4, _n5);
    return JS_WARNING;
  }

  //    fprintf(stderr, "JS write_frameHeader:(%d %d %d)\n", iframe,  ivolume, ihypercube);

  int i3 = ihypercube * _n4 * _n3 + ivolume * _n3 + iframe;
  int ires = jsWrt.writeFrameHeader(i3, hdr);
  return JS_OK;
}

int oJseisND::write_frame(float *data, char *hdr, int iframe, int ivolume, int ihypercube) { // index start from 0

  if(iframe >= _n3 || (_ndim > 3 && ivolume >= _n4) || (_ndim > 4 && ihypercube >= _n5)) {
    fprintf(stderr, "JS write_frame:(%d %d %d) out of bound(%d %d %d)\n", iframe, ivolume, ihypercube, _n3, _n4, _n5);
    return JS_WARNING;
  }

  //  fprintf(stderr, "JS write_frame:(%d %d %d)\n", iframe,  ivolume, ihypercube);

  int numLiveTraces = jsWrt.leftJustify(data, hdr, _n2); // does not needed for all trc_type == 1
  int i3 = ihypercube * _n4 * _n3 + ivolume * _n3 + iframe;
  int ires = jsWrt.writeFrame(i3, data, hdr, numLiveTraces);
  if(ires != numLiveTraces) {
    fprintf(stderr, "Error while writing frame # %d\n", i3);
    return JS_FATALERROR;
  }
  return JS_OK;
}

int oJseisND::write_frame(float *data, int iframe, int ivolume, int ihypercube) { // index start from 0

  if(iframe >= _n3 || (_ndim > 3 && ivolume >= _n4) || (_ndim > 4 && ihypercube >= _n5)) {
    fprintf(stderr, "JS write_frame:(%d %d %d) out of bound(%d %d %d)\n", iframe, ivolume, ihypercube, _n3, _n4, _n5);
    return JS_FATALERROR;
  }

  //  fprintf(stderr, "JS write_frame:(%d %d %d)\n", iframe,  ivolume, ihypercube);

  for(int i2 = 0; i2 < _n2; i2++) {
    itrcTypeHdr.setIntVal(&hdbuf[i2 * traceheaderSize], 1); // default 1 is OK
    itrcHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io2 + i2 * _inc2);
    ifrmHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io3 + iframe * _inc3);
    if(_ndim > 3) ivolHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io4 + ivolume * _inc4);
    if(_ndim > 4) ihyperHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io5 + ihypercube * _inc5);
    fRecElevHdr.setFloatVal(&hdbuf[i2 * traceheaderSize], 0);
    dRecXHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o2 + i2 * _d2);
    dRecYHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o3 + i2 * _d3);
    fSouElevHdr.setFloatVal(&hdbuf[i2 * traceheaderSize], 0);
    dSouXHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o2 + i2 * _d2);
    dSouYHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o3 + i2 * _d3);
    dCdpXHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o2 + i2 * _d2);
    dCdpYHdr.setDoubleVal(&hdbuf[i2 * traceheaderSize], _o3 + i2 * _d3);
    fAOffsetHdr.setFloatVal(&hdbuf[i2 * traceheaderSize], 0);
    iXLineHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io2 + i2 * _inc2);
    iInLineHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io3 + iframe * _inc3);
    iSourceHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io3 + iframe * _inc3);
    iChanHdr.setIntVal(&hdbuf[i2 * traceheaderSize], _io2 + i2 * _inc2);
  }

  int numLiveTraces = _n2; //jsWrt.leftJustify(data, hdbuf, _n2); // does not needed for all trc_type == 1
  int i3 = ihypercube * _n4 * _n3 + ivolume * _n3 + iframe;
  int ires = jsWrt.writeFrame(i3, data, hdbuf, numLiveTraces);
  if(ires != numLiveTraces) {
    fprintf(stderr, "Error while writing frame # %d\n", i3);
    return JS_FATALERROR;
  }

  return JS_OK;
}

int oJseisND::write_volume(float *data, char *hdr, int ivolume, int ihypercube) { // index start from 0
  for(int ifr = 0; ifr < _n3; ifr++) {
    write_frame(data + (size_t)_n1 * _n2 * ifr, hdr + (size_t)traceheaderSize * _n2 * ifr, ifr, ivolume, ihypercube);
  }
  return JS_OK;
}

int oJseisND::write_volume(float *data, int ivolume, int ihypercube) { // index start from 0
  for(int ifr = 0; ifr < _n3; ifr++) {
    write_frame(data + (size_t)_n1 * _n2 * ifr, ifr, ivolume, ihypercube);
  }
  return JS_OK;
}

int oJseisND::write_volume_reg(float *data, char *hdr, int ivolume, int ihypercube) { // index start from 0
  if((_ndim > 3 && ivolume >= _n4) || (_ndim > 4 && ihypercube >= _n5)) {
    fprintf(stderr, "JS write_volume_reg:(%d %d) out of bound(%d %d)\n", ivolume, ihypercube, _n4, _n5);
    return JS_FATALERROR;
  }
  if(!(jsWrt.getTraceFormatName() == "FLOAT")) {
    fprintf(stderr, "Must be float and full size to use write_volume_reg method!!!");
    return JS_FATALERROR;
  }

  long frameIndex = (long)ihypercube * _n4 * _n3 + ivolume * _n3;
  int ires = jsWrt.writeFrames(frameIndex, data, _n3);
  if(ires != JS_OK) {
    fprintf(stderr, "write data volume error");
    return ires;
  }

  long frameHeaderSize = jsWrt.getHdrBufSize();
  long glb_hd_offset = frameIndex * frameHeaderSize;
  ires = jsWrt.writeHeaderBuffer(glb_hd_offset, hdr, frameHeaderSize * _n3);
  if(ires != JS_OK) {
    fprintf(stderr, "write header volume error");
    return ires;
  }
  return JS_OK;
}

string& jseisUtil::js_dir() {
  if(JS_DIR.empty()) {
    const char *dir = getenv("PROMAX_RUNTIME_DIRECTORY");
    if(!dir) {
      char *dir2 = getenv("JSEIS_DEFAULT_DIR");
      char *cwd = getcwd(NULL, 0);
      JS_DIR = dir2 ? dir2 : cwd;
      free(cwd);
      if(JS_DIR.back() != '/') JS_DIR += "/";
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

string jseisUtil::fullname(const char *fname0, string &descname) {
  string fname = (fname0[0] == '/') ? fname0 : jseisUtil::js_dir() + fname0;
  if(fname.rfind('/') != fname.length() - 1 && fname.rfind(".js") != fname.length() - 3 && fname.rfind(".VID") != fname.length() - 4
      && fname.rfind(".vid") != fname.length() - 4) fname = fname + ".js"; // if ends with / then do not append .js

  descname = fname;
  size_t pos1 = descname.rfind(".");
  size_t pos0 = descname.rfind('/');
  descname = descname.substr(pos0 + 1, pos1 - pos0 - 1);
  if((pos0 = descname.rfind('@')) != string::npos) descname = descname.substr(pos0 + 1);  // handle groupname for JavaSeis

  // pos0 = fname.rfind(descname + ".");
  // std::transform(fname.begin() + pos0, fname.end(), fname.begin() + pos0, ::tolower); // convert file name corresponding to descname to lower case

  return fname;
}

int jseisUtil::save_zxy(const char *fname, float *data, int nz, int nx, int ny, float dz, float dx, float dy, int iz0, float x0, float y0,
    int io2, int inc2, int io3, int inc3, DataFormat dataFormat, bool is_depth, float t0) {

  oJseis3D js(fname, nz, nx, ny, dz, dx, dy, iz0, x0, y0, io2, inc2, io3, inc3, dataFormat, is_depth, t0);

  // js.jsWrt.writeTrace(0,data);
  for(int iy = 0; iy < ny; iy++) {
    js.write_frame(data + (size_t)iy * nx * nz, iy);
  }

  std::cout << "Finished writing " << fname << std::endl;

  return JS_OK;
}

int jseisUtil::save_zxy(const char *fname, vector<vector<float> > data, float dz, float dx, float dy, int iz0, float x0, float y0, int io2,
    int inc2, int io3, int inc3, jsIO::DataFormat dataFormat, bool is_depth, float t0) {
  if(data.empty()) return JS_WARNING;

  size_t nx = data.size(), nz = data[0].size();
  for(auto &trc : data)
    nz = std::max(nz, trc.size());
  vector<float> buf(nz * nx, 0);
  for(size_t ix = 0; ix < nx; ix++)
    memcpy(&buf[(ix * nz)], &data[ix][0], sizeof(float) * data[ix].size());
  return save_zxy(fname, &buf[0], nz, nx, 1, dz, dx, dy, iz0, x0, y0, io2, inc2, io3, inc3, dataFormat, is_depth, t0);
}

bool jseisUtil::file_exists(const char *fname0) {
  string descname;
  string fname = fullname(fname0, descname);

  string metafile = fname + "/FileProperties.xml";
  std::ifstream f(metafile.c_str());
  return f.good();
}

bool jseisUtil::nfs_check(const char *path) {
  char *pp;
  char *sp;
  DIR *dirp = NULL;
  char *copypath = strdup(path);
  pp = copypath;
  while((sp = strchr(pp, '/')) != 0) {
    if(sp != pp) {
      *sp = '\0';
      dirp = opendir(copypath);
      if(dirp == NULL) {
        // printf("empty %s\n", copypath);
        free(copypath);
        return false;
      }
      // printf("opendir %s\n", copypath);
      closedir(dirp);
      *sp = '/';
    }
    pp = sp + 1;
  }
  free(copypath);
  struct stat buffer;
  if(stat(path, &buffer) != 0) {
    printf("stat false %s\n", path);
    return false;
  }
  if(S_ISDIR(buffer.st_mode)) {
    dirp = opendir(path);
    if(dirp == NULL) return false;
    closedir(dirp);
  }
  return true;
}

bool jseisUtil::file_valid(const char *fname0) {
  string descname;
  string fname = fullname(fname0, descname);

  jsFileReader js;
  int ierr = js.Init(fname);
  return ierr == 1 ? true : false;
}

int jseisUtil::check_vol(jsFileReader &js, const char *fname0, vector<int> &axisLens, vector<int> &logicOrigs, vector<int> &logicDeltas,
    vector<double> &physOrigs, vector<double> &physDeltas) {
  string descname;
  string fname = fullname(fname0, descname);

  int ierr = js.Init(fname);
  if(ierr != 1) {
    fprintf(stderr, "Error in JavaSeis file %s\n", fname.c_str());
    exit(-1);
  }

#if 0
  // version
  printf("Version: %s\n", js.getVersion().c_str());

  //Custom property
  printf("Stacked: %s\n", js.getCustomProperty("Stacked").c_str());
  printf("sourceType: %s\n", js.getCustomProperty("FieldInstruments/sourceType").c_str());

  long Ntr = js.getNtr();
  printf("Total number of traces in %s is %ld\n", fname.c_str(), Ntr);

  // Number of frames
  long numFrames = js.getNFrames();
  printf("Total number of frames in %s is %ld\n", fname.c_str(), numFrames);

  int nSamples = js.getAxisLen(0);
  int nTraces = js.getAxisLen(1);
  int frameSize = nSamples * nTraces;
#endif

  int ndim = js.getNDim();
  axisLens.resize(ndim);
  for(int i = 0; i < ndim; i++)
    axisLens[i] = js.getAxisLen(i);

  physOrigs.resize(ndim);
  physDeltas.resize(ndim);
  logicOrigs.resize(ndim);
  logicDeltas.resize(ndim);

  vector<double> physVals;
  vector<long> logicVals;
  for(int i = 0; i < ndim; i++) {
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

int jseisUtil::load_vol(const char *fname0, vector<float> &data, vector<int> &axisLens, vector<int> &logicOrigs, vector<int> &logicDeltas,
    vector<double> &physOrigs, vector<double> &physDeltas) {
  jsFileReader js;
  int ndim = check_vol(js, fname0, axisLens, logicOrigs, logicDeltas, physOrigs, physDeltas);

  int n1 = axisLens[0];
  int n2 = axisLens[1];
  int n3 = axisLens[2];
  fprintf(stderr, "CDataFormats::read_vol: resizing data to %dx%dx%d\n", n1, n2, n3);
  data.resize((size_t)n1 * n2 * n3);

  read_vol(js, &data[0], n1, n2, n3, logicOrigs[1], logicDeltas[1]);

  return ndim;
}

int jseisUtil::load_vol(const char *fname0, float *data, size_t bufsize, vector<int> &axisLens, vector<int> &logicOrigs,
    vector<int> &logicDeltas, vector<double> &physOrigs, vector<double> &physDeltas) {
  jsFileReader js;
  int ndim = check_vol(js, fname0, axisLens, logicOrigs, logicDeltas, physOrigs, physDeltas);

  int n1 = axisLens[0];
  int n2 = axisLens[1];
  int n3 = axisLens[2];
  assert(bufsize >= (size_t ) n1 * n2 * n3);

  read_vol(js, data, n1, n2, n3, logicOrigs[1], logicDeltas[1]);

  return ndim;
}

int jseisUtil::read_vol(jsFileReader &js, float *data, int n1, int n2, int n3, int io2, int inc2) {
  vector<string> axisHdrs;
  getAxisHdrs(js, axisHdrs);
  catalogedHdrEntry frameHdr = js.getHdrEntry(axisHdrs[1]);

  int nbhdr = js.getNumBytesInHeader();
  size_t sizeHdrs = (size_t)n2 * nbhdr;
  size_t size2d = (size_t)n1 * n2;

  vector<float> buf(size2d);
  char *hdbuf = NULL;
  for(int i = 0; i < n3; i++) {
    int n2live = js.readFrame(i, &data[size2d * i]);

    if(n2live < n2) {  // handle non-full frames and put into the right positions in array
      memcpy(&buf[0], &data[size2d * i], sizeof(float) * n1 * n2live);
      memset(&data[size2d * i], 0, sizeof(float) * size2d);
      if(hdbuf == NULL) hdbuf = new char[sizeHdrs];
      js.readFrameHeader(i, hdbuf);
      for(int ir = 0; ir < n2live; ir++) {
        int i2 = (int)nearbyintf((frameHdr.getLongVal(hdbuf + (size_t)ir * nbhdr) - io2) / (float)inc2);
        memcpy(&data[size2d * i + (size_t)i2 * n1], &buf[(size_t)ir * n1], sizeof(float) * n1);
      }
    }
  }
  delete[] hdbuf;

  return JS_OK;
}

int jseisUtil::getAxisHdrs(jsFileReader &js, vector<string> &axisHdrs) {
  int iret = JS_OK;
  int ndim = js.getNDim();
  vector<string> axisLabels;
  iret = js.getAxisLabels(axisLabels);

  axisHdrs.resize(ndim);
  for(int i = 0; i < ndim; i++) {
    string label = axisLabels[i];
    axisHdrs[i] = AxisLabel::LABEL2HDR.count(label) ? AxisLabel::LABEL2HDR[label] : label;
  }

  return iret;
}

oJseisShots::oJseisShots(const char *fname0, bool recreate, int n1, int n2, int n3, float dt_seconds, float d2, float d3, int io1, float o2,
    float o3, int io2, int inc2, int io3, int inc3, jsIO::DataFormat dataFormat) : _n1(n1), _n2(n2), _n3(n3), _i3(0), _io1(io1), _io2(io2), _io3(
    io3), _inc2(inc2), _inc3(inc3), _d1(dt_seconds * 1000), _d2(d2), _d3(d3), _o1(io1 * _d1), _o2(o2), _o3(o3) {

  jsWrt.setFileName(fname0);
  const char *jseis_nparts = getenv("JSEIS_EXTENTS");
  int nparts = jseis_nparts ? atoi(jseis_nparts) : 1;
  jsWrt.initDataType("SOURCE", "FLOAT", true, nparts);
  jsWrt.initGridDim(3);
  jsWrt.initGridAxis(0, "TIME", "MILLISECONDS", "TIME", _n1, _io1, 1, _o1, _d1);
  jsWrt.initGridAxis(1, "CHANNEL", "METERS", "SPACE", _n2, _io2, inc2, _o2, _d2);
  jsWrt.initGridAxis(2, "SOURCE", "METERS", "SPACE", _n3, _io3, inc3, _o3, _d3);
  jsWrt.addCustomProperty("Stacked", "boolean", "false");
  jsWrt.addSurveyGeom(_io3, _io3 + (_n3 - 1) * inc3, _n3, _io2, _io2 + (_n2 - 1) * inc2, _n2, _o2 + (_n2 - 1) * _d2, _o3, _o2, _o3, _o2,
                      _o3 + (_n3 - 1) * d3, 1);

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

  if(recreate || !jseisUtil::file_exists(fname0)) { // TODO: if data exists and overwrite==false, check the data contexts and headers are the same!!
    ires = jsWrt.writeMetaData();
    if(ires != JS_OK) {
      fprintf(stderr, "Error writing meta data!\n");
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
int oJseisShots::write_frame(int iframe, float *data, char *hdr, int ntrc) {
  return jsWrt.writeFrame(iframe, data, hdr, ntrc);
}

}
