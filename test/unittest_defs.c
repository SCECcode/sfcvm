#define _DEFAULT_SOURCE  /* Required for gethostname */ 
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "unittest_defs.h"

int track_failure_count = 0;

void _reset_failure() {
    track_failure_count=0;
}

int _has_failure() {
    return track_failure_count;
}

int _success() {
   printf("PASS\n");
   return(0);
}

int _failure(char* estr) {
   track_failure_count ++;
   if(estr) {
     fprintf(stderr,"ERROR: %s\n", estr);
   }
   printf("FAIL\n");
   return(1);
}

int test_assert_file_exist(const char* filename)
{
  FILE *fp;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    fclose(fp);
    return(1);
  }
  return(0);
}

int test_assert_int(int val1, int val2)
{
  if (val1 != val2) {
    fprintf(stderr, "FAIL: assertion %d != %d\n", val1, val2);
    return(1);
  }
  return(0);
}

int test_assert_float(float val1, float val2)
{
  if (fabsf(val1 - val2) > 0.01) {
    fprintf(stderr, "FAIL: assertion %f != %f\n", val1, val2);
    return(1);
  }
  return(0);
}

// because of query using elevation, difference might be more obious
int test_assert_float_big(float val1, float val2)
{
  if (fabsf(val1 - val2) > 1) {
    fprintf(stderr, "FAIL: assertion %f != %f\n", val1, val2);
    return(1);
  }
  return(0);
}

int test_assert_double(double val1, double val2)
{
  if (fabs(val1 - val2) > 0.01) {
    fprintf(stderr, "FAIL: assertion %lf != %lf\n", val1, val2);
    return(1);
  }
  return(0);
}

char *_split(char *s, char *d, char *target) {
    // Get the first substring
    char *ss = strtok(s, d);
    while (ss != NULL) {
        if(strstr(ss,target)!=NULL) {
          // split after :
	  char *pos=strstr(ss,":");
          return pos+1;
        }

        // Get the next token
        ss = strtok(NULL, d);
    }
    return "";
}

/*
{ "lon":-121.9410,"lat":37.4550,"Z":0.000,"surf":1.493,"vs30":203.485,"crustal":"sfcvm",
   "cr_vp":1801.430,"cr_vs":500.477,"cr_rho":2015.827,"gtl":"none","gtl_vp":0.000,"gtl_vs":0.000,
   "gtl_rho":0.000,"cmb_algo":"crust","cmb_vp":1801.430,"cmb_vs":500.477,"cmb_rho":2015.827 }
*/
char *_split_vs_on_uline(char *uline) {
   return _split(uline,",","cmb_vs");
}

/*
vs:500.476797 vp:1801.430420 rho:2015.826750
*/
char *_split_vs_on_sline(char *sline) {
   return _split(sline," ","vs");
}

int _get_next_line(FILE *fp, char *line, char *target) {
    while(fgets(line, 500, fp) != NULL) {
      char *pos=strstr(line,target);
      if( pos == NULL) {
        continue;
        } else {
          return(0);
      }
      
    }
    return(1);
}

// file1 needs to skip a line
int test_assert_file_vs(const char *file1, const char *file2, int dontcare)
{
  FILE *fp1, *fp2;
  char line1[500], line2[500];

  fp1 = fopen(file1, "r");
  fp2 = fopen(file2, "r");
  if ((fp1 == NULL) || (fp2 == NULL)) {
    printf("FAIL: unable to open %s and/or %s\n", file1, file2);
    return(1);
  }
  // get to first line..
  // line1 is from sfcvm_query, check for vs 
  while(1) { 
  // line1 is from sfcvm_query, check for vs 
    int sfcvm_rc=_get_next_line(fp1, line1, "vs:");
    int ucvm_rc=_get_next_line(fp2, line2, "cmb_vs");
    if(sfcvm_rc || ucvm_rc) { 
        fclose(fp1); fclose(fp2); return(0);
    }

    // compare them
    //fprintf(stderr,"LINE1 %s\n",line1);
    //fprintf(stderr,"LINE2 %s\n",line2);
    char *ucvm_vs=_split_vs_on_uline(line2); 
    char *sfcvm_vs=_split_vs_on_sline(line1);
    
    if(dontcare) {
        fprintf(stderr,"  DONT CARE:  ucvm(%s) and sfcvm(%s)\n", ucvm_vs, sfcvm_vs);
        } else {
          if(test_assert_float(atof(ucvm_vs), atof(sfcvm_vs))) {
              fprintf(stderr,"  NOT MATCH: ucvm(%s) and sfcvm(%s)\n", ucvm_vs, sfcvm_vs);
              fclose(fp1); fclose(fp2); return(1);
              } else {
                  fprintf(stderr,"  MATCH:  ucvm(%s) and sfcvm(%s)\n", ucvm_vs, sfcvm_vs);
          }
    }
  }
  fclose(fp1); fclose(fp2);
  return (0);
}


