/*
 * Assertion.h
 *
 *  Created on: Aug 15, 2015
 *      Author: owl
 *      Copyright: SeisWave Corp
 */

#ifndef JSIO_ASSERTION_H_
#define JSIO_ASSERTION_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

namespace jsIO {

class Assertion {
public:
  Assertion(bool condition, const char *file, int line, const char *format, ...);
  virtual ~Assertion();
private:
  enum {
    MAX_MESSAGE_BYTES = 1024
  };
};


} // namespace

#ifndef assertion
#define assertion(condition, format, ...) \
  jsIO::Assertion(condition, __FILE__, __LINE__, format, ##__VA_ARGS__)
#endif


#endif /* JSIO_ASSERTION_H_ */
