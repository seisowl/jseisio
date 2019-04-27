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
For installation to user's target directory, e.g., $HOME/local/lib :
```shell
cd jseisio
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/local ../src 
make
make install
```
