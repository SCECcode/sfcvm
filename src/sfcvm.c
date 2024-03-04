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
 * on mac,
 * run_ucvm_query.sh -l 37.455000,-121.941000,3000 -m sfcvm -f $UCVM_INSTALL_PATH/conf/ucvm.conf -P sfcvm_param:SquashMinElev,-5000.0
 */

#include <assert.h>

#include "ucvm_model_dtypes.h"
#include "sfcvm.h"
#include "cJSON.h"

#include "geomodelgrids/serial/cquery.h"
#include "geomodelgrids/utils/cerrorhandler.h"

#define ROUND_2_INT(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

int _processUCVMConfiguration(char *confstr);

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
/** The grid height of the detail and regional sections, in meters. */
double sfcvm_grid_height_m = 0;
double sfcvm_grid_height_regional_m = 0;

#define sfcvm_true 1
#define sfcvm_false 0

int sfcvm_plugin=sfcvm_false; 

/* Values and order to be returned in queries.  */
static const size_t sfcvm_numValues = 4;
static const char* const sfcvm_valueNames[4] = { "Vp", "Vs", "density","zone_id" };
// "San Leandro G";
int sfcvm_san_leandro_gabbro_type_id = 4; 
// "Logan G"
int sfcvm_logan_gabbro_type_id = 5; 

// max is 10
char* sfcvm_filenames[10];  
char* sfcvm_gridheights[10];  // one per data file in meter 
int sfcvm_filenames_cnt;

/* Coordinate reference system of points passed to queries.
*
* The string can be in the form of EPSG:ABCD, WKT, or Proj
* parameters. In this case, we will specify the coordinates in
i latitude, longitude, elevation in the WGS84 horizontal
* datum. The elevation is with respect to the WGS84 ellipsoid.
*/
// NAD83/UTM Zone 10N
const char* const sfcvm_utm_crs = "EPSG:26910";
// WGS84
const char* const sfcvm_geo_crs = "EPSG:4326";

void* sfcvm_geo_query_object=0;
void* sfcvm_utm_query_object=0;
void* sfcvm_geo_error_handler=0;
void* sfcvm_utm_error_handler=0;

const size_t sfcvm_spaceDim = 3;

/* Whitespace characters */
const char *WHITESPACE = " \t\n";


/*************************************/

int sfcvm_ucvm_debug=0;
int sfcvm_gabbro_on=1;
int sfcvm_gabbro_count=0;
int sfcvm_query_count=0; // total number of query location
int sfcvm_water_count=0; // total number of water location
int sfcvm_water_step_count=0; // total number of water location that needed to step down
int sfcvm_water_2step_count=0; // total number of water location that needed to 2 step down
int sfcvm_water_loop_count=0; // total number of water location that needed loops for more step downs
int sfcvm_max_water_loops=0;   // max number of loops needed to find valid data


FILE *stderrfp;

int sfcvm_force_depth=0;

double SFCVM_SquashMinElev=-5000.0;

// set in, sfcvm_setparam(int id, int param, ...)
int sfcvm_zmode=SFCVM_ZMODE_DEPTH; // SFCVM_ZMODE_DEPTH or SFCVM_ZMODE_ELEVATION

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

    if(sfcvm_ucvm_debug) {
      stderrfp = fopen("sfcvm_debug.log", "w+");
      fprintf(stderrfp," ===== START ===== \n");
    }
    char configbuf[512];

    // Initialize variables.
    sfcvm_configuration = (sfcvm_configuration_t *)calloc(1, sizeof(sfcvm_configuration_t));
    sfcvm_velocity_model = (sfcvm_model_t *)calloc(1, sizeof(sfcvm_model_t));
    sfcvm_config_string = (char *)calloc(SFCVM_CONFIG_MAX, sizeof(char));

    // Configuration file location when built with UCVM
    sprintf(configbuf, "%s/model/%s/data/config", dir, label);

    // Read the configuration file.
    if (sfcvm_read_configuration(configbuf, sfcvm_configuration) != UCVM_MODEL_CODE_SUCCESS) {

           // Try another, when is running in standalone mode..
       sprintf(configbuf, "%s/data/config", dir);
       if (sfcvm_read_configuration(configbuf, sfcvm_configuration) != UCVM_MODEL_CODE_SUCCESS) {
           sfcvm_print_error("No configuration file was found to read from.");
           return UCVM_MODEL_CODE_ERROR;
           } else {
           // Set up the data directory.
               sprintf(sfcvm_data_directory, "%s/data/%s", dir, sfcvm_configuration->model_dir);
       }
       } else {
           // Set up the data directory.
           sprintf(sfcvm_data_directory, "%s/model/%s/data/%s", dir, label, sfcvm_configuration->model_dir);
    }


    // dir/data/model_dir/filename
    sfcvm_filenames_cnt =sfcvm_configuration->data_cnt;
    if(sfcvm_filenames_cnt > 10) {
        fprintf(stderr,"BADD");
        exit(1);
    }

