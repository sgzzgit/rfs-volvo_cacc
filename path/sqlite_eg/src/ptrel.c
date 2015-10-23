/**\file 
 *
 * ptrel.c  Reads points out of a GPS point table "pttbl" and  
 *	    and writes to a "reltbl" where the GPS coordinates
 *	    are translated into coordinates relative to 
 *	    an input GPS point (origin ) and heading (positive Y axis). 
 *
 *	    By default the output database is assumed to be
 *	    the same as the input, but a different one can
 *	    be specified. The tables are assumed to have the
 *	    the same field structures as pttbl and reltbl in 
 *	    /home/gems/bin/gps.dump, but the names can be
 *	    different. In normal operation, queries would
 *	    be used to create the input pttbl of interest
 *	    as well as to get the GPS point coordinates
 *	    for the command line flags.
 *	
 *  Copyright (c) 2008   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <local.h>
#include <timing.h>
#include <timestamp.h>
#include <udp_utils.h>
#include <sqlite3.h>
#include "path_gps_lib.h"

static jmp_buf exit_env;
static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

static void sig_hand(int code)
{
        if (code == SIGALRM)
                return;
        else
                longjmp(exit_env, code);
}

// can't seem to pass data frame to call back, so using globals
static path_gps_point_t origin_pt;	/// GPS of reference point	
static path_gps_point_t gps_pt;	
static int row_count = 0;	
static char object_id_str[2*GPS_OBJECT_ID_SIZE+4];
static double lat0, long0;		/// origin GPS coordinates (degrees)
static double xzero;
static double yzero;			/// lat0, long0 after scaling 
static double heading0;			/// GPS heading (deg clockwise from N)
static double scalex, scaley;		/// scale factors
static double theta;			/// angle from X-axis, radians	
static char insert_buf[MAX_CMD_STR];	/// to hold insert command string
char *in_db = "trace_gps.db";	/// default name for input database
char *out_db = NULL;		/// by default, output same as input 
char *pttbl_name = "pttbl";	/// GPS point table
char *reltbl_name = "reltbl";	/// relative coordinates 

static int verbose = 0; 	/// if 1, print extra info for debugging	

/**
 *      callback function, used with query returning number of rows
 *	from pttbl to handle results
 */
static int cntcallback(void *not_used, int argc, char **argv, char **az_col)
{
	row_count = atoi(argv[0]);
	return 0;
}

int get_blob_from_string(char *blob_str, unsigned char *blob, int length)
{
        int i;
        char hex_str[5];        /// "0x", two hex digits and final '\0';
        hex_str[0] = '0';
        hex_str[1] = 'x';
        hex_str[4] = '\0';
        if (blob_str[0] != 'x' && blob_str[0] != 'X') {
                return 0;
        }
        for (i = 0; i < length; i++) {
                hex_str[2] = blob_str[2*i];
                hex_str[3] = blob_str[2*i+1];
                sscanf(hex_str, "%hhi", &blob[i]);
        }
        return 1;
}

char *reltbl_field_str= "(object_id, local_time, local_ms, utc_time, utc_ms, x, y, speed, heading, distance, date_time)"; 

void rel_table_insert_str(char *cmd_str, char *object_id, char *local_time,
		char *local_ms, char *utc_time, char *utc_ms,
		double x, double y, double speed, 
		double heading, double distance)
{
	char tmp_str[MAX_CMD_STR];
	memset(tmp_str, 0, MAX_CMD_STR);
	snprintf(tmp_str, MAX_CMD_STR-1,
	"(\"%s\",\"%s\",%s,\"%s\",%s, %.7f,%.7f,%.3f,%.3f,%.3f,%s)",
		object_id, local_time, local_ms, utc_time, utc_ms,
		x, y, speed, heading, distance,"datetime('now')");
	snprintf(cmd_str, MAX_CMD_STR-1, "insert into %s%s values %s",	
		reltbl_name, reltbl_field_str, tmp_str);
}

