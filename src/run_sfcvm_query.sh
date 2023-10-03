#!/bin/bash
# run_sfcvm_query.sh

# Process options
FLAGS=""

# Pass along any arguments to 
while getopts 'c:ghz:' OPTION
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

echo "${SCRIPT_DIR}/sfcvm_query ${FLAGS} < ${IN_FILE} > ${OUT_FILE}" >> run.log
${SCRIPT_DIR}/sfcvm_query ${FLAGS} < ${IN_FILE} > ${OUT_FILE}

if [ $? -ne 0 ]; then
    exit 1
fi

exit 0

