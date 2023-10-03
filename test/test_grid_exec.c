/**
   test_grid_exec.c


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
#include "test_grid_exec.h"


// test with elevation by sfcvm's digital elevation
int test_sfcvm_grid_elev()
{
  char infile[1280];
  char outfile[1280];
  char reffile[1280];
  char currentdir[1000];

  printf("\nTest: model with large grid in elevation mode\n");

  /* Save current directory */
  getcwd(currentdir, 1000);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-grid-elev.in");
  sprintf(outfile, "%s/%s", currentdir, "test-grid-sfcvm-elev.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test-grid-sfcvm-elev.ref");

  if (test_assert_int(runSFCVM(BIN_DIR, MODEL_DIR,infile, outfile,
				MODE_ELEVATION), 0) != 0) {
    printf("sfcvm failure\n");
    return(1);
  }

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    printf("unmatched result\n");
    printf("%s\n",outfile);
    printf("%s\n",reffile);
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");

  return(0);
}

int test_sfcvm_grid_depth()
{
  char infile[1280];
  char outfile[1280];
  char reffile[1280];
  char currentdir[1000];

  printf("\nTest: model with large grid in depth mode\n");

  /* Save current directory */
  getcwd(currentdir, 1000);

  sprintf(infile, "%s/%s", currentdir, "./inputs/test-grid-depth.in");
  sprintf(outfile, "%s/%s", currentdir, "test-grid-sfcvm-depth.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test-grid-sfcvm-depth.ref");

  if (test_assert_int(runSFCVM(BIN_DIR, MODEL_DIR,infile, outfile,
				MODE_DEPTH), 0) != 0) {
    printf("sfcvm failure\n");
    return(1);
  }

  /* Perform diff btw outfile and ref */
  if (test_assert_file(outfile, reffile) != 0) {
    printf("unmatched result\n");
    printf("%s\n",outfile);
    printf("%s\n",reffile);
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");

  return(0);
}

int suite_grid_exec(const char *xmldir)
{
  suite_t suite;
  char logfile[256];
  FILE *lf = NULL;

  /* Setup test suite */
  strcpy(suite.suite_name, "suite_grid_exec");
  suite.num_tests = 4;
  suite.tests = malloc(suite.num_tests * sizeof(test_t));
  if (suite.tests == NULL) {
    fprintf(stderr, "Failed to alloc test structure\n");
    return(1);
  }
  test_get_time(&suite.exec_time);

  /* Setup test cases */
  strcpy(suite.tests[0].test_name, "test_sfcvm_grid_elev");
  suite.tests[0].test_func = &test_sfcvm_grid_elev;
  suite.tests[0].elapsed_time = 0.0;

  strcpy(suite.tests[1].test_name, "test_sfcvm_grid_depth");
  suite.tests[1].test_func = &test_sfcvm_grid_depth;
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
