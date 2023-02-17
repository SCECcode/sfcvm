/**
 * @file sfcvm.c
 * @brief Main file for SFCVM library.
 * @author - SCEC 
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Delivers San Francisco Community Velocity Model
 *
 */

#include "ucvm_model_dtypes.h"
#include "sfcvm.h"

/************ Constants and Variables ********/
/** The version of the model. */
const char *sfcvm_version_string = "SFCVM";

/** The config of the model */
char *sfcvm_config_string;

int sfcvm_is_initialized = 0;

/** Location of the binary data files. */
char sfcvm_data_directory[2000];

/** Configuration parameters. */
sfcvm_configuration_t *sfcvm_configuration;

/** Holds pointers to the velocity model data OR indicates it can be read from file. */
sfcvm_model_t *sfcvm_velocity_model;

/** The height of this model's region, in meters. */
double sfcvm_total_height_m = 0;

/** The width of this model's region, in meters. */
double sfcvm_total_width_m = 0;
/*************************************/

int sfcvm_ucvm_debug=0;
int sfcvm_force_depth=0;
int sfcvm_zmode=0; // ZMODE_DEPTH or ZMODE_ELEVATION

/**
 * Initializes the SFCVM plugin model within the UCVM framework. In order to initialize
 * the model, we must provide the UCVM install path and optionally a place in memory
 * where the model already exists.
 *
 * @param dir The directory in which UCVM has been installed.
 * @param label A unique identifier for the velocity model.
 * @return Success or failure, if initialization was successful.
 */
int sfcvm_init(const char *dir, const char *label) {
	char configbuf[512];

	// Initialize variables.
	sfcvm_configuration = calloc(1, sizeof(sfcvm_configuration_t));
	sfcvm_velocity_model = calloc(1, sizeof(sfcvm_model_t));
	sfcvm_config_string = calloc(SFCVM_CONFIG_MAX, sizeof(char));
        sfcvm_config_string[0]='\0';

	// Configuration file location when built with UCVM
	sprintf(configbuf, "%s/model/%s/data/config", dir, label);

	// Read the configuration file.
	if (sfcvm_read_configuration(configbuf, sfcvm_configuration) != UCVM_CODE_SUCCESS) {

           // Try another, when is running in standalone mode..
	   sprintf(configbuf, "%s/data/config", dir);
	   if (sfcvm_read_configuration(configbuf, sfcvm_configuration) != UCVM_CODE_SUCCESS) {
               sfcvm_print_error("No configuration file was found to read from.");
               return UCVM_CODE_ERROR;
               } else {
	       // Set up the data directory.
	       sprintf(sfcvm_data_directory, "%s/data/%s", dir, sfcvm_configuration->model_dir);
           }
           } else {
	   // Set up the data directory.
	   sprintf(sfcvm_data_directory, "%s/model/%s/data/%s", dir, label, sfcvm_configuration->model_dir);
        }

        /* setup config_string  interp=0 or interp= 1*/
        sprintf(sfcvm_config_string,"config = %s, interp = %d\n",configbuf, sfcvm_configuration->interp);
	// Let everyone know that we are initialized and ready for business.
	sfcvm_is_initialized = 1;

	return UCVM_CODE_SUCCESS;
}

/**  
  * 
**/

/* Setparam SFCVM */
int sfcvm_setparam(int id, int param, ...)
{
  va_list ap;
  int zmode;

  va_start(ap, param);

  switch (param) {
    case UCVM_PARAM_QUERY_MODE:
      zmode = va_arg(ap,int);
      switch (zmode) {
        case UCVM_COORD_GEO_DEPTH:
          sfcvm_zmode = ZMODE_DEPTH;
          if(sfcvm_ucvm_debug) fprintf(stderr,"calling sfcvm_setparam >>  depth\n");
          break;
        case UCVM_COORD_GEO_ELEV:
/*****
even if ucvm_query set elevation mode, still need to run as depth
from ucvm_query, the depth is already proprocessed with (ucvm_surface - elevation)
          sfcvm_zmode = ZMODE_ELEV;
****/
          if(sfcvm_ucvm_debug) fprintf(stderr,"calling sfcvm_setparam >>  elevation\n");
          break;
        default:
          break;
       }
       break;
  }
  va_end(ap);
  return UCVM_CODE_SUCCESS;
}


/**
 * Queries SFCVM at the given points and returns the data that it finds.
 *
 * @param points The points at which the queries will be made.
 * @param data The data that will be returned (Vp, Vs, density, Qs, and/or Qp).
 * @param numpoints The total number of points to query.
 * @return UCVM_CODE_SUCCESS or UCVM_CODE_ERROR.
 */
int sfcvm_query(sfcvm_point_t *points, sfcvm_properties_t *data, int numpoints) {

  // setup >> points -> entry (assume always Q in depth)
  // retrieve >> entry -> data

//  if(sfcvm_zmode == ZMODE_ELEV) { fprintf(stderr,"sfcvm_query: ZMODE= elev\n"); }
//  if(sfcvm_zmode == ZMODE_DEPTH) { fprintf(stderr,"sfcvm_query: ZMODE= dep\n"); }

  for(int i=0; i<numpoints; i++) {

if(sfcvm_ucvm_debug) {
fprintf(stderr,"\n **** get incoming DATA ..(%lf %lf %lf) \n",
                  points[i].longitude, points[i].latitude, points[i].depth);
}
    /*
       By the time here, Conditions:

       Following condition must have met,
         1) Point data has not been filled in by previous model
         2) Point falls in crust or interpolation zone
         3) Point falls within the configured model region
     */

      /* Force depth mode if directed and point is above surface */
      if ((sfcvm_force_depth) && (sfcvm_zmode == ZMODE_ELEV) &&
          (points[i].depth < 0.0)) {
      }

      data[i].vp=-1;
      data[i].vs=-1;
      data[i].rho=-1;

  }
  return UCVM_CODE_SUCCESS;
}

