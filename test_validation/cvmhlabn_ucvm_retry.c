/*
 * @file cvmhlabn_ucvm_retry.c
 * @brief retry the failed case from cvmhlabn_ucvm_rerun
 * @author - SCEC
 * @version 1.0
 *
 * Tests the CVMHLABN library by running with ucvm_query
 *
 *
 *  ./cvmhlabn_ucvm_retry -e -c ucvm.conf -f ucvm_rerun_bad.txt
 *
 *  test mode: query-by-depth with lat lon depth
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
int has_negative_depth = 0;

// from gctpc
void gctp();

/*********************************************/
typedef struct dat_entry_t 
{
  int id_idx;
  int x_idx;
  int y_idx;
  int z_idx;
  int lat_idx;
  int lon_idx;
  int depth_idx;
  int ucvm_depth_idx;
  int vp_idx;
  int vs_idx;
  int vpvp_idx; // don't care
  int vsvs_idx; // don't care
} dat_entry_t;

typedef struct dat_data_t 
{
  long id;
  double x;
  double y;
  double z;
  double lat;
  double lon;
  double depth;
  double ucvm_depth;
  double vp;
  double vs;
} dat_data_t;

dat_entry_t dat_entry;

/*
idx,x,y,z,depth,lon,lat,ucvm_depth,vp63_basin,vs63_basin,vp,vs
*/
FILE *_process_datfile(char *fname) {

  char dat_line[1028];
  FILE *fp = fopen(fname, "r");
  if (fp == NULL) {
    fprintf(stderr,"RETRY_UCVM: FAIL: Unable to open the validation data file %s\n", fname);
    exit(1);
  }

  int done=0;
  while(!done) {

  /* read the title line */
    if (fgets(dat_line, 1028, fp) == NULL) {
      fprintf(stderr,"RETRY_UCVM: FAIL: Unable to extract validation data file %s\n", fname);
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
  
    while(p != NULL)
    {
      if(validate_debug) { printf("RETRY_UCVM:'%s'\n", p); }
      if(strcmp(p,"id")==0)
          dat_entry.id_idx=counter;
      else if(strcmp(p,"lon")==0)
          dat_entry.lon_idx=counter;
      else if(strcmp(p,"lat")==0)
          dat_entry.lat_idx=counter;
      else if(strcmp(p,"Y")==0)
          dat_entry.y_idx=counter;
      else if(strcmp(p,"X")==0)
          dat_entry.x_idx=counter;
      else if(strcmp(p,"Z")==0)
          dat_entry.z_idx=counter;
      else if(strcmp(p,"depth")==0)
          dat_entry.depth_idx=counter;
      else if(strcmp(p,"ucvm_depth")==0)
          dat_entry.ucvm_depth_idx=counter;
      else if(strcmp(p,"vp63_basin")==0)
          dat_entry.vp_idx=counter;
      else if(strcmp(p,"vs63_basin")==0)
          dat_entry.vs_idx=counter;
      else if(strcmp(p,"vp")==0)
          dat_entry.vpvp_idx=counter;
      else if(strcmp(p,"vs")==0)
          dat_entry.vsvs_idx=counter;
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

    char delimiter[] = ",";
    char *p = strtok(dat_line, delimiter);
    int counter=0;
  
  //lon,lat,depth,vp63_basin,vs63_basin
    while(p != NULL) {
      double val = atof(p);
      if(counter == dat_entry.id_idx)
        dat->id=(long) val;
      else if(counter == dat_entry.lon_idx)
        dat->lon=val;
      else if (counter == dat_entry.lat_idx)
        dat->lat=val;
      else if (counter == dat_entry.x_idx)
        dat->x=val;
      else if (counter == dat_entry.y_idx)
        dat->y=val;
      else if (counter == dat_entry.z_idx)
        dat->z=val;
      else if (counter == dat_entry.depth_idx)
        dat->depth=val;
      else if (counter == dat_entry.ucvm_depth_idx)
        dat->ucvm_depth=val;
      else if (counter == dat_entry.vs_idx)
        dat->vs=val;
      else if (counter == dat_entry.vp_idx)
        dat->vp=val;
          p = strtok(NULL, delimiter);
      counter++;
    }

    done=1;
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

/*********************************************/

/* Usage function */
void usage() {
  printf("     cvmhlabn_ucvm_retry - (c) SCEC\n");
  printf("\tusage: cvmhlabn_ucvm_retry [-d] -c ucvm.conf -f data.txt\n\n");
  printf("Flags:\n");
  printf("\t-c ucvm.conf\n\n");
  printf("\t-f data.txt\n\n");
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
  cvmhlabn_point_t *pnts;
  cvmhlabn_properties_t *rets;


  int idx=0;
  long tcount=0;  // total entry
  int mcount=0;  // real mismatch
  int mmcount=0; // fake mismatch -- no data
  int okcount=0;

  FILE *bfp= fopen("ucvm_retry_bad.txt","w");
  fprintf(bfp,"id,X,Y,Z,depth,lon,lat,ucvm_depth,vp63_basin,vs63_basin,vp,vs\n");

  FILE *gfp= fopen("ucvm_retry_good.txt","w");
  fprintf(gfp,"id,X,Y,Z,depth,lon,lat,ucvm_depth,vp63_basin,vs63_basin,vp,vs\n");

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


// USING UCVM to query for points with positive depth
  FILE *fp=_process_datfile(datfile);


  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    assert(model_init(envstr, "cvmhlabn")==0);
    } else {
      assert(model_init("..", "cvmhlabn")==0);
  }

  assert(model_setparam(0, UCVM_PARAM_QUERY_MODE, cmode)==0);

  /* Allocate buffers */
  dat = malloc(NUM_POINTS * sizeof(dat_data_t));
  pnts = malloc(NUM_POINTS * sizeof(cvmhlabn_point_t));
  rets = malloc(NUM_POINTS * sizeof(cvmhlabn_properties_t));

  while(rc==0 && idx < NUM_POINTS) {
        memset(&(pnts[idx]), 0, sizeof(ucvm_point_t));
        memset(&(dat[idx]), 0, sizeof(dat_data_t));

        rc=_next_datfile(fp, &dat[idx]);
        if(rc) continue;

        tcount++;

//try ucvm query with ucvm_depth
        pnts[idx].longitude=dat[idx].lon;
        pnts[idx].latitude=dat[idx].lat;
        pnts[idx].depth=dat[idx].depth;

        idx++;

        if(idx == NUM_POINTS) {
          /* Query the UCVM */
          if (model_query(pnts, rets, NUM_POINTS) != UCVM_CODE_SUCCESS) {
            fprintf(stderr, "Query CVM Failed\n");
            return(1);
          }
      
          // compare result
          idx=0;
          // is result matching ?
          for(int j=0; j<NUM_POINTS; j++) {

            if(_compare_double(rets[j].vs, dat[j].vs) || _compare_double(rets[j].vp, dat[j].vp)) { 

               if (!_compare_double(rets[j].vs, 0) && !_compare_double(rets[j].vp, 0) &&
                        !_compare_double(dat[j].vs, -99999.0) && !_compare_double(dat[j].vp, -99999.0)) {
                 mmcount++;  // just 0 vs -99999
                 fprintf(gfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                    dat[j].id,dat[j].x,dat[j].y,dat[j].z, dat[j].depth,
                    dat[j].lon,dat[j].lat,dat[j].ucvm_depth,
                    dat[j].vp, dat[j].vs, rets[j].vp,rets[j].vs);
                 } else { // ohoh
                    fprintf(bfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                      dat[j].id,dat[j].x,dat[j].y,dat[j].z, dat[j].depth,
                      dat[j].lon,dat[j].lat,dat[j].ucvm_depth,
                      dat[j].vp, dat[j].vs, rets[j].vp,rets[j].vs);
		    mcount++;
fprintf(stderr,"HERE inside...%d,%d,%d\n",okcount,mmcount,mcount);
               }

              } else {
                okcount++;
                fprintf(gfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                    dat[j].id,dat[j].x,dat[j].y,dat[j].z, dat[j].depth,
                    dat[j].lon,dat[j].lat,dat[j].ucvm_depth,
                    dat[j].vp, dat[j].vs, rets[j].vp,rets[j].vs);
            }
          }
          idx=0;
        }
  }


  if(idx > 0) {
      /* Query the UCVM */

      if (model_query(pnts,rets,idx) != UCVM_CODE_SUCCESS) {
        fprintf(stderr, "Query CVM Failed\n");
        return(1);
      }

      // compare result
      // is result matching ?
      for(int j=0; j<idx; j++) {

        if(_compare_double(rets[j].vs, dat[j].vs) || _compare_double(rets[j].vp, dat[j].vp)) {

           if (!_compare_double(rets[j].vs, 0) && !_compare_double(rets[j].vp, 0) &&
                        !_compare_double(dat[j].vs, -99999.0) && !_compare_double(dat[j].vp, -99999.0)) {
             mmcount++;  // just 0 vs -99999
             fprintf(gfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                    dat[j].id,dat[j].x,dat[j].y,dat[j].z, dat[j].depth,
                    dat[j].lon,dat[j].lat,dat[j].ucvm_depth,
                    dat[j].vp, dat[j].vs,
                    rets[j].vp,rets[j].vs);
             } else {
               fprintf(bfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                    dat[j].id,dat[j].x,dat[j].y,dat[j].z, dat[j].depth,
                    dat[j].lon,dat[j].lat,dat[j].ucvm_depth,
                    dat[j].vp, dat[j].vs,
                    rets[j].vp,rets[j].vs);
	        mcount++;
            }
          } else {
            okcount++;
             fprintf(gfp,"%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                    dat[j].id,dat[j].x,dat[j].y,dat[j].z, dat[j].depth,
                    dat[j].lon,dat[j].lat,dat[j].ucvm_depth,
                    dat[j].vp, dat[j].vs,
                    rets[j].vp,rets[j].vs);

        }
      }
  }
    

  assert(model_finalize() == 0);
  printf("RETRY_UCVM:Model closed successfully.\n");

  fprintf(stderr,"RETRY_UCVM: %d mismatch out of %ld \n", mcount, tcount);
  fprintf(stderr,"RETRY_UCVM: good with matching values(%d) mmcount(%d) \n",okcount, mmcount );

  free(rets);
  free(pnts);
  fclose(fp);
  fclose(bfp);
  fclose(gfp);

  return 0;
}



