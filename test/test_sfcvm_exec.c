/**
   test_sfcvm_exec.c

   uses sfcvm's model api,
       model_init, model_setparam, model_query, model_finalize
**/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include "sfcvm.h"
#include "unittest_defs.h"
#include "test_helper.h"
#include "test_sfcvm_exec.h"
#include "ucvm_model_dtypes.h"

int test_setup()
{
  printf("\nTest: model_init() and model_finalize()\n");

  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "sfcvm"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "sfcvm"), 0) != 0) {
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
  printf("\nTest: model_setparam() with depth\n");

// Initialize the model, try to use Use UCVM_INSTALL_PATH
  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "sfcvm"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "sfcvm"), 0) != 0) {
    return(1);
  }

  int zmode = UCVM_MODEL_COORD_GEO_DEPTH;
  if (test_assert_int(model_setparam(0, UCVM_MODEL_PARAM_QUERY_MODE, zmode), 0) != 0) {
      return(1);
  }

  // Close the model.
  assert(model_finalize() == 0);

  printf("PASS\n");
  return(0);
}


int test_query_by_depth()
{
  printf("\nTest: model_query() by depth\n");

  sfcvm_point_t pt;
  sfcvm_properties_t expect;
  sfcvm_properties_t ret;

// Initialize the model, try to use Use UCVM_INSTALL_PATH
  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "sfcvm"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "sfcvm"), 0) != 0) {
    return(1);
  }

  int zmode = UCVM_MODEL_COORD_GEO_DEPTH;
  if (test_assert_int(model_setparam(0, UCVM_MODEL_PARAM_QUERY_MODE, zmode), 0) != 0) {
      return(1);
  }

  if( get_depth_test_point(&pt,&expect) != 0) {
      return(1);
  }

  if (test_assert_int(model_query(&pt, &ret, 1), 0) != 0) {
      return(1);
  }

  //fprintf(stderr,"depth test point (%lf,%lf,%lf)\n",pt.longitude,pt.latitude,pt.depth);
  //fprintf(stderr,"depth expected result (%lf,%lf,%lf)\n",expect.vs,expect.vp,expect.rho);
  //fprintf(stderr,"depth got result (%lf,%lf,%lf)\n",ret.vs,ret.vp,ret.rho);

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
  printf("\nTest: model_query() by elevation\n");

  sfcvm_point_t pt;
  sfcvm_properties_t ret;
  sfcvm_properties_t expect;

// Initialize the model, try to use Use UCVM_INSTALL_PATH
  char *envstr=getenv("UCVM_INSTALL_PATH");
  if(envstr != NULL) {
    if (test_assert_int(model_init(envstr, "sfcvm"), 0) != 0) {
      return(1);
    }
  } else if (test_assert_int(model_init("..", "sfcvm"), 0) != 0) {
    return(1);
  }

  int zmode = UCVM_MODEL_COORD_GEO_ELEV;
  if (test_assert_int(model_setparam(0, UCVM_MODEL_PARAM_QUERY_MODE, zmode), 0) != 0) {
    return(1);
  }

  double pt_elevation;
  double pt_surf;
  if( get_elev_test_point(&pt, &expect, &pt_elevation, &pt_surf) != 0 ) {
      return(1);
  }

  pt.depth = pt_surf - pt_elevation; // elevation

  if (test_assert_int(model_query(&pt, &ret, 1), 0) != 0) {
      return(1);
  }

  //fprintf(stderr,"elev test point (%lf,%lf,%lf,%lf)\n",pt.longitude,pt.latitude,pt_elevation,pt_surf);
  //fprintf(stderr,"elev expected result (%lf,%lf,%lf)\n",expect.vs, expect.vp,expect.rho);
  //fprintf(stderr,"elev got result (%lf,%lf,%lf)\n",ret.vs,ret.vp,ret.rho);

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


int suite_sfcvm_exec(const char *xmldir)
{
  suite_t suite;
  char logfile[256];
  FILE *lf = NULL;

  /* Setup test suite */
  strcpy(suite.suite_name, "suite_sfcvm_exec");

  suite.num_tests = 4;
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
