#ifndef VX_IO_H
#define VX_IO_H

#include <stdlib.h>
#include <stdio.h>

typedef enum { VX_PNUMBER_VP = 1, VX_PNUMBER_TAG=2, VX_PNUMBER_VS=3 } vx_pnumber_t;

/* Initialize voxel prop reader */
int vx_io_init(char *);


/* Finalize voxel prop reader */
int vx_io_finalize();

/* Get property key from voxel property file */
int vx_io_getpropkey(char *);

/* Get vector from voxel property file */
int vx_io_getvec(char *,float *);


/* Get model dimensions from voxel property file */
int vx_io_getdim(char *,int *);


/* Get property name from voxel property file */
int vx_io_getpropname(char *, vx_pnumber_t, char *);


/* Get property size from voxel property file */
int vx_io_getpropsize(char *, vx_pnumber_t, int *);


/* Get property value from voxel property file */
int vx_io_getpropval(char *, vx_pnumber_t, float *);


/* Load voxel volume from disk to memory. Translate 
   endian if necessary */
int vx_io_loadvolume(const char *, const char *, int, int, char *);


#endif
