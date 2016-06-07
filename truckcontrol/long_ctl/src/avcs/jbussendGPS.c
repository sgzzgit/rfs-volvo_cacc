/*
 * jbussendGPS.c 	Process to send custom CAN messages to Volvo CAN2
 *
 * Copyright (c) 2008   Regents of the University of California
 *
 */
#include <enu2lla.h>
#include "can_defs.h"
#include "can_client.h"
#include <sys_os.h>
#include <local.h>
#include <sys_list.h>
#include <sys/select.h>
#include <sys_lib.h>
#include <sys_rt.h>
#include <sys_ini.h>
#include "db_comm.h"
#include "db_clt.h"
#include "db_utils.h"
#include "timestamp.h"
#include "jbus_vars.h"
#include "jbus_extended.h"
#include "j1939.h"
#include "j1939pdu_extended.h"
#include "j1939db.h"
#include "clt_vars.h"
#include "veh_trk.h"
#include <timing.h>
#include "db_utils.h"
#include <path_gps_lib.h>
#include "jbussendGPS.h"


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

int update_gps_position(path_gps_point_t *hb, unsigned char *buf) {
	int i;
	unsigned int latitude;
	unsigned int longitude;
	unsigned short checksum = 0;

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
	int i;
	int year = 2015;
	char month = 3;
	short day = 26;
	
	year = (hb->date / 10000) + 2000;
	month = (hb->date / 100) % 100;
	day = hb->date % 100;
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
	buf[7] |= id & 0xFF;

	printf("GPS time msg: ");
	for(i=0; i<8; i++)
		printf("%hhx ", buf[i]);
	printf("%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%f %d",
		month,
		day,
		year,
		hb->utc_time.hour, 
		hb->utc_time.min, 
		hb->utc_time.sec + hb->utc_time.sec/1000.0,
		hb->date 
	);
	printf("\n");
	
	return 0;
};

int main( int argc, char *argv[] )
{
	int ch;		
        db_clt_typ *pclt = NULL;  	/* Database pointer */
	posix_timer_typ *ptimer;      	/* Timing proxy */
	int interval = 50; /* Main loop execution interval */
	char *gps_file = NULL;
	int gps_fd = -1;
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
	can_debug_t engine_debug;
	char *engine_debug_buf = (char *)&engine_debug;
	can_debug_t engine_retarder_debug;
	char *engine_retarder_debug_buf = (char *)&engine_retarder_debug;
	int i;

	get_local_name(hostname, MAXHOSTNAMELEN);

        while ((ch = getopt(argc, argv, "b:cde:t:v:n:g:")) != EOF) {
                switch (ch) {
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
		db_clt_read(pclt, DB_ENGINE_DEBUG_VAR, sizeof(can_debug_t), &engine_debug);
		db_clt_read(pclt, DB_ENGINE_RETARDER_DEBUG_VAR, sizeof(can_debug_t), &engine_retarder_debug);
		if(use_can) {
			update_gps_position(&hb74, buf74);
			printf("lat %f long %f\n", hb74.latitude, hb74.longitude);
			can_write(gps_fd, 0x10, 0, buf74, 12);
			update_gps_time(&hb74, buf74, 74);
			can_write(gps_fd, 0x13, 0, buf74, 8);

			update_gps_position(&hb75, buf75);
			printf("lat %f long %f\n", hb75.latitude, hb75.longitude);
			can_write(gps_fd, 0x11, 0, buf75, 12);
			update_gps_time(&hb75, buf75, 75);
			can_write(gps_fd, 0x14, 0, buf75, 8);

			update_gps_position(&hb76, buf76);
			printf("lat %f long %f\n", hb76.latitude, hb76.longitude);
			can_write(gps_fd, 0x12, 0, buf76, 12);
			update_gps_time(&hb76, buf76, 76);
			can_write(gps_fd, 0x15, 0, buf76, 8);
			can_write(gps_fd, 0x16, 0, &engine_debug, 7);
			can_write(gps_fd, 0x17, 0, &engine_retarder_debug, 7);
			printf("Engine_debug: ");
			for(i=0; i<sizeof(can_debug_t); i++)
				printf("%#hhx ", engine_debug_buf[i]);
				printf("\n");
			printf("Engine_retarder_debug: ");
			for(i=0; i<sizeof(can_debug_t); i++)
				printf("%#hhx ", engine_retarder_debug_buf[i]);
				printf("\n");
		}
		/* Now wait for proxy from timer */
		TIMER_WAIT( ptimer );
	}
}
