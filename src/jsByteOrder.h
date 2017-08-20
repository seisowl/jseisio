#ifndef BYTEORDER_H
#define BYTEORDER_H

#include <strings.h>

namespace jsIO
{
  enum JS_BYTEORDER { JSIO_LITTLEENDIAN, JSIO_BIGENDIAN};

  JS_BYTEORDER nativeOrder(void);
  void endian_swap(void *a, int n, int nb);
}

#endif
