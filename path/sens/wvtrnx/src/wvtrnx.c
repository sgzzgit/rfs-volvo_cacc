/**\file wvtrnx.c
 *
 * Copyright (c) 2009   Regents of the University of California
 *
 *  Process to read Wavetronix messages, parse that information, and put 
 *  the value into the data server.
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate.  Upon termination log out of the database.
 *
 */

#include <sys_os.h>	/// In path/local, OS-dependent definitions
#include <sys_rt.h>	/// In path/local, "real-time" definitions
#include <sys_ini.h>	/// In path/local, for reading configuration files
#include <timestamp.h>	/// In path/local, for using hh:mm:ss.sss timestamps
#include "db_clt.h"	/// In path/db, for using basic DB client services
#include "db_utils.h"	/// In path/db, for using DB client convenience utilities
#include "wvtrnx.h"	/// Device-specific header file

/** File descriptors for reading and writing the Wavetronix serial port
 */
int fpin;
int fpout;

/** Signal handling required set-up
 */
static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

jmp_buf env;
int status;
static void sig_hand(int sig);
static void debug_only( unsigned char readbuff, int index);

int main(int argc, char *argv[])
{
	unsigned char readbuff[MAX_WVTRNX_MSG_LENGTH + 10];	/// 10 = 3 + 4 + 3 (3 bytes of header + 4 bytes of checksum + 3 bytes of footer)

	char hostname[MAXHOSTNAMELEN+1];
	int option;
	char *domain = DEFAULT_SERVICE; /// set up in sys_os.h, used for DB data server 
	int xport = COMM_OS_XPORT;	/// set up in sys_os.h, used for DB data server
	timestamp_t ts;			/// use when writing to dataserver
	db_clt_typ *pclt = NULL;               /// Database client pointer
	char *serialname = "/dev/ttyS0";	/// Can change this on command line
	int verbose = 0;		/// Set to 1 to print debug
	int use_db = 0;		/// Set to 1 to print debug
	int msg_count = 0;	/// Counts messages from Wavetronix

	int index = 0;
	ssize_t val;
	int track_no = 0;
	unsigned int checksum = 0;

        /** Get command line options */
	while ((option = getopt(argc, argv, "dvs:")) != EOF) {
		switch(option) {
		case 'd':
			use_db = 1;
			break;
		case 's':
			serialname = strdup(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			printf("Usage: %s [-v] [-s <serial port>]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/* Log in to the database (shared global memory).  Default to the
	* the current host. */
	if (use_db) {
		get_local_name(hostname,MAXHOSTNAMELEN);
		if ((pclt = db_list_init(argv[0], hostname, domain, xport,
		      NULL, 0, NULL, 0)) == NULL) {
			printf("Database initialization error in %s. \n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/* Initialize serial port. */  
	fpin = open(serialname,  O_RDONLY);
	if (fpin <= 0){
		printf("Error opening device %s for input\n", serialname);
		exit(EXIT_FAILURE);
	}
	fpout = open(serialname,  O_WRONLY | O_NONBLOCK);
	if (fpout <= 0){
		printf("Error opening device %s for output\n", serialname);
		exit(EXIT_FAILURE);
	}
	
	printf("%s successfully opened %s\n", argv[0], serialname);

	/** Here is where program will go when it handles a signal
	 */
	if (setjmp(env) != 0) {
		if (pclt != NULL)
			db_list_done(pclt, NULL, 0, NULL, 0);
		printf("\nWavetronix exits, %d messages received\n", msg_count);
		fflush(stdout);
		exit(EXIT_SUCCESS);
	} else 
		    sig_ign(sig_list, sig_hand);

	while(1) {
		wvtrnx_track_t wvtrnx_track;
		wvtrnx_msg_t wvtrnx_msg;

		/** Send tracking command to Wavetronix. 
		 */
		strncpy(readbuff, "XT\rXT\r", MAX_WVTRNX_MSG_LENGTH);
		write(fpout, &readbuff[0], sizeof(readbuff));
		
		get_current_timestamp(&ts);
		if (verbose) {
			wvtrnx_msg.ts  = ts;
			print_timestamp(stdout, &ts);
			printf(" ");
			fflush(stdout);
		}
	
		checksum =0;
		/** Header
		 */
		while(index < 3){
			val = read (fpin, &readbuff[index], 1);
			if (val == 1){				
				if (verbose)
					debug_only(readbuff[index], index);
					
				switch(index){
					case 0:
						if(readbuff[index] == 'X'){}
						else{}
						break;
					case 1:
						if(readbuff[index] == 'T'){}
						else{}
						break;
					case 2:
						if(readbuff[index] == 'K'){}
						else{}
						break;					
				}
			}
			index++;
		}

		/** 25 tracks of Wavetronix message which includes status, distance and speed of 
		 *  targets.
		 */
		while(index < 78){
			val = read (fpin, &readbuff[index], 1);
			if (val == 1){				
				if (verbose)
					debug_only(readbuff[index], index);
					
				track_no = (index / 3) - 1;
				switch(index%3){
					case 0: 
						wvtrnx_msg.tracks[track_no].status 
							= readbuff[index];
						break;
					case 1: 
						wvtrnx_msg.tracks[track_no].distance 
							= readbuff[index];
						break;
					case 2:
						//wvtrnx_track.speed = readbuff[index];
						//wvtrnx_msg.tracks[track_no] = wvtrnx_track;
						wvtrnx_msg.tracks[track_no].speed 
							= readbuff[index]; 
						break;
				}					
			}
			index++;
		}

		/** Four bytes of checksum
		 */
		while(index < 82){
			val = read (fpin, &readbuff[index], 1);
			if (val == 1){				
				if (verbose)
					debug_only(readbuff[index], index);
					
				switch(index%78){
					case 0:						
						checksum = readbuff[index] << 0;
						break;
					case 1:
						checksum = (readbuff[index] << 8) 
							| checksum;
						break;
					case 2:
						checksum |= readbuff[index] << 16;
						break;
					case 3:
						checksum |= readbuff[index] << 24;
						break;
				}					
			}
			index++;
		}

		if (verbose) {
			printf("checksum: 0x%08x \n", checksum);
		}

		/** Each message ends with ~\r\r
		 */
		while(index < 85){
			val = read (fpin, &readbuff[index], 1);
			if (val == 1){							
				if (verbose)
					debug_only(readbuff[index], index);
				
				switch(index%82){
					case 0:
						if(readbuff[index] == '~'){}
						else{}
						break;
					case 1:
						if(readbuff[index] == '\r'){}
						else{}
						break;
					case 2:
						if(readbuff[index] == '\r'){}
						else{}
						break;
				}					
			}
			index++;
		}

		index = 0;
		msg_count++;
		if (pclt != NULL)
			db_clt_write(pclt, DB_WVTRNX_RADAR0_VAR, sizeof(wvtrnx_msg_t), &wvtrnx_msg);
		else
			fprintf(stderr, "NULL data server\n");

		if(verbose)
			print_wvtrnx(wvtrnx_msg);
	}
}

static void sig_hand(int sig)
{
	if(sig == SIGALRM)
		return;
	else
		longjmp(env, sig);
}

static void debug_only( unsigned char readbuff, int index)
{
	printf("(%03d 0x%02hhx) ", readbuff, readbuff);

	if(index == 84)
		printf("\n");

	fflush(stdout);

}
