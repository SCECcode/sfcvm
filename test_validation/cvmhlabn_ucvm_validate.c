/*
 * @file cvmhlabn_ucvm_validate.c
 * @brief test with a full set of validation points in depth
 * @author - SCEC
 * @version 1.0
 *
 * Tests the CVMHLABN library by running with UCVM
 *
 *
 *  ./cvmhlabn_ucvm_validate -e -c ucvm.conf -f validate_api_good.txt
 *
 *  test mode: query-by-depth
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include "ucvm.h"
#include "ucvm_utils.h"

#include "cvmhlabn.h"

#define NUM_POINTS 10000

int validate_debug = 0;

// from gctpc
void gctp();

/*********************************************/
typedef struct dat_entry_t 
{
  int id_idx;
  int x_idx;
  int y_idx;
  int z_idx;
  int depth_idx; //cvmh_depth
  int vp_idx;
  int vs_idx;
} dat_entry_t;

typedef struct dat_data_t 
{
  long id;
  double x;
  double y;
  double z;
  double depth;
  double vp;
  double vs;
  double lon;
  double lat;
} dat_data_t;

dat_entry_t dat_entry;

void utm2geo(double utmX, double utmY, double *lon, double *lat) {
    double SPUTM[2];
    double SPGEO[2];
    long insys_geo = 1;
    long inzone_geo= 11;
    long inunit_geo = 2;
    long indatum_geo = 0;
    long outsys_geo = 0;
    long outzone_geo= 0;
    long outunit_geo = 4;
    long outdatum_geo = 0;
    double inparm[15];
    long ipr=5;
    char efile[256]="errfile";
    long jpr=5;
    char file27[256]="proj27";
    char file83[256]="file83";
    long iflg=0;

    for(int n=0;n>15;n++) inparm[n]=0;
    SPUTM[0]=utmX;
    SPUTM[1]=utmY;

    gctp(SPUTM,&insys_geo,&inzone_geo,inparm,&inunit_geo,&indatum_geo,&ipr,
         efile,&jpr,efile,SPGEO,&outsys_geo,&outzone_geo,inparm,&outunit_geo,
         &outdatum_geo,file27,file83,&iflg);

if(validate_debug) {
fprintf(stderr,"GEO: SPUTM (%lf, %lf) SPGEO (%lf, %lf)\n", 
              SPUTM[0], SPUTM[1], SPGEO[0], SPGEO[1]);
fprintf(stderr,"  2 after GEO: insys_geo(%ld) inzone_geo(%ld)\n",
              insys_geo, inzone_geo);
fprintf(stderr,"  2 after GEO: inunit_geo(%ld) indatum_geo(%ld) ipr(%ld) jpr(%ld)\n",
              inunit_geo, indatum_geo, ipr, jpr);
fprintf(stderr,"  2 after GEO: outsys_geo(%ld) outzone_geo(%ld) outunit_geo(%ld) outdatum_geo(%ld) iflg(%ld)\n",
              outsys_geo, outzone_geo, outunit_geo, outdatum_geo, iflg);
}

    *lon=SPGEO[0];
    *lat=SPGEO[1];
}


