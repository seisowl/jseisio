/*
 * Assertion.cpp
 *
 *  Created on: Aug 15, 2015
 *      Author: owl
 *      Copyright: SeisWave Corp
 */

#include "Assertion.h"

namespace jsIO {

Assertion::Assertion(bool condition, const char *file, int line, const char *format, ...) {
  if(condition) return;

  char message[MAX_MESSAGE_BYTES];
  const size_t maxPrefixBytes = MAX_MESSAGE_BYTES - 1;
  snprintf(message, maxPrefixBytes, "%s L%d: ", file, line);

  va_list arguments;
  va_start(arguments, format);
  size_t length = strlen(message);
  vsnprintf(message + length, maxPrefixBytes - length, format, arguments);
  va_end(arguments);

  fprintf(stderr, "%s\n", message);
  fflush(stderr);
  exit(-1);
}

Assertion::~Assertion() {
}

} // namespace
