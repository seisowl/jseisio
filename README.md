# jseisio
Development based on [JSeisIO 1.0](http://jseisio.com/).

# Build and install JSeisIO

Default is to build and install shared library. For administrators:
```shell
cd jseisio
mkdir build && cd build
cmake ../src -DBUILD_SHARED_LIBS=TRUE
make
sudo make install
```
For installation to user's target directory, e.g., $HOME/local/lib :
```shell
cd jseisio
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/local/lib -DBUILD_SHARED_LIBS=TRUE ../src 
make
make install
```

