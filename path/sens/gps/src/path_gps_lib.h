/**\file
 * 
 * Header file for GPS info.
 * 
 * Includes data types and function prototypes for processing information
 * as read directly from GPS device, as well as data types and function
 * prototypes useful for processing position and speed data obtained
 * from GPS.
 */

#ifndef GPS_LIB_H
#define GPS_LIB_H

#define MAX_GPS_SENTENCE 256
#define MAX_GPS_FIELD 64
#define KNOTS_TO_METERS_PER_SEC	0.51444444
#define GPS_OBJECT_ID_SIZE	6	/// enough bytes for 1609 BSSID (MAC)

#define MAX_CMD_STR 512	/// sqlite3 command string max length

extern unsigned char path_gps_checksum(char *s); 

/* Note in some situations you may need project specific
 * database variable numbers for GPS data. However, please use these
 * #define names or the names plus an offset rather than hard-coding
 * values, since these are subject to change.
 *
 * Group for raw NMEA messages begin at 8001
 *
 * Point reference structure is at 8010
 *
 * Locally received filtered point data begins at 8021 (usually be only one) 
 * unless different GPSes are being compared.
 *
 * Remote GPS point data received from vehicles, bikes, peds, etc. begins
 * at 8030. Maximum of 64 targets as part of path_gps_pt_ref structure, 
 * which will indicate active GPS variables in a mask, include timestamp
 * when most recently updated. 
 */

#define DB_GPS_GGA_VAR 8001
#define DB_GPS_VTG_VAR 8002
#define DB_GPS_RMC_VAR 8003
#define DB_GPS_PT_REF_VAR 8010
#define DB_GPS_PT_LCL_VAR 8021
#define DB_GPS_PT_RCV_VAR 8030

#define DB_GPS_GGA_TYPE DB_GPS_GGA_VAR
#define DB_GPS_VTG_TYPE DB_GPS_VTG_VAR
#define DB_GPS_RMS_TYPE DB_GPS_RMS_VAR
#define DB_GPS_PT_REF_TYPE DB_GPS_PT_REF_VAR
#define DB_GPS_PT_LCL_TYPE DB_GPS_PT_LCL_VAR
#define DB_GPS_PT_RCV_TYPE DB_GPS_PT_RCV_VAR


/* Following is the definition for DB_GPS_GGA_TYPE, DB_GPS_GGA_VAR in
 * the database and for wireless packet transfer. Since these structures
 * may be used for network transfer, they must be packed. */
typedef struct
{
	float utc_time;     /// Current UTC time of fix in hours, minutes,
	                    /// and seconds (hhmmss.ss) 
	double latitude;    /// Latitude component of position in degrees
	                    /// and decimal minutes (ddmm.mmmmm) 
	double longitude;   /// Longitude component of position in degrees
	                    /// and decimal minutes (ddmm.mmmmm) 
	int pos;	    /// Fix quality: 0 invalid, 1 GPS, 2 DGPS
	int num_sats;       /// Number of satellites 
	float hdop;         /// Horizontal dilution of position 
	float altitude;     /// Altitude in meters above geoid (mean sea level) 
	float alt_offset;   /// Height of geoid above WGS84 ellipsoid
	float time_since_dgps;	/// Time in seconds since last DGPS update
	int dgps_station_id;	/// DGPS station ID number
} IS_PACKED gps_gga_typ;

/** Following is the definition for DB_GPS_VTG_TYPE, DB_GPS_VTG_VAR in
 * the database. */
typedef struct
{
	float true_track;	/// Course Made Good, true north
	float magnetic_track;	/// Course Made Good, magnetic
	float speed_knots;  	/// Speed over ground in knots 
	float speed_kph;	/// Speed over ground in km per hour
} IS_PACKED gps_vtg_typ;

typedef struct
{
	float utc_time;     /// Current UTC time of fix in hours, minutes,
	                    /// and seconds (hhmmss.ss) 
	char nav_warning;   /// Navigation receiver warning A=OK, V=warning
	double latitude;    /// Latitude component of position in degrees
	                    /// and decimal minutes (ddmm.mmmmm) 
	double longitude;   /// Longitude component of position in degrees
	                    /// and decimal minutes (ddmm.mmmmm) 
	float speed_knots;  	/// Speed over ground in knots 
	float true_track;	/// Course Made Good, true north
	int date_of_fix;	/// Date of fix DDMMYY
	float magnetic_variation;	/// Magnetic variation, degrees
} IS_PACKED gps_rmc_typ;

/** This type can be extended with further union types, if additional
 *  GPS messages should be parsed.
 */
typedef struct {
	char gps_id[6];		/// identifies GPS source sentence	
	timestamp_t ts;
	union {
		gps_gga_typ gga;
		gps_vtg_typ vtg;
		gps_rmc_typ rmc;
	} data;
} IS_PACKED gps_data_typ;

/* 
 * int flags bit specification
 * #1: 1 = WGS-84 ellipsoid, 0 = spherical earth
 * #2: 1 = units are miles [per hour], 0 = units are metres [per second]
 * #3: 1 = synchronise to local time, 0 = synchronise to UTC time
 */
#define GPS_ELLIPSOID_EARTH 1
#define GPS_UNITS_MILES 2
#define GPS_USE_LOCALTIME 4

/*
 * gps_data_t is used to store a particular GPS data, with UTC and local
 * time standardised in the timestamp_t format (as defined in timestamp.c)
 * latitude and longitude are in degrees (double float), speed is in meters/sec 
 * The pos, num_sats and hdop fields are included in the type in case 
 * processing on GPS points is defined that checks for the validity
 * of the GPS information. 
 */