//
//  TODO:  not sure if have more than 1 data files, which gridheight should we be using??
//  need to check the boundary ?? -- actually geomodelgrid should expose API to retrieve that
//  from the backend
//
    for(int i=0; i < sfcvm_filenames_cnt; i++) {
       sfcvm_filenames[i]= (char *)calloc(1,
           strlen(dir)+(strlen(sfcvm_configuration->model_dir)*2)+strlen(sfcvm_configuration->data_files[i]) +15);
       sprintf(sfcvm_filenames[i],"%s/model/%s/data/%s/%s",
           dir,
           sfcvm_configuration->model_dir,
           sfcvm_configuration->model_dir,
           sfcvm_configuration->data_files[i]);

//if(sfcvm_ucvm_debug) fprintf(stderrfp,"using %s\n", sfcvm_filenames[i]);
       if( strcmp(sfcvm_configuration->data_labels[i],"sfcvm") == 0) {
           sfcvm_grid_height_m = sfcvm_configuration->data_gridheights[i];
       }
       if( strcmp(sfcvm_configuration->data_labels[i],"regional") == 0) {
           sfcvm_grid_height_regional_m = sfcvm_configuration->data_gridheights[i];
       }
    }

/* Create and initialize serial query object using the parameters  stored in local variables.  */
    sfcvm_geo_query_object = geomodelgrids_squery_create();
    assert(sfcvm_geo_query_object);

// GEO
/** Log warnings and errors to "sfcvm_geo_error.log". **/
    sfcvm_geo_error_handler = geomodelgrids_squery_getErrorHandler(sfcvm_geo_query_object);
    assert(sfcvm_geo_error_handler);
//    geomodelgrids_cerrorhandler_setLogFilename(sfcvm_geo_error_handler, "sfcvm_geo_error.log");

    int geo_err=geomodelgrids_squery_initialize(sfcvm_geo_query_object, sfcvm_filenames,
                 sfcvm_filenames_cnt, sfcvm_valueNames, sfcvm_numValues, sfcvm_geo_crs);
    assert(!geo_err);

    int geo_setsquash=geomodelgrids_squery_setSquashing(sfcvm_geo_query_object, GEOMODELGRIDS_SQUASH_TOPOGRAPHY_BATHYMETRY);
    assert(!geo_setsquash);
    geomodelgrids_squery_setSquashMinElev(sfcvm_geo_query_object, SFCVM_SquashMinElev);

// UTM
/* Create and initialize serial query object using the parameters  stored in local variables.  */
    sfcvm_utm_query_object = geomodelgrids_squery_create();
    assert(sfcvm_utm_query_object);

/** Log warnings and errors to "sfcvm_utm_error.log". **/
    sfcvm_utm_error_handler = geomodelgrids_squery_getErrorHandler(sfcvm_utm_query_object);
    assert(sfcvm_utm_error_handler);