/**
 * Called when the model is being discarded. Free all variables.
 *
 * @return UCVM_CODE_SUCCESS
 */
int sfcvm_finalize() {
	sfcvm_is_initialized = 0;

	free(sfcvm_configuration);
	free(sfcvm_velocity_model);
	free(sfcvm_config_string);

	return UCVM_CODE_SUCCESS;
}

/**
 * Returns the version information.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int sfcvm_version(char *ver, int len)
{
  //const char *sfcvm_version_string = "SFCVM";
  return UCVM_CODE_SUCCESS;
}

/**
 * Returns the model config information.
 *
 * @param key Config key string to return.
 * @return Zero
 */
int sfcvm_config(char **config, int *sz)
{
  int len=strlen(sfcvm_config_string);
  if(len > 0) {
    *config=sfcvm_config_string;
    return UCVM_CODE_SUCCESS;
  }
  return UCVM_CODE_ERROR;
}


/**
 * Reads the configuration file describing the various properties of SFCVM and populates
 * the configuration struct. This assumes configuration has been "calloc'ed" and validates
 * that each value is not zero at the end.
 *
 * @param file The configuration file location on disk to read.
 * @param config The configuration struct to which the data should be written.
 * @return Success or failure, depending on if file was read successfully.
 */
int sfcvm_read_configuration(char *file, sfcvm_configuration_t *config) {
	FILE *fp = fopen(file, "r");
	char key[40];
	char value[80];
	char line_holder[128];

	// If our file pointer is null, an error has occurred. Return fail.
	if (fp == NULL) {
		return UCVM_CODE_ERROR;
	}

        config->interp=0;

	// Read the lines in the configuration file.
	while (fgets(line_holder, sizeof(line_holder), fp) != NULL) {
		if (line_holder[0] != '#' && line_holder[0] != ' ' && line_holder[0] != '\n') {
			sscanf(line_holder, "%s = %s", key, value);

			// Which variable are we editing?
			if (strcmp(key, "utm_zone") == 0)
  				config->utm_zone = atoi(value);
			if (strcmp(key, "model_dir") == 0)
				sprintf(config->model_dir, "%s", value);
			if (strcmp(key, "interp") == 0)
  				config->interp = atoi(value);
		}
	}

	// Have we set up all configuration parameters?
	if (config->utm_zone == 0 || config->model_dir[0] == '\0' ) {
		sfcvm_print_error("One configuration parameter not specified. Please check your configuration file.");
		return UCVM_CODE_ERROR;
	}

	fclose(fp);

	return UCVM_CODE_SUCCESS;
}

/*
 * @param err The error string to print out to stderr.
 */
void sfcvm_print_error(char *err) {
	fprintf(stderr, "An error has occurred while executing SFCVM. The error was:\n\n");
	fprintf(stderr, "%s", err);
	fprintf(stderr, "\n\nPlease contact software@scec.org and describe both the error and a bit\n");
	fprintf(stderr, "about the computer you are running SFCVM on (Linux, Mac, etc.).\n");
}

// The following functions are for dynamic library mode. If we are compiling
// a static library, these functions must be disabled to avoid conflicts.
#ifdef DYNAMIC_LIBRARY

/**
 * Init function loaded and called by the UCVM library. Calls sfcvm_init.
 *
 * @param dir The directory in which UCVM is installed.
 * @return Success or failure.
 */
int model_init(const char *dir, const char *label) {
	return sfcvm_init(dir, label);
}

/**
 * Query function loaded and called by the UCVM library. Calls sfcvm_query.
 *
 * @param points The basic_point_t array containing the points.
 * @param data The basic_properties_t array containing the material properties returned.
 * @param numpoints The number of points in the array.
 * @return Success or fail.
 */
int model_query(sfcvm_point_t *points, sfcvm_properties_t *data, int numpoints) {
	return sfcvm_query(points, data, numpoints);
}

/**
 * Setparam function loaded and called by the UCVM library. Calls sfcvm_setparam.
 *
 * @param id  don'care
 * @param param 
 * @param val, it is actually just 1 int
 * @return Success or fail.
 */
int model_setparam(int id, int param, int val) {
	return sfcvm_setparam(id, param, val);
}

/**
 * Finalize function loaded and called by the UCVM library. Calls sfcvm_finalize.
 *
 * @return Success
 */
int model_finalize() {
	return sfcvm_finalize();
}

/**
 * Version function loaded and called by the UCVM library. Calls sfcvm_version.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int model_version(char *ver, int len) {
	return sfcvm_version(ver, len);
}

/**
 * Version function loaded and called by the UCVM library. Calls sfcvm_config.
 *
 * @param config Config string to return.
 * @param sz length of Config terms.
 * @return Zero
 */
int model_config(char **config, int *sz) {
        return sfcvm_config(config, sz);
}


int (*get_model_init())(const char *, const char *) {
        return &sfcvm_init;
}
int (*get_model_query())(sfcvm_point_t *, sfcvm_properties_t *, int) {
         return &sfcvm_query;
}
int (*get_model_finalize())() {
         return &sfcvm_finalize;
}
int (*get_model_version())(char *, int) {
         return &sfcvm_version;
}
int (*get_model_config())(char **, int*) {
         return &sfcvm_config;
}
int (*get_model_setparam())(int, int, ...) {
         return &sfcvm_setparam;
}




#endif
