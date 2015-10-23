/**\file 
 *
 * gpsread.c      Read GPS data and write to file (for debugging)
 *
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *
 * MODIFIED: 7/19/2006 Bernard Liang - added HDOP and numsats info to output,
 * [document the other changes also]
 * 
 *  Process to read GPS data messages, parse that information, and record
 * data (timestamp, UTC time, latitude, longitude, and speed) to a file.
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.
 *
 */

#include <sys_os.h>
#include <timestamp.h>

#define MAX_MSG_LENGTH         128

int fpin;	
jmp_buf env;
FILE *g_data;

static int print_data_line(int index);
static void open_files();
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	ERROR,
};

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
float rmc_utc;
char rmc_status;
float rmc_lat;
char rmc_lat_dir;
float rmc_long;
char rmc_long_dir;
float rmc_sog_knot;
float rmc_cog_truenorth;
char *rmc_date;
float rmc_mag_var;
char rmc_mag_dir;
struct timespec now;
int status;
int file_time;		// number of minutes to record to a file
double start_time;	// start_time and current_time
double curr_time;	// used to measure file open and close interval
timestamp_t ts;

char** gga;
char** rmc;
int valid[60] = {0};

int main(int argc, char *argv[]) {
	char readBuff[MAX_MSG_LENGTH];
	char header[10];
	char temp;
	int i;
	char *ser_name = "/dev/ttyUSB0";
	char *domain = "trolley";
	file_time = 15;     // number of minutes to record to a file 	
	int option;
	char numstr[16];
	int index;
	gga = (char **) malloc (60 * sizeof (char *));
	rmc = (char **) malloc (60 * sizeof (char *));
	for (i = 0; i < 60; i++) {
		gga[i] = (char *) malloc (160 * sizeof (char));
		rmc[i] = (char *) malloc (160 * sizeof (char));
	}

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "m:d:f:")) != EOF) {
		switch(option) {
	        case 'm':
			if ((file_time = atoi(optarg)) <= 0) {
				printf("Bad parameter for flag -m.\n");
				printf("Using 15 minute file close interval\n");
	                }
			break;
		case 'd':
			domain = strdup(optarg);
			break;
		case 'f':
			ser_name = strdup(optarg);
			break;
		default:
			fprintf(stderr, 
			"Usage %s: -m <file time> -d <domain> -f <serial device name>",
				argv[0]);
		}
	}


	/* Initialize serial port. */  
	fpin = open(ser_name,  O_RDONLY);
	if (fpin <= 0) {
		printf("Error opening %s for input\n", ser_name);
	}

	if (setjmp(env) != 0) {
		fclose( g_data );
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	/* Determine the start time.  */
	status = clock_gettime(CLOCK_REALTIME, &now);
	start_time = now.tv_sec + ((double) now.tv_nsec/ 1000000000L);


	open_files();

	while(1) {
		/* Read until the beginning of a message, signified by '$' */
		temp = 0;
		while (temp != '$')
			read(fpin, &temp, 1);
		/* Now read the header, i.e. until the first comma. 
		 * If more than 10 characters read, break. */
		i = 0;
		read (fpin, &temp, 1);
		while ((temp != ',') && (i < 10)) {
			header[i] = temp;
			i++;
			read (fpin, &temp, 1);
	        }

		if (i >= 10)
	        printf("Bad GPS message.\n");
//			break;
		header[i] = '\0';
	    
		/* Now read the remainder of this messge into readBuff. 
		 * Read no more than MAX_MSG_LENGTH characters. 
		 * Read until the '*' character, which is the last
		 * character before the checksum.
		 */
		memset(readBuff, 0x0, MAX_MSG_LENGTH);
		for (i=0; i<MAX_MSG_LENGTH; i++) {
			read(fpin, &readBuff[i], 1);
			if (readBuff[i] == '*')
				break;
	        }
		readBuff[i] = '\0';

		/* Now interpret the message depending upon header type. */
		if (strcmp(header, "GPGGA") == 0) {
			strncpy(numstr, &readBuff[4], 2);
			numstr[2] = '\0';
			index = atoi(numstr);
			if ((index < 0) || (index > 59)) continue;
			strcpy(gga[index], readBuff);
			valid[index] |= 2;
			if (valid[index] == 3) {
				print_data_line(index);
			}
	        } else if (strcmp(header, "GPVTG") == 0) {
//			sscanf(readBuff, "%f,%c,%f,%c,%f,%c,%f,%c",
//				&vtg_cog_truenorth, &vtg_cog_trueorient,
//				&vtg_cog_magnorth, &vtg_cog_magorient,
//				&vtg_sog_knot, &vtg_sog_knot_unit,
//				&vtg_sog_kmhr, &vtg_sog_kmhr_unit);
//			gps_vtg.speed_over_ground = vtg_sog_kmhr;
//			(void) db_clt_write(pclt, DB_GPS_VTG_VAR, 
//				sizeof(gps_vtg_typ), (void *) &gps_vtg);
		} else if (strcmp(header, "GPRMC") == 0) {
			strncpy(numstr, &readBuff[4], 2);
			numstr[2] = '\0';
			index = atoi(numstr);
			if ((index < 0) || (index > 59)) continue;
			strcpy(rmc[index], readBuff);
			valid[index] |= 1;
			if (valid[index] == 3) {
				print_data_line(index);
			}
	        }// else
//			printf("Unknown message from GPS.\n");

	}

}

