##
##  validate using UCVM api
##
rm -rf validate_api_bad.txt
rm -rf validate_api_goodr.txt

if [ ! -f ./validate_vxlite_good.txt  ]; then 
  echo "need to run run_vxlite_validate.sh first!!!"
  exit 1 
fi

if [ "x$UCVM_INSTALL_PATH" != "x" ] ; then
  if [ -f $SCRIPT_DIR/../conf/ucvm_env.sh  ] ; then
    SCRIPT_DIR="$UCVM_INSTALL_PATH"/bin
    source $SCRIPT_DIR/../conf/ucvm_env.sh
    ./cvmhlabn_api_validate -f ./validate_vxlite_good.txt
    exit
  fi
fi

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
env DYLD_LIBRARY_PATH=${SCRIPT_DIR}/../src LD_LIBRARY_PATH=${SCRIPT_DIR}/../src ./cvmhlabn_api_validate -f ./validate_vxlite_good.txt
