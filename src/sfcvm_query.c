/*
 * @file sfcvm_query.c
 * @brief Bootstraps the test framework for the SFCVM library.
 * @author - SCEC
 * @version 1.0
 *
 * Tests the SFCVM library by loading it and executing the code as
 * UCVM would.
 *
 */

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ucvm_model_dtypes.h"
#include "sfcvm.h"

int sfcvm_debug=1;

int _compare_double(double f1, double f2) {
  double precision = 0.00001;
  if (((f1 - precision) < f2) && ((f1 + precision) > f2)) {
    return 1;
    } else {
      return 0;
  }
}

/* Usage function */
void usage() {
  printf("     sfcvm_query - (c) SCEC\n");
  printf("Extract velocities from a SFCVM\n");
  printf("\tusage: sfcvm_query [-c ge/gd][-d][-h] < file.in\n\n");
  printf("Flags:\n");
  printf("\t-d enable debug/verbose mode\n\n");
  printf("\t-h usage\n\n");
  printf("Output format is:\n");
  printf("\tvp vs rho\n\n");
  exit (0);
}

extern char *optarg;
extern int optind, opterr, optopt;

/**
 * Initializes and SFCVM in standalone mode with ucvm plugin 
 * api.
 *
 * @param argc The number of arguments.
 * @param argv The argument strings.
 * @return A zero value indicating success.
 */
int main(int argc, char* const argv[]) {

	// Declare the structures.
	sfcvm_point_t pt;
	sfcvm_properties_t ret;
        int zmode=UCVM_COORD_GEO_DEPTH;
        int rc;
        int opt;


        /* Parse options */
        while ((opt = getopt(argc, argv, "dhc:")) != -1) {
          switch (opt) {
          case 'c':
            if (strcasecmp(optarg, "gd") == 0) {
              zmode = UCVM_COORD_GEO_DEPTH;
            } else if (strcasecmp(optarg, "ge") == 0) {
              zmode = UCVM_COORD_GEO_ELEV;
            }
            break;
          case 'd':
            sfcvm_debug=1;
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

        if(sfcvm_debug) { sfcvm_setdebug(); }

	// Initialize the model. 
        // try to use Use UCVM_INSTALL_PATH
        char *envstr=getenv("UCVM_INSTALL_PATH");
        if(envstr != NULL) {
	   assert(sfcvm_init(envstr, "sfcvm") == 0);
           } else {
	     assert(sfcvm_init("..", "sfcvm") == 0);
        }
	printf("Loaded the model successfully.\n");

        char line[1001];
        while (fgets(line, 1000, stdin) != NULL) {
           if(line[0]=='#') continue; // comment line
           if (sscanf(line,"%lf %lf %lf",
                   &pt.longitude,&pt.latitude,&pt.depth) == 3) {

//  need to convert to depth since geomodelgrid got squashing
              if(zmode == UCVM_COORD_GEO_ELEV ) {
                double elev=pt.depth;
                double surface;
                void *error_handler;
                
                rc=sfcvm_getsurface(pt.longitude, pt.latitude, &surface);
                if(rc == 1) {
                  continue;
                }			 

                // reset it
                pt.depth = surface - elev;

                if(sfcvm_debug) {
                  fprintf(stderr, "  calling : surface is %f, initial elevation %f using > depth(%f)\n",
                         surface, elev, pt.depth);
                }
              }

	      rc=sfcvm_query(&pt, &ret, 1);
              if(rc == 0) {
                printf("vs : %lf vp: %lf rho: %lf\n",ret.vs, ret.vp, ret.rho);
                } else {
                   printf("BAD: %lf %lf %lf\n",pt.longitude, pt.latitude, pt.depth);
              }
              } else {
                 break;
           }
        }

	assert(sfcvm_finalize() == 0);
	printf("Model closed successfully.\n");

	return 0;
}
