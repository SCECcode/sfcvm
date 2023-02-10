/**
   test_cvmhlabn_exec.c

   uses cvmhlabn's model api,
       model_init, model_setparam, model_query, model_finalize
**/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include "cvmhlabn.h"
#include "unittest_defs.h"
#include "test_helper.h"
#include "test_cvmhlabn_exec.h"
#include "ucvm_model_dtypes.h"

int test_setup()
{
  printf("Test: model_init() and model_finalize()\n");

  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "cvmhlabn"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "cvmhlabn"), 0) != 0) {
    return(1);
  }

  if (test_assert_int(model_finalize(), 0) != 0) {
    return(1);
  }

  printf("PASS\n");
  return(0);
}

int test_setparam()
{
  printf("Test: model_setparam() with depth\n");

// Initialize the model, try to use Use UCVM_INSTALL_PATH
  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "cvmhlabn"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "cvmhlabn"), 0) != 0) {
    return(1);
  }

  int zmode = UCVM_COORD_GEO_DEPTH;
  if (test_assert_int(model_setparam(0, UCVM_PARAM_QUERY_MODE, zmode), 0) != 0) {
      return(1);
  }

  // Close the model.
  assert(model_finalize() == 0);

  printf("PASS\n");
  return(0);
}


int test_query_by_depth()
{
  printf("Test: model_query() by depth\n");

  cvmhlabn_point_t pt;
  cvmhlabn_properties_t expect;
  cvmhlabn_properties_t ret;

// Initialize the model, try to use Use UCVM_INSTALL_PATH
  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "cvmhlabn"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "cvmhlabn"), 0) != 0) {
    return(1);
  }

  int zmode = UCVM_COORD_GEO_DEPTH;
  if (test_assert_int(model_setparam(0, UCVM_PARAM_QUERY_MODE, zmode), 0) != 0) {
      return(1);
  }

  if( get_depth_test_point(&pt,&expect) != 0) {
      return(1);
  }

  //fprintf(stderr,"depth test points (%lf,%lf,%lf)\n",pt.longitude,pt.latitude,pt.depth);
  //fprintf(stderr,"depth result expected (%lf,%lf,%lf)\n",expect.vp,expect.vs,expect.rho);
 
  if (test_assert_int(model_query(&pt, &ret, 1), 0) != 0) {
      return(1);
  }

  // Close the model.
  assert(model_finalize() == 0);

  if ( test_assert_double(ret.vs, expect.vs) ||
       test_assert_double(ret.vp, expect.vp) ||
       test_assert_double(ret.rho, expect.rho) ) {
     printf("FAIL\n");
     return(1);
     } else {
       printf("PASS\n");
       return(0);
  }

}

int test_query_by_elevation()
{
  printf("Test: model_query() by elevation\n");

  cvmhlabn_point_t pt;
  cvmhlabn_properties_t ret;
  cvmhlabn_properties_t expect;

// Initialize the model, try to use Use UCVM_INSTALL_PATH
  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "cvmhlabn"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "cvmhlabn"), 0) != 0) {
    return(1);
  }

  int zmode = UCVM_COORD_GEO_ELEV;
  if (test_assert_int(model_setparam(0, UCVM_PARAM_QUERY_MODE, zmode), 0) != 0) {
    return(1);
  }

  double pt_elevation;
  double pt_surf;
  if( get_elev_test_point(&pt, &expect, &pt_elevation, &pt_surf) != 0 ) {
      return(1);
  }

 // fprintf(stderr,"elev test points (%lf,%lf,%lf,%lf)\n",pt.longitude,pt.latitude,pt_elevation,pt_surf);
 // fprintf(stderr,"depth result expected (%lf,%lf,%lf)\n",expect.vp,expect.vs,expect.rho);

  pt.depth = pt_surf - pt_elevation; // elevation

  if (test_assert_int(model_query(&pt, &ret, 1), 0) != 0) {
      return(1);
  }

  // Close the model.
  assert(model_finalize() == 0);

  if ( test_assert_double(ret.vs, expect.vs) ||
       test_assert_double(ret.vp, expect.vp) ||
       test_assert_double(ret.rho, expect.rho) ) {
     printf("FAIL\n");
     return(1);
     } else {
       printf("PASS\n");
       return(0);
  }
}

