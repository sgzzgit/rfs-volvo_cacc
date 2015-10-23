/**\file *
 * gpsdemo.c      demo code for vehicle to vehicle demo
 *
 *		For now expects path_gps_point_t as body of message. 
 *		and writes only most recent point to data server.
 *		Need to add discrimination on the basis of IP address
 *		to store in different data server variables. 
 *
 *		Records local timestamp when received as well as 
 *		UTC time and local timestamp when sent from the message.	
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
#include <timestamp.h>
#include <timing.h>
#include <udp_utils.h>
#include <db_clt.h>
#include <db_utils.h>
#include <math.h>
#include <stdio.h>
#include "path_gps_lib.h"

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

/* Pre-create 8 receive targets for now
 */
static db_id_t db_vars[] = {
        {DB_GPS_PT_REF_VAR, sizeof(path_gps_pt_ref_t)},
        {DB_GPS_PT_RCV_VAR, sizeof(path_gps_point_t)},
        {DB_GPS_PT_RCV_VAR+1, sizeof(path_gps_point_t)},
        {DB_GPS_PT_RCV_VAR+2, sizeof(path_gps_point_t)},
        {DB_GPS_PT_RCV_VAR+3, sizeof(path_gps_point_t)},
        {DB_GPS_PT_RCV_VAR+4, sizeof(path_gps_point_t)},
        {DB_GPS_PT_RCV_VAR+5, sizeof(path_gps_point_t)},
        {DB_GPS_PT_RCV_VAR+6, sizeof(path_gps_point_t)},
        {DB_GPS_PT_RCV_VAR+7, sizeof(path_gps_point_t)},
};

double d(double lat1, double lon1, double lat2, double lon2) {
	double dlon;
	double dlat;
	double rad = 0.017453293;
	double a;
	double R = 6367000.0;
	double c;
	double dret;
	printf("lat1 = %f, lon1 = %f, lat2 = %f, lon2 = %f \n", lat1, lon1, lat2, lon2);
	lat1 = lat1 * rad;
	lon1 = lon1 * rad;
	lat2 = lat2 * rad;
	lon2 = lon2 * rad;
	dlon = lon2 - lon1;
	dlat = lat2 - lat1;
	a = (sin(dlat/2)*sin(dlat/2)) + (cos(lat1) * cos(lat2) * (sin(dlon/2)*sin(dlon/2)));
	c = 2 * atan2(sqrt(a), sqrt(1-a));
        dret = R * c;
        return dret;
}

#define NUM_DB_VARS (sizeof(db_vars)/sizeof(db_id_t))