/**
 *      callback function, used with query returning row from pttbl
 *      to handle results to be inserted in 
 *	no need to fill most of the structure, only latitude, longitude,
 *	speed and heading are needed for this
 *	blobs and times will be copied as strings to new table
 *	structure is filled in just for printing points when debugging
 */
static int relcallback(void *not_used, int argc, char **argv, char **az_col)
{
        path_gps_point_t *pp = &gps_pt;
        char tsbuf[20];
        char msbuf[5];
	char *blob = argv[0];
	double sx, sy; 		/// scaled lat, long
	double xi, yi;		/// transformed coordinates of row point 
	double headingi;	/// heading of row point 
	double distancei;	/// distance from origin (checking haversine)

        if ((strlen(argv[0]) != 15) || !get_blob_from_string(argv[0],
                                         &pp->object_id[0], GPS_OBJECT_ID_SIZE))
                printf("Badly formatted blob string %s\n", argv[0]);

        strncpy(tsbuf, argv[1], 20);            // should always be HH:MM:SS
        snprintf(msbuf, 5, ".%03d", argv[2]);   // arg[1] should always be ms
        strncat(tsbuf, msbuf, 20);              // put in PATH timestamp format
        str2timestamp(tsbuf, &pp->local_time);

        strncpy(tsbuf, argv[3], 20);            // should always be HH:MM:SS
        snprintf(msbuf, 5, ".%03d", argv[4]);   // arg[1] should always be ms
        strncat(tsbuf, msbuf, 20);              // put in PATH timestamp format
        str2timestamp(tsbuf, &pp->utc_time);

        pp->latitude = atof(argv[5]);
        pp->longitude = atof(argv[6]);
        pp->speed = atof(argv[7]);
        pp->heading = atof(argv[8]);
        pp->pos = atoi(argv[9]);
        pp->num_sats = atoi(argv[10]);
        pp->hdop = atof(argv[11]);
        pp->sequence_no = atof(argv[12]);
	sx = scalex * path_deg2rad(pp->latitude); 
	sy = scaley * path_deg2rad(pp->longitude); 
	xi = sx*cos(theta) - sy*sin(theta)
		 - xzero*cos(theta) + yzero*sin(theta);
	yi = sx*sin(theta) + sy*cos(theta)
		 - xzero*sin(theta) + yzero*sin(theta);
	headingi = pp->heading - heading0;
	distancei = path_gps_get_distance(&origin_pt, &gps_pt, 0, 
			0.000000001);
	rel_table_insert_str(insert_buf, argv[0], 
		argv[1], argv[2], argv[3], argv[4],
		xi, yi, pp->speed, headingi, distancei);
	return 0;
}
/**
 * 	No op callback for insert operation
 */
static int callback(void *not_used, int argc, char **argv, char **az_col){
        return 0;
}

