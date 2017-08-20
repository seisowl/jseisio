/***************************************************************************
                          CustomProperties.h  -  description
                             -------------------
    copyright            : (C) 2012-2012 Fraunhofer ITWM

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

#ifndef CUSTOMPROPERTIES_H
#define CUSTOMPROPERTIES_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "xmlreader.h"

#include "stringfuncs.h"
#include "jsDefs.h"

#include <stdexcept>

namespace jsIO
{

class SurveyGeometry { 
  public:   
   SurveyGeometry():minILine(0),
   		    maxILine(-1),
                    minXLine(0),
                    maxXLine(-1),
                    xILine1End(0),
                    yILine1End(0),
                    xILine1Start(0),
                    yILine1Start(0),
                    xXLine1End(0),
                    yXLine1End(0){}
    void setGeom(int i1, int i2, int i3, int i4,
    		  float f1, float f2, float f3, float f4, float f5, float f6)
    {
                 	minILine=i1;
                 	maxILine=i2;
                 	minXLine=i3;
                 	maxXLine=i4;
     	   		xILine1End=f1;
     	    		yILine1End=f2;
    	    		xILine1Start=f3; 
    	    		yILine1Start=f4; 
     		    	xXLine1End=f5;
    	            	yXLine1End=f6;

    }
    void getGeom(int &i1, int &i2, int &i3, int &i4,
    		  float &f1, float &f2, float &f3, float &f4, float &f5, float &f6)
   {
                 	i1=minILine;
                 	i2=maxILine;
                 	i3=minXLine;
                 	i4=maxXLine;
     	   		f1=xILine1End;
     	    		f2=yILine1End;
    	    		f3=xILine1Start; 
    	    		f4=yILine1Start; 
     		    	f5=xXLine1End;
    	            	f6=yXLine1End;
   }
  private:
    int minILine;
    int maxILine;
    int minXLine;
    int maxXLine;
    float xILine1End;
    float yILine1End;
    float xILine1Start;
    float yILine1Start;
    float xXLine1End;
    float yXLine1End;
};


class CustomProperties 
{
  struct Property
  {
    std::string name;
    std::string type;
    std::string value;
  };
  
 public:
   SurveyGeometry survGeom;

 public:
  ~CustomProperties();
  /** No descriptions */
   CustomProperties();
 
   void addProperty(std::string name, std::string type, std::string value);
       
   int load(std::string &XMLstring);
   int save(std::string &XMLstring);

 private:
   std::vector<Property> m_properties;
   
 protected:

};


}



#endif


