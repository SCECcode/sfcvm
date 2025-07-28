# San Francisco Community Velocity Model (sfcvm)

<a href="https://github.com/sceccode/sfcvm.git"><img src="https://github.com/sceccode/sfcvm/wiki/images/sfcvm_logo.png"></a>

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
![GitHub repo size](https://img.shields.io/github/repo-size/sceccode/sfcvm)
[![sfcvm-ucvm-ci Actions Status](https://github.com/SCECcode/sfcvm/workflows/sfcvm-ucvm-ci/badge.svg)](https://github.com/SCECcode/sfcvm/actions)(linux only)

USGS San Francisco Bay region 3D seismic velocity model v21.1

The USGS San Francisco Bay Region three-dimensional (3D) seismic velocity model includes a detailed domain covering the greater San Francisco Bay urban region and a regional domain at a coarser resolution covering a larger region. Version 21.1 updates only the detailed domain with adjustments to the elastic properties east and north of the San Francisco Bay. There are no changes to the underlying 3D geologic model or the regional domain seismic velocity model. Version 21.1 of the detailed domain fits seamlessly inside version 21.0 of the regional domain without any jumps in elastic properties across the boundary between the two domains. The model was constructed by assigning elastic properties (density, Vp, Vs, Qp, and Qs) to grids of points based on the geologic unit and depth from the ground surface.

Aagaard, B.T., and Hirakawa, E.T., 2021, San Francisco Bay region 3D seismic velocity model v21.1: U.S. Geological Survey data release, https://doi.org/10.5066/P9TRDCHE

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
  libtoolize
  aclocal -I m4
  autoreconf -i -f
  automake --add-missing --force-missing
  ./configure --prefix=/path/to/install
  cd data; ./make_data_files.py 
  make
  make install
</pre>

### sfcvm_query

A command line program accepts Geographic Coordinates or UTM Zone 11 to extract velocity values
from SFCVM.

