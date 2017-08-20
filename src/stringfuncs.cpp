
#include "stringfuncs.h"

namespace jsIO
{
  int str2darr(const char* str, int ndim, double *outdarr)
  {
    int slen= strlen(str);
    char *cstr = new char [slen+1];
    char * pEnd = cstr;
    strcpy (pEnd, str);
    for(int i=0;i<ndim;i++){
      outdarr[i] = strtod (pEnd,&pEnd);
    }
    delete[]cstr;
    return 1;
  }

  int str2larr(const char*  str, int ndim, long *outlarr)
  {
    int slen= strlen(str);
    char *cstr = new char [slen+1];
    char * pEnd = cstr;
    strcpy (pEnd, str);
    for(int i=0;i<ndim;i++){
      outlarr[i] = strtol (pEnd,&pEnd,10);
    }
    delete[]cstr;
    return 1;
  }

  int str2sarr(const char* str, int ndim, std::string *outsarr)
  {
    std::string buf;
    buf.assign(str);
    std::stringstream ss(buf); 
    buf.clear();
    int i=0;
    while (ss >> buf && i<ndim){
      outsarr[i]=buf;
      i++;
    }
    return 1;
  }


  int str2lvec(const char*  str, std::vector<long> &outlvec)
  {
    int slen= strlen(str);
    char *cstr = new char [slen+1];
    char * pEnd = cstr;
    char * pEnd_prev;
    strcpy (pEnd, str);
    while(pEnd-cstr<slen){
      pEnd_prev=pEnd;
      long l=strtol (pEnd,&pEnd,10);
      if(pEnd_prev==pEnd) break; //it means the the conversion was not successful
      while((isspace(pEnd[0]) ||  pEnd[0]==',' ||  pEnd[0]==';' ||  pEnd[0]==':') && (pEnd-cstr)<slen) pEnd++;
      outlvec.push_back(l);
    }
    delete[]cstr;
    return  outlvec.size();
  }

  int str2dvec(const char*  str, std::vector<double> &outdvec)
  {
    outdvec.clear();
    int slen= strlen(str);
    char *cstr = new char [slen+1];
    char * pEnd = cstr;
    char * pEnd_prev;
    strcpy (pEnd, str);
    while(pEnd-cstr<slen) {
      pEnd_prev=pEnd;
      double d=strtod (pEnd,&pEnd);
      if(pEnd_prev==pEnd) break; //it means the the conversion was not successful
      while((isspace(pEnd[0]) ||  pEnd[0]==',' ||  pEnd[0]==';' ||  pEnd[0]==':') && (pEnd-cstr)<slen) pEnd++;
      outdvec.push_back(d);
    }
    delete[]cstr;
    return outdvec.size();
  }


  void StrToUpper(std::string &str)
  {
    for(int i=0;i<str.length();i++){
      if(str[i]>96 && str[i]<122) str[i] -= 32;
    }
  }

  void StrToLower(std::string &str)
  {
    for(int i=0;i<str.length();i++){
      if(str[i]>64 && str[i]<91) str[i] += 32;
    }
  }

  int str2svec(const char* cstr, std::vector<std::string> &outsvec)
  {
    std::string str;
    std::string str_part;
    size_t found;
    str.assign(cstr);
    outsvec.clear();
    ltrimStr(str);
    rtrimStr(str);

    if(str.length()==0) return 0;
    
    if(str[0]=='"' && str[str.length()-1]=='"'){
      outsvec.push_back(str);
    }else{
      found=str.find_first_of('\n');
      while (found!=std::string::npos){
        str_part = str.substr(0, found);
        ltrimStr(str_part);
        rtrimStr(str_part);
        if(str_part!=""){
          outsvec.push_back(str_part);
        }   
        str.erase(0,found+1);
        found=str.find_first_of('\n');
      }
      if(outsvec.size()==0) outsvec.push_back(str);
    }
    return outsvec.size();
  }

  void ltrimStr(std::string &str)
  {
    size_t startpos = str.find_first_not_of(" \t");
    if( std::string::npos != startpos )
    {
      str = str.substr( startpos );
    }
  }

  void rtrimStr(std::string &str)
  {
    size_t endpos = str.find_last_not_of(" \t");
    if( std::string::npos != endpos )
    {
      str = str.substr( 0, endpos+1 );
    }
  }
}

