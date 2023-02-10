/** 
    vx_lite_cvmhlabn - A command line program to extract velocity values
    from CVMHLABN . Accepts Geographic Coordinates or UTM Zone 11 
    coordinates.

    10/2011: MHS: Initial implementation derived from cvmh vx_lite.c
                  simplified  for cvmh basin models
**/


#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include "params.h"
#include "vx_sub_cvmhlabn.h"

extern int cvmhlabn_debug;

/* Usage function */
void usage() {
  printf("     vx_lite_cvmhlabn - (c) Harvard University, SCEC\n");
  printf("Extract velocities from a simple GOCAD voxet. Accepts\n");
  printf("geographic coordinates and UTM Zone 11, NAD27 coordinates in\n");
  printf("X Y Z columns. Z is expressed as elevation elevation by default.\n\n");
  printf("\tusage: vx_lite_cvmhlabn [-d] [-m dir] [-z dep/elev] < file.in\n\n");
  printf("Flags:\n");
  printf("\t-d enable debug/verbose mode.\n");
  printf("\t-m directory containing model files (default is '.').\n");
  printf("\t-z directs use of dep/elev for Z column (default is elev).\n\n");
  printf("Output format is:\n");
  printf("\tX Y Z utmX utmY elevX elevY topo mtop base moho hr/lr/cm cellX cellY cellZ tg vp vs rho\n\n");
  exit (0);
}

extern char *optarg;
extern int optind, opterr, optopt;


int main (int argc, char *argv[])
{
  vx_entry_t entry;
  char modeldir[CMLEN];
  vx_zmode_t zmode;
  int opt;
  
  zmode = VX_ZMODE_ELEV;
  strcpy(modeldir, ".");

  /* Parse options */
  while ((opt = getopt(argc, argv, "dm:z:h")) != -1) {
    switch (opt) {
    case 'd': // enable debug mode
      cvmhlabn_debug=1;
      break;
    case 'm':
      strcpy(modeldir, optarg);
      break;
    case 'z':
      if (strcasecmp(optarg, "dep") == 0) {
	zmode = VX_ZMODE_DEPTH;
      } else if (strcasecmp(optarg, "elev") == 0) {
	zmode = VX_ZMODE_ELEV;
      } else {
	fprintf(stderr, "Invalid coord type %s", optarg);
	usage();
	exit(0);
      }
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

  /* Perform setup */
  if (vx_setup(modeldir, 0) != 0) {
    fprintf(stderr, "Failed to init vx\n");
    exit(1);
  }

  /* Set zmode */
  vx_setzmode(zmode);

  /* now let's start with searching .... */
  char line[2001];
  while (fgets(line, 2000, stdin) != NULL) {
    if(strlen(line) == 1) continue;
    if(line[0]=='#') continue; // comment line
    if (sscanf(line,"%lf %lf %lf",
	       &entry.coor[0],&entry.coor[1],&entry.coor[2]) == 3) {

      if (entry.coor[1]<10000000) {
	printf("%14.6f %15.6f %9.2f ", 
	       entry.coor[0], entry.coor[1], entry.coor[2]);
      }

      /* In case we got anything like degrees */
      if ((entry.coor[0]<360.) && (fabs(entry.coor[1])<90)) {
	entry.coor_type = VX_COORD_GEO;
      } else {
	entry.coor_type = VX_COORD_UTM;
      }

      /* Query the point */
      vx_getcoord(&entry);

      /*** Prevent all to obvious bad coordinates from being displayed */
      if (entry.coor[1]<10000000) {
	//printf("%14.6f %15.6f %9.2f ", 
	//       entry.coor[0], entry.coor[1], entry.coor[2]);
	/* AP: Let's provide the computed UTM coordinates as well */
	printf("%10.2f %11.2f ", entry.coor_utm[0], entry.coor_utm[1]);
	
	printf("%10.2f %11.2f ", entry.elev_cell[0], entry.elev_cell[1]);
	printf("%9.2f ", entry.topo);
	printf("%9.2f ", entry.mtop);
	printf("%9.2f ", entry.base);
	printf("%9.2f ", entry.moho);
	printf("%s %10.2f %11.2f %9.2f ", VX_SRC_NAMES[entry.data_src], 
	       entry.vel_cell[0], entry.vel_cell[1], entry.vel_cell[2]);
	printf("%9.2f %9.2f %9.2f ", entry.provenance, entry.vp, entry.vs);
	printf("%9.2f\n", entry.rho);
      }
    }
  }

  /* Perform cleanup */
  vx_cleanup();

  return 0;
}
