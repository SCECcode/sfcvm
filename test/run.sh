
#TOP_LOC=/Users/mei/scec/geomodelgrids/build/bin/.libs
TOP_LOC=/Users/mei/scec/TARGET_UCVM/ucvm/work/model/sfcvm/dependencies/geomodelgrids-build/bin/.libs

rm -rf one-block-flat_latlon.out one-block-fat_utm.out
rm -rf one-block-flat_latlon.in one-block-fat_utm.in

cat << LATLONE &> one-block-flat_latlon_elev.in
37.455 -121.941 0.0
37.455 -121.941 -500.0
37.479 -121.734 -5.0e+3
37.381 -121.581 -3.0e+3
LATLONE

cat << LATLON &> one-block-flat_latlon.in
37.455 -121.941
37.455 -121.941
37.479 -121.734
37.381 -121.581
LATLON


#--models=../dependencies/geomodelgrids/tests/data/one-block-flat.h5 \
#--models=../dependencies/geomodelgrids/tests/data/one-block-topo.h5,../dependencies/geomodelgrids/tests/data/three-blocks-flat.h5 \

${TOP_LOC}/geomodelgrids_query \
--models=../dependencies/geomodelgrids/tests/data/one-block-topo.h5,../dependencies/geomodelgrids/tests/data/three-blocks-flat.h5 \
--points=./one-block-flat_latlon_elev.in \
--output=./one-block-flat_latlon_elevX.out \
--values=one,two \
--points-coordsys=EPSG:4326 

cat one-block-flat_latlon_elevX.out

${TOP_LOC}/geomodelgrids_queryelev \
--models=../dependencies/geomodelgrids/tests/data/one-block-topo.h5 \
--points=./one-block-flat_latlon.in \
--output=./one-block-flat_latlon_surf.out \
--points-coordsys=EPSG:4326 \
--surface=top_surface

cat one-block-flat_latlon_surf.out

${TOP_LOC}/geomodelgrids_queryelev \
--models=../dependencies/geomodelgrids/tests/data/one-block-topo.h5 \
--points=./one-block-flat_latlon.in \
--output=./one-block-flat_latlon_topo.out \
--points-coordsys=EPSG:4326 \
--surface=topography_bathymetry

cat one-block-flat_latlon_topo.out


cat <<  UTME &> one-block-flat_utm_elev.in
593662.64 4145875.37 0.00
593662.64 4145875.37 -500.00
611935.55 4148764.09 -5000.00
625627.70 4138083.87 -3000.00
UTME

${TOP_LOC}/geomodelgrids_query \
--models=../dependencies/geomodelgrids/tests/data/one-block-topo.h5,../dependencies/geomodelgrids/tests/data/three-blocks-flat.h5 \
--points=./one-block-flat_utm_elev.in \
--output=./one-block-flat_utm_elevX.out \
--values=one,two \
--points-coordsys=EPSG:26910 


cat one-block-flat_utm_elevX.out

