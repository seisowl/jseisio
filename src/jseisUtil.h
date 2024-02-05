/*
 * jseisUtil.h
 *
 */

#ifndef JSEISUTIL_H_
#define JSEISUTIL_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

#include "jsFileReader.h"
#include "jsFileWriter.h"
#include "jsWriterInput.h"
#include "TraceProperties.h"
#include "catalogedHdrEntry.h"

namespace jsIO {

class oJseis3D {
private:
  int _n1, _n2, _n3, _i3;
  int _io1, _io2, _io3, _inc2, _inc3;
  float _d1, _d2, _d3, _o1, _o2, _o3, t0;
  bool _is_depth; // is sample depth or time domain?

  // float *frame;
  char *hdbuf;

  int traceheaderSize;
  jsIO::jsFileWriter jsWrt;
  jsIO::catalogedHdrEntry itrcTypeHdr, iSampleHdr, dCdpXHdr, dCdpYHdr, iInLineHdr, iXLineHdr, T0Hdr;

public:
  oJseis3D(const char *fname, int n1, int n2, int n3, float d1 = 1.0f, float d2 = 1.0f, float d3 = 1.0f, int io1 = 0, float o2 = 0,
      float o3 = 0, int io2 = 1, int inc2 = 1, int io3 = 1, int inc3 = 1, jsIO::DataFormat dataFormat = jsIO::DataFormat::COMPRESSED_INT16,
      bool is_depth = true, float t0 = 0);
  virtual ~oJseis3D();

  int write_frame(float *data, int iframe);

};

class oJseisND {

public:
  char *hdbuf = nullptr;
  int traceheaderSize = 0;
  jsIO::jsFileWriter jsWrt;

  string axisHdr1, axisHdr2, axisHdr3, axisHdr4, axisHdr5;
  jsIO::catalogedHdrEntry itrcTypeHdr, itrcHdr, ifrmHdr, ivolHdr, ihyperHdr;
  jsIO::catalogedHdrEntry dRecXHdr, dRecYHdr, dSouXHdr, dSouYHdr, dCdpXHdr, dCdpYHdr, iInLineHdr, iXLineHdr, iSourceHdr, iChanHdr,
      fSouElevHdr, fRecElevHdr, fAOffsetHdr;
  int _ndim = 3;
  int _n1 = 1, _n2 = 1, _n3 = 1, _n4 = 1, _n5 = 1;
  int _io1 = 0, _io2 = 0, _io3 = 0, _io4 = 0, _io5 = 0;
  int _inc1 = 1, _inc2 = 1, _inc3 = 1, _inc4 = 1, _inc5 = 1;
  float _d1 = 1, _d2 = 1, _d3 = 1, _d4 = 1, _d5 = 1;
  float _o1 = 0, _o2 = 0, _o3 = 0, _o4 = 0, _o5 = 0;

public:
  //constructor used by slave after the master have created the output file already
  oJseisND(string fname0);
  // constuctor
  oJseisND(string fname0, int ndim, int *lengths, int *logicalOrigins, int *logicalDeltas, double *physicalOrigins, double *physicalDeltas,
      std::vector<string> axisHdrs, DataFormat dataFormat = jsIO::DataFormat::COMPRESSED_INT16, bool is_depth = true,
      std::vector<string> *extendHdrNames = NULL, std::vector<string> *extendHdrTypes = NULL);
  oJseisND(string fname0, vector<int> lengths, vector<int> logicalOrigins, vector<int> logicalDeltas, vector<double> physicalOrigins,
      vector<double> physicalDeltas, std::vector<string> axisHdrs, DataFormat dataFormat = jsIO::DataFormat::COMPRESSED_INT16,
      bool is_depth = true, std::vector<string> *extendHdrNames = NULL, std::vector<string> *extendHdrTypes = NULL);
  virtual ~oJseisND();
  void Close();
  int write_frameHeader(char *hdr, int iframe, int ivolume = 0, int ihypercube = 0); // index start from 0
  int write_frame(float *data, int iframe, int ivolume = 0, int ihypercube = 0); // index start from 0
  int write_frame(float *data, char *hdr, int iframe, int ivolume = 0, int ihypercube = 0); // index start from 0
  int write_volume(float *data, char *hdr, int ivolume, int ihypercube = 0); // index start from 0
  int write_volume(float *data, int ivolume, int ihypercube = 0); // index start from 0
  int write_volume_reg(float *data, char *hdr, int ivolume, int ihypercube = 0); // index start from 0
  float* allocFrameBuf();
  char* allocHdrBuf(bool initVals = true);

};

class oJseisShots {
public:
  int _n1, _n2, _n3, _i3;
  int _io1, _io2, _io3, _inc2, _inc3;
  float _d1, _d2, _d3, _o1, _o2, _o3;

public:
  jsIO::jsFileWriter jsWrt;
  int traceheaderSize;
  jsIO::catalogedHdrEntry iSourceHdr, iChanHdr, dSouXHdr, dSouYHdr, fSouElevHdr, dRecXHdr, dRecYHdr, fRecElevHdr, dCdpXHdr, dCdpYHdr,
      fAOffsetHdr;

public:
  oJseisShots(const char *fname, bool recreate, int n1, int n2, int n3, float dt_seconds = 0.004f, float d2 = 1.0f, float d3 = 1.0f,
      int io1 = 0, float o2 = 1, float o3 = 1, int io2 = 1, int inc2 = 1, int io3 = 1, int inc3 = 1, jsIO::DataFormat dataFormat =
          jsIO::DataFormat::COMPRESSED_INT16);
  virtual ~oJseisShots();