// using points by ucvm's digital elevation
int test_query_points_by_elevation()
{
  printf("Test: model_query() points by elevation\n");

  FILE  *infp, *outfp;
  cvmhlabn_point_t pt;
  cvmhlabn_properties_t ret;
  double elev;
  double surf;

  char infile[1280];
  char outfile[1280];
  char reffile[1280];
  char currentdir[1000];

  /* Save current directory */
  getcwd(currentdir, 1000);

// ge part
  sprintf(infile, "%s/%s", currentdir, "./inputs/test_latlons_ucvm_ge.txt");
  sprintf(outfile, "%s/%s", currentdir, "test_latlons_ucvm_ge.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test_latlons_ucvm_ge.ref");

  if (test_assert_file_exist(infile) != 0) {
    printf("file:%s not found\n",infile);
    return(1);
  }

  // pick up every point and convert the elevation to depth in pt structure
  infp = fopen(infile, "r");
  if (infp == NULL) {
    printf("FAIL: cannot open %s\n", infile);
    return(1);
  }
  outfp = fopen(outfile, "w");
  if (outfp == NULL) {
    printf("FAIL: cannot open %s\n", outfile);
    return(1);
  }

  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "cvmhlabn"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "cvmhlabn"), 0) != 0) {
    return(1);
  }

  int zmode = UCVM_COORD_GEO_ELEV;
  if (test_assert_int(model_setparam(0, UCVM_PARAM_QUERY_MODE, zmode), 0) != 0) {
      return(1);
  }

/* process one term at a time */
/* currently one for now, (-118.1,34.0) */
  char line[1001];

  init_preset_ucvm_surface(test_surfs);

  while(fgets(line, 1000, infp) != NULL) {
    if(line[0] == '#') continue; // a comment
    if (sscanf(line,"%lf %lf %lf",
         &pt.longitude,&pt.latitude,&elev) == 3) {
      surf = get_preset_ucvm_surface(pt.longitude, pt.latitude);
      pt.depth= surf - elev;
      if (test_assert_int(model_query(&pt, &ret, 1), 0) == 0) {
         fprintf(outfp,"%lf %lf %lf\n",ret.vs, ret.vp, ret.rho);
      }
    }
  }
  fclose(infp);
  fclose(outfp);

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    printf("unmatched result\n");
    printf("%s\n",outfile);
    printf("%s\n",reffile);
    return(1);
  }

  // Close the model.
  assert(model_finalize() == 0);

  unlink(outfile);

  printf("PASS\n");
  return(0);
}


int test_query_points_by_depth()
{
  printf("Test: model_query() points by depth\n");

  FILE  *infp, *outfp;
  cvmhlabn_point_t pt;
  cvmhlabn_properties_t ret;

  char infile[1280];
  char outfile[1280];
  char reffile[1280];
  char currentdir[1000];

  /* Save current directory */
  getcwd(currentdir, 1000);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-depth-ucvm.in");
  sprintf(outfile, "%s/%s", currentdir, "test-depth-ucvm.out");

/*** seems to differ in result from mac and linux ***/
/*** DOES NOT NEED anymore??? 
  #ifdef __APPLE__
    sprintf(reffile, "%s/%s", currentdir, "./ref/test-depth-ucvm_mac.ref");
  #else
    sprintf(reffile, "%s/%s", currentdir, "./ref/test-depth-ucvm.ref");
  #endif
***/
  sprintf(reffile, "%s/%s", currentdir, "./ref/test-depth-ucvm.ref");

  if (test_assert_file_exist(infile) != 0) {
    return _failure("filed not found");
  }

  infp = fopen(infile, "r");
  if (infp == NULL) {
    return _failure("input file not found");
  }
  outfp = fopen(outfile, "w");
  if (outfp == NULL) {
    return _failure("output file can not be open");
  }

  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "cvmhlabn"), 0) != 0) {
      return _failure("model_init failed");
    }
  } else if (test_assert_int(model_init("..", "cvmhlabn"), 0) != 0) {
    return _failure("model_init failed");
  }

  int zmode = UCVM_COORD_GEO_DEPTH;
  if (test_assert_int(model_setparam(0, UCVM_PARAM_QUERY_MODE, zmode), 0) != 0) {
      return _failure("model_setparam failed");
  }

