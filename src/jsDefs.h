
/***************************************************************************
                           jsDefs.h  -  description
                             -------------------
common defenitions for JavaSeisIO

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

#ifndef  JSDEFS_H
#define  JSDEFS_H

/**
 * Common definitions
*/
  
enum JS_ERROR_TYPE {JS_OK = 1, JS_FATALERROR = -1, JS_USERERROR = -2, JS_WARNING = -3}; 

#define ISNOTZERO(A) ((A)<0.f || (A)>0.f)


#endif
