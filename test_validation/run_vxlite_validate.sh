#!/bin/bash
##
## validate with cvmhlabn vxlite api
## 
rm -rf validate_vxlite_bad.txt 
rm -rf validate_vxlite_good.txt

if [ ! -f ../data/cvmhlabn/CVMHB-Los-Angeles-Basin.dat ]; then 
  echo "need to retrieve CVMHB-Los-Angeles-Basin.dat first!!!"
  exit 1
fi

if [ "x$UCVM_INSTALL_PATH" != "x" ] ; then
  if [ -f $SCRIPT_DIR/../conf/ucvm_env.sh  ] ; then
    SCRIPT_DIR="$UCVM_INSTALL_PATH"/bin
    source $SCRIPT_DIR/../conf/ucvm_env.sh
    ./cvmhlabn_vxlite_validate -m ../data/cvmhlabn -f ../data/cvmhlabn/CVMHB-Los-Angeles-Basin.dat
    exit
  fi
fi

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
env DYLD_LIBRARY_PATH=${SCRIPT_DIR}/../src LD_LIBRARY_PATH=${SCRIPT_DIR}/../src ./cvmhlabn_vxlite_validate -m ../data/cvmhlabn -f ../data/cvmhlabn/CVMHB-Los-Angeles-Basin.dat
