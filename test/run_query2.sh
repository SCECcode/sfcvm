
## run_query.sh

TOP_LOC=../dependencies/geomodelgrids-build/bin/.libs

rm -rf sfcvm_depth.in sfcvm_depth.out sfcvm_elev.in sfcvm_elev.out sfcvm_depth.surf
rm -rf sfcvm_latlon.in sfcvm_latlon.surf


cat << LL &> sfcvm_latlon.in
37.455 -121.941
37.455 -121.941 
37.479 -121.734 
37.381 -121.581
LL

#           x0            x1    elevation
#  3.745500e+01 -1.219410e+02  1.046728e+00
#  3.745500e+01 -1.219410e+02  1.046728e+00
#  3.747900e+01 -1.217340e+02  4.815300e+02
#  3.738100e+01 -1.215810e+02  5.740770e+02


cat << LLL &> sfcvm.in
37.455 -121.941 101.046728
37.455 -121.941 1.046728
37.455 -121.941 0
37.455 -121.941 -101.046728
37.455 -121.941 -1000.046728
LLL


echo "\n====== none "
${TOP_LOC}/geomodelgrids_query \
--models=../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5 \
--points=./sfcvm.in \
--output=./sfcvm3.out \
--squahs-min-elev=ELEV \
--values=Vs,Vp,density \
--points-coordsys=EPSG:4326 

cat sfcvm3.out 

