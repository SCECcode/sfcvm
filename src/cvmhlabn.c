/**
 * @file cvmhlabn.c
 * @brief Main file for CVMHLABN library.
 * @author - SCEC 
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Delivers CVMH CVMH15-1 Los Angeles Basin Velocity Model
 *
 */

#include "ucvm_model_dtypes.h"
#include "cvmhlabn.h"

/************ Constants and Variables ********/
/** The version of the model. */
const char *cvmhlabn_version_string = "CVMHLABN";

/** The config of the model */
char *cvmhlabn_config_string;
int cvmhlabn_config_sz=0;

int cvmhlabn_is_initialized = 0;

/** Location of the binary data files. */
char cvmhlabn_data_directory[2000];

/** Configuration parameters. */
cvmhlabn_configuration_t *cvmhlabn_configuration;

/** Holds pointers to the velocity model data OR indicates it can be read from file. */
cvmhlabn_model_t *cvmhlabn_velocity_model;

/** The height of this model's region, in meters. */
double cvmhlabn_total_height_m = 0;

/** The width of this model's region, in meters. */
double cvmhlabn_total_width_m = 0;
/*************************************/

int cvmhlabn_ucvm_debug=0;
int cvmhlabn_force_depth = 0;
int cvmhlabn_zmode = VX_ZMODE_DEPTH;

/**
 * Initializes the CVMHLABN plugin model within the UCVM framework. In order to initialize
 * the model, we must provide the UCVM install path and optionally a place in memory
 * where the model already exists.
 *
 * @param dir The directory in which UCVM has been installed.
 * @param label A unique identifier for the velocity model.
 * @return Success or failure, if initialization was successful.
 */
int cvmhlabn_init(const char *dir, const char *label) {
	char configbuf[512];

	// Initialize variables.
	cvmhlabn_configuration = calloc(1, sizeof(cvmhlabn_configuration_t));
	cvmhlabn_velocity_model = calloc(1, sizeof(cvmhlabn_model_t));
	cvmhlabn_config_string = calloc(CVMHLABN_CONFIG_MAX, sizeof(char));
        cvmhlabn_config_string[0]='\0';

	// Configuration file location when built with UCVM
	sprintf(configbuf, "%s/model/%s/data/config", dir, label);

	// Read the configuration file.
	if (cvmhlabn_read_configuration(configbuf, cvmhlabn_configuration) != UCVM_CODE_SUCCESS) {

           // Try another, when is running in standalone mode..
	   sprintf(configbuf, "%s/data/config", dir);
	   if (cvmhlabn_read_configuration(configbuf, cvmhlabn_configuration) != UCVM_CODE_SUCCESS) {
               cvmhlabn_print_error("No configuration file was found to read from.");
               return UCVM_CODE_ERROR;
               } else {
	       // Set up the data directory.
	       sprintf(cvmhlabn_data_directory, "%s/data/%s", dir, cvmhlabn_configuration->model_dir);
           }
           } else {
	   // Set up the data directory.
	   sprintf(cvmhlabn_data_directory, "%s/model/%s/data/%s", dir, label, cvmhlabn_configuration->model_dir);
        }

        /* Init vx */
        if (vx_setup(cvmhlabn_data_directory, cvmhlabn_configuration->interp) != 0) {
          return UCVM_CODE_ERROR;
        }

        /* setup config_string  interp=0 or interp= 1*/
        sprintf(cvmhlabn_config_string,"config = %s, interp = %d\n",configbuf, cvmhlabn_configuration->interp);
        cvmhlabn_config_sz=2;

	// Let everyone know that we are initialized and ready for business.
	cvmhlabn_is_initialized = 1;

	return UCVM_CODE_SUCCESS;
}

/**  
  * 
**/

/* Setparam CVMHLABN */
int cvmhlabn_setparam(int id, int param, ...)
{
  va_list ap;
  int zmode;

  va_start(ap, param);

  switch (param) {
    case UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF:
      cvmhlabn_force_depth = va_arg(ap, int);
      break;
    case UCVM_PARAM_QUERY_MODE:
      zmode = va_arg(ap,int);
      switch (zmode) {
        case UCVM_COORD_GEO_DEPTH:
          cvmhlabn_zmode = VX_ZMODE_DEPTH;
          if(cvmhlabn_ucvm_debug) fprintf(stderr,"calling cvmhlabn_setparam >>  depth\n");
          break;
        case UCVM_COORD_GEO_ELEV:
/*****
even if ucvm_query set elevation mode, still need to run as depth
from ucvm_query, the depth is already proprocessed with (ucvm_surface - elevation)
          cvmhlabn_zmode = VX_ZMODE_ELEV;
****/
          if(cvmhlabn_ucvm_debug) fprintf(stderr,"calling cvmhlabn_setparam >>  elevation\n");
          break;
        default:
          break;
       }
       vx_setzmode(cvmhlabn_zmode);
       break;
  }
  va_end(ap);
  return UCVM_CODE_SUCCESS;
}