  int write_frame(int iframe, float *data, char *hdr, int ntrc);

};

class jseisUtil {
public:
  static string JS_DIR;
public:
  jseisUtil();
  virtual ~jseisUtil();

  static string& js_dir();
  static string fullname(const char *fname0, string &descname);
  static int save_zxy(const char *fname, float *data, int nz, int nx, int ny, float dz = 10.0f, float dx = 50.0f, float dy = 50.0f,
      int iz0 = 0, float x0 = 0.0f, float y0 = 0.0f, int io2 = 1, int inc2 = 1, int io3 = 1, int inc3 = 1, jsIO::DataFormat dataFormat =
          jsIO::DataFormat::COMPRESSED_INT16, bool is_depth = true, float t0 = 0);
  static int save_zxy(const char *fname, vector<vector<float>> data, float dz = 10.0f, float dx = 50.0f, float dy = 50.0f, int iz0 = 0,
      float x0 = 0.0f, float y0 = 0.0f, int io2 = 1, int inc2 = 1, int io3 = 1, int inc3 = 1, jsIO::DataFormat dataFormat =
          jsIO::DataFormat::COMPRESSED_INT16, bool is_depth = true, float t0 = 0);

  static bool file_exists(const char *fname);
  static bool nfs_check(const char *fname);
  static bool file_valid(const char *fname);
  static int read_vol(jsFileReader &js, float *data, int n1, int n2, int n3, int io2, int inc2);
  static int check_vol(jsFileReader &js, const char *fname, vector<int> &axisLens, vector<int> &logicOrigs, vector<int> &logicDeltas,
      vector<double> &physOrigs, vector<double> &physDeltas);
  static int load_vol(const char *fname, vector<float> &data, vector<int> &axisLens, vector<int> &logicOrigs, vector<int> &logicDeltas,
      vector<double> &physOrigs, vector<double> &physDeltas);
  static int load_vol(const char *fname, float *data, size_t bufsize, vector<int> &axisLens, vector<int> &logicOrigs,
      vector<int> &logicDeltas, vector<double> &physOrigs, vector<double> &physDeltas);

  static int getAxisHdrs(jsFileReader &js, vector<string> &axisHdrs);

};

}

#endif /* JSEISUTIL_H_ */