int test_assert_file(const char *file1, const char *file2)
{
  FILE *fp1, *fp2;
  char line1[128], line2[128];

  fp1 = fopen(file1, "r");
  fp2 = fopen(file2, "r");
  if ((fp1 == NULL) || (fp2 == NULL)) {
    printf("FAIL: unable to open %s and/or %s\n", file1, file2);
    return(1);
  }
  while ((!feof(fp1)) && (!feof(fp2))) {
    memset(line1, 0, 128);
    memset(line2, 0, 128);
    fread(line1, 1, 127, fp1);
    fread(line2, 1, 127, fp2);
    if (test_assert_int(strcmp(line1, line2), 0) != 0) {
      printf("FAIL: %s and %s are of unequal length\n", file1, file2);
      return(1);
    }
  }
  if ((!feof(fp1)) || (!feof(fp2))) {
    printf("FAIL: %s and %s are of unequal length\n", file1, file2);
    return(1);
  }

  return(0);
}


/* Get time */
int test_get_time(time_t *ts)
{
  time(ts);
  return(0);
}



/* Test execution */
int test_run_suite(suite_t *suite)
{
  struct timeval start, end;

  int i;

  for (i = 0; i < suite->num_tests; i++) {
    gettimeofday(&start,NULL);
    if ((suite->tests[i].test_func)() != 0) {
      suite->tests[i].result = 1;
    } else {
      suite->tests[i].result = 0;
    }
    gettimeofday(&end,NULL);
    suite->tests[i].elapsed_time = (end.tv_sec - start.tv_sec) * 1.0 +
      (end.tv_usec - start.tv_usec) / 1000000.0;

  }

  return(0);
}


/* XML formatted logfiles */
FILE *init_log(const char *logfile)
{
  char line[256];
  FILE *lf;

  lf = fopen(logfile, "w");
  if (lf == NULL) {
    fprintf(stderr, "Failed to initialize logfile %s\n", logfile);
    return(NULL);
  }

  strcpy(line, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
  fwrite(line, 1, strlen(line), lf);

  return(lf);
}


int close_log(FILE *lf)
{
  if (lf != NULL) {
    fclose(lf);
  }
  return(0);
}


int write_log(FILE *lf, suite_t *suite)
{
  char hostname[256];
  char datestr[256];
  char line[1000];
  int i;
  int num_fail = 0;
  double suite_elapsed = 0.0;
  struct tm *tmp;
  
  for (i = 0; i < suite->num_tests; i++) {
    if (suite->tests[i].result != 0) {
      num_fail++;
    }
    suite_elapsed = suite_elapsed + suite->tests[i].elapsed_time;
  }

  /* Get host name */
  if (gethostname(hostname, (size_t)256) != 0) {
    return(1);
  }

  /* Get timestamp */
  tmp = localtime(&(suite->exec_time));
  if (tmp == NULL) {
    fprintf(stderr, "Failed to retrieve time\n");
    return(1);
  }
  if (strftime(datestr, 256, "%Y-%m-%dT%H:%M:%S", tmp) == 0) {
    return(1);
  }

  if (lf != NULL) {
    sprintf(line, "<testsuite errors=\"0\" failures=\"%d\" hostname=\"%s\" name=\"%s\" tests=\"%d\" time=\"%lf\" timestamp=\"%s\">\n", num_fail, hostname, suite->suite_name, suite->num_tests, suite_elapsed, datestr);
    fwrite(line, 1, strlen(line), lf);

    for (i = 0; i < suite->num_tests; i++) {
      sprintf(line, "  <testcase classname=\"C func\" name=\"%s\" time=\"%lf\">\n",
	      suite->tests[i].test_name, suite->tests[i].elapsed_time);
      fwrite(line, 1, strlen(line), lf);

      if (suite->tests[i].result != 0) {
	sprintf(line, " <failure message=\"fail\" type=\"test failed\">test case FAIL</failure>\n");
	fwrite(line, 1, strlen(line), lf);
      }

      sprintf(line, "  </testcase>\n");
      fwrite(line, 1, strlen(line), lf);
    }

    sprintf(line, "</testsuite>\n");
    fwrite(line, 1, strlen(line), lf);

  }

  return(0);
}

