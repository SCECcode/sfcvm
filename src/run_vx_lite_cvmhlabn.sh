#!/bin/bash
#
#   input file: inf
#   -118.938 32.97 1000
#   usage:
#   ./run_vx_lite_cvmhlabn.sh  -m ../data/cvmhlabn inf outf
#

# Process options
FLAGS=""

# Pass along any arguments to vx_lite_cvmhlabn
while getopts 'm:dhsz:' OPTION
do
  if [ "$OPTARG" != "" ]; then
      FLAGS="${FLAGS} -$OPTION $OPTARG"
  else
      FLAGS="${FLAGS} -$OPTION"
  fi
done
shift $(($OPTIND - 1))


if [ $# -lt 2 ]; then
	printf "Usage: %s: [options] <infile> <outfile>\n" $(basename $0) >&2    
    	exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
IN_FILE=$1
OUT_FILE=$2

echo "${SCRIPT_DIR}/vx_lite_cvmhlabn ${FLAGS} < ${IN_FILE} > ${OUT_FILE}" >> run.log
${SCRIPT_DIR}/vx_lite_cvmhlabn ${FLAGS} < ${IN_FILE} > ${OUT_FILE}

if [ $? -ne 0 ]; then
    exit 1
fi

exit 0