//    geomodelgrids_cerrorhandler_setLogFilename(sfcvm_utm_error_handler, "sfcvm_utm_error.log");

    int utm_err=geomodelgrids_squery_initialize(sfcvm_utm_query_object, sfcvm_filenames,
                 sfcvm_filenames_cnt, sfcvm_valueNames, sfcvm_numValues, sfcvm_utm_crs);
    assert(!utm_err);

    int utm_setsquash=geomodelgrids_squery_setSquashing(sfcvm_utm_query_object, GEOMODELGRIDS_SQUASH_TOPOGRAPHY_BATHYMETRY);
    assert(!utm_setsquash);
    geomodelgrids_squery_setSquashMinElev(sfcvm_utm_query_object, SFCVM_SquashMinElev);

    // Let everyone know that we are initialized and ready for business.
    sfcvm_is_initialized = 1;

    sfcvm_query_count=0;
    sfcvm_gabbro_count=0;
    sfcvm_water_count=0;
    sfcvm_water_step_count=0;
    sfcvm_water_2step_count=0;
    sfcvm_water_loop_count=0;
    sfcvm_max_water_loops=0;

    return UCVM_MODEL_CODE_SUCCESS;
}

void set_setSquashMinElev(double val) {
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"sfcvm.c: SETTING new squashing min value (%lf)\n",val); }
    SFCVM_SquashMinElev=val;
    geomodelgrids_squery_setSquashMinElev(sfcvm_geo_query_object, SFCVM_SquashMinElev);
    geomodelgrids_squery_setSquashMinElev(sfcvm_utm_query_object, SFCVM_SquashMinElev);
}

/**  
  * 
**/

/* Setparam SFCVM */
int sfcvm_setparam(int id, int param, ...)
{
  va_list ap;
  char *bstr;
  char *pstr;
  double pval;
  int zmode;

  va_start(ap, param);

  switch (param) {
    case UCVM_MODEL_PARAM_MODEL_CONF: // from ucvm.conf
      pstr = va_arg(ap, char *);
      pval = va_arg(ap, double);
      if (strcmp(pstr, "SquashMinElev") == 0) {
        set_setSquashMinElev(pval);
      }
      break;
    case UCVM_MODEL_PARAM_CONF_BLOB: // from standalone
      bstr = va_arg(ap, char *);
      _processUCVMConfiguration(bstr);
      break;
    case UCVM_MODEL_PARAM_FORCE_DEPTH_ABOVE_SURF:
      sfcvm_force_depth = va_arg(ap, int);
      break;
    case UCVM_MODEL_PARAM_PLUGIN_MODE:
      sfcvm_plugin = sfcvm_true;
      sfcvm_zmode = SFCVM_ZMODE_DEPTH; // even if it were set earlier 
      break;
    case UCVM_MODEL_PARAM_QUERY_MODE:
      zmode = va_arg(ap,int);
      switch (zmode) {
/*** from UCVM plugin module, this will always be search by depth **/
        case UCVM_MODEL_COORD_GEO_DEPTH:
          sfcvm_zmode = SFCVM_ZMODE_DEPTH;
//if(sfcvm_ucvm_debug) fprintf(stderrfp,"calling sfcvm_setparam >>  depth\n");
          break;
/*** as standalone, it is possible to pick the elevation mode ***/
        case UCVM_MODEL_COORD_GEO_ELEV:
          if( sfcvm_plugin == sfcvm_false) {
            sfcvm_zmode = SFCVM_ZMODE_ELEVATION;
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"calling sfcvm_setparam >>  elevation\n"); }
          }
          break;
        default:
          break;
       }
       break;
  }
  va_end(ap);
  return UCVM_MODEL_CODE_SUCCESS;
}

/**
* Parse the configurations
*
*  example:  "{'SQUASH_MIN_ELEV':-5000}"
**/
int _processUCVMConfiguration(char *confstr) {

  cJSON *confjson = cJSON_Parse(confstr);
  if(confjson == NULL)
  {
    const char *eptr = cJSON_GetErrorPtr();
    if(eptr != NULL) {
//if(sfcvm_ucvm_debug){ fprintf(stderrfp, "Config processing error before 1: (%s)(%s)\n", eptr,confstr); }
      return UCVM_MODEL_CODE_ERROR;
    }
  }
  cJSON *squash_min_elev = cJSON_GetObjectItemCaseSensitive(confjson, "SQUASH_MIN_ELEV");
  if(cJSON_IsNumber(squash_min_elev)){
    set_setSquashMinElev(squash_min_elev->valuedouble);
//if(sfcvm_ucvm_debug){ fprintf(stderrfp, "Using %lf as SquashMinEelv\n",SFCVM_SquashMinElev); }
    } else {
      cJSON_Delete(confjson);
      return UCVM_MODEL_CODE_ERROR;
  }

  cJSON_Delete(confjson);
  return UCVM_MODEL_CODE_SUCCESS;
}


