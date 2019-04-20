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
#include "catalogedHdrEntry.h"

namespace jsIO {

class oJseis3D {
private:
  int _n1, _n2, _n3, _i3;
  int _io1, _io2, _io3, _inc2, _inc3;
  float _d1, _d2, _d3, _o1, _o2, _o3;
  bool _is_depth; // is sample depth or time domain?

  // float *frame;
  char *hdbuf;

  int traceheaderSize;
  jsIO::jsFileWriter jsWrt;
  jsIO::catalogedHdrEntry itrcTypeHdr, iSampleHdr, dCdpXHdr, dCdpYHdr, iInLineHdr, iXLineHdr;

public:
  oJseis3D(const char * fname, int n1, int n2, int n3, float d1 = 1.0f, float d2 = 1.0f, float d3 = 1.0f, int io1 = 0,
      float o2 = 0, float o3 = 0, int io2 = 1, int inc2 = 1, int io3 = 1, int inc3 = 1, jsIO::DataFormat dataFormat =
          jsIO::DataFormat::COMPRESSED_INT16, bool is_depth = true);
  virtual ~oJseis3D();

  int write_frame(float* data, int iframe);

};

class oJseisShots {
public:
  int _n1, _n2, _n3, _i3;
  int _io1, _io2, _io3, _inc2, _inc3;
  float _d1, _d2, _d3, _o1, _o2, _o3;

public:
  jsIO::jsFileWriter jsWrt;
  int traceheaderSize;
  jsIO::catalogedHdrEntry iSourceHdr, iChanHdr, dSouXHdr, dSouYHdr, fSouElevHdr, dRecXHdr, dRecYHdr, fRecElevHdr,
      dCdpXHdr, dCdpYHdr, fAOffsetHdr;

public:
  oJseisShots(const char * fname, bool recreate, int n1, int n2, int n3, float dt_seconds = 0.004f, float d2 = 1.0f,
      float d3 = 1.0f, int io1 = 0, float o2 = 1, float o3 = 1, int io2 = 1, int inc2 = 1, int io3 = 1, int inc3 = 1,
      jsIO::DataFormat dataFormat = jsIO::DataFormat::COMPRESSED_INT16);
  virtual ~oJseisShots();

  int write_frame(int iframe, float* data, char* hdr, int ntrc);

};

class jseisUtil {
public:
  static string JS_DIR;
  static map<string, string> LABEL2HDR;
public:
  jseisUtil();
  virtual ~jseisUtil();

  static string & js_dir();
  static string fullname(const char* fname0, string & descname);
  static int save_zxy(const char * fname, float* data, int nz, int nx, int ny, float dz = 10.0f, float dx = 50.0f,
      float dy = 50.0f, int iz0 = 0, float x0 = 0.0f, float y0 = 0.0f, int io2 = 1, int inc2 = 1, int io3 = 1,
      int inc3 = 1, jsIO::DataFormat dataFormat = jsIO::DataFormat::COMPRESSED_INT16, bool is_depth = true);

  static bool file_exists(const char * fname);
  static bool file_valid(const char * fname);
  static int read_vol(jsFileReader & js, float* data, int n1, int n2, int n3, int io2, int inc2);
  static int check_vol(jsFileReader & js, const char * fname, vector<int>& axisLens, vector<int>& logicOrigs,
      vector<int>& logicDeltas, vector<double>& physOrigs, vector<double>& physDeltas);
  static int load_vol(const char * fname, vector<float>& data, vector<int>& axisLens, vector<int>& logicOrigs,
      vector<int>& logicDeltas, vector<double>& physOrigs, vector<double>& physDeltas);
  static int load_vol(const char * fname, float* data, size_t bufsize, vector<int>& axisLens, vector<int>& logicOrigs,
      vector<int>& logicDeltas, vector<double>& physOrigs, vector<double>& physDeltas);

  static int getAxisHdrs(jsFileReader & js, vector<string>& axisHdrs);

};

}

#endif /* JSEISUTIL_H_ */
