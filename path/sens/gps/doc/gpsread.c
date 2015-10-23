/* \file gpsread.c
 *
 *
 * Copyright (c) 2000-2007   Regents of the University of California
 *
 *
 *  Process to read GPS data messages, parse that information, and put 
 *  the value into the database. Also saves trace info in an array.
 *
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for timers. Upon termination
 *  log out of the database.
 *
 *  Database access for Linux boilerplate added, but not yet tested
 *  Sue September 2007
 *
 */

#include <sys_os.h>
#include <local.h>
#include <timing.h>
#include <timestamp.h>
#include <sys_rt.h>

#include "db_clt.h"
#include "db_utils.h"
#include "clt_vars.h"
#include "gps.h"


int fpin;
int fpout;

static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

static jmp_buf exit_env;

static void sig_hand( int code )
{
	if (code == SIGALRM)
		return;
	else
		longjmp( exit_env, code );
}

#ifdef __QNX__
#include "db_vars_list.h"
#endif

#define MAX_MSG_LENGTH			128

static void outfile();
#define MAX_INDEX 30000
int saved_index = 0;

double clock_time[MAX_INDEX];
float utc[MAX_INDEX];
float latitude[MAX_INDEX];
float longitude[MAX_INDEX];
float altitude[MAX_INDEX];
float speed_over_ground[MAX_INDEX];
int num_sats[MAX_INDEX];
float speed1[MAX_INDEX];
float speed2[MAX_INDEX];
float brake[MAX_INDEX];

static int verbose = 0;

void do_usage(char *progname)
{
        fprintf(stderr, "Usage %s:\n", progname);
        fprintf(stderr, "-d data server domain name for QNX6 ");
        fprintf(stderr, "-e <devname> echo output to this serial port");
        fprintf(stderr, "-f <devname> read from this serial port ");
        fprintf(stderr, "-v verbose debugging output");
        fprintf(stderr, "-w write a trace file gps*.dat");
        fprintf(stderr, "-x don't update the data server ");
        fprintf(stderr, "\n");
        exit(1);
}

