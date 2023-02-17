/** 
    util.c - Commonly used math and interpolation routines.
**/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "util.h"


/* Determine system endian */
int util_system_endian()
{
  int num = 1;
  if(*(char *)&num == 1) {
    return UTIL_BYTEORDER_LSB;
  } else {
    return UTIL_BYTEORDER_MSB;
  }
}

/*
 * util_minf
 * Return min(v1, v2)
 * Get minimum of two float values
 */
float util_minf(float v1, float v2)
{
  if (v1 < v2)
    return v1;
  else
    return v2;
}


/* Interpolate value linearly between two end points */
double util_interpolate(double v1, double v2, double ratio) {
  return(ratio*v2 + v1*(1-ratio));
}


/* 2D Distance */
double util_dist_2d(double x1, double y1, double x2, double y2) {
  return(sqrt(pow(x2-x1, 2.0) + pow(y2-y1, 2.0)));
}
