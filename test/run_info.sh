
TOP_LOC=${UCVM_SRC_PATH}/work/model/sfcvm/dependencies/geomodelgrids-build/bin

echo "ONE -- TWO"
${TOP_LOC}/geomodelgrids_info \
--models=../data/sfcvm/one-block-topo.h5,../data/sfcvm/three-blocks-flat.h5 \
--all

echo "SFCVM"

${TOP_LOC}/geomodelgrids_info \
--models=../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5 \
--all
