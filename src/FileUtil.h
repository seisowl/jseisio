/***************************************************************************
 * wrap file io against partial read/write completion
 ***************************************************************************/
#ifndef WRAP_FILE_H
#define WRAP_FILE_H

#include <cerrno>
#include <cstddef>
#include <type_traits>

namespace jsIO {

/*
 * wrap call to read/write/pread/pwrite(fd, buf, count, offset) to retry on incomplete reads/writes
 */
template <class F>
ssize_t wrapIOFull(F f, int fd, const void *buf0, size_t nbytes, off_t offset) {
  ssize_t nwrite = 0;
  ssize_t ret;
  char *buf = (char*)buf0;
  while(nbytes > 0) {
    if((ret = f(fd, buf, nbytes, offset)) == (ssize_t)-1) {
      perror("wrapIOFull(): ");
      return ((ssize_t)-1);
    }
    buf += ret;
    offset += ret;
    nbytes -= ret;
    nwrite += ret;
  }
  return nwrite;
}

}

#endif

