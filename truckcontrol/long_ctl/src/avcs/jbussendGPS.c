/*
 * jbussend.c 	Process to send commands, including heartbeat
 *		commands, to all j1939 buses in the vehicle.	
 *
 *		Updated for QNX6
 *
 * Copyright (c) 2008   Regents of the University of California
 *
 */
#include "jbussendGPS.h"
#include "path_gps_lib.h"
#include <enu2lla.h>
#include "can_defs.h"
#include "can_client.h"
#include "sys_os.h"
#include "db_utils.h"

#define DB_VOLVO_GPS_CAN_TYPE	2345
#define DB_VOLVO_GPS_CAN_VAR	DB_VOLVO_GPS_CAN_TYPE

extern void print_long_output_typ (long_output_typ *ctrl);
static struct avcs_timing tmg;

int debug = 0;

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else
		longjmp(exit_env, code);
}

#define MAX_LOOP_NUMBER 8

typedef struct {
	unsigned int sof: 1; //Start-of-frame = 0
	unsigned int gps_position_pdu: 11; // =0x0010, 0x0011, 0x0012
	unsigned int rtr: 1 ; //Remote transmission request=0
	unsigned int extension: 1; //Extension bit=0
	unsigned int reserved: 1; //Reserved bit=0
	unsigned int dlc: 4; 	//Data length code=8
	unsigned int latitude : 31; 	//Max=124.748, Min=-90.000, resolution=1E-07, offset=-90.000
	unsigned int pos_data_valid: 1;
	unsigned int longitude : 32; ; 	//Max=249.497, Min=-180.000, resolution=1E-07, offset=-180.000
	unsigned int crc: 15; 		//Cyclic redundancy check. Calculated
	unsigned int crcd: 1;		//CRC delimiter=1
	unsigned int ack_slot: 1;	//Acknowledge slot=1
	unsigned int ack_delimiter: 1;	//Acknowledge delimiter=1
	unsigned int eof:7;		//End-of-frame=0x07
} IS_PACKED gps_position_t;

typedef struct {
	unsigned short gps_time_pdu; // =0x0013, 0x0014, 0x0015
	unsigned int year :12; 	//Max=4095, Min=0, resolution=1, offset=0
	unsigned int month :4; 	//Max=15, Min=0, resolution=1, offset=0
	unsigned int day :9; 	//Max=511, Min=0, resolution=1, offset=0
	unsigned int hour :5; 	//Max=31, Min=0, resolution=1, offset=0
	unsigned int minute :6; //Max=63, Min=0, resolution=1, offset=0
	unsigned int second :6; //Max=63, resolution=1, offset=0
	unsigned int split_second :10, :12; 	//Max=102.3, Min=0, resolution=0.1, offset=0
	unsigned int id :8; 	//74, 75, or 76
} IS_PACKED gps_time_t;

unsigned long LAT_MULTIPLIER=10E7;
unsigned long LAT_MIN	=0;
unsigned long LAT_MAX	=(unsigned int)0x7FFFFFFF;
unsigned long LAT_OFFSET=-90;

unsigned long LONGITUDE_MULTIPLIER=10E7;
unsigned long LONGITUDE_MIN=0;
unsigned long LONGITUDE_MAX=(unsigned int)0xFFFFFFFF;
unsigned long LONGITUDE_OFFSET=-180;

unsigned long POS_DATA_IS_VALID=0x10000000;
unsigned long POS_DATA_INVALID=0x7FFFFFFF;

int update_gps_position(path_gps_point_t *hb, unsigned char *buf) {
//	int retval;
	int i;
	unsigned int latitude;
	unsigned int longitude;
	unsigned short checksum = 0;

//	hb->latitude = 37.914773;
//	hb->longitude = -122.334691;
printf("update_gps_position: hb.latitude %.7f hb.longitude %.7f\n", hb->latitude, hb->longitude);
	latitude = (unsigned int)( 10000000 * (hb->latitude + 90.0));
	longitude = (unsigned int)( 10000000 * (hb->longitude + 180.0));
	if( (latitude >= LAT_MIN) &&
	    (latitude >= LAT_MAX) &&
	    (longitude <= LONGITUDE_MIN) &&
	    (longitude >= LONGITUDE_MAX) )
		latitude |= POS_DATA_IS_VALID;
	else
		latitude &= POS_DATA_INVALID;
//printf("latitude %u %#X longitude %u %#X\n", latitude, latitude, longitude, longitude);

	buf[0] = latitude & 0xFF;
	buf[1] = (latitude >> 8) & 0xFF;
	buf[2] = (latitude >> 16) & 0xFF;
	buf[3] = (latitude >> 24) & 0xFF;
	buf[4] = longitude & 0xFF;
	buf[5] = (longitude >> 8) & 0xFF;
	buf[6] = (longitude >> 16) & 0xFF;
	buf[7] = (longitude >> 24) & 0xFF;

	checksum = 0;
	for(i=0; i<8;i++) 
		checksum += buf[i];
	checksum &= 0x7FFF;
	buf[8] = (checksum >> 8) & 0x7;
	buf[9] = checksum & 0xFF;
	buf[10] = 0xBF;
	buf[11] = 0xFF;

	printf("GPS Position msg: ");
	for(i=0; i<12; i++)
		printf("%hhx ", buf[i]);
	printf("\n");

	return 0;
};

