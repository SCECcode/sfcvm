# San Francisco Community Velocity Model (sfcvm)

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)


## Description

SFCVM San Francisco Community Velocity Model

## Table of Contents
1. [Software Documentation](https://github.com/SCECcode/sfcvm/wiki)
2. [Installation](#installation)
3. [Usage](#usage)
4. [Contributing](#contributing)
5. [Credits](#credit)
6. [License](#license)

## Installation
This package is intended to be installed as part of the UCVM framework,
version 23.4.0 or higher. 

This package can also be build as a standalone program

<pre>
aclocal
autoreconf -fi
automake --add-missing
./configure --prefix=/path/to/install
cd data; ./make_data_files.py 
make
make install
</pre>

## Usage

### UCVM

As part of [UCVM](https://github.com/SCECcode/ucvm) installation, use 'sfcvm' as the model.

### sfcvm_query

A command line program accepts Geographic Coordinates or UTM Zone 11 to extract velocity values
from SFCVM.

## Support
Support for SFCVM is provided by the Southern California Earthquake Center
(SCEC) Research Computing Group.  Users can report issues and feature requests 
using SFCVM's github-based issue tracking link below. Developers will also 
respond to emails sent to the SCEC software contact listed below.
1. [SFCVM Github Issue Tracker](https://github.com/SCECcode/sfcvm/issues)
2. Email Contact: software@scec.usc.edu

## Contributing
We welcome contributions to the SFCVM, please contact us at software@scec.usc.edu.

## Credits
* Brad Aagaard <baagaard@usgs.gov>

## License
This software is distributed under the BSD 3-Clause open-source license.
Please see the [LICENSE.txt](LICENSE.txt) file for more information.