/**
Nakata and Pitarka gabbro correction in the SFCVM
to modify the near-surface gabbro regions in the East Bay and Gilroy 
  depth= -(elev)
  Vp 4.2 to 5.7 km/s from depth=0 to 7.75km
  Vs = 0.7858 - 1.2344*Vp + 0.7949*Vp^2 - 0.1238*Vp^3 + 0.0064*Vp^4
  density = 2.4372 + 0.0761*Vp
**/
static const double sfcvm_gabbro_vp_delta = ((5.7- 4.2) / 7.75);
int _gabbro(double elevation, sfcvm_properties_t *data) {
  double depth= (elevation == 0.0) ? 0.0 : ((0.0 - elevation)/1000); // turn into km
								     //
//if(sfcvm_ucvm_debug) { fprintf(stderrfp, "\ndelta: %lf, diff\n", sfcvm_gabbro_vp_delta, (depth) * sfcvm_gabbro_vp_delta); }

  if(depth < 7.750) {
    double vp = 4.2 + (depth) * sfcvm_gabbro_vp_delta; 
    double vs = 0.7858 - (1.2344 * vp) + (0.7949 * vp * vp) - (0.1238 * vp * vp * vp) + (0.0064 * vp * vp * vp * vp);
    double rho = 2.4372 + (0.0761 * vp);
    if(sfcvm_gabbro_on) {
      data->vp= vp * 1000;
      data->vs= vs * 1000;
      data->rho = rho * 1000;
    }
    sfcvm_gabbro_count++;
  }
  return UCVM_MODEL_CODE_SUCCESS;
}

/** 
  step down for water

       double gridcell= 125.0;
       double groundElev = -3993.2749422271368;
       double squashMinElev = -5000.0;

//       double zsquash= groundElev - gridcell;
       double zsquash= -4125.0;

       double elev= (zsquash - groundElev) * squashMinElev / (squashMinElev - groundElev);
       fprintf(stderr,"elev = %lf\n", elev);
       // elev = 4239.941478
       first valid value is returned in squashing mode
**/
double _water(double gridcell, double groundElev, double squashMinElev) {
       double zsquash= groundElev - gridcell;
       double elev= (zsquash - groundElev) * squashMinElev / (squashMinElev - groundElev);
       return elev;
}

/**
 * Queries SFCVM at the given points and returns the data that it finds.
 *
 * @param points The points at which the queries will be made.
 * @param data The data that will be returned (Vp, Vs, density, Qs, and/or Qp).
 * @param numpoints The total number of points to query.
 * @return UCVM_MODEL_CODE_SUCCESS or UCVM_MODEL_CODE_ERROR.
 */
int sfcvm_query(sfcvm_point_t *points, sfcvm_properties_t *data, int numpoints) {

// NOTE: even though 3rd item in points struct is name 'depth', it could be depth in
// elevation data model or depth in depth data model 

    double values[sfcvm_numValues];
    double entry_latitude;
    double entry_longitude;
    double entry_depth;
    double entry_elevation;
    double topoBathyElev;

    void *query_object;
    void *error_handler;

    for(int i=0; i<numpoints; i++) {
      sfcvm_query_count++;
      data[i].vp=-1;
      data[i].vs=-1;
      data[i].rho=-1;

      /* Force depth mode if directed and point is above surface */
      /* Setup point to query */
      entry_longitude=points[i].longitude;
      entry_latitude=points[i].latitude;


//if(sfcvm_ucvm_debug) { fprintf(stderrfp, "\nsfcvm_query: USING lat(%lf)) lon(%lf) depth(%lf)\n", points[i].latitude, points[i].longitude, points[i].depth); }

      if((entry_longitude<360.) && (fabs(entry_latitude)<90)) {
      // GEO;
        query_object= sfcvm_geo_query_object;
        error_handler = sfcvm_geo_error_handler;
        } else { // UTM;
          query_object= sfcvm_utm_query_object;
          error_handler= sfcvm_utm_error_handler;
      }

      int rc=sfcvm_getsurface(entry_longitude, entry_latitude, &topoBathyElev);
      if( rc != 0) {
        continue;
      } 

//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"\n with topoBathyElev : %f\n", topoBathyElev); }

      if( topoBathyElev == NO_DATA ) { // outside of the model
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"        OUTside of MODEL by NO_DATA surface..\n"); }
        geomodelgrids_cerrorhandler_resetStatus(error_handler);
	continue;
      }

      // Since it is squashed.. the surface has moved to sea level
      if (topoBathyElev - NO_DATA < 0.01) { 
        entry_elevation= 0.0 ;
        } else {
          entry_elevation= 0.0 - points[i].depth;
      }

