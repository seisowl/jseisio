/***************************************************************************
                          Parameter.cpp  -  description
                             -------------------
 
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
  
#include "Parameter.h"
 
#include "PSProLogging.h"
 
namespace jsIO
{
  DECLARE_LOGGER(ParameterLog);

  Parameter::~Parameter()
  {
    if(bvalues!=NULL) delete[]bvalues;
    if(ivalues!=NULL) delete[]ivalues;
    if(lvalues!=NULL) delete[]lvalues;
    if(fvalues!=NULL) delete[]fvalues;
    if(dvalues!=NULL) delete[]dvalues;
    if(svalues!=NULL) delete[]svalues;
  }

  Parameter::Parameter()
  {
    name="";
    units="";
//     values="";
    type=0;
    N_values=0; 
    bvalues=NULL;
    ivalues=NULL;
    lvalues=NULL;
    fvalues=NULL;
    dvalues=NULL;
    svalues=NULL;

  }

  int Parameter::Init(std::string &_name, std::string &_units, std::string &_values, int _type)
  {
    name=_name;
    units=_units;
    bool bres=setValues(_values,_type);
    if(!bres) return JS_USERERROR;
    return JS_OK;
  }

  int Parameter::Init(std::string &_name, std::string &_values, int _type)
  {
    name=_name;
    units="";
    bool bres=setValues(_values,_type);
    if(!bres) return JS_USERERROR;
    return JS_OK;
  }

  int Parameter::Init(std::string &_name, std::string &_values, std::string _stype)
  {
    name=_name;
    units="";
    setTypeAsString(_stype);
    bool bres=setValues(_values,type);
    if(!bres) return JS_USERERROR;
    return JS_OK;
  }

  std::string Parameter::saveAsXML()  const
  {
    std::string str;
    str="<par name=\""+name+"\" type=\""+getTypeAsString()+"\" > "+getValuesAsString()+" </par>\n";
    return str;
  }


  std::string Parameter::getValuesAsString() const
  {
    std::string str="\n\t";
    switch (type) {
      case BOOLEAN:
        for(int i=0;i<N_values;i++)
          if(bvalues[i])  str+="true\n\t";
        else  str+="false\n\t";
        break;
      case INT:
        for(int i=0;i<N_values;i++)
          str+=num2Str(ivalues[i])+"\n\t";
        break;
      case LONG:
        for(int i=0;i<N_values;i++)
          str+=num2Str(lvalues[i])+"\n\t";
        break;
      case FLOAT:
        for(int i=0;i<N_values;i++)
          str+=num2Str(fvalues[i])+"\n\t";
        break;
      case DOUBLE:
        for(int i=0;i<N_values;i++)
          str+=num2Str(dvalues[i])+"\n\t";
        break;
      case STRING:
        for(int i=0;i<N_values;i++)
          str+=svalues[i]+"\n\t";
        break;
    }

    if(N_values<2)
      str=str.substr(3,str.length()-2);

    return str;
  }

  std::string Parameter::getTypeAsString() const
  {
    switch (type) {
      case BOOLEAN:
        return "boolean";
      case INT:
        return "int";
      case LONG:
        return "long"; 
      case FLOAT:
        return "float"; 
      case DOUBLE:
        return "double"; 
      case STRING:
        return "string"; 
      default:
        return ""; 
    }
  }

  void Parameter::setTypeAsString(std::string stype){
    if(stype=="boolean" || stype=="bool")
      type=BOOLEAN;
    else if(stype=="int")
      type=INT;
    else if(stype=="long")
      type=LONG;
    else if(stype=="float")
      type=FLOAT;
    else if(stype=="double")
      type=DOUBLE;
    else if(stype=="string")
      type=STRING;
    else 
      type=UNDEFINED;
  }


  bool Parameter::setValues(std::string &_values, int _type)
  {
    if(bvalues!=NULL) delete[]bvalues;
    if(ivalues!=NULL) delete[]ivalues;
    if(lvalues!=NULL) delete[]lvalues;
    if(fvalues!=NULL) delete[]fvalues;
    if(dvalues!=NULL) delete[]dvalues;
    if(svalues!=NULL) delete[]svalues;

    bvalues=NULL;
    ivalues=NULL;
    lvalues=NULL;
    fvalues=NULL;
    dvalues=NULL;
    svalues=NULL;

    if(_type==BOOLEAN){
      std::vector<std::string> vec;
      int N=str2svec(_values.c_str(), vec);
      int k=0;
      for(int i=0;i<N;i++){
        if(vec[i]!="true" && vec[i]!="false") k++;
      }  
      if(N==0 || k>0){
        ERROR_PRINTF(ParameterLog, "Type mismach: %s can not be interpreted as an array of booleans.",_values.c_str());
        type=UNDEFINED;
        N_values=0;
        return false;
      }
      size_t found;
      bvalues = new bool[N];
      for(int i=0;i<N;i++){
        found=vec[i].find("true");
        if (found!=std::string::npos) //      if(vec[i]!=" true ") 
          bvalues[i]=true;
        else 
          bvalues[i]=false;
      }   
      N_values=N;
      type=BOOLEAN;
    }
    else if(_type==INT){
      std::vector<long> vec;
      int N=str2lvec(_values.c_str(), vec);
      if(N==0){
        ERROR_PRINTF(ParameterLog, "Type mismach: %s can not be interpreted as an array of integers.",_values.c_str());
        type=UNDEFINED;
        N_values=0;
        return false;
      }
      ivalues = new int[N];
      for(int i=0;i<N;i++) ivalues[i]=(int)vec[i];
      N_values=N;
      type=INT;
    }
    else if(_type==LONG){
      std::vector<long> vec;
      int N=str2lvec(_values.c_str(), vec);
      if(N==0){
        ERROR_PRINTF(ParameterLog, "Type mismach: %s can not be interpreted as an array of integers.",_values.c_str());
        type=UNDEFINED;
        N_values=0;
        return false;
      }
      lvalues = new long[N];
      for(int i=0;i<N;i++) lvalues[i]=vec[i];
      N_values=N;
      type=LONG;
    }
    else if(_type==FLOAT){
      std::vector<double> vec;
      int N=str2dvec(_values.c_str(), vec);
      if(N==0){
        ERROR_PRINTF(ParameterLog, "Type mismach: %s can not be interpreted as an array of floats.",_values.c_str());
        type=UNDEFINED;
        N_values=0;
        return false;
      }
      fvalues = new float[N];
      for(int i=0;i<N;i++) fvalues[i]= (float) vec[i];
      N_values=N;
      type=FLOAT;
    }  
    else if(_type==DOUBLE){
      std::vector<double> vec;
      int N=str2dvec(_values.c_str(), vec);
      if(N==0){
        ERROR_PRINTF(ParameterLog, "Type mismach: %s can not be interpreted as an array of floats.",_values.c_str());
        type=UNDEFINED;
        N_values=0;
        return false;
      }
      dvalues = new double[N];
      for(int i=0;i<N;i++) dvalues[i]= (float) vec[i];
      N_values=N;
      type=DOUBLE;
    }  
    else if(_type==STRING){
      std::vector<std::string> vec;
      int N=str2svec(_values.c_str(), vec);
      if(N==0){
        ERROR_PRINTF(ParameterLog, "Type mismach: %s can not be interpreted as an array of strings.",_values.c_str());
        type=UNDEFINED;
        N_values=0;
        return false;
      }
      svalues = new std::string[N];
      for(int i=0;i<N;i++){
        svalues[i]= vec[i];
      }
      N_values=N;
      type=STRING;
    }
    else{
      type=UNDEFINED;
      return false;
    } 

    return true;
  }
 
  bool Parameter::valuesAsBooleans(bool *bval) const
  {
    if(type!=BOOLEAN) return false;
    memcpy(bval, bvalues, N_values*sizeof(bool));
    return true;
  }
  
  bool Parameter::valuesAsInts(int *ival) const
  {
    if(type!=INT) return false;
    memcpy(ival, ivalues, N_values*sizeof(int));
    return true;
  }

  bool Parameter::valuesAsLongs(long *lval) const
  {
    if(type!=LONG) return false;
    memcpy(lval, lvalues, N_values*sizeof(long));
    return true;
  }

  bool Parameter::valuesAsFloats(float *fval) const
  {
    if(type!=FLOAT) return false;
    memcpy(fval, fvalues, N_values*sizeof(float));
    return true;
  }

  bool Parameter::valuesAsDoubles(double *dval) const
  {
    if(type!=DOUBLE) return false;
    memcpy(dval, dvalues, N_values*sizeof(double));
    return true;
  }
 
  bool Parameter::valuesAsStrings(std::string *sval) const
  {
    if(type!=STRING) return false;
    for(int i=0;i<N_values;i++) 
      sval[i]=svalues[i];
    return true;
  }
}  

