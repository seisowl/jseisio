#ifndef STRINGFUNCS_H
#define STRINGFUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <strings.h>
#include <string.h>

namespace jsIO
{
  int str2darr(const char* str, int ndim, double *outdarr);
  int str2larr(const char* str, int ndim, long *outlarr);
  int str2sarr(const char* str, int ndim, std::string *outsarr);
 
  int str2lvec(const char*  str, std::vector<long> &outlvec);
  int str2dvec(const char*  str, std::vector<double> &outdvec);
  int str2svec(const char* str, std::vector<std::string> &outsvec);

  template <typename T> std::string num2Str(T number){
    std::stringstream ss;
    ss << number;
    return ss.str();
  }

  void ltrimStr(std::string &str);
  void rtrimStr(std::string &str);
 
  void StrToUpper(std::string &str);
  void StrToLower(std::string &str);
}

#endif
