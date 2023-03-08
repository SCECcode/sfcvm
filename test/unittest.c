#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "unittest_defs.h"
#include "test_ucvm_sfcvm_exec.h"
#include "test_sfcvm_exec.h"


int main (int argc, char *argv[])
{
  char *xmldir;

  if (argc == 2) {  
    xmldir = argv[1];
  } else {
    xmldir = NULL;
  }

  /* Run test suites */
  suite_sfcvm_exec(xmldir);
  suite_ucvm_sfcvm_exec(xmldir);

  return 0;
}
