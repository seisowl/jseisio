# jseisio
Development based on [JSeisIO 1.0](http://jseisio.com/).

# Build and install JSeisIO

Default is to build and install both shared and static library. For administrators:
```shell
cd jseisio
mkdir build && cd build
cmake ../src
make
sudo make install
```
To build and install complete examples and docs, replace the partial cmake line by:
```shell
cmake ..
```
## Exception Classes

The library provides exception classes in `jsException.h` that derive from `std::runtime_error`:

- `jsException` — general jseisIO error
- `jsAssertionError` — assertion failure (replaces `exit(-1)` in `Assertion.cpp`)
- `jsIOError` — file I/O initialisation failure (replaces `exit(-1)` in writer/util code)
- `jsRangeError` — out-of-range access (replaces `raise(SIGINT)` in extent lookup)

Callers should catch `jsException` (or a specific subclass) rather than relying on process termination or signal handlers.

For installation to user's target directory, e.g., $HOME/local/lib :
```shell
cd jseisio
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/local ../src 
make
make install
```