/*
id,X,Y,Z,depth,vp63_basin,vs63_basin,vp,vs
*/
FILE *_process_datfile(char *fname) {

  char dat_line[1028];
  FILE *fp = fopen(fname, "r");
  if (fp == NULL) {
    fprintf(stderr,"VALIDATE_UCVM: FAIL: Unable to open the validation data file %s\n", fname);
    exit(1);
  }

  int done=0;
  while(!done) {

  /* read the title line */
    if (fgets(dat_line, 1028, fp) == NULL) {
      fprintf(stderr,"VALIDATE_UCVM: FAIL: Unable to extract validation data file %s\n", fname);
      fclose(fp);
      exit(1);
    }

    if(dat_line[0]=='#') continue;
    done=1;

  /* Strip terminating newline */
    int slen = strlen(dat_line);
    if ((slen > 0) && (dat_line[slen-1] == '\n')) {
      dat_line[slen-1] = '\0';
    }

    char delimiter[] = ",";
    char *p = strtok(dat_line, delimiter);
    int counter=0;
  
//id,X,Y,Z,depth,vp63_basin,vs63_basin
    while(p != NULL)
    {
      if(validate_debug) { printf("VALIDATE_UCVM:'%s'\n", p); }
      if(strcmp(p,"X")==0)
        dat_entry.x_idx=counter;
      else if(strcmp(p,"id")==0)
        dat_entry.id_idx=counter;
      else if(strcmp(p,"Y")==0)
        dat_entry.y_idx=counter;
      else if(strcmp(p,"Z")==0)
        dat_entry.z_idx=counter;
      else if(strcmp(p,"vp63_basin")==0)
        dat_entry.vp_idx=counter;
      else if(strcmp(p,"vs63_basin")==0)
        dat_entry.vs_idx=counter;
      else if(strcmp(p,"depth")==0)
        dat_entry.depth_idx=counter;
      p = strtok(NULL, delimiter);
      counter++;
    }
  }
  return fp;
}
int _next_datfile(FILE *fp, dat_data_t *dat) {

  char dat_line[1028];
  int done=0;

  while(!done) {
    if (fgets(dat_line, 1028, fp) == NULL) {
      return(1); 
    }
    if(dat_line[0]=='#') continue;
    done=1;

    char delimiter[] = ",";
    char *p = strtok(dat_line, delimiter);
    int counter=0;
  
//id, X,Y,Z,depth,vp63_basin,vs63_basin
    while(p != NULL) {
      double val = atof(p);
      if(counter == dat_entry.x_idx)
          dat->x=val;
        else if (counter == dat_entry.id_idx)
          dat->id=(long)val;
        else if (counter == dat_entry.y_idx)
          dat->y=val;
        else if (counter == dat_entry.z_idx)
          dat->z=val;
        else if (counter == dat_entry.vs_idx)
          dat->vs=val;
        else if (counter == dat_entry.vp_idx)
          dat->vp=val;
        else if (counter == dat_entry.depth_idx)
          dat->depth=val;
      p = strtok(NULL, delimiter);
      counter++;
    }
    utm2geo(dat->x,dat->y,&dat->lon,&dat->lat);
  }
  return(0);
}


/*********************************************/

int _compare_double(double f1, double f2) {
  double precision = 0.00001;
  if (((f1 - precision) < f2) && ((f1 + precision) > f2)) {
    return 0; // good
    } else {
      return 1; // bad
  }
}

/* Usage function */
void usage() {
  printf("     cvmhlabn_ucvm_validate - (c) SCEC\n");
  printf("\tusage: cvmhlabn_ucvm_validate [-d] -c ucvm.conf -f file.dat\n\n");
  printf("Flags:\n");
  printf("\t-c ucvm.conf\n\n");
  printf("\t-f point.dat\n\n");
  printf("\t-d enable debug/verbose mode\n\n");
  exit (0);
}

extern char *optarg;
extern int optind, opterr, optopt;

/**
 *
 * @param argc The number of arguments.
 * @param argv The argument strings.
 * @return A zero value indicating success.
 */