int main( int argc, char *argv[] )
{
	char readBuff[MAX_MSG_LENGTH];
	char outBuff[MAX_MSG_LENGTH + 12];
	char header[11];
	char hostname[MAXHOSTNAMELEN+1];
	gps_gga_typ gps_gga;
	gps_vtg_typ gps_vtg;
	char temp;
	int i;
	int j;
	float gga_utc;
	float gga_lat;
	char gga_lat_dir;
	float gga_long;
	char gga_long_dir;
	int gga_pos_typ;
	int gga_sats;
	float gga_HDOP;
	float gga_alt;
	char gga_alt_unit;
	float gga_geod_sep;
	char gga_geod_sep_unit;
	float vtg_cog_truenorth;
	char vtg_cog_trueorient;
	float vtg_cog_magnorth;
	char vtg_cog_magorient;
	float vtg_sog_knot;
	char vtg_sog_knot_unit;
	float vtg_sog_kmhr;
	char vtg_sog_kmhr_unit;
	float taurus_speed1, taurus_speed2;
	float taurus_brake;
	timestamp_t current_ts;
	timestamp_t *ts = &current_ts;
	char *sername = "/dev/ttyS0";
	char *echoname = NULL; 
        int xport = COMM_PSX_XPORT;	// for QNX6, no-op
	int use_database = 1;	// normally use database, for testing turn off
	int do_echo = 0;
	int write_trace = 0;
	char *domain = DEFAULT_SERVICE;	// for QNX6, read in
        db_clt_typ *pclt;
	db_id_t *vars_list;
	int num_vars;
	int option;

        while ((option = getopt(argc, argv, "d:e:f:p:wvx")) != EOF)
	    {
                switch(option)
	        {
                case 'd':
                        domain = strdup(optarg);
                        break;
                case 'e':
                        echoname = strdup(optarg);
			do_echo = 1;
                        break;
                case 'f':
                        sername = strdup(optarg);
                        break;
                case 'w':
                        write_trace = 1;
                        break;
                case 'v':
                        verbose = 1;
                        break;
                case 'x':
                        use_database = 0; 
                        break;
                default:
                        do_usage(argv[0]);
                        break;
                }
	    }

#ifdef __QNX__
	vars_list = &db_vars_list[0];
	num_vars = NUM_DB_VARS;
#else
	vars_list = NULL;
	num_vars = 0;
#endif

	/* Log in to the database (shared global memory).  Default to the
	 * the current host. */
	if (use_database)
	    {
	    get_local_name(hostname, MAXHOSTNAMELEN);
	    if (( pclt = db_list_init( argv[0], hostname, domain, xport,
			vars_list, num_vars, NULL, 0))
			 == NULL )
	        {
	        printf("Database initialization error in gpsread\n");
	        return;
	        }
	    }
	/* Initialize serial port. */  
	fpin = open( sername,  O_RDONLY );
	if ( fpin <= 0 )
	    {
		printf( "Error opening device %s for input\n", sername );
	    }

	/* Initialize serial port for output (all input is echoed
	 * to an output port which is connected to a laptop which also wants
	 * to see the GPS data. */
	if (do_echo)
	    {
	    fpout = open( echoname, O_WRONLY | O_NONBLOCK );
	    if ( fpout <= 0 )
	        {
	        printf( "Error opening /dev/ser2 for output.\n" );
	        }
	    }
	/*	set jmp */
	if ( setjmp(exit_env) != 0)
	    {

	    /* Log out from the database. */
	    if (use_database)
	        {
		db_list_done(pclt, vars_list, num_vars, NULL, 0);
		}
	    if (write_trace)
		outfile();

		exit( EXIT_SUCCESS);
	}
	else 
		sig_ign( sig_list, sig_hand);

	while(1)
	    {

    /* Read until the beginning of a message, signified by the '$' char. */
	    temp = '\0';
	    while ( temp != '$' )
	        {
	        read( fpin, &temp, 1 );
	        outBuff[0] = temp;
	        }

	    /* Now read the header, i.e. until the first comma.  If more than
	     * 10 characters read, break. */
	    i = 0;
	    read ( fpin, &temp, 1 );
	    outBuff[i+1] = temp;
	    while ( (temp != ',') && (i < 10) )
	        {
	        header[i] = temp;
	        i++;
	        read ( fpin, &temp, 1 );
	        outBuff[i+1] = temp;
	        }
	    if (i >= 10)
	        {
	        printf("Bad GPS message,");
	        for ( j=0; j<i; j++)
	            printf("%c", header[j]);
	        printf("\n");
	        }
	    header[i] = '\0';
	    
	    /* Now read the remainder of this messge into readBuff.  Read no
	     * more than MAX_MSG_LENGTH characters. 
	     * Read until the '*' character,
	     * which is the last character before the checksum. */
	    memset( readBuff, 0x0, MAX_MSG_LENGTH );
	    for ( i=0; i<MAX_MSG_LENGTH; i++ )
	        {
	        read( fpin, &readBuff[i], 1 );
	        outBuff[i+7] = readBuff[i];
	        if ( readBuff[i] == '*' )
	            break;
	        }
	    read( fpin, &outBuff[i+8], 1 );
	    read( fpin, &outBuff[i+9], 1 );
	    read( fpin, &outBuff[i+10], 1 );
	    read( fpin, &outBuff[i+11], 1 );

	    readBuff[i+1] = '\0';

	    /* Now write the entire message to the serial port connected to
	     * laptop. */
	    if (do_echo)
		write( fpout, &outBuff, i+12 );

	    /* Now interpret the message depending upon header type. */
	    if ( strcmp( header, "TAURUS" ) == 0 )
	        {
	        sscanf( readBuff, "%f,%f,%f",
	                &taurus_speed1, &taurus_speed2, &taurus_brake );

		speed1[saved_index] = taurus_speed1;
		speed2[saved_index] = taurus_speed2;
		brake[saved_index] = taurus_brake;

        	if (saved_index < MAX_INDEX)
			saved_index++;
	    }
	    else if ( strcmp( header, "GPGGA" ) == 0 )
	        {
	        sscanf( readBuff, "%f,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c",
	                &gga_utc, &gga_lat, &gga_lat_dir, 
			&gga_long, &gga_long_dir,
	                &gga_pos_typ, &gga_sats, &gga_HDOP, &gga_alt,
	                &gga_alt_unit, &gga_geod_sep, &gga_geod_sep_unit );

		if (verbose) 
			printf("GPSGGA: lat %.6f long %.6f utc %.3f\n",
			 gga_lat, gga_long, gga_utc);
	        if ( gga_lat_dir != 'N' )
		    gga_lat = -gga_lat;
	        if ( gga_long_dir != 'E' )
		    gga_long = -gga_long;
		latitude[saved_index] = gps_gga.latitude = gga_lat;
		longitude[saved_index] = gps_gga.longitude = gga_long;

		get_current_timestamp(ts);
		clock_time[saved_index] = 
			ts->hour*3600.0 + ts->min * 60.0 + ts->sec +
					ts->millisec/1000.0;
		utc[saved_index] = gps_gga.utc_time = gga_utc;
		altitude[saved_index] = gga_alt;
		num_sats[saved_index] = gps_gga.num_sats = gga_sats;
		gps_gga.HDOP = gga_HDOP;

		if (saved_index < MAX_INDEX)
			saved_index++;
		if (use_database)
			db_clt_write(pclt, DB_GPS_GGA_VAR, sizeof(gps_gga_typ),
				&gps_gga);

	        }
	    else if (strcmp( header, "GPVTG" ) == 0 )
	        {
	        sscanf( readBuff, "%f,%c,%f,%c,%f,%c,%f,%c", &vtg_cog_truenorth,
	                &vtg_cog_trueorient, &vtg_cog_magnorth,
			&vtg_cog_magorient,
	                &vtg_sog_knot, &vtg_sog_knot_unit, &vtg_sog_kmhr,
	                &vtg_sog_kmhr_unit );
		if (verbose) 
			printf("GPSVTG: speed %.6f course %.6f\n", 
				vtg_sog_kmhr, vtg_cog_truenorth);
		speed_over_ground[saved_index] = vtg_sog_kmhr;
		gps_vtg.speed_over_ground = vtg_sog_kmhr;
		gps_vtg.course_over_ground = vtg_cog_truenorth;
		if (use_database)
			db_clt_write(pclt, DB_GPS_VTG_VAR, sizeof(gps_gga_typ),
				&gps_vtg);
	        }
	    else
		if (verbose)
			printf("Unknown message from GPS. Header %s\n", header);

	    }

}

static void outfile()
{
	int i;
	char filename[80];
	bool_typ file_exists = TRUE;
	static int file_serialno = 0;
	FILE *fd;

	while( file_exists == TRUE )
	{
		sprintf( filename, "gps%d.dat", file_serialno );
		fd = fopen(filename, "r");
		if (fd <= 0)
			file_exists = FALSE;
		else
		{
			fclose( fd );
			file_serialno++;
		}
	}

	/* We've found a valid serial no, so open the file */
	fd = fopen(filename, "w");

	for (i=0; i<saved_index; i++)
	    {
	    fprintf(fd, "%lf %f %f %f %f %d %f %f %f\n", 
		clock_time[i], utc[i], 
		latitude[i], longitude[i], altitude[i], 
		num_sats[i], speed1[i], speed2[i], brake[i]);
	    }
	fclose(fd);

	file_serialno++;
	return;
}
