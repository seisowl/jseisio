#include "jsByteOrder.h"

namespace jsIO {
JS_BYTEORDER nativeOrder(void) {
  short int word = 0x0001;
  char *byte = (char *) &word;
  return (byte[0] ? JSIO_LITTLEENDIAN : JSIO_BIGENDIAN);
}

void endian_swap(void *a, int n, int nb) {
  char tmp[16];
  char *b = (char *) a;
  int i, j;
  char *cd = tmp;
  char *cs;

  for(i = 0; i < n; i++) {
    cs = &b[nb * i];
    for(j = 0; j < nb; j++)
      cd[j] = cs[nb - 1 - j];
    bcopy(cd, cs, nb);
  }
}
}