int main(int argc, char* const argv[]) {
        int rc=0;
        int opt;

        char datfile[250]="";
        char configfile[250]="";
        ucvm_ctype_t cmode=UCVM_COORD_GEO_DEPTH;

        dat_data_t *dat;
        ucvm_point_t *pnts;
        ucvm_data_t *props;

        int idx=0;
        long tcount=0;  // total entry
        int mcount=0;  // real mismatch
        int mmcount=0; // fake mismatch -- no data
        int okcount=0;

        double delta=0.0;
        double cvmh_surface=0.0;
        double ucvm_surface=0.0;
        double ucvm_depth=0.0;

        FILE *gfp= fopen("validate_ucvm_good.txt","w");
        fprintf(gfp,"id,X,Y,Z,depth,lon,lat,ucvm_depth,vp63_basin,vs63_basin,vp,vs,delta\n");
// depth is cvmh_depth, 
        FILE *bfp= fopen("validate_ucvm_bad.txt","w");
        fprintf(bfp,"id,X,Y,Z,depth,lon,lat,ucvm_depth,vp63_basin,vs63_basin,vp,vs,delta\n");

        /* Parse options */
        while ((opt = getopt(argc, argv, "edf:c:h")) != -1) {
          switch (opt) {
          case 'c':
            strcpy(configfile, optarg);
            break;
          case 'f':
            strcpy(datfile, optarg);
            break;
          case 'd':
            validate_debug=1;
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

      
        FILE *fp=_process_datfile(datfile);

        /* Initialize interface */
        if (ucvm_init(configfile) != UCVM_CODE_SUCCESS) {
          fprintf(stderr, "Failed to initialize UCVM API\n");
          return(1);
        }

        /* Add models */
        if (ucvm_add_model_list("cvmhlabn") != UCVM_CODE_SUCCESS) {
          fprintf(stderr, "Failed to enable model list: cvmhlabn\n");
          return(1);
        }

        if (ucvm_setparam(UCVM_PARAM_QUERY_MODE, cmode) != UCVM_CODE_SUCCESS) {
          fprintf(stderr, "Failed to set z mode\n");
          return(1);
        }

        /* Allocate buffers */
        dat = malloc(NUM_POINTS * sizeof(dat_data_t));
        pnts = malloc(NUM_POINTS * sizeof(ucvm_point_t));
        props = malloc(NUM_POINTS * sizeof(ucvm_data_t));

        while(rc==0 && idx < NUM_POINTS) {
              memset(&(pnts[idx]), 0, sizeof(ucvm_point_t));
              memset(&(dat[idx]), 0, sizeof(dat_data_t));

              rc=_next_datfile(fp, &dat[idx]); 
              if(rc) continue;

if(validate_debug) {
fprintf(stderr,"lon %lf lat %lf dep %lf z %lf\n", dat[idx].lon, dat[idx].lat, dat[idx].depth, dat[idx].z);
}

              tcount++;

//  ucvm query with cvmh's depth
              pnts[idx].coord[0]=dat[idx].lon;
              pnts[idx].coord[1]=dat[idx].lat;
              pnts[idx].coord[2]=dat[idx].depth; 
              idx++;

              if(idx == NUM_POINTS) {
                /* Query the UCVM */
                if (ucvm_query(NUM_POINTS, pnts, props) != UCVM_CODE_SUCCESS) {
                  fprintf(stderr, "Query CVM Failed\n");
                  return(1);
                }
      
                // compare result
                idx=0;
                // is result matching ?
                for(int j=0; j<NUM_POINTS; j++) {

                  ucvm_surface=props[j].surf;
                  ucvm_depth=ucvm_surface - dat[j].z;
                  cvmh_surface=dat[j].z + dat[j].depth;
                  delta = ucvm_depth - dat[j].z;

                  if(_compare_double(props[j].cmb.vs, dat[j].vs) || _compare_double(props[j].cmb.vp, dat[j].vp)) { 

                     // okay if ( dat[j].vp == -99999, dat[j].vs== -99999 ) and (props[j].cmb.vs == 0, props[j].cmb.vp == 0)
                     if (!_compare_double(props[j].cmb.vs, 0) && !_compare_double(props[j].cmb.vp, 0) &&
                              !_compare_double(dat[j].vs, -99999.0) && !_compare_double(dat[j].vp, -99999.0)) {
                       mmcount++;  // just 0 vs -99999
                       fprintf(gfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                          dat[j].id,dat[j].x,dat[j].y,dat[j].z,
                          dat[j].depth,
                          dat[j].lon,dat[j].lat,ucvm_depth,
                          dat[j].vp,dat[j].vs,props[j].cmb.vp,props[j].cmb.vs,delta);
                       } else {
                          fprintf(bfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                              dat[j].id, dat[j].y, dat[j].x, dat[j].z,dat[j].depth,
                              dat[j].lon,dat[j].lat,ucvm_depth,dat[j].vp,dat[j].vs,props[j].cmb.vp,props[j].cmb.vs,delta);
                          mcount++;
                     }

                    } else {
                      okcount++;
                      fprintf(gfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                          dat[j].id,dat[j].x,dat[j].y,dat[j].z,
                          dat[j].depth,
                          dat[j].lon,dat[j].lat,ucvm_depth,
                          dat[j].vp,dat[j].vs,props[j].cmb.vp,props[j].cmb.vs,delta);
                  }
                }
                idx=0;
              }
        }
        if(idx > 0) {
            /* Query the UCVM */
if(validate_debug) {
fprintf(stderr," V|| pnts >> x(%lf) y(%lf) z(%lf) \n", pnts[0].coord[0], pnts[0].coord[1], pnts[0].coord[2]);
}

            if (ucvm_query(idx, pnts, props) != UCVM_CODE_SUCCESS) {
              fprintf(stderr, "Query CVM Failed\n");
              return(1);
            }

            // compare result
            // is result matching ?
            for(int j=0; j<idx; j++) {

if(validate_debug && j==0) {
fprintf(stderr," VV|| pnts  << x(%lf) y(%lf) z(%lf) \n", pnts[j].coord[0], pnts[j].coord[1], pnts[j].coord[2]);
fprintf(stderr," VV|| props << surf(%lf) vs30(%lf) depth(%lf) \n", props[j].surf, props[j].vs30, props[j].depth);
fprintf(stderr," Vv|| props << vp(%lf) vs(%lf) rho(%lf) \n", props[j].cmb.vp, props[j].cmb.vs, props[j].cmb.rho);
}

              ucvm_surface=props[j].surf;
              ucvm_depth=ucvm_surface - dat[j].z;
              cvmh_surface=dat[j].z + dat[j].depth;
              delta = ucvm_depth - dat[j].z;

              if(_compare_double(props[j].cmb.vs, dat[j].vs) || _compare_double(props[j].cmb.vp, dat[j].vp)) {

                // okay if ( dat[j].vp == -99999, dat[j].vs== -99999 ) and (props[j].cmb.vs == 0, props[j].cmb.vp == 0)
                 if (!_compare_double(props[j].cmb.vs, 0) && !_compare_double(props[j].cmb.vp, 0) &&
                              !_compare_double(dat[j].vs, -99999.0) && !_compare_double(dat[j].vp, -99999.0)) {
                   mmcount++;  // just 0 vs -99999
                   fprintf(gfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                          dat[j].id,dat[j].x,dat[j].y,dat[j].z, dat[j].depth,
                          dat[j].lon,dat[j].lat,ucvm_depth,
                          dat[j].vp,dat[j].vs,props[j].cmb.vp,props[j].cmb.vs,delta);
                   } else {
                      mcount++;
                      fprintf(bfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                              dat[j].id, dat[j].y, dat[j].x, dat[j].z,dat[j].depth,
                              dat[j].lon,dat[j].lat,ucvm_depth,dat[j].vp,dat[j].vs,props[j].cmb.vp,props[j].cmb.vs,delta);
                 }
                } else {
                  okcount++;
                  fprintf(gfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                          dat[j].id,dat[j].x,dat[j].y,dat[j].z,
                          dat[j].depth,
                          dat[j].lon,dat[j].lat,ucvm_depth,
                          dat[j].vp,dat[j].vs,props[j].cmb.vp,props[j].cmb.vs,delta);

              }
            }
        }

        fprintf(stderr,"VALIDATE_UCVM: %d mismatch out of %ld \n", mcount, tcount);
        fprintf(stderr,"VALIDATE_UCVM: good with matching values(%d) mmcount(%d) \n",okcount, mmcount );
	assert(ucvm_finalize() == 0);
	printf("VALIDATE_UCVM:Model closed successfully.\n");

        fclose(fp);
        fclose(gfp);
        fclose(bfp);

	return 0;
}