int main(int argc, char *argv[])
{
	int status;
        char sentence[MAX_GPS_SENTENCE];
        gps_data_typ gps_data, other_data;
	double speed_diff;
	double distance_diff;
	double lon1, lat1, lon2, lat2, speed1, speed2, heading1, heading2;
	int num_sats, num_sats2;
	
	FILE *fpout;		/// file pointer to write output 
	FILE *f;
	int old_fileday = 99;	/// invalid day value for initialiation
	int file_serialno = 0;	/// start numbering files at 0
	char suffix[80];	/// file extension for data logging 
	char prefix[80];	/// identifier for files logged 
	double start_time;	/// when last file was opened
	int file_time = 5;	/// minutes file duration 
	int use_file = 0;	/// if 1, write files
	char mygps[MAX_GPS_SENTENCE];
        char othergps[MAX_GPS_SENTENCE];
	char hostname[MAXHOSTNAMELEN];
	char *domain = DEFAULT_SERVICE;	/// on Linux sets DB q-file directory
        db_clt_typ *pclt;       /// data server client pointer
        int xport = COMM_PSX_XPORT;	/// Linux uses Posix message queues
	int use_db = 0;		/// set to 1 to use data server
	int db_num = 0;		/// must be set to non-zero for DB update.

	int verbose = 0;	/// if 1, print extra info for debugging	
	int do_timing = 0;	/// if 1, print users and sys time at end 
	struct avcs_timing timing;	
	int udp_port = 7015;	/// port for receiving heartbeat
	int option;
        int pid = 0;
	path_gps_point_t hb, hb2;	/// fill in from GPS messages received

	path_gps_pt_ref_t pt_ref;	/// maintained by this program

	strncpy(suffix, ".dat", 80);
	strncpy(prefix, "/big/data", 80);
	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "df:vtu:w")) != EOF) {
		switch(option) {
		case 'd':
                        use_db = 1;
                        break;
		case 'f':
			strncpy(prefix, optarg, 80);
			break;
		case 't':
			do_timing = 1;	// call timing utilities	
			break;
		case 'v':
			verbose = 1;	//
			break;
		case 'u':
			udp_port = atoi(optarg);
			printf("setting udp port to %d \n", udp_port);
			break;
		case 'w':
			use_file = 1; 
			break;

		default:
			fprintf(stderr, "Usage %s: ", argv[0]); 
			fprintf(stderr, " -u  (UDP port number for input) ");
			fprintf(stderr, " -d  write data server ");
			fprintf(stderr, " -v  (verbose, info to stdout) ");
			fprintf(stderr, " -w  write log files ");
			fprintf(stderr, " -f <prefix for log file>");
			fprintf(stderr, "\n");
			fprintf(stderr, " Need -w,-d, or -v!\n ");
			exit(EXIT_FAILURE);
		}
	}

	printf("PATH GPS demo app\n"); 


	if (setjmp(env) != 0) {
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	memset(&hb, 0, sizeof(hb));
        pid = fork();
        if (pid == 0) {       
	int sd_in;		/// socket descriptor for UDP send
	int bytes_rcvd;		/// returned from recvfrom
	struct sockaddr_in src_addr;	/// used in recvfrom call
	unsigned int socklen;

	sd_in = udp_allow_all(udp_port);
	if (sd_in < 0) {
		printf("failure opening socket on %d\n", udp_port);
		exit(EXIT_FAILURE);
	}
          	socklen = sizeof(src_addr);
	memset(&src_addr, 0, socklen);
memset(&hb2, 0, sizeof(hb));
                printf("child started on port %d\n", udp_port);
	while (1) {
		timestamp_t ts;
		timing.exec_num++;
		bytes_rcvd = recvfrom(sd_in, &hb2, sizeof(hb), 0,
				(struct sockaddr *) &src_addr, &socklen);
		if (bytes_rcvd < 0) {
			perror("recvfrom failed\n");
			continue;
		}
		lon2 = hb2.longitude;
		lat2 = hb2.latitude;
		speed2 = hb2.speed;
		f = fopen("gps1", "r");
		memset(&lat1, 0, sizeof(double));
		memset(&lon1, 0, sizeof(double));
		memset(&speed1, 0, sizeof(double));
		memset(&num_sats2, 0, sizeof(int));
		fscanf(f, "%lf %lf %lf %d", &lat1, &lon1, &speed1, &num_sats2);
		fclose(f);

		distance_diff = d(lat1, lon1, lat2, lon2); //meters
		speed_diff = speed2-speed1; //kph
		printf("speed_diff = %f,num_sats = %d \n", speed_diff, num_sats2);
		if ( speed1 > 8 && speed2 < 2.0 && distance_diff < 45 && speed2 != 0.0) {
			printf("stopped car warning\n");
			printf("speed1 = %f, speed2 = %f, sats = %d, d = %f \n", speed1, speed2, num_sats2, distance_diff);
			system("/usr/bin/sox -q /root/gpsdemo/stopped.wav -t ossdsp /dev/audio");
			sleep(15);
                } else if ( speed1 > 12 && speed2 < 2.5  && distance_diff < 65 && speed2 != 0.0) {
			printf("stopped car warning\n");
			printf("speed1 = %f, speed2 = %f, sats = %d, d = %f \n", speed1, speed2, num_sats2, distance_diff);
			system("/usr/bin/sox -q /root/gpsdemo/stopped.wav -t ossdsp /dev/audio");
			sleep(15);
                }
 
       printf("distance = %f m, speed1 = %f kph, speed2 = %f kph\n",
				distance_diff, speed1, speed2); 
	}
        } else {
                printf("parent started\n");
        while (1) {
                // Blocks until it gets a good GPS sentence
                path_gps_get_sentence(sentence, stdin, 0, 0); 
                path_gps_parse_sentence(sentence, &gps_data);
                if (strcmp(gps_data.gps_id, "GPGGA") == 0) {                
    
                        gps_gga_typ *pgga = &gps_data.data.gga;             
    
                        get_current_timestamp(&hb.local_time);
                        hb.utc_time = gpsutc2ts(pgga->utc_time);
                        hb.latitude = path_gps2degree(pgga->latitude);      
    
                        hb.longitude = path_gps2degree(pgga->longitude);    
    
                        hb.pos = pgga->pos;                                 
    
                        hb.num_sats = pgga->num_sats;
		        lat1 = hb.latitude;
		        lon1 = hb.longitude;
			num_sats = hb.num_sats;
                } else if (strcmp(gps_data.gps_id, "GPRMC") == 0) {
                        gps_rmc_typ *prmc = &gps_data.data.rmc;
                        hb.utc_time = gpsutc2ts(prmc->utc_time);
                        hb.latitude = path_gps2degree(prmc->latitude); 
                        hb.longitude = path_gps2degree(prmc->longitude); 
                        hb.speed = path_knots2mps(prmc->speed_knots);
                        hb.heading = prmc->true_track;
		        speed1 = hb.speed;
		        lat1 = hb.latitude;
		        lon1 = hb.longitude;
                } else if (strcmp(gps_data.gps_id, "GPVTG") == 0) { 
                        gps_vtg_typ *pvtg = &gps_data.data.vtg;
                        hb.speed = path_knots2mps(pvtg->speed_knots);
                        hb.heading = pvtg->true_track;
		        speed1 = hb.speed;
                } else {
			continue;
		}
		if (num_sats < 3) continue;
		f = fopen("gps1", "w");
		fprintf(f, "%lf %lf %lf %d", lat1, lon1, speed1, num_sats);
		fclose(f);
		//printf("lat1 = %f, lon1 = %f, speed1= %f m/s\n", lat1, lon1, speed1);
        }

        }

}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
