# jseisio
Development based on [JSeisIO 1.0](http://jseisio.com/).

# Build and install JSeisIO

Default is to build and install shared library. For administrators:
```shell
cd jseisio
cmake src
make
sudo make install
```
For installation to user's target directory, e.g., $HOME/local/lib :
```shell
cd jseisio
cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME/local/lib src 
make
make install
```

