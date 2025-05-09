# San Francisco Community Velocity Model (sfcvm)

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
![GitHub repo size](https://img.shields.io/github/repo-size/sceccode/sfcvm)
[![sfcvm-ucvm-ci Actions Status](https://github.com/SCECcode/wfcvm/workflows/sfcvm-ucvm-ci/badge.svg)](https://github.com/SCECcode/sfcvm/actions)

USGS San Francisco Bay region 3D seismic velocity model v21.1

## Installation

This package is intended to be installed as part of the UCVM framework,
version 25.7 or higher. 

## Contact the authors

If you would like to contact the authors regarding this software,
please e-mail software@scec.org. Note this e-mail address should
be used for questions regarding the software itself (e.g. how
do I link the library properly?). Questions regarding the model's
science (e.g. on what paper is the SFCVM based?) should be directed
to the model's authors, located in the AUTHORS file.

## To build in standalone mode

To install this package on your computer, please run the following commands:

<pre>
  aclocal
  autoreconf -fi
  automake --add-missing
  ./configure --prefix=/path/to/install
  cd data; ./make_data_files.py 
  make
  make install
</pre>

### sfcvm_query

A command line program accepts Geographic Coordinates or UTM Zone 11 to extract velocity values
from SFCVM.

