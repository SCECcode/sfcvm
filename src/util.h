#ifndef UTIL_H
#define UTIL_H

/* Byte order */
typedef enum { UTIL_BYTEORDER_LSB = 0, 
               UTIL_BYTEORDER_MSB } util_byteorder_t;

/* Determine system endian */
int util_system_endian();

/* Minimum of two values */
float util_minf(float v1, float v2);

/* Interpolate between two values */
double util_interpolate(double v1, double v2, double ratio);

/* 2D distance */
double util_dist_2d(double x1, double y1, double x2, double y2);

#endif
