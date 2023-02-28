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

#include <assert.h>

#include "ucvm_model_dtypes.h"
#include "sfcvm.h"

#include "geomodelgrids/serial/cquery.h"
#include "geomodelgrids/utils/cerrorhandler.h"


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


/* Values and order to be returned in queries.  */
static const size_t sfcvm_numValues = 2;
static const char* const sfcvm_valueNames[2] = { "two", "one" };
char* sfcvm_filenames[2];
int sfcvm_filenames_cnt;

// temp one, need to replace later
static const size_t sfcvm_numModels = 2;

/* Coordinate reference system of points passed to queries.
*
* The string can be in the form of EPSG:XXXX, WKT, or Proj
* parameters. In this case, we will specify the coordinates in
* latitude, longitude, elevation in the WGS84 horizontal
* datum. The elevation is with respect to the WGS84 ellipsoid.
*/
const char* const sfcvm_crs = "EPSG:4326";
const size_t sfcvm_spaceDim = 3;

void* sfcvm_query_obj;

/*************************************/

int sfcvm_ucvm_debug=1;
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
    sfcvm_configuration = (sfcvm_configuration_t *)calloc(1, sizeof(sfcvm_configuration_t));
    sfcvm_velocity_model = (sfcvm_model_t *)calloc(1, sizeof(sfcvm_model_t));
    sfcvm_config_string = (char *)calloc(SFCVM_CONFIG_MAX, sizeof(char));

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


/* Create and initialize serial query object using the parameters  stored in local variables.  */
    sfcvm_query_obj = geomodelgrids_squery_create();
    assert(sfcvm_query_obj);

    // dir/data/model_dir/filename
    sfcvm_filenames_cnt =sfcvm_configuration->data_filenames_cnt;
    for(int i=0; i < sfcvm_filenames_cnt; i++) {
       sfcvm_filenames[i]= (char *)calloc(1,
         strlen(dir)+(strlen(sfcvm_configuration->model_dir)*2)+strlen(sfcvm_configuration->data_filenames[i]) +15);
       sprintf(sfcvm_filenames[i],"%s/model/%s/data/%s/%s",
           dir,
           sfcvm_configuration->model_dir,
           sfcvm_configuration->model_dir,
           sfcvm_configuration->data_filenames[i]);
//fprintf(stderr,"sfcvm filename %s\n",sfcvm_filenames[i]);
    }

    int err = geomodelgrids_squery_initialize(sfcvm_query_obj, sfcvm_filenames,
                  sfcvm_filenames_cnt, sfcvm_valueNames, sfcvm_numValues, sfcvm_crs);
    assert(!err);

/* Log warnings and errors to "error.log". */
    void* errorHandler = geomodelgrids_squery_getErrorHandler(sfcvm_query_obj);
    assert(errorHandler);
    geomodelgrids_cerrorhandler_setLogFilename(errorHandler, "sfcvm_error.log");

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

    double values[sfcvm_numValues];
    for(int i=0; i<numpoints; i++) {

        double latitude=points[i].latitude;
        double longitude=points[i].longitude;
        double depth=points[i].depth; // UCVM depth = ucvm_surf - elevation 
        double elevation;

        // need to calculate elevation before calling undelying model

        const double surfaceElev = geomodelgrids_squery_queryTopElevation(sfcvm_query_obj, latitude, longitude);

        if(surfaceElev != NO_DATA) {
            elevation = surfaceElev - depth;
            } else { 
               // or use ucvm surface ???
               // elevation = points[i].surf - depth;
	       sfcvm_print_error("BAD.. no surface data here.");
	       return UCVM_CODE_ERROR;
        }
            

if(sfcvm_ucvm_debug) {
fprintf(stderr,"\n **** Calling squery sfcvm surface (%lf) supplied depth(%lf) \n", surfaceElev, depth);
fprintf(stderr," **** Calling squery with..lon(%lf) lat(%lf) elevation(%lf) \n\n",
                  longitude, latitude, elevation);
}

/*** XXX ??? need to do elevation squashing ???  ****/

        int err = geomodelgrids_squery_query(sfcvm_query_obj, values, latitude, longitude, elevation);

fprintf(stderr,"calling query error code %d\n", err);
        if(err) {
            data[i].vp=-1;
            data[i].vs=-1;
            data[i].rho=-1;
            } else {
                data[i].vp=values[0];
                data[i].vs=values[1];
// XX not sure where is this..
                data[i].rho=-1;
        }
  }
  return UCVM_CODE_SUCCESS;
}