/* process one term at a time */
  char line[1001];
  while(fgets(line, 1000, infp) != NULL) {
    if(line[0] == '#') continue; // a comment
    if (sscanf(line,"%lf %lf %lf",
         &pt.longitude,&pt.latitude,&pt.depth) == 3) {
      if (test_assert_int(model_query(&pt, &ret, 1), 0) == 0) {
         fprintf(outfp,"%lf %lf %lf\n",ret.vs, ret.vp, ret.rho);
      }
    }
  }
  fclose(infp);
  fclose(outfp);

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    printf("unmatched result\n");
    printf("%s\n",outfile);
    printf("%s\n",reffile);
    return _failure("diff file");
  }

  // Close the model.
  if(model_finalize() != 0) {
      return _failure("model_finalize failed");
  }
  unlink(outfile);

  return _success();
}


int suite_cvmhlabn_exec(const char *xmldir)
{
  suite_t suite;
  char logfile[256];
  FILE *lf = NULL;

  /* Setup test suite */
  strcpy(suite.suite_name, "suite_cvmhlabn_exec");

  suite.num_tests = 6;
  suite.tests = malloc(suite.num_tests * sizeof(test_t));
  if (suite.tests == NULL) {
    fprintf(stderr, "Failed to alloc test structure\n");
    return(1);
  }
  test_get_time(&suite.exec_time);

  /* Setup test cases */
  strcpy(suite.tests[0].test_name, "test_setup");
  suite.tests[0].test_func = &test_setup;
  suite.tests[0].elapsed_time = 0.0;

  strcpy(suite.tests[1].test_name, "test_separam");
  suite.tests[1].test_func = &test_setparam;
  suite.tests[1].elapsed_time = 0.0;

  strcpy(suite.tests[2].test_name, "test_query_by_depth");
  suite.tests[2].test_func = &test_query_by_depth;
  suite.tests[2].elapsed_time = 0.0;

  strcpy(suite.tests[3].test_name, "test_query_by_elevation");
  suite.tests[3].test_func = &test_query_by_elevation;
  suite.tests[3].elapsed_time = 0.0;

  strcpy(suite.tests[4].test_name, "test_query_points_by_elevation");
  suite.tests[4].test_func = &test_query_points_by_elevation;
  suite.tests[4].elapsed_time = 0.0;

  strcpy(suite.tests[5].test_name, "test_query_points_by_depth");
  suite.tests[5].test_func = &test_query_points_by_depth;
  suite.tests[5].elapsed_time = 0.0;

  if (test_run_suite(&suite) != 0) {
    fprintf(stderr, "Failed to execute tests\n");
    return(1);
  }

  if (xmldir != NULL) {
    sprintf(logfile, "%s/%s.xml", xmldir, suite.suite_name);
    lf = init_log(logfile);
    if (lf == NULL) {
      fprintf(stderr, "Failed to initialize logfile\n");
      return(1);
    }
    
    if (write_log(lf, &suite) != 0) {
      fprintf(stderr, "Failed to write test log\n");
      return(1);
    }

    close_log(lf);
  }

  free(suite.tests);

  return 0;
}