/* Returns 1 for success, 0 on error */
int print_data_line(int index) {
	char gga_time[16];
	char rmc_time[16];
	/* First make sure the times agree */
	strncpy(gga_time, gga[index], 10);
	strncpy(rmc_time, rmc[index], 10);
	if (strncmp(gga_time, rmc_time, 10)) return 0;
	
	/* Now get the necessary data and combine into one line */
	sscanf(gga[index], "%f,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c",
	&gga_utc, &gga_lat, &gga_lat_dir, &gga_long, &gga_long_dir,
	&gga_pos_typ, &gga_sats, &gga_HDOP, &gga_alt,
	&gga_alt_unit, &gga_geod_sep, &gga_geod_sep_unit);

	sscanf(rmc[index], "%f,%c,%f,%c,%f,%c,%f,%f,%s,%f,%c",
	    &rmc_utc, &rmc_status, &rmc_lat, &rmc_lat_dir,
	    &rmc_long, &rmc_long_dir, &rmc_sog_knot, 
	    &rmc_cog_truenorth, &rmc_date,
	    &rmc_mag_var, &rmc_mag_dir );
	if ( rmc_lat_dir == 'S' )
	    rmc_lat = -rmc_lat;
	if ( rmc_long_dir == 'W' )
	    rmc_long = -rmc_long;
	get_current_timestamp( &ts);
	print_timestamp( g_data, &ts );
	fprintf(g_data, " %f %f %f %f %3.1f %02d\n",
	    rmc_utc, rmc_lat, rmc_long, rmc_sog_knot, gga_HDOP, gga_sats );
	/* Determine the time since files were opened. */
	status = clock_gettime(CLOCK_REALTIME, &now);
	curr_time = now.tv_sec + 
		((double) now.tv_nsec/1000000000L) - start_time;

	/* If the current time is more than the allotted time for file
	* collection, close the current files and open a new set.
	*/
	if (curr_time >= file_time * 60.0) {
		start_time = now.tv_sec + ((double) now.tv_nsec/ 1000000000L);
		fclose(g_data);
		open_files();
	}
	valid[index] = 0;
	return 1;
}

/* The following function will open a set of data files for all the
 * data that needs to be saved.  This includes:
 * tMMDDSSS.dat for all trolley data
 *
 * In the above names, MM is replaced by a 2-digit month code, DD is
 * replaced by a 2-digit day code, and SSS is replaced by a 3-digit
 * serial code.  Serial codes for a given day start at 000 and proceed
 * to 999.  If more than 999 files are written on the same date, the number
 * will be reset to 000 and an attempt to find a number with no file is
 * made.  If no serial number is available (VERY UNLIKELY -SHOULD NEVER
 * HAPPEN) no files will be opened so no data will be recorded.  Only the
 * radar A data file is checked during the serial number check.
 *
 * If the value in old_fileday does not match the current date, the
 * serial number (file_serialno) will be reset to zero.  This will happen
 * the first time open_files is called (since old_fileday was deliberately
 * initialized to 99, an invalid day) and if the system ever runs through
 * a day change at midnight.
 */
static void open_files()
{
	static int old_fileday = 99;   /* Initialize with something illegal so
                                * file_serialno is automatically set to zero
                                * during initialization. */
	static int file_serialno = 0;

	struct timeb timeptr_raw;
	struct tm time_converted;
	char filename[80];
	bool_typ file_exists = TRUE;
	char outstring[20];
	char sum;
	int i;

	/* Get date information. */
	ftime (&timeptr_raw);
	localtime_r (&timeptr_raw.time, &time_converted);

	/* If old_fileday is not the same as time_converted.tm_mday, reset
	 * the serial number to zero.  This will happen first time through, or
	 * if we run through midnight. */
	if (old_fileday != time_converted.tm_mday)
	    file_serialno = 0;
	old_fileday = time_converted.tm_mday;

	while(file_exists == TRUE) {
		sprintf(filename, "/share/gps/data/g%2.2d%2.2d%3.3d.dat",
			 time_converted.tm_mon+1,
			 time_converted.tm_mday, file_serialno);
		g_data = fopen(filename, "r");
		if (g_data == NULL)
			file_exists = FALSE;
		else {
			fclose(g_data);
			file_serialno++;
		}
	}

	/* We've found a valid serial number.
	 */
	g_data = fopen(filename, "w");
//printf("Opened file %s\n",filename);
	file_serialno++;
	return;

}
  
static void sig_hand(int sig)
{
	longjmp(env, 1);
}