/**
 * Queries CVMHLABN at the given points and returns the data that it finds.
 *
 * @param points The points at which the queries will be made.
 * @param data The data that will be returned (Vp, Vs, density, Qs, and/or Qp).
 * @param numpoints The total number of points to query.
 * @return UCVM_CODE_SUCCESS or UCVM_CODE_ERROR.
 */
int cvmhlabn_query(cvmhlabn_point_t *points, cvmhlabn_properties_t *data, int numpoints) {

  // setup >> points -> entry (assume always Q in depth)
  // retrieve >> entry -> data

//  if(cvmhlabn_zmode == VX_ZMODE_ELEV) { fprintf(stderr,"cvmhlabn_query: ZMODE= elev\n"); }
//  if(cvmhlabn_zmode == VX_ZMODE_DEPTH) { fprintf(stderr,"cvmhlabn_query: ZMODE= dep\n"); }

  for(int i=0; i<numpoints; i++) {
      vx_entry_t entry;
      float vx_surf=0.0;

if(cvmhlabn_ucvm_debug) {
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
      if ((cvmhlabn_force_depth) && (cvmhlabn_zmode == VX_ZMODE_ELEV) &&
          (points[i].depth < 0.0)) {
fprintf(stderr," **** in HERE looking for a new surface ..\n");
        /* Setup point to query */
        entry.coor[0]=points[i].longitude;
        entry.coor[1]=points[i].latitude;
        entry.coor_type = cvmhlabn_zmode;
        vx_getsurface(&(entry.coor[0]), entry.coor_type, &vx_surf);
     
        if(cvmhlabn_ucvm_debug) {
           fprintf(stderr, "cvmhlabn_query: surface is %f vs initial query depth %f\n", vx_surf, points[i].depth);
        }
        if (vx_surf - VX_NO_DATA < 0.01) {
          /* Fallback to using UCVM topo */
          entry.coor[2]=points[i].depth;
        } else {
          if(cvmhlabn_ucvm_debug) {
            fprintf(stderr,"POTENTIAL problem if cvmhlabn surface differ from UCVM surface !!!\n");
          }
          entry.coor[2]=vx_surf - points[i].depth;
        }
      } else {
        /* Setup with direct point to query */
        entry.coor[0]=points[i].longitude;
        entry.coor[1]=points[i].latitude;
        entry.coor[2]=points[i].depth;
      }

      /* In case we got anything like degrees */
      if ((entry.coor[0]<360.) && (fabs(entry.coor[1])<90)) {
        entry.coor_type = VX_COORD_GEO;
      } else {
        entry.coor_type = VX_COORD_UTM;
      }

      /* Query the point */
      int rc=vx_getcoord(&entry);

      if(cvmhlabn_ucvm_debug) {
        printf("X||lonlat(%.6f %.6f %.4f)\n",
               entry.coor[0], entry.coor[1], entry.coor[2]);
        /* AP: Let's provide the computed UTM coordinates as well */
        printf("X||utm(%.2f %.2f)\n", entry.coor_utm[0], entry.coor_utm[1]);
        printf("X||elev_cell(%10.2f %11.2f)\n", entry.elev_cell[0], entry.elev_cell[1]);
        printf("X||topo(%.2f) mtop(%.2f) base(%.2f) moho(%.2f)\n", entry.topo, entry.mtop, entry.base, entry.moho);
        printf("X||src(%s) vel_cell(%.2f %.2f %.2f) provenance(%.2f)\n", VX_SRC_NAMES[entry.data_src], 
            entry.vel_cell[0], entry.vel_cell[1], entry.vel_cell[2], entry.provenance);
        printf("X||vp(%.4f) vs(%.4f) rho(%.4f)\n", entry.vp, entry.vs, entry.rho);
      }

      if(cvmhlabn_ucvm_debug) {
        fprintf(stderr,">>> a point..rc(%d)->",rc);
        switch(entry.data_src) {
          case VX_SRC_NR: {fprintf(stderr,"GOT VX_SRC_NR\n"); break; }
          case VX_SRC_HR: {fprintf(stderr,"GOT VX_SRC_HR\n"); break; }
          case VX_SRC_LR: {fprintf(stderr,"GOT VX_SRC_LR\n"); break; }
          case VX_SRC_CM: {fprintf(stderr,"GOT VX_SRC_CM\n"); break; }
          case VX_SRC_TO: {fprintf(stderr,"GOT VX_SRC_TO\n"); break; }
          case VX_SRC_BK: {fprintf(stderr,"GOT VX_SRC_BK\n"); break; }
          case VX_SRC_GT: {fprintf(stderr,"GOT VX_SRC_GT\n"); break; }
          default: {fprintf(stderr,"???\n"); break; }
        }
      }

      // 1 is bad, 0 is good and anything not in HR region/ie cvmhlabn 
      if(rc || entry.data_src != VX_SRC_HR) { 
        data[i].vp=-1;
        data[i].vs=-1;
        data[i].rho=-1;
        } else {
          data[i].vp=entry.vp;
          data[i].vs=entry.vs;
          data[i].rho=entry.rho;
      }

  }
  return UCVM_CODE_SUCCESS;
}

/**
 * Called when the model is being discarded. Free all variables.
 *
 * @return UCVM_CODE_SUCCESS
 */
int cvmhlabn_finalize() {
        vx_cleanup();

	cvmhlabn_is_initialized = 0;

	free(cvmhlabn_configuration);
	free(cvmhlabn_velocity_model);
	free(cvmhlabn_config_string);
	cvmhlabn_config_sz=0;

	return UCVM_CODE_SUCCESS;
}

/**
 * Returns the version information.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int cvmhlabn_version(char *ver, int len)
{
  //const char *cvmhlabn_version_string = "CVMHLABN";
  vx_version(ver);
  return UCVM_CODE_SUCCESS;
}

/**
 * Returns the model config information.
 *
 * @param key Config key string to return.
 * @return Zero
 */
int cvmhlabn_config(char **config, int *sz)
{
  int len=strlen(cvmhlabn_config_string);
  if(len > 0) {
    *config=cvmhlabn_config_string;
    *sz=cvmhlabn_config_sz;
    return UCVM_CODE_SUCCESS;
  }
  return UCVM_CODE_ERROR;
}


/**
 * Reads the configuration file describing the various properties of CVMHLABN and populates
 * the configuration struct. This assumes configuration has been "calloc'ed" and validates
 * that each value is not zero at the end.
 *
 * @param file The configuration file location on disk to read.
 * @param config The configuration struct to which the data should be written.
 * @return Success or failure, depending on if file was read successfully.
 */
int cvmhlabn_read_configuration(char *file, cvmhlabn_configuration_t *config) {
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
		cvmhlabn_print_error("One configuration parameter not specified. Please check your configuration file.");
		return UCVM_CODE_ERROR;
	}

	fclose(fp);

	return UCVM_CODE_SUCCESS;
}