// under the sea level
      int err;
      double detail_offset, regional_offset;
      if(topoBathyElev < 0) { // under the sea level
	  sfcvm_water_count++;     
// try first without step down
          err = geomodelgrids_squery_query(query_object, values, entry_latitude, entry_longitude, entry_elevation);

	  if(err || values[0]<0  || values[1]<0 ) {
            sfcvm_water_step_count++;
// try with first step down -- detail version
//            double n_entry_elevation = entry_elevation - sfcvm_grid_height_m;

            detail_offset = _water(sfcvm_grid_height_m, topoBathyElev, SFCVM_SquashMinElev);
            double n_entry_elevation = entry_elevation + detail_offset;
            err = geomodelgrids_squery_query(query_object, values, entry_latitude, entry_longitude, n_entry_elevation);

// try with second step down -- detail version
	    if(err || values[0]<0  || values[1]<0 ) {
                sfcvm_water_2step_count++;
//                double n_entry_elevation = entry_elevation - sfcvm_grid_height_regional_m;
                regional_offset = _water(sfcvm_grid_height_regional_m, topoBathyElev, SFCVM_SquashMinElev);
                double n_entry_elevation = entry_elevation + regional_offset;
                err = geomodelgrids_squery_query(query_object, values, entry_latitude, entry_longitude, n_entry_elevation);

	        if(err || values[0]<0  || values[1]<0 ) {
                  sfcvm_water_loop_count++;
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"WATER: still bad, found: at %lf %lf %lf -- surface %lf regional_offset %lf\n", entry_longitude, entry_latitude, n_entry_elevation, topoBathyElev, regional_offset); }

                  int step_cnt=1;
                  while(1) {
                    double nn_entry_elevation = entry_elevation + 
			    regional_offset - (sfcvm_grid_height_regional_m * step_cnt);
                    err = geomodelgrids_squery_query(query_object, values, entry_latitude, entry_longitude, nn_entry_elevation);
if(sfcvm_ucvm_debug) { fprintf(stderrfp,"Water still : at %lf %lf %lf -- surface %lf regional_offset %lf (%d)\n", entry_longitude, entry_latitude, nn_entry_elevation, topoBathyElev, regional_offset,step_cnt); }
		    if(step_cnt > sfcvm_max_water_loops) {
                      sfcvm_max_water_loops=step_cnt;
                    }
		    if( (values[0]> 0 && values[1]>0) || step_cnt > 100) {
if(sfcvm_ucvm_debug) { fprintf(stderrfp,"  done(%d) : at %lf %lf %lf -- surface %lf regional_offset %lf \n", step_cnt, entry_longitude, entry_latitude, nn_entry_elevation, topoBathyElev, regional_offset); }
                      break;
                    }
		    step_cnt++;
                  }
                }

            } else { // good catch the 2nd time
            }

          } else { // good catch the first time
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"WATER: good, 1st at (%lf %lf %lf) surface %lf\n", entry_longitude, entry_latitude, entry_elevation, topoBathyElev); }
          }

          } else { // above the sea level
              err = geomodelgrids_squery_query(query_object, values, entry_latitude, entry_longitude, entry_elevation);
      }

      if(!err) {
        data[i].vp=values[0];
        data[i].vs=values[1];
        data[i].rho=values[2];

// Nakata and Pitarka gabbro correction, near-surface gabbro regions in the East Bay and Gilroy in the SFCVM
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"At b %lf %lf type(%lf) -- vp(%lf) vs(%lf)\n", entry_longitude, entry_latitude, values[3], values[0], values[1]); }

        int typeid= ROUND_2_INT(values[3]);
        if( (typeid == sfcvm_san_leandro_gabbro_type_id) ||
               (typeid == sfcvm_logan_gabbro_type_id )) {
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"GABBRO: found: at %lf %lf\n", entry_longitude, entry_latitude); }
           _gabbro(entry_elevation,&data[i]);
        } else {
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"GABBRO: no: at %lf %lf %lf\n", entry_longitude, entry_latitude, values[3]); }
        }

