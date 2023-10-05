/*
  geomodelgrids_query.c 
  Setup geomodelgrids in squashed mode
  and interactively process query 

        format: lat lon elev

  ./geomodelgrids_query -c ge < water_squash.in
  ./geomodelgrids_query -c ge < land_squash.in
*/

#include "../dependencies/geomodelgrids/libsrc/geomodelgrids/serial/cquery.h"
#include "../dependencies/geomodelgrids/libsrc/geomodelgrids/utils/cerrorhandler.h"

#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <assert.h>

/* Usage function */
void usage() {
  printf("     geomodelgrids_query - (c) SCEC\n");
  printf("Extract SFCVM velocities via geomodelgrids C api\n");
  printf("\tusage: geomodelgrids_query [-c ge/gd][-d][-h] < file.in\n\n");
  printf("Flags:\n");
  printf("\t-d enable debug/verbose mode\n\n");
  printf("\t-h usage\n\n");
  printf("Output format is:\n");
  printf("\tvp vs density\n\n");
  exit (0);
}

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char* argv[]) {

    int zmode=0;  //1 for depth, 0 for elevation
    int geomodelgrids_debug=0;
    int opt;

    /* Models to query. */
    static const size_t numModels = 1;
    static const char* const filenames[1] = {
        "../data/sfcvm/USGS_SFCVM_v21-1_detailed.h5"
    };

    static const size_t numValues = 3;
    static const char* const valueNames[3] = { "vp", "vs", "density" };

    static const char* const crs = "EPSG:4326";

    while ((opt = getopt(argc, argv, "dhc:")) != -1) {
        switch (opt) {
        case 'c':
          if (strcasecmp(optarg, "gd") == 0) {
            zmode = 1;
          } else if (strcasecmp(optarg, "ge") == 0) {
            zmode = 0;
          }
          break;
        case 'd':
          geomodelgrids_debug=1;
          break;
        case 'h':
          usage();
          exit(0);
          break;
        default: /* '?' */
          usage();
          exit(1);
        }
    }


    void* handle = geomodelgrids_squery_create();
    assert(handle);

    void* errorHandler = geomodelgrids_squery_getErrorHandler(handle);
    assert(errorHandler);
    geomodelgrids_cerrorhandler_setLogFilename(errorHandler, "geomodelgrids_error.log");

    geomodelgrids_squery_initialize(handle, filenames, numModels, valueNames, numValues, crs);

    int geo_setsquash=geomodelgrids_squery_setSquashing(handle, GEOMODELGRIDS_SQUASH_TOP_SURFACE);
    assert(!geo_setsquash);
    int geo_minquash=geomodelgrids_squery_setSquashMinElev(handle, -5000);
    assert(!geo_minquash);


    char line[1001];
    double longitude, latitude, depth, elevation, foo;
    while (fgets(line, 1000, stdin) != NULL) {
        if(line[0]=='#') continue; // comment line
        double myvalues[3];
        if (sscanf(line,"%lf %lf %lf",
                   &latitude,&longitude,&foo) == 3) {

           if(zmode) {
              depth=foo;
              elevation= 0 - depth; // down is negative in geomodelgrids
              } else {
                elevation = foo;
                depth=9999;
           }

           fprintf(stderr,"\n  FOR  for %f %f elev(%g) depth(%g)\n",longitude,latitude,elevation,depth);

           int err=geomodelgrids_squery_query(handle,myvalues,latitude,longitude,elevation);
           assert(!err);

           /* Query for elevation of top surface. */
           double surfaceElev = geomodelgrids_squery_queryTopElevation(handle,latitude,longitude);
           double topoBathyElev = geomodelgrids_squery_queryTopoBathyElevation(handle,latitude,longitude);

           fprintf(stderr,"    surfaceElev(%f) topoBathyElev(%f)\n",surfaceElev,topoBathyElev);
           fprintf(stderr,"    vp (%.6f)  vs (%g)  density (%.6f)\n",myvalues[0],myvalues[1],myvalues[2]);

           if(err) geomodelgrids_cerrorhandler_resetStatus(errorHandler);
        }
    } /* while */

    /* Destroy query object. */
    geomodelgrids_squery_destroy(&handle);
    return 0;
} /* main */
