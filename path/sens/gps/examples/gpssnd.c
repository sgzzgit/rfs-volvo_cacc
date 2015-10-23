/**\file 
 *
 * gpssnd.c     Read GPS data and write to a local heartbeat structure. 
 *
 *		Reads three message types, if present:
 *		GGA, VTG and RMC 
 *
 *		Can use gpsd by opening TCP socket, putting
 *		the interface in raw mode, then reading from 
 *		socket. But better to read from stdin, with tcp_client
 *		reading gpsd.
 *
 *		Writes GPS data to datahub and sends heartbeat message
 * 		in UDP packet whenever RMC message is received.
 *
 *		Set system clock to match GPS on first RMC message,
 *		because embedded systems often lose their date memory.
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
#include <db_clt.h>
#include <db_utils.h>
#include "path_gps_lib.h"
#undef DO_TRACE

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

#define MAX_SENTENCE	256

static db_id_t db_vars[] = {
        {DB_GPS_GGA_VAR, sizeof(gps_data_typ)},
        {DB_GPS_VTG_VAR, sizeof(gps_data_typ)},
        {DB_GPS_RMC_VAR, sizeof(gps_data_typ)},
        {DB_GPS_PT_LCL_VAR, sizeof(path_gps_point_t)},
};

#define NUM_DB_VARS	sizeof(db_vars)/sizeof(db_id_t)

int main(int argc, char *argv[])
{
	int status;
	char hostname[MAXHOSTNAMELEN];
	char *domain = DEFAULT_SERVICE;	/// on Linux sets DB q-file directory
        db_clt_typ *pclt;       /// data server client pointer
        int xport = COMM_PSX_XPORT;	/// Linux uses Posix message queues
	int use_db = 0;		/// set to 1 to use data server
	int db_num = 0;		/// must be set to non-zero for DB update.

	char sentence[MAX_GPS_SENTENCE];
	gps_data_typ gps_data;
	timestamp_t ts;
	FILE *fpin;		/// file pointer for GPS device input 
	int sd;			/// socket descriptor for gpsd input
	int gpsd_port = 2947;	/// this is the default, unlikely to change
	int use_gpsd = 0;	/// don't use gpsd socket, read serial port
	int use_pipe = 0;	/// if 1 pipe data from tcp_client reading gpsd
	char *name= "/dev/ttyS2";	/// use IP address for gpsd, else dev
	char *udp_name = NULL;		/// address of UDP destination
	int do_broadcast = 1;	/// by default broacast information
	
	int verbose = 0;	/// if 1, print extra info for debugging	
	int do_trace = 0;	/// if 1, print way too much info
	int do_timing = 0;	/// if 1, print user and system time 
	struct avcs_timing timing;

	timestamp_t current_ts;
	timestamp_t prev_ts;

	int udp_port = 7015;	/// port to send out heartbeat
	int option;
	int first_msg = 1;	/// used to sync clocks on only first RMC 

	path_gps_point_t hb;	/// fill in from GPS messages received
	int sd_out;		/// socket descriptor for UDP send
	int bytes_sent;		/// returned from sendto
	struct sockaddr_in snd_addr;	/// used in sendto call
	int millisecs = 0;		/// minimum interval for send

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "a:df:gim:np:tvu:w")) != EOF) {
		switch(option) {
	        case 'a':
			udp_name = strdup(optarg);
			break;
		case 'd':
                        use_db = 1;
                        break;
		case 'f':
			name = strdup(optarg);
			break;
		case 'g':
			use_gpsd = 1;
			break;
		case 'm':
			millisecs = atoi(optarg);
			break;
		case 'i':	// read GPS from pipe on stdin
			use_pipe = 1;
			use_gpsd = 0;	// don't use socket
			break;
		case 'n':
			do_broadcast = 0;
			break;
		case 'p':
			gpsd_port = atoi(optarg);;
			break;
		case 't':
			do_timing = 1;	/// prints sys and user time
			break;
		case 'v':
			verbose = 1;
			break;
		case 'u':
			udp_port = atoi(optarg);
			break;
		case 'w':
			do_trace = 1;	/// traces raw GPS messages
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -a <dest IP (default broadcast)>\n");
			fprintf(stderr, " -f <device or gpsd IP address>\n");
			fprintf(stderr, " -g  (use gpsd) \n ");
			fprintf(stderr, " -i  (pipe NEMA into stdin) \n ");
			fprintf(stderr, " -n  (no broadcast)  \n");
			fprintf(stderr, " -p  <gpsd port number>  \n");
			fprintf(stderr, " -d  (use database) \n ");
			fprintf(stderr, " -t  (do timing) \n ");
			fprintf(stderr, " -v  (verbose) \n ");
			fprintf(stderr, " -u  (UDP port number for output) ");
			fprintf(stderr, " -w  (very verbose) \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
	printf("PATH GPS send app, dev name %s %s %s\n", name,
			use_gpsd?"using gpsd.":".",
			use_pipe?"using pipe":".");
	fflush(stdout);
	if (use_pipe) 
		fpin = stdin;
	else
		path_gps_get_fd(name, &fpin, use_gpsd, gpsd_port, &sd);
	if (udp_name == NULL) {
		do_broadcast = 1;
		udp_name = "255.255.255.255";
	}
	if (do_broadcast) { 	
		sd_out = udp_broadcast();
	} else 
		sd_out = udp_unicast();

	if (sd_out < 0) {
		printf("failure opening socket on %s %p\n",
				 udp_name, udp_port);
	}

        if (use_db){
                get_local_name(hostname, MAXHOSTNAMELEN);
                if (( pclt = db_list_init( argv[0], hostname, domain, xport,
                        db_vars, NUM_DB_VARS, NULL, 0)) == NULL ) {
                        printf("Database initialization error in %s\n", argv[0]);
                        exit(EXIT_FAILURE);
                }
        }

	if (do_timing)
		avcs_start_timing(&timing);

	if (setjmp(env) != 0) {
		if (do_timing) {
			printf("Exiting gpssnd\n");
			fflush(stdout);
			avcs_end_timing(&timing);
			avcs_print_timing(stdout, &timing);
			fflush(stdout);
		}
		if (use_db)
		      db_list_done(pclt, db_vars, NUM_DB_VARS, NULL, 0);
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	memset(&hb, 0, sizeof(hb));

	get_current_timestamp(&prev_ts);
	while (1) {
		/// Blocks until it gets a good GPS sentence
		timing.exec_num++;
#ifdef DO_TRACE
	printf("calling gps_get_sentence, sentence 0x%08x\n", sentence);
	fflush(stdout);
#endif
		path_gps_get_sentence(sentence, fpin, use_gpsd, sd); 
#ifdef DO_TRACE
	printf("calling gps_parse_sentence\n");
	fflush(stdout);
#endif
		path_gps_parse_sentence(sentence, &gps_data);
#ifdef DO_TRACE
	printf("calling path_gps_print_data (if do_trace)\n");
	fflush(stdout);
#endif
		if (do_trace) {
			path_gps_print_data(stdout, &gps_data);
			printf("\n");
		}
		if (strcmp(gps_data.gps_id, "GPGGA") == 0) {                    
                        gps_gga_typ *pgga = &gps_data.data.gga;                 
                        get_current_timestamp(&hb.local_time);
                        hb.utc_time = gpsutc2ts(pgga->utc_time);
                        hb.latitude = path_gps2degree(pgga->latitude);          
                        hb.longitude = path_gps2degree(pgga->longitude);        
                        hb.pos = pgga->pos;                                     
                        hb.num_sats = pgga->num_sats;
                        hb.hdop = pgga->hdop;
			db_num = DB_GPS_GGA_VAR;
		} else if (strcmp(gps_data.gps_id, "GPRMC") == 0) {
			gps_rmc_typ *prmc = &gps_data.data.rmc;
			get_current_timestamp(&hb.local_time);
			hb.utc_time = gpsutc2ts(prmc->utc_time);	
			hb.latitude = path_gps2degree(prmc->latitude); 
			hb.longitude = path_gps2degree(prmc->longitude); 
			hb.speed = path_knots2mps(prmc->speed_knots);
			hb.heading = prmc->true_track;
			if (first_msg) {	/// sync local date
				if (verbose)
					printf("date %d, time %.3f\n", 
					    prmc->date_of_fix, prmc->utc_time);
				clockset2gps(prmc->date_of_fix,prmc->utc_time);	
				first_msg = 0;
			}
			db_num = DB_GPS_RMC_VAR;
		} else if (strcmp(gps_data.gps_id, "GPVTG") == 0) { 
			gps_vtg_typ *pvtg = &gps_data.data.vtg;
			get_current_timestamp(&hb.local_time);
			hb.speed = path_knots2mps(pvtg->speed_knots);
			hb.heading = pvtg->true_track;
			db_num = DB_GPS_VTG_VAR;
		}	
		/// write to data server if enabled and GGA, VTG or RMC
		if ((use_db) && (db_num != 0)) {
			db_clt_write(pclt, db_num, sizeof(gps_data), &gps_data);
			db_num = 0;
		}

		set_inet_addr(&snd_addr, inet_addr(udp_name), udp_port);
		if  (strcmp(gps_data.gps_id, "GPRMC") == 0) {
			get_current_timestamp(&current_ts);
			if ((TS_TO_MS(&current_ts) - TS_TO_MS(&prev_ts))
				>= millisecs) {
				prev_ts = current_ts;
				hb.sequence_no++;
				bytes_sent = sendto(sd_out, &hb, sizeof(hb), 0,
					(struct sockaddr *) &snd_addr,
					sizeof(snd_addr));
				if (use_db) {
					db_clt_write(pclt, DB_GPS_PT_LCL_VAR,
						sizeof(path_gps_point_t), &hb);
				}
				if (verbose) {
					print_timestamp(stdout, &current_ts);
					printf(" ");
					path_gps_print_point(stdout, &hb);
					printf("\n");
				}
			}
		}
			
	}
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
