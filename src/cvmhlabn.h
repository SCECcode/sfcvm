#ifndef CVMHLABN_H
#define CVMHLABN_H

/**
 * @file cvmhlabn.h
 * @brief Main header file for CVMHLABN library.
 * @author - SCEC 
 * @version 1.0
 *
 * Delivers CVMH CVMH15-1 Los Angeles Basin Velocity Model
 *
 */

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdarg.h>

#include "vx_sub_cvmhlabn.h"

// Constants
#ifndef M_PI
	/** Defines pi */
	#define M_PI 3.14159265358979323846
#endif

#define VX_NO_DATA -99999.0
#define CVMHLABN_CONFIG_MAX 1000

// Structures
/** Defines a point (latitude, longitude, and depth) in WGS84 format */
typedef struct cvmhlabn_point_t {
	/** Longitude member of the point */
	double longitude;
	/** Latitude member of the point */
	double latitude;
	/** Depth member of the point */
	double depth;
} cvmhlabn_point_t;

/** Defines the material properties this model will retrieve. */
typedef struct cvmhlabn_properties_t {
	/** P-wave velocity in meters per second */
	double vp;
	/** S-wave velocity in meters per second */
	double vs;
	/** Density in g/m^3 */
	double rho;
        /** NOT USED from basic_property_t */
        double qp;
        /** NOT USED from basic_property_t */
        double qs;
} cvmhlabn_properties_t;

/** The CVMHLABN configuration structure. */
typedef struct cvmhlabn_configuration_t {
	/** The zone of UTM projection */
	int utm_zone;
	/** The model directory */
	char model_dir[1000];
        /** interp */
	int interp;

} cvmhlabn_configuration_t;

/** The model structure which points to available portions of the model. */
typedef struct cvmhlabn_model_t {
	/** A pointer to the Vp data either in memory or disk. Null if does not exist. */
	void *vp;
	/** Vp status: 0 = not found, 1 = found and not in memory, 2 = found and in memory */
	int vp_status;
} cvmhlabn_model_t;

// Constants
/** The version of the model. */
extern const char *cvmhlabn_version_string;

/** The config of the model. */
extern char *cvmhlabn_config_string;
extern int cvmhlabn_config_sz;

// Variables
/** Set to 1 when the model is ready for query. */
extern int cvmhlabn_is_initialized;

/** Location of the binary data files. */
extern char cvmhlabn_data_directory[2000];

/** Configuration parameters. */
extern cvmhlabn_configuration_t *cvmhlabn_configuration;
/** Holds pointers to the velocity model data OR indicates it can be read from file. */
extern cvmhlabn_model_t *cvmhlabn_velocity_model;

/** The height of this model's region, in meters. */
extern double cvmhlabn_total_height_m;
/** The width of this model's region, in meters. */
extern double cvmhlabn_total_width_m;

// UCVM API Required Functions

#ifdef DYNAMIC_LIBRARY

/** Initializes the model */
int model_init(const char *dir, const char *label);
/** Cleans up the model (frees memory, etc.) */
int model_finalize();
/** Returns version information */
int model_version(char *ver, int len);
/** Returns config information */
int model_config(char **config, int *sz);
/** Queries the model */
int model_query(cvmhlabn_point_t *points, cvmhlabn_properties_t *data, int numpts);
/** Setparam */
int model_setparam(int, int, int);

#endif

// CVMHLABN Related Functions

/** Initializes the model */
int cvmhlabn_init(const char *dir, const char *label);
/** Cleans up the model (frees memory, etc.) */
int cvmhlabn_finalize();
/** Returns version information */
int cvmhlabn_version(char *ver, int len);
/** Queries the model */
int cvmhlabn_query(cvmhlabn_point_t *points, cvmhlabn_properties_t *data, int numpts);
/** Setparam*/
int cvmhlabn_setparam(int, int, ...);

// Non-UCVM Helper Functions
/** Reads the configuration file. */
int cvmhlabn_read_configuration(char *file, cvmhlabn_configuration_t *config);
void cvmhlabn_print_error(char *err);
int cvmhlabn_setzmode(char* z);

#endif
