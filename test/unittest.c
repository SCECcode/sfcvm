#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "unittest_defs.h"
#include "test_vx_lite_cvmhlabn_exec.h"
#include "test_vx_cvmhlabn_exec.h"
#include "test_cvmhlabn_exec.h"


int main (int argc, char *argv[])
{
  char *xmldir;

  if (argc == 2) {  
    xmldir = argv[1];
  } else {
    xmldir = NULL;
  }

  /* Run test suites */
  suite_cvmhlabn_exec(xmldir);
  suite_vx_cvmhlabn_exec(xmldir);
  suite_vx_lite_cvmhlabn_exec(xmldir);

  return 0;
}
