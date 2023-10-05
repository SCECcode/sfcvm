/*
  geomodelgrids_query.c 
  Setup geomodelgrids' and interactively process query 
  longitude,latitude,depth one by one
 */

#include "../dependencies/geomodelgrids/libsrc/geomodelgrids/serial/cquery.h"
#include "../dependencies/geomodelgrids/libsrc/geomodelgrids/utils/cerrorhandler.h"

#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

int
main(int argc, char* argv[]) {

    /* Models to query. */
    static const size_t numModels = 1;
    static const char* const filenames[1] = {
        "../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5"
    };

    static const size_t numValues = 3;
    static const char* const valueNames[3] = { "vp", "vs", "density" };

    static const char* const crs = "EPSG:4326";
    static const size_t spaceDim = 3;

    void* handle = geomodelgrids_squery_create();
    assert(handle);
    int err = geomodelgrids_squery_initialize(handle, filenames, numModels, valueNames, numValues, crs);
    assert(!err);

    void* errorHandler = geomodelgrids_squery_getErrorHandler(handle);
    assert(errorHandler);
    geomodelgrids_cerrorhandler_setLogFilename(errorHandler, "error.log");

    int geo_setsquash=geomodelgrids_squery_setSquashing(handle, GEOMODELGRIDS_SQUASH_TOP_SURFACE);
    assert(!geo_setsquash);
    int geo_minquash=geomodelgrids_squery_setSquashMinElev(handle, -5000);
    assert(!geo_minsquash);


    char line[1001];
    double longitude, latitude, depth;
    while (fgets(line, 1000, stdin) != NULL) {
        if(line[0]=='#') continue; // comment line
        if (sscanf(line,"%lf %lf %lf",
                   &longitude,&latitude,&depth) == 3) {

          double elevation= 0 - depth; // down is negative in geomodelgrids
          int err = geomodelgrids_squery_query(handle, values, latitude, longitude, elevation);

           /* Query for elevation of top surface. */
           double surfaceElev = geomodelgrids_squery_queryTopElevation(query, latitude, longitude);
             { fprintf(stderr,"     surfaceElev(%lf) --  for %lf %lf\n", surfaceElev, latitude, longitude); }
           double topoBathyElev = geomodelgrids_squery_topoBathyElevation(query, latitude, longitude);
             { fprintf(stderr,"     topoBathyElev(%lf) --  for %lf %lf\n", surfaceElev, latitude, longitude); }
        }
    } /* while */

    /* Destroy query object. */
    geomodelgrids_squery_destroy(&handle);
    return 0;
} /* main */
