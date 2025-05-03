/**  
   test_ucvm_sfcvm_exec.c

   invokes src/run_ucvm_sfcvm.sh 
        which invokes ucvm_query with -m sfcvm
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
#include "test_ucvm_sfcvm_exec.h"

int UCVM_TESTS=2;

int test_ucvm_sfcvm_points_ge()
{
  char infile[1280];
  char outfile[1280];
  char reffile[1280];
  char currentdir[1000];

  printf("\nTest: ucvm_sfcvm validate ge option\n");

  /* Save current directory */
  getcwd(currentdir, 1000);

// ge part
  sprintf(infile, "%s/../test/%s", currentdir, "inputs/test_ucvm_sfcvm_ge.txt");
  sprintf(outfile, "%s/../test/%s", currentdir, "test_ucvm_sfcvm_ge.out");
  sprintf(reffile, "%s/../test/%s", currentdir, "ref/test_ucvm_sfcvm_ge.ref");

  if (test_assert_file_exist(infile) != 0) {
    printf("file:%s not found\n",infile);
    printf("FAIL\n");
    return(1);
  }

  if (test_assert_int(runUCVMSFCVM(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_ELEVATION), 0) != 0) {
    printf("ucvm_sfcvm failure\n");
    printf("FAIL\n");
    return(1);
  }
 
  /* Perform diff btw outfile and ref */
  if (test_assert_file_vs(outfile, reffile, 1) != 0) {
    printf("FAIL\n");
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");
  return(0);
}

int test_ucvm_sfcvm_points_gd()
{
  char infile[1280];
  char outfile[1280];
  char reffile[1280];
  char currentdir[1000];

  printf("\nTest: ucvm_sfcvm validate gd option\n");

  /* Save current directory */
  getcwd(currentdir, 1000);

// ge part
  sprintf(infile, "%s/%s", currentdir, "./inputs/test_ucvm_sfcvm_gd.txt");
  sprintf(outfile, "%s/%s", currentdir, "test_ucvm_sfcvm_gd.out");
  sprintf(reffile, "%s/%s", currentdir, "./ref/test_ucvm_sfcvm_gd.ref");

  if (test_assert_file_exist(infile) != 0) {
    printf("file:%s not found\n",infile);
    printf("FAIL\n");
    return(1);
  }

  if (test_assert_int(runUCVMSFCVM(BIN_DIR, MODEL_DIR, infile, outfile, 
				MODE_DEPTH), 0) != 0) {
    printf("ucvm_sfcvm failure\n");
    printf("FAIL\n");
    return(1);
  }

  /* Perform diff btw outfile and ref */
  if (test_assert_file_vs(outfile, reffile, 0) != 0) {
    printf("FAIL\n");
    return(1);
  }

  unlink(outfile);

  printf("PASS\n");
  return(0);
}

int suite_ucvm_sfcvm_exec(const char *xmldir)
{
  suite_t suite;
  char logfile[1280];
  FILE *lf = NULL;

  /* Setup test suite */
  strcpy(suite.suite_name, "suite_ucvm_sfcvm_exec");

  suite.num_tests = UCVM_TESTS;
  suite.tests = malloc(suite.num_tests * sizeof(test_t));
  if (suite.tests == NULL) {
    fprintf(stderr, "Failed to alloc test structure\n");
    return(1);
  }
  test_get_time(&suite.exec_time);

  /* Setup test cases */
  strcpy(suite.tests[0].test_name, "test_ucvm_sfcvm_points_gd");
  suite.tests[0].test_func = &test_ucvm_sfcvm_points_gd;
  suite.tests[0].elapsed_time = 0.0;

  strcpy(suite.tests[1].test_name, "test_ucvm_sfcvm_points_ge");
  suite.tests[1].test_func = &test_ucvm_sfcvm_points_ge;
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
