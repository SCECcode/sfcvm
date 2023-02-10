##
##  validate using UCVM 
##
#rm -rf validate_ucvm_bad.txt

if [ ! -f ./validate_api_good.txt  ]; then 
  echo "need to run run_api_validate.sh first!!!"
  exit 1 
fi

if [ "x${UCVM_INSTALL_PATH}" != "x" ] ; then
  SCRIPT_DIR=${UCVM_INSTALL_PATH}/bin
  source $SCRIPT_DIR/../conf/ucvm_env.sh
  echo " === UCVM query by cvmh depth ==="
  ./cvmhlabn_ucvm_validate -c ${SCRIPT_DIR}/../conf/ucvm.conf -f ./validate_api_good.txt
  echo " === UCVM query by ucvm depth ==="
  ./cvmhlabn_ucvm_rerun -c ${SCRIPT_DIR}/../conf/ucvm.conf -f ./validate_ucvm_bad.txt
  echo " === ucvm_query with cvmh depth  ==="
  ./cvmhlabn_ucvm_retry -c ${SCRIPT_DIR}/../conf/ucvm.conf -f ./ucvm_rerun_bad.txt
  else
    echo "need to have UCVM_INSTALL_PATH set!!!"
fi
