/* \file
 *   Reads message type 82 (or 89) (FE target report) from
 *   Eaton Vorad EVT-300 radar and saves data in the database
 *
 *  Copyright (c) 1999-2007   Regents of the University of California
 *
 *  Process to communicate with the Eaton Vorad EVT-300 radar.  
 *
 *  Currently up to 4 radars (A, B, C and D) can be read and their
 *  data stored in separate database variables. To add more, just
 *  add another database variable name in evt300.h and another
 *  branch to the switch statement below.
 *
 *  This test program does not create its own DB variables, it is
 *  currently assumed that they are created by a separate program
 *  that is creating all the process variables, as is done for CICAS.
 *
 *  Written by Paul Kretz, ported to Linux from IDS QNX version by Sue Dickey
 *  and John Spring.
 *  Generalized and updated in January 2008, with MySQL added by Jeff Ko. 
 *
 */

#include "sys_os.h"
#include "sys_rt.h"
#include "sys_list.h"
#include "sys_buff.h"
#include "local.h"
#include "timestamp.h"
#include "data_log.h"
#include "db_clt.h"
#include "db_utils.h"
#include "evt300.h"


/** Usage:
 *  -d not needed for Linux, DEFAULT_SERVICE is Linux value
 *  -o output mask 1 trace 2 DB 4 MySQl (can OR)
 *  -r can be 'a' or 'b' to select radar database variable  
 *  -s takes the serial port string, e.g. "/dev/ttyS0" 
 *  -t time in minutes for trace file duration 
 *  -v verbose, prints trace output
 *
 *  Typical usage: 
 *  evt300 -r a -s /dev/ttyS0 -o 2
 *  will write Radar A to DB data server
 *
 *  evt300 -r B -s /dev/ttyS1 -o 1 -t 3
 *  will write Radar B to a trace file, opening a new file with
 *  a different name every 3 minutes
 */
static char *usage= "-d domain -r radar letter -s serial port -v verbose -o output mask (1 trace, 2 DB, 4 MySQL, can OR) -t trace file time in minutes"; 

/** Signal handling is required for clean exits when using the
 *  data server. SIGALRM is requird for use of timers.
 */
static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else
		longjmp(exit_env, code);
}

/** Possible to do all three at once by setting -o to 7, or
 *  any two, e.g, 3 for TRACE_FILE | USE_DB will both write a
 *  trace file and write the DB server.
 */
#define TRACE_FILE	1	
#define USE_DB		2
#define USE_MYSQL	4

int main(int argc, char *argv[])
{
	char hostname[MAXHOSTNAMELEN+1];
	evt300_radar_typ evt300_radar;
	evt300_radar_info_t evt300_info;
	db_clt_typ *pclt = NULL;
	int radar_db_num = DB_EVT300_RADAR1_VAR;
	int fpin;
	FILE *f_radar;
	int output_mask = 1;	// 1 trace, 2 DB server, 4 MySQL
	int option;
	char *serial_name = "/dev/ttyS0";
	char *domain = DEFAULT_SERVICE; //for QNX6, use e.g. "ids"
	int xport = COMM_OS_XPORT;	//for QNX6, no-op

	/** Variables used in call top open_data_log and reopen_data_log
         */
	char file_prefix[16];
	char file_suffix[16];
	int old_fileday = 99;	/// Non-occuring value for initialization 
	int file_serialno = 0;	
	double start_time;	/// time last file was opened
	int file_time = 15;	/// number of minutes to record to a file 
	char radar_letter;

	evt300_info.verbose = 0;
	evt300_info.error_count = 0;
        while ((option = getopt(argc, argv, "d:o:r:s:t:v")) != EOF) {
                switch(option) {
		case 'd':
			domain = strdup(optarg);
			break;
		case 'o':
			output_mask = atoi(optarg);	
			break;
		case 'r':
			radar_letter = optarg[0];
			evt300_info.id = strdup(optarg);
			break;
		case 's':
			serial_name = strdup(optarg);
			break;
		case 't':
			file_time = atoi(optarg);	
			break;
		case 'v':
			evt300_info.verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage: %s %s\n",
						 argv[0], usage);
			exit(EXIT_FAILURE);
			break;
		}
	}
	switch (radar_letter) {
	case 'a':
	case 'A':
		radar_db_num = DB_EVT300_RADAR1_VAR;
		break;
	case 'b':
	case 'B':
		radar_db_num = DB_EVT300_RADAR2_VAR;
		break;
	case 'c':
	case 'C':
		radar_db_num = DB_EVT300_RADAR3_VAR;
		break;
	case 'd':
	case 'D':
		radar_db_num = DB_EVT300_RADAR4_VAR;
		break;
	default:
		printf("Unknown radar letter %c\n", radar_letter);
		exit(EXIT_FAILURE);
	}
	sprintf(file_prefix, "%c", radar_letter);
	sprintf(file_suffix, ".dat");

	if (output_mask & USE_DB) {
		get_local_name(hostname, MAXHOSTNAMELEN);
		if ((pclt = db_list_init(argv[0], hostname, domain, xport, 
			NULL, 0, NULL, 0)) == NULL) {
			printf("Database initialization error in %s.\n", 
						argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	
	/* Initialize serial port. */  
	fpin = open(serial_name,  O_RDONLY);
	if (fpin <= 0) {
		printf("Error opening device %s for input\n", serial_name);
	}

	/** Open trace file if wanted */
	if (output_mask & TRACE_FILE) {
		if (!open_data_log(&f_radar, file_prefix, file_suffix,
			 &start_time, &old_fileday, &file_serialno, 
				NULL)){
				 printf("Error opening %s%s\n",
					 file_prefix, file_suffix);
				 exit (EXIT_FAILURE);
		}
	}

	/** Receiving signal will cause program to exit here
	*/
	if (setjmp(exit_env) != 0) {
		/** Log out from the DB data server. */
		if (pclt != NULL) 
			db_list_done(pclt, NULL, 0, NULL, 0);
			printf("%s radar %s exits, %d checksum errors\n", 
				argv[0], evt300_info.id,
				evt300_info.error_count);
		if (f_radar != NULL) {
			fflush(f_radar);
			fclose(f_radar);
		}
		exit(EXIT_SUCCESS);
	} else 
		sig_ign(sig_list, sig_hand);


	for (;;) {
		timestamp_t ts;		// time for writing file
		if (evt300_ser_driver_read (fpin, 
				&evt300_radar, &evt300_info)) {
			get_current_timestamp(&ts);

			if (output_mask & TRACE_FILE) {
				evt300_print_radar(f_radar, &evt300_radar, 
					ts, &evt300_info);
				reopen_data_log(&f_radar, file_time,
					file_prefix, file_suffix,
					&start_time, &old_fileday,
					&file_serialno, NULL, NULL);
			}
			if (output_mask & USE_DB)
				db_clt_write(pclt, radar_db_num, 
					sizeof(evt300_radar_typ),
					 &evt300_radar);
			if (output_mask & USE_MYSQL)
				evt300_mysql_save_radar(&evt300_radar, 
					&evt300_info);
		} 
	} 
}
