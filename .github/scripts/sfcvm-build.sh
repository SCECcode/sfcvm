#!/bin/bash

tmp=`uname -s`

if [ $tmp == 'Darwin' ]; then
##for macOS, make sure have automake/aclocal
  brew install automake
  brew reinstall gcc
fi

## need to grab some python libs
python3 -m pip install scipy h5py numpy pandas pybind11

libtoolize
autoreconf -i
./configure --prefix=$UCVM_INSTALL_PATH/model/sfcvm
make
make install