void _free_sfcvm_configuration(sfcvm_configuration_t *config) {

    for(int i=0; i< config->data_filenames_cnt; i++) {
       free(config->data_filelabels[i]);
       free(config->data_filenames[i]);
    }
    free(config);
}

/**
 * Called when the model is being discarded. Free all variables.
 *
 * @return UCVM_CODE_SUCCESS
 */
int sfcvm_finalize() {
    sfcvm_is_initialized = 0;

    _free_sfcvm_configuration(sfcvm_configuration);
    free(sfcvm_velocity_model);
    free(sfcvm_config_string);

    for(int i=0; i<sfcvm_filenames_cnt; i++) {
        free(sfcvm_filenames[i]);
    }

/* Destroy query object. */
    geomodelgrids_squery_destroy(&sfcvm_query_obj);
    assert(!sfcvm_query_obj);

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
    char line_holder[1280];

    // If our file pointer is null, an error has occurred. Return fail.
    if (fp == NULL) {
        return UCVM_CODE_ERROR;
    }

    // Read the lines in the configuration file.
    while (fgets(line_holder, sizeof(line_holder), fp) != NULL) {
        if (line_holder[0] != '#' && line_holder[0] != ' ' && line_holder[0] != '\n') {
            sscanf(line_holder, "%s = %s", key, value);

            // Which variable are we editing?
            if (strcmp(key, "utm_zone") == 0) {
                config->utm_zone = atoi(value);
            } else if (strcmp(key, "model_dir") == 0) {
                sprintf(config->model_dir, "%s", value);
            } else if (strcmp(key, "data_filenames") == 0) {
                config->data_filenames_cnt=sfcvm_numModels;

                config->data_filelabels[0]=(char *)calloc(1, strlen("topo")+1);
                strcpy(config->data_filelabels[0],"topo");
                config->data_filelabels[1]=(char *)calloc(1, strlen("flat")+1);
                strcpy(config->data_filelabels[1],"flat");

                config->data_filenames[0]=(char *)calloc(1,strlen("one-block-topo.h5")+1);
                strcpy(config->data_filenames[0],"one-block-topo.h5");
                config->data_filenames[1]=(char *)calloc(1, strlen("three-blocks-flat.h5")+1);
                strcpy(config->data_filenames[1],"three-blocks-flat.h5");
//[ { "LABEL" : "topo", "NAME" : "one-block-topo.h5" },
     { "LABEL" : "flat", "NAME" : "three-blocks-flat.h5" } ]
//
                char *ptr = value;
                char *value;
                
                while (ptr) {
                    int rc=sscanf(ptr," %s : %s 
                    ptr = strstr(ptr, "\"LABEL\"");
                    if (ptr == NULL) {
                        break;
                    }
                    ptr += (strlen("LABEL")+2)
                    ptr = strchr(ptr, ':');
                    if (ptr == NULL) {
                        break;
                    }
                    ptr++;
                    // parse out a string
                    char TMP[100];
                    sscanf(ptr,"\"%s\",TMP);
// pick up values
                    value = strtol(ptr, &ptr, 10);
                    if (*ptr != '}') {
                        break;
                    }
                    ptr++;
                    printf("%lu\n", value);
                }
                return 1;
            }
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