typedef struct {
	double longitude;	// in degrees, +/-
	double latitude;	// in degrees, +/-
	float altitude;	        // in meters, +/-
	double speed;		// in meters/sec
	timestamp_t utc_time;	// from GPS
	timestamp_t local_time;	// from computer clock
	int date;		// DDMMYY number form
	float heading;		// course made good, true north
	int pos;	    /// Fix quality: 0 invalid, 1 GPS, 2 DGPS
	int num_sats;       /// Number of satellites 
	float hdop;         /// Horizontal dilution of position 
	short sequence_no;    /// For packet ID in heartbeats
	unsigned char object_id[GPS_OBJECT_ID_SIZE]; /// could be IP or MAC or?
} IS_PACKED path_gps_point_t;

/* This structure is written to the database, and indicates which
 * GPS data server variables are active. It is updated by the
 * process that receives messages. If a bit is set, the data server
 * variable which is DB_GPS_PT_RC_VAR + bit number has data. 
 */
typedef struct {
	unsigned int mask[2];	/// up to 64 different vehicles
	timestamp_t ts;		/// UTC time of last update to any variable
} path_gps_pt_ref_t;
	 
/*
 * path_gps_polygon_t is  used to describe a "polygonal" region
 * to be used in filtering of GPS data points to only include points
 * contained in a particular region, for instance. Vertex coordinates are
 * in GPS format. Because of the fact that these are latitude/longitude
 * coordinates, the polygon may not really be a mathematically well-defined
 * polygon, but with relatively local regions the side-effects are
 * negligible.
 */

typedef struct {
    path_gps_point_t* vertex;  // points to array of vertex data
    int size;  // size of vertex array
} path_gps_polygon_t;


/*
 * Converts GPS format (dddmm.mmmm...) to degrees, double float precision
 */
static inline double path_gps2degree(double d)
{
	double dg, m;
	m = modf(d/100.0, &dg);	/// minutes/100 is returned
	return (dg + (m * 100.0)/60.0);  
}

/*
 *  Function converts degrees to radians
 */
static inline double path_deg2rad(double d)
{
	return (((2.0 * M_PI)/360.0) * d); 
}


/*
 *  Function converts speed in knots to meters per second
 */
static inline double path_knots2mps(double d)
{
	return (d * 0.51444444);
}

/**
 *	Functions for reading, checking, interpreting and store GPS
 */
extern int path_gps_good_checksum(char *s);
extern void path_gps_get_fd(char *name, FILE **pfp,
                int use_gpsd, unsigned short port, int *psd);
extern void path_gps_get_sentence(char *sentence, FILE *fp,
		int use_gpsd, int sd);
extern int path_gps_get_field(int start, char *sentence,
		char *field_buf, int max);
extern void path_gps_parse_gga(char *sentence, gps_gga_typ *p);
extern void path_gps_parse_vtg(char *sentence, gps_vtg_typ *p);
extern void path_gps_parse_rmc(char *sentence, gps_rmc_typ *p);
extern void path_gps_print_data(FILE *fp, gps_data_typ *pg);
extern void path_gps_parse_sentence(char *sentence, gps_data_typ *pg);
extern void path_gps_update_object_id(void *id, path_gps_point_t *pt, int size);
extern int path_gps_get_point(gps_data_typ *gps_array, int array_size,
		path_gps_point_t *pp);
extern int path_gps_print_point(FILE *fp, path_gps_point_t *pp);
extern int path_gps_update_pt_ref(path_gps_pt_ref_t *pt_ref,
		path_gps_point_t *pt);

/* Following functions were originally written by summer student
 * Bernard Liang. When retval is of type int, returns 0 for error,
 * 1 for success (except for path_fcompare, which follows the model of strcmp)
 */
extern double path_gps_get_distance(path_gps_point_t* point1,
	path_gps_point_t* point2, int flags, double float_tolerance);
extern double path_gps_get_speed(path_gps_point_t* point1,
	path_gps_point_t* point2, int flags, double float_tolerance);
extern int path_gps_interpolate_time(path_gps_point_t* point1,
	path_gps_point_t* point2, path_gps_point_t* result,
	timestamp_t* at_time, int flags, double float_tolerance);
extern int path_gps_interpolate_point(path_gps_point_t* point1,
	path_gps_point_t* point2, path_gps_point_t* result,
	double desired_lat, double desired_long, int flags, 
	double float_tolerance);
extern int path_gps_interpolate_offset(path_gps_point_t* lower,
	path_gps_point_t* upper, path_gps_point_t* result, double offset, 
	double float_tolerance);
extern int path_gps_is_inside_polygon(path_gps_polygon_t* closed_region,
	double latitude, double longitude, double float_tolerance);

/** Functions for storing GPS points in sqlite3 database
 */
extern void path_gps_data_insert_str(char *cmd_str, gps_data_typ *pg);
extern void path_gps_point_insert_str(char *cmd_str, path_gps_point_t *pp);
extern int path_gps_point_fill_from_row(void *ptr, int argc, char **argv, 
		char **az_col);

extern void path_gps_latlong2xy(path_gps_point_t *pt, path_gps_point_t origin,
                        double *yout, double *xout, int n, int flags);

extern void path_gps_xy2latlong(double *xin, double *yin, 
				path_gps_point_t origin, 
				path_gps_point_t *pts_out, int n);

#endif