//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"  At %lf %lf type(%lf) -- vp(%lf)vs(%lf)\n", entry_longitude, entry_latitude, values[3], data[i].vp, data[i].vs); }

//if(sfcvm_ucvm_debug) { fprintf(stderrfp," RESULT from calling squery ==> vp(%f) vs(%f) rho(%f) \n\n",values[0], values[1], values[2]); }
        } else { // need to reset the error handler
             geomodelgrids_cerrorhandler_resetStatus(error_handler);
      }		
  }
  return UCVM_MODEL_CODE_SUCCESS;
}

/**
 * Queries SFCVM inner for the surface 
 **/
int sfcvm_getsurface(double entry_longitude, double entry_latitude, 
                               double *surface) {
  void *query_object;
  void *error_handler;

  if((entry_longitude<360.) && (fabs(entry_latitude)<90)) {
      // GEO;
        query_object= sfcvm_geo_query_object;
        error_handler = sfcvm_geo_error_handler;
        } else { // UTM;
          query_object= sfcvm_utm_query_object;
          error_handler= sfcvm_utm_error_handler;
  }

//  double topElev = geomodelgrids_squery_queryTopElevation(query_object, entry_latitude, entry_longitude);
  double topoBathyElev = geomodelgrids_squery_queryTopoBathyElevation(query_object, entry_latitude, entry_longitude);

//if(sfcvm_ucvm_debug) { fprintf(stderrfp,">>>    surface: topoBathy %f\n", topoBathyElev); }

  if( topoBathyElev == NO_DATA ) { // outside of the model
//if(sfcvm_ucvm_debug) { fprintf(stderrfp,"        OUTside of MODEL by NO_DATA surface..\n"); }
      geomodelgrids_cerrorhandler_resetStatus(error_handler);
      return 1;
  }

  *surface=topoBathyElev;
  return 0;
}

void sfcvm_setdebug() {
   sfcvm_ucvm_debug=1;
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
       if(sfcvm_ucvm_debug) { fprintf(stderrfp,"    <%d>  %s: %s\n",i,config->data_labels[i], config->data_files[i]); }
    }
}

/**
 * Called when the model is being discarded. Free all variables.
 *
 * @return UCVM_MODEL_CODE_SUCCESS
 */