int main(int argc, char *argv[])
{
	int do_timing = 0;	/// if 1, print user and system time 
	struct avcs_timing timing;	/// used to log timing info
	int option;		/// used with getopt

	char *udp_name = "192.168.255.255";	/// default broadcast 
	int udp_port = 7015;
        int sd_out;             /// socket descriptor for UDP send
        int bytes_sent;         /// returned from sendto
        struct sockaddr_in snd_addr;    /// used in sendto call
	int do_broadcast = 1;	/// by default, broadcast socket

	sqlite3 *in_sqlt = NULL;
	sqlite3 *out_sqlt = NULL;

	char cmd_str[MAX_CMD_STR];	/// pass to sqlite3_exec for select
	char *zerr = NULL;
	int i;				/// counts rows 
	
	while ((option = getopt(argc, argv, "h:i:o:p:r:tvx:y:")) != EOF) {
		switch(option) {
		case 'h':
			sscanf(optarg, "%lf", &heading0);
			break;
		case 'i':
			in_db = strdup(optarg);
			break;
		case 'o':
			out_db = strdup(optarg);
			break;
		case 'p':
			pttbl_name = strdup(optarg);
			break;
		case 'r':
			reltbl_name = strdup(optarg);
			break;
		case 't':
			do_timing = 1;	/// prints sys and user time
			break;
		case 'v':
			verbose = 1;
			break;
		case 'x':
			sscanf(optarg, "%lf", &lat0);
			break;
		case 'y':
			sscanf(optarg, "%lf", &long0);
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -h  <heading>\n ");
			fprintf(stderr, " -i  <in_db>\n ");
			fprintf(stderr, " -o  <out_db>\n ");
			fprintf(stderr, " -p  <pttbl name>\n ");
			fprintf(stderr, " -r  <reltbl name>\n ");
			fprintf(stderr, " -t  (measure timing) \n ");
			fprintf(stderr, " -v  (verbose mode) \n ");
			fprintf(stderr, " -x  (latitude, degrees) \n ");
			fprintf(stderr, " -y  (longitude, degrees) \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
	memset(cmd_str, 0, MAX_CMD_STR);	/// make sure NULL at end
	origin_pt.latitude = lat0;
	origin_pt.longitude = long0;
	origin_pt.heading = heading0;

	if (do_timing)
		avcs_start_timing(&timing);

	if (sqlite3_open(in_db, &in_sqlt) != 0) {
		fprintf(stderr, "Can't open %s\n", sqlite3_errmsg(in_sqlt));
		sqlite3_close(in_sqlt);
		exit(EXIT_FAILURE);
	}
	if (out_db != NULL) {
		if (sqlite3_open(out_db, &out_sqlt) != 0) {
			fprintf(stderr, "Can't open %s\n",
				 sqlite3_errmsg(out_sqlt));
			sqlite3_close(out_sqlt);
			exit(EXIT_FAILURE);
		}
	} else
		out_sqlt = in_sqlt;
		
	if (setjmp(exit_env) != 0) {
		if (do_timing) {
			printf("Exiting %s\n", argv[0]);
			fflush(stdout);
			avcs_end_timing(&timing);
			avcs_print_timing(stdout, &timing);
			fflush(stdout);
		}
		sqlite3_close(in_sqlt);
		if (out_sqlt != NULL)
			sqlite3_close(out_sqlt);
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	scalex = (20000000/M_PI) * cos(fabs(lat0));
	scaley = (20000000/M_PI);
	xzero = scalex * lat0;
	yzero = scaley * long0;
	theta = M_PI/2 - path_deg2rad(heading0);

	snprintf(cmd_str, MAX_CMD_STR-1, "select count(*) from %s", pttbl_name);
	if (verbose) 
		printf("cmd_str %s\n", cmd_str);
		
	if (sqlite3_exec(in_sqlt, cmd_str, cntcallback, 0, &zerr)
							 != SQLITE_OK) {
			fprintf(stderr, "sqlite3 error: %s\n", zerr);
			sqlite3_free(zerr);
	}  
	if (verbose)
		printf("row count %d\n", row_count);

	memset(&gps_pt, 0, sizeof(gps_pt));

	for (i = 1; i < row_count; i++) {
		timing.exec_num++;
		snprintf(cmd_str, MAX_CMD_STR-1, 
			"select * from %s where ROWID = %d", pttbl_name, i);	

		if (verbose)
			printf("%s\n", cmd_str);

		if (sqlite3_exec(in_sqlt, cmd_str, relcallback, 0, &zerr)
					 != SQLITE_OK) {
			fprintf(stderr, "sqlite3 error: %s\n", zerr);
			sqlite3_free(zerr);
		}  
		if (verbose) {
			path_gps_print_point(stdout, &gps_pt);
			printf("\n");
		}
		if (sqlite3_exec(out_sqlt, insert_buf, callback, 0, &zerr)
					 != SQLITE_OK) {
                        fprintf(stderr, "sqlite3 error: %s\n", zerr);
                        sqlite3_free(zerr);
		}
	}
}

