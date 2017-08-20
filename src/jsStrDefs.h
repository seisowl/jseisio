
/***************************************************************************
                           jsStrDefs.h  -  description
                             -------------------
common string definitions for JavaSeisIO

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

#ifndef  JSSTRDEFS_H
#define  JSSTRDEFS_H

#include <string>

namespace jsIO
{
  /**
   * Definitions of common strings such that file names, header-word name etc.
  */
  
  //file  names
  const std::string JS_FILE_PROPERTIES_XML = "FileProperties.xml";
  const std::string JS_FILE_PROPERTIES_OBS = "FileProperties";
  const std::string JS_FILE_STUB = "Name.properties";
  const std::string JS_TRACE_DATA = "TraceFile";
  const std::string JS_TRACE_HEADERS = "TraceHeaders";
  const std::string JS_HISTORY_XML = "History.xml";
  const std::string JS_TRACE_MAP = "TraceMap";
  const std::string JS_HAS_TRACES_FILE = "Status.properties";
  const std::string JS_TRACE_DATA_XML = "TraceFile.xml";
  const std::string JS_VIRTUAL_FOLDERS_XML = "VirtualFolders.xml";
  const std::string JS_TRACE_HEADERS_XML = "TraceHeaders.xml";
//flags
  const std::string JS_SORT_FILES = "^Sort.*$";
  const std::string JS_MODE_READ_ONLY = "r";
  const std::string JS_MODE_READ_WRITE = "rw";
//constants
  const std::string JS_EARLYVERSION1 = "2006.01";
  const std::string JS_PREXMLVERSION = "2006.2";
  const std::string JS_VERSION = "2006.3";


  enum JS_HEADER {JSHDR_UNKNOWN=-1,JSHDR_TRC_TYPE=0,JSHDR_T0=1,JSHDR_Z0=2,JSHDR_SX=3,JSHDR_SY=4,JSHDR_SZ=5,JSHDR_GX=6,
    JSHDR_GY=7,JSHDR_GZ=8, JSHDR_OFFSET=9, JSHDR_OFFSET_BIN=10, JSHDR_INLINE_NO=11, JSHDR_XLINE_NO=12, JSHDR_CDPX=13,
    JSHDR_CDPY=14, JSHDR_CDPZ=15, JSHDR_ANG_VALU=16, JSHDR_ANG_VBIN=17,JSHDR_SR_AZIM=18,JSHDR_SRAZ_BIN=19, JSHDR_TLIVE_S=20,
    JSHDR_TFULL_S=21,JSHDR_TFULL_E=22, JSHDR_TLIVE_E=23,JSHDR_LEN_SURG=24,JSHDR_TOT_STAT=25,JSHDR_NA_STAT=26,
    JSHDR_AMP_NORM=27,JSHDR_TR_FOLD=28,JSHDR_SKEWSTAT=29,JSHDR_PAD_TRC=30,JSHDR_NMO_APLD=31, 
    JSHDR_TIME=32, JSHDR_FREQUENCY=33, JSHDR_DEPTH=34, JSHDR_F0=35,
    JSHDR_OFFSET_INLINE=36, JSHDR_OFFSET_INLINE_BIN=37,JSHDR_OFFSET_XLINE=38, JSHDR_OFFSET_XLINE_BIN=39};


  static const std::string JS_HEADER_NAMES[]={"TRC_TYPE","T0","DEPTH","SOU_XD","SOU_YD","SOU_ELEV","REC_XD","REC_YD","REC_ELEV","OFFSET",
      "OFB_NO","ILINE_NO","XLINE_NO","CDP_XD", "CDP_YD", "CDP_ELEV", "ANG_VALU", "ANG_VBIN", "SR_AZIM", 
      "SRAZ_BIN", "TLIVE_S","TFULL_S","TFULL_E","TLIVE_E","LEN_SURG","TOT_STAT","NA_STAT","AMP_NORM",
      "TR_FOLD","SKEWSTAT","PAD_TRC","NMO_APLD",
      "TIME", "FREQ_IND", "DEPTH", "FREQUENCY",
      "OFFSET_IL","OFB_IL","OFFSET_XL","OFB_XL"};


  static const std::string JS_HEADER_DESC[]={
        "Trace type (data, aux,etc.)", "Start time of live samples", "Source Depth",
        "Source X coordinate (double)","Source Y coordinate (double)","Source elevation(Z coordinate)",
        "Receiver X coordinate (double)","Receiver Y coordinate (double)","Receiver elevation(Z coordinate)",
        "Signed source-receiver offset","Offset bin number","Inline number","Crossline number",
        "X coordinate of CDP (double)","Y coordinate of CDP (double)","Elevation of CDP (Z coordinate)", 
        "Angle of incidence","Angle of incidence bin","Source to receiver azimuth","Source to receiver azimuth bin",
        "Start time of live samples","Start time of full samples","End time of full samples","End time of live samples",
        "Length of surgical mute taper","Total static for this trace*","Portion of static not applied*",
        "Amplitude normalization factor","Actual trace fold","Multiplex skew static","Artificially padded trace",
        "Indicates whether or not NMO has been applied",
        "Vertical time index","Vertical frequency sample index","Vertical depth index", "Start frequency of live samples",
        "Signed source-receiver offset along crossline","Inline indexed offset bin number",
        "Signed source-receiver offset along inline","Crossline indexed offset bin number"};


  static const std::string JS_HEADER_TYPES[]={"INTEGER","FLOAT","FLOAT","DOUBLE","DOUBLE","FLOAT","DOUBLE","DOUBLE","FLOAT","FLOAT",
          "INTEGER","INTEGER","INTEGER","DOUBLE", "DOUBLE", "FLOAT", "FLOAT", "INTEGER", "FLOAT", 
          "INTEGER", "FLOAT","FLOAT","FLOAT","FLOAT","FLOAT","FLOAT","FLOAT","FLOAT",
          "FLOAT","FLOAT","INTEGER","INTEGER",
          "INTEGER", "INTEGER", "INTEGER", "FLOAT",
          "FLOAT", "INTEGER", "FLOAT", "INTEGER"};

  static const int JS_HEADER_TYPES_INT[]={3,5,5,6,6,5,6,6,5,5,3,3,3,6, 6, 5, 5, 3, 5, 3, 5,5,5,5,5,5,5,5, 5,5,3,3, 3, 3, 3, 5, 5, 3,5,3};

//correct later
  static const std::string JS_HEADER_UNITS[]={"meters","seconds","meters","meters","meters","meters","meters","meters","meters","meters",
            "meters","meters","meters","meters", "meters", "meters", "degrees", "degrees", "degrees",
            "degrees", "seconds","seconds","seconds","seconds","meters","meters","meters","meters",
            "meters","meters","meters","meters",
            "seconds","hertz","meters", "hertz",
            "meters","meters","meters","meters"};

  static const std::string JS_HEADER_DOMAINS[]={"space","time","space","space","space","space","space","space","space","space",
              "space","space","space","space", "space", "space", "incidence angle", "incidence angle", "rotation angle",
              "rotation angle", "space","space","space","space","space","space","space","space",
              "space","space","space","space",
              "time","frequency","space", "frequency",
              "space","space","space","space"};


}

#endif