/*
 * @param err The error string to print out to stderr.
 */
void cvmhlabn_print_error(char *err) {
	fprintf(stderr, "An error has occurred while executing CVMHLABN. The error was:\n\n");
	fprintf(stderr, "%s", err);
	fprintf(stderr, "\n\nPlease contact software@scec.org and describe both the error and a bit\n");
	fprintf(stderr, "about the computer you are running CVMHLABN on (Linux, Mac, etc.).\n");
}

// The following functions are for dynamic library mode. If we are compiling
// a static library, these functions must be disabled to avoid conflicts.
#ifdef DYNAMIC_LIBRARY

/**
 * Init function loaded and called by the UCVM library. Calls cvmhlabn_init.
 *
 * @param dir The directory in which UCVM is installed.
 * @return Success or failure.
 */
int model_init(const char *dir, const char *label) {

	return cvmhlabn_init(dir, label);
}

/**
 * Query function loaded and called by the UCVM library. Calls cvmhlabn_query.
 *
 * @param points The basic_point_t array containing the points.
 * @param data The basic_properties_t array containing the material properties returned.
 * @param numpoints The number of points in the array.
 * @return Success or fail.
 */
int model_query(cvmhlabn_point_t *points, cvmhlabn_properties_t *data, int numpoints) {
	return cvmhlabn_query(points, data, numpoints);
}

/**
 * Setparam function loaded and called by the UCVM library. Calls cvmhlabn_setparam.
 *
 * @param id  don'care
 * @param param 
 * @param val, it is actually just 1 int
 * @return Success or fail.
 */
int model_setparam(int id, int param, int val) {
	return cvmhlabn_setparam(id, param, val);
}

/**
 * Finalize function loaded and called by the UCVM library. Calls cvmhlabn_finalize.
 *
 * @return Success
 */
int model_finalize() {
	return cvmhlabn_finalize();
}

/**
 * Version function loaded and called by the UCVM library. Calls cvmhlabn_version.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int model_version(char *ver, int len) {
	return cvmhlabn_version(ver, len);
}

/**
 * Version function loaded and called by the UCVM library. Calls cvmhlabn_config.
 *
 * @param config Config string to return.
 * @param sz length of Config terms.
 * @return Zero
 */
int model_config(char **config, int *sz) {
        return cvmhlabn_config(config, sz);
}


int (*get_model_init())(const char *, const char *) {
        return &cvmhlabn_init;
}
int (*get_model_query())(cvmhlabn_point_t *, cvmhlabn_properties_t *, int) {
         return &cvmhlabn_query;
}
int (*get_model_finalize())() {
         return &cvmhlabn_finalize;
}
int (*get_model_version())(char *, int) {
         return &cvmhlabn_version;
}
int (*get_model_config())(char **, int*) {
         return &cvmhlabn_config;
}
int (*get_model_setparam())(int, int, ...) {
         return &cvmhlabn_setparam;
}




#endif
