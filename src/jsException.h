/***************************************************************************
 jsException.h  -  Exception classes for jseisIO
 -------------------

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

#ifndef JSEXCEPTION_H
#define JSEXCEPTION_H

#include <stdexcept>
#include <string>

namespace jsIO {

class jsException : public std::runtime_error {
public:
  explicit jsException(const std::string &msg) : std::runtime_error(msg) {}
};

class jsAssertionError : public jsException {
public:
  explicit jsAssertionError(const std::string &msg) : jsException(msg) {}
};

class jsIOError : public jsException {
public:
  explicit jsIOError(const std::string &msg) : jsException(msg) {}
};

class jsRangeError : public jsException {
public:
  explicit jsRangeError(const std::string &msg) : jsException(msg) {}
};

}

#endif