int update_gps_time(path_gps_point_t *hb, unsigned char *buf, unsigned char id) {
//	int retval;
	int year = 2015;
	char month = 3;
	short day = 26;
	

	buf[0] = year & 0xFF;
	buf[1] = (year >> 8) & 0x0F;
	buf[1] |= (month << 4) & 0xF0;
	buf[2] = day & 0xFF;
	buf[3] = (day >> 8) & 0x01;
	buf[3] |= (hb->utc_time.hour << 1) & 0x3E;
	buf[3] |= (hb->utc_time.min << 6) & 0xC0;
	buf[4] = (hb->utc_time.min >> 2) & 0x0F;
	buf[4] |= (hb->utc_time.sec << 4) & 0xF0;
	buf[5] = (hb->utc_time.sec >> 4) & 0x03;
	buf[5] |= (hb->utc_time.millisec/100 << 2) & 0xFC;
	buf[6] |= id & 0xFF;
	
	return 0;
};

int main( int argc, char *argv[] )
{
	int ch;		
        db_clt_typ *pclt = NULL;  	/* Database pointer */
	posix_timer_typ *ptimer;      	/* Timing proxy */
	int interval = JBUS_INTERVAL_MSECS; /* Main loop execution interval */
	char *gps_file = NULL;
	int gps_fd = -1;
	int active_mask = (1 << NUM_JBUS_SEND) - 1;	/* all active */  
	int rtn_jmp = -1;	/* value returned from setjmp */
	char hostname[MAXHOSTNAMELEN+1]; /* used in opening database */
        int xport = COMM_OS_XPORT;
	char *domain = DEFAULT_SERVICE;
	char *vehicle_str = "VLN475";	/* vehicle identifier */
	char use_can = 0;
	path_gps_point_t hb74;
	path_gps_point_t hb75;
	path_gps_point_t hb76;
//	gps_position_t gps_position;
//	gps_time_t gps_time;
	int gps_db_num = DB_GPS_PT_LCL_VAR;
	unsigned char buf74[17];
	unsigned char buf75[17];
	unsigned char buf76[17];

	get_local_name(hostname, MAXHOSTNAMELEN);

        while ((ch = getopt(argc, argv, "a:b:cde:t:v:n:g:")) != EOF) {
                switch (ch) {
		case 'a': active_mask = atoi(optarg);
			  break;	
		case 'c': use_can = 1;
			  break;
		case 'd': debug = 1;
			  break;
		case 'g': gps_file = strdup(optarg);
			  break;
		case 'v': vehicle_str = strdup(optarg);
			  break;
		case 'n': gps_db_num = atoi(optarg);
			  break;	
                default:  printf( "Usage: %s ", argv[0]);
                	  printf("-c(use CAN) -d(debug) \n");
                	  printf("-n(db number) -g(/dev/can1) \n");
			  exit(EXIT_FAILURE);
                          break;
                }
        }

        pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0);

	if(use_can) {
		gps_fd = can_open(gps_file, O_WRONLY);
		if(gps_fd < 0) {
			printf("Failed to connect to %s. Exiting....\n", gps_file);
			exit(EXIT_FAILURE);
		}
		else
			printf("Connection to %s successful!\n", gps_file);
	}

	if ((ptimer = timer_init( interval, 0 )) == NULL) {
		printf("Unable to initialize jbussend timer\n");
		exit (EXIT_FAILURE);
	}
	avcs_start_timing(&tmg);

	if( (rtn_jmp = setjmp( exit_env )) != 0 ) {
		avcs_end_timing(&tmg);
                close_local_database(pclt);

		avcs_print_timing(stdout, &tmg);
		fprintf(stderr, "jbussend exit %d\n", rtn_jmp);
		fflush(stderr);

		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	if (debug) {
		printf("Begin loop, interval %d\n", interval);
		fflush(stdout);
	}

	for ( ; ; ) {

		db_clt_read(pclt, gps_db_num, sizeof(path_gps_point_t), &hb74);
		db_clt_read(pclt, gps_db_num, sizeof(path_gps_point_t), &hb75);
		db_clt_read(pclt, gps_db_num, sizeof(path_gps_point_t), &hb76);
		if(use_can) {
			update_gps_position(&hb74, buf74);
			printf("lat %f long %f\n", hb74.latitude, hb74.longitude);
			can_write(gps_fd, 0x10, 0, buf74, 12);
			update_gps_time(&hb74, buf74, 74);
			can_write(gps_fd, 0x13, 0, buf74, 7);

			update_gps_position(&hb75, buf75);
			printf("lat %f long %f\n", hb75.latitude, hb75.longitude);
			can_write(gps_fd, 0x11, 0, buf75, 12);
			update_gps_time(&hb75, buf75, 75);
			can_write(gps_fd, 0x14, 0, buf75, 7);

			update_gps_position(&hb76, buf76);
			printf("lat %f long %f\n", hb76.latitude, hb76.longitude);
			can_write(gps_fd, 0x12, 0, buf76, 12);
			update_gps_time(&hb76, buf76, 76);
			can_write(gps_fd, 0x13, 0, buf76, 7);
		}
		/* Now wait for proxy from timer */
		TIMER_WAIT( ptimer );
	}
}