int sfcvm_finalize() {

    sfcvm_is_initialized = 0;

    _free_sfcvm_configuration(sfcvm_configuration);
    free(sfcvm_velocity_model);
    free(sfcvm_config_string);

/* Destroy query object. */
    geomodelgrids_squery_destroy(&sfcvm_geo_query_object);
    sfcvm_geo_query_object=0;

    geomodelgrids_squery_destroy(&sfcvm_utm_query_object);
    sfcvm_utm_query_object=0;

    if(sfcvm_ucvm_debug) { 
     fprintf(stderrfp,"DONE:\n"); 
     fprintf(stderrfp,"    total query count=(%d)\n",sfcvm_query_count);
     fprintf(stderrfp,"    total gabbro count=(%d)\n",sfcvm_gabbro_count);
     fprintf(stderrfp,"    total water count=(%d)\n",sfcvm_water_count);
     fprintf(stderrfp,"    total water step count=(%d)\n",sfcvm_water_step_count);
     fprintf(stderrfp,"    total water 2 step count=(%d)\n",sfcvm_water_2step_count);
     fprintf(stderrfp,"    total water loop count=(%d)\n",sfcvm_water_loop_count);
     fprintf(stderrfp,"    max water loops =(%d)\n",sfcvm_max_water_loops);
     fclose(stderrfp);
    }

    return UCVM_MODEL_CODE_SUCCESS;
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
  return UCVM_MODEL_CODE_SUCCESS;
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
    return UCVM_MODEL_CODE_SUCCESS;
  }
  return UCVM_MODEL_CODE_ERROR;
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
* Parse the sfcvm data configurations
*
* {"LABEL":"sfcvm","FILE":"USGS_SFCVM_v21-1_detailed.h5","GRIDHEIGHT":25}
**/
int _processSFCVMConfiguration(sfcvm_configuration_t *config, char *confstr,int idx) {

  cJSON *confjson = cJSON_Parse(confstr);
  if(confjson == NULL)
  {
    const char *eptr = cJSON_GetErrorPtr();
    if(eptr != NULL) {
      if(sfcvm_ucvm_debug){ fprintf(stderrfp, "Config processing error before 2: (%s)(%s)\n", eptr,confstr); }
      return UCVM_MODEL_CODE_ERROR;
    }
  }
  cJSON *label = cJSON_GetObjectItemCaseSensitive(confjson, "LABEL");
  if(cJSON_IsString(label)){
    config->data_labels[idx]=strdup(label->valuestring);
  }
  cJSON *file = cJSON_GetObjectItemCaseSensitive(confjson, "FILE");
  if(cJSON_IsString(file)){
    config->data_files[idx]=strdup(file->valuestring);
  }
  cJSON *gridheight = cJSON_GetObjectItemCaseSensitive(confjson, "GRIDHEIGHT");
  if(cJSON_IsNumber(gridheight)){
    config->data_gridheights[idx]=gridheight->valuedouble;
  }
  cJSON_Delete(confjson);
  return UCVM_MODEL_CODE_SUCCESS;
}


void _trimLast(char *str, char m) {
  int i;
  i = strlen(str);
  while (str[i-1] == m) {
    str[i-1] = '\0';
    i = i - 1;
  }
  return;
}

void _splitline(char* lptr, char key[], char value[]) {

  char *kptr, *vptr;

  for(int i=0; i<strlen(key); i++) { key[i]='\0'; }

  _trimLast(lptr,'\n');
  vptr = strchr(lptr, '=');
  int pos=vptr - lptr;

// skip space in key token from the back
  while ( lptr[pos-1]  == ' ') {
    pos--;
  }

  strncpy(key,lptr, pos);
  key[pos] = '\0';

  vptr++;
  while( vptr[0] == ' ' ) {
    vptr++;
  }
  strcpy(value,vptr);
  _trimLast(value,' ');
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

    // If our file pointer is null, an error has occurred. Return fail.
    if (fp == NULL) {
        return UCVM_MODEL_CODE_ERROR;
    }

    // Read the lines in the configuration file.
    config->data_cnt=0;
    while (fgets(line_holder, sizeof(line_holder), fp) != NULL) {
        if (line_holder[0] != '#' && line_holder[0] != ' ' && line_holder[0] != '\n') {
           
            _splitline(line_holder, key, value);

//if(sfcvm_ucvm_debug) fprintf(stderrfp," input line >> (%s) \n", line_holder);
//if(sfcvm_ucvm_debug) fprintf(stderrfp," key >> (%s), value >> (%s) \n", key, value);

            // Which variable are we editing?
            if (strcmp(key, "utm_zone") == 0) {
                config->utm_zone = atoi(value);
            } else if (strcmp(key, "model_dir") == 0) {
                sprintf(config->model_dir, "%s", value);
            } else if (strcmp(key, "data_file") == 0) {
                // value is in json format 
//if(sfcvm_ucvm_debug) fprintf(stderrfp," value  is: (%s)\n", value);

                int rc=_processSFCVMConfiguration(config,value,config->data_cnt);
                if(rc == UCVM_MODEL_CODE_SUCCESS) {
                  config->data_cnt++;
                } 
            }
       }

    }
//if(sfcvm_ucvm_debug) _dump_sfcvm_configuration(config);
    // Have we set up all configuration parameters?
    if (config->utm_zone == 0 || config->model_dir[0] == '\0' ) {
	    sfcvm_print_error("One configuration parameter not specified. Please check your configuration file.");
	    return UCVM_MODEL_CODE_ERROR;
    }

    fclose(fp);

    return UCVM_MODEL_CODE_SUCCESS;
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
