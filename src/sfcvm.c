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
static const size_t sfcvm_numValues = 3;
static const char* const sfcvm_valueNames[3] = { "Vp", "Vs", "density" };

char* sfcvm_filenames[10];  // max is 10
int sfcvm_filenames_cnt;

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
    sfcvm_filenames_cnt =sfcvm_configuration->data_cnt;
    if(sfcvm_filenames_cnt > 10) {
        fprintf(stderr,"BADD");
        exit(1);
    }

    for(int i=0; i < 2; i++) {
       sfcvm_filenames[i]= (char *)calloc(1,
           strlen(dir)+(strlen(sfcvm_configuration->model_dir)*2)+strlen(sfcvm_configuration->data_files[i]) +15);
       sprintf(sfcvm_filenames[i],"%s/model/%s/data/%s/%s",
           dir,
           sfcvm_configuration->model_dir,
           sfcvm_configuration->model_dir,
           sfcvm_configuration->data_files[i]);

       fprintf(stderr,"using %s\n", sfcvm_filenames[i]);
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
    case UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF:
      sfcvm_force_depth = va_arg(ap, int);
      break;
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
from ucvm_query, the depth is already proprocessed with (ucvm_surface - user_elevation)
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

// NOTE: even though 3rd item in points struct is name 'depth', it could be depth in
// elevation data model or depth in depth data model 

    double values[sfcvm_numValues];
    double entry_latitude;
    double entry_longitude;
    double entry_depth;
    double entry_elevation;
    double surfaceElev;

    for(int i=0; i<numpoints; i++) {

      /* Force depth mode if directed and point is above surface */
      if ((sfcvm_force_depth) && (sfcvm_zmode == ZMODE_ELEV) &&
          (points[i].depth < 0.0)) {

fprintf(stderr," **** in HERE looking for a new surface ..\n");
        /* Setup point to query */
        entry_longitude=points[i].longitude;
        entry_latitude=points[i].latitude;

        // need to retrieve sfcvm elevation before calling undelying model
        surfaceElev = geomodelgrids_squery_queryTopElevation(sfcvm_query_obj, entry_latitude, entry_longitude);

        if(sfcvm_ucvm_debug) {
           fprintf(stderr, "sfcvm_query: surface is %f vs initial query depth %f\n", surfaceElev, points[i].depth);
        }

        if (surfaceElev - NO_DATA < 0.01) {
          /* Fallback to using UCVM topo */
          entry_elevation=points[i].depth;
        } else {
          if(sfcvm_ucvm_debug) {
            fprintf(stderr,"POTENTIAL problem if sfcvm surface differ from UCVM surface !!!\n");
          }
          entry_elevation=surfaceElev - points[i].depth;
        }
      } else {
        /* Setup with direct point to query */
        entry_longitude=points[i].longitude;
        entry_latitude=points[i].latitude;
        entry_elevation=points[i].depth;
      }


      if(sfcvm_ucvm_debug) {
        fprintf(stderr," **** Calling squery with..lon(%lf) lat(%lf) elevation(%lf) \n\n",
                                                 entry_longitude, entry_latitude, entry_elevation);
      }

      int err = geomodelgrids_squery_query(sfcvm_query_obj, values, entry_latitude, entry_longitude, entry_elevation);

      if(sfcvm_ucvm_debug) {
        fprintf(stderr,"rc from calling squery ==> %d\n", err);
      }
      if(err) {
        data[i].vp=-1;
        data[i].vs=-1;
        data[i].rho=-1;
        } else {
            data[i].vp=values[0];
            data[i].vs=values[1];
            data[i].rho=values[2];
        }
  }
  return UCVM_CODE_SUCCESS;
}

void _free_sfcvm_configuration(sfcvm_configuration_t *config) {

    for(int i=0; i< config->data_cnt; i++) {
       free(config->data_labels[i]);
       free(config->data_files[i]);
    }
    free(config);
}

void _dump_sfcvm_configuration(sfcvm_configuration_t *config) {
    for(int i=0; i< config->data_cnt; i++) {
       fprintf(stderr,"    <%d>  %s: %s\n",i,config->data_labels[i], config->data_files[i]);
    }
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

// skip double-quote
int _skip(char *str) {
    char *p = strchr(str, '"');
    if (p == NULL) { return 0; }
    return p-str;
}

char *_search(char *str, char *label, char** vptr) {
    char value[1000];
    str = strstr(str, label);
    if (str == NULL) { return NULL; }
    str += (strlen(label)+2);
    str = strchr(str, '"');
    if (str == NULL) { return NULL; }
    // parse out a string
    sscanf(str,"\"%s",value);
    int len=strlen(value);
    str += (len+2);
    int s=_skip(value);
    if(s) { value[s] = '\0'; }
    *vptr=strdup(value);
    return str;
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
    char value[2000];
    char line_holder[2000];

fprintf(stderr,"READING configuration..\n");

    // If our file pointer is null, an error has occurred. Return fail.
    if (fp == NULL) {
        return UCVM_CODE_ERROR;
    }

    // Read the lines in the configuration file.
    while (fgets(line_holder, sizeof(line_holder), fp) != NULL) {
        if (line_holder[0] != '#' && line_holder[0] != ' ' && line_holder[0] != '\n') {
            sscanf(line_holder, "%s = %s", key, value);
            if(sfcvm_ucvm_debug) fprintf(stderr," >> %s", line_holder);

            // Which variable are we editing?
            if (strcmp(key, "utm_zone") == 0) {
                config->utm_zone = atoi(value);
            } else if (strcmp(key, "model_dir") == 0) {
                sprintf(config->model_dir, "%s", value);
            } else if (strcmp(key, "data_files") == 0) {
                int idx=0;
                char *p = strchr(line_holder, '=');
                char *ptr=&line_holder[p-line_holder];

                while (ptr) {
                    // look for the label
                    ptr=_search(ptr,"LABEL",&config->data_labels[idx]);
                    if(ptr == NULL) break;
                    ptr=_search(ptr,"NAME",&config->data_files[idx]);
                    if(ptr == NULL) break;
                    idx=idx+1;
                    if (*ptr == ']') break;
                    ptr++;
                }
                config->data_cnt=idx;
            }
       }

    }
    if(sfcvm_ucvm_debug) _dump_sfcvm_configuration(config);
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
