#!/bin/bash
#
# run_geomodelgrids.sh
#


TOP_LOC=../dependencies/geomodelgrids-build/bin/.libs

if [ ! -f ${TOP_LOC}/geomodelgrids_query ]; then
  echo "Need to run 'make' at ../dependencies/geomodelgrids-build " 
  exit
fi

cat << A &> geomodelgrids_latlon.in
37.455 -121.941 
37.479 -121.734 
37.381 -121.581
A

cat << AA &> geomodelgrids_depth.in
37.455 -121.941 -98.533
37.455 -121.941 0.0
37.455 -121.941 1.0467
37.455 -121.941 101.467
37.455 -121.941 1001.467
AA

cat << AAA &> geomodelgrids_elev.in
37.455 -121.941 100.0
37.455 -121.941 1.046728
37.455 -121.941 0
37.455 -121.941 -100.0
37.455 -121.941 -1000.0
AAA

cat <<  U &> geomodelgrids_utm.in
593662.64 4145875.37 0.00
593662.64 4145875.37 -500.00
611935.55 4148764.09 -5000.00
625627.70 4138083.87 -3000.00
U


echo "====== squash:none "
${TOP_LOC}/geomodelgrids_query \
--models=../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5 \
--points=./geomodelgrids_depth.in \
--output=./geomodelgrids_depth.out \
--values=Vs,Vp,density \
--points-coordsys=EPSG:4326 

echo "====== squash:top_surface "
${TOP_LOC}/geomodelgrids_query \
--models=../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5 \
--points=./geomodelgrids_depth.in \
--output=./geomodelgrids_depth_2.out \
--values=Vs,Vp,density \
--squash-surface=top_surface \
--points-coordsys=EPSG:4326 

echo "====== queryelev "
${TOP_LOC}/geomodelgrids_queryelev \
--models=../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5 \
--points=./geomodelgrids_latlon.in \
--output=./geomodelgrids_latlon.surf \
--points-coordsys=EPSG:4326 \


echo "====== utm query "
${TOP_LOC}/geomodelgrids_query \
--models=../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5 \
--points=./geomodelgrids_utm.in \
--output=./geomodelgrids_utm.out \
--values=Vs,Vp,density \
--points-coordsys=EPSG:26910 

echo "====== info "
${TOP_LOC}/geomodelgrids_info \
--models=../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5 \
--all

