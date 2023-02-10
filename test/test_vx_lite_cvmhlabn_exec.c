/**  
   test_vx_lite_cvmhlabn_exec.c

   invokes src/run_vx_lite_cvmhlabn.sh/vx_lite_cvmhlabn
     which uses vx_lite api,
       vx_setup, vx_setzmode, vx_getcoord, vx_cleanup
**/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include "unittest_defs.h"
#include "test_helper.h"
#include "test_vx_lite_cvmhlabn_exec.h"

int VX_LITE_TESTS=2;

int test_vx_lite_cvmhlabn_points_elevation()
{
  char infile[1280];
  char outfile[1280];
  char reffile[1280];
  char currentdir[1000];

  printf("Test: vx_lite_cvmhlabn executable with elevation option\n");

  /* Save current directory */
  getcwd(currentdir, 1000);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-elev.in");
  sprintf(outfile, "%s/%s", currentdir, "test-vx-lite-cvmhlabn-elev.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test-vx-lite-cvmhlabn-elev.ref");

  if (test_assert_int(runVXLiteCVMHLABN(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_ELEVATION), 0) != 0) {
    printf("vx_lite_cvmhlabn failure\n");
    return(1);
  }

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");
  return(0);
}



int test_vx_lite_cvmhlabn_points_depth()
{
  char infile[1280];
  char outfile[1280];
  char reffile[1280];
  char currentdir[1000];

  printf("Test: vx_lite_cvmhlabn executable with depth option\n");

  /* Save current directory */
  getcwd(currentdir, 1000);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-depth.in");
  sprintf(outfile, "%s/%s", currentdir, "test-vx-lite-cvmhlabn-depth.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test-vx-lite-cvmhlabn-depth.ref");

  if (test_assert_int(runVXLiteCVMHLABN(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_DEPTH), 0) != 0) {
    printf("vx_lite_cvmhlabn failure\n");
    return(1);
  }  

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    printf("diff failure\n");
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");
  return(0);
}

int suite_vx_lite_cvmhlabn_exec(const char *xmldir)
{
  suite_t suite;
  char logfile[1280];
  FILE *lf = NULL;

  /* Setup test suite */
  strcpy(suite.suite_name, "suite_vx_lite_cvmhlabn_exec");

  suite.num_tests = VX_LITE_TESTS;
  suite.tests = malloc(suite.num_tests * sizeof(test_t));
  if (suite.tests == NULL) {
    fprintf(stderr, "Failed to alloc test structure\n");
    return(1);
  }
  test_get_time(&suite.exec_time);

  /* Setup test cases */
  strcpy(suite.tests[0].test_name, "test_vx_lite_cvmhlabn_points_elevation");
  suite.tests[0].test_func = &test_vx_lite_cvmhlabn_points_elevation;
  suite.tests[0].elapsed_time = 0.0;

  strcpy(suite.tests[1].test_name, "test_vx_lite_cvmhlabn_points_depth");
  suite.tests[1].test_func = &test_vx_lite_cvmhlabn_points_depth;
  suite.tests[1].elapsed_time = 0.0;

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
