/**\file
 *	trk_comm_mgr.c 
 *
 *	Code for experimenting with platoon communication.	
 *	Wrties VEH_COMM_TX variable based on local GPS and
 *	other communication packets received.
 *
 * Copyright (c) 2008   Regents of the University of California
 *
 */
#include <sys_os.h>
#include <db_clt.h>
#include <db_utils.h>
#include <timestamp.h>
#include <local.h>
#include <sys_rt.h>
#include <sys_ini.h>
#include "path_gps_lib.h"
#include "long_comm.h"
#include "udp_utils.h"

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	(-1)
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
        if (code == SIGALRM)
                return;
        else
                longjmp(exit_env, code);
}

static db_id_t db_vars_list[] = {
        {DB_COMM_LEAD_VAR, sizeof(veh_comm_packet_t)},
        {DB_COMM_PREC_VAR, sizeof(veh_comm_packet_t)},
        {DB_COMM_TX_VAR, sizeof(veh_comm_packet_t)},
	{DB_GPS_PT_LCL_VAR, sizeof(path_gps_point_t)}
};

int num_db_vars = sizeof(db_vars_list)/sizeof(db_id_t);

int main(int argc, char *argv[])
{
	int ch;		
        db_clt_typ *pclt;  		/// data bucket pointer	
        char *domain=DEFAULT_SERVICE;
        char hostname[MAXHOSTNAMELEN+1];
        int xport = COMM_OS_XPORT;
	int sd;				/// socket descriptor
	veh_comm_packet_t neighbors[MAX_PLATOON_SIZE];
	path_gps_point_t self_gps;
	int verbose = 0;
	short msg_count = 0;
	char *vehicle_str = "Blue";
	posix_timer_typ *ptmr;
	int interval = 20;	/// milliseconds

        while ((ch = getopt(argc, argv, "i:t:v")) != EOF) {
                switch (ch) {
		case 'i': interval = atoi(optarg);
			  break;
		case 't': vehicle_str = strdup(optarg);
			  break;
		case 'v': verbose = 1; 
			  break;
                default:  printf("Usage: %s [-v (verbose)]", argv[0]);
			  exit(EXIT_FAILURE);
                          break;
                }
        }
       if ((ptmr = timer_init(interval, ChannelCreate(0))) == NULL) {
                printf("timer_init failed\n");
                exit(EXIT_FAILURE);
        }

        get_local_name(hostname, MAXHOSTNAMELEN);

	/** Log in to data server and create all the DB_COMM variablse
	 */
	pclt = db_list_init(argv[0], hostname, domain, xport, db_vars_list,
			 num_db_vars, NULL, 0); 

	for (i = 0; i < MAX_PLATOON_SIZE; i++) {
		int db_num = DB_COMM_BASE_VAR+i;
		clt_create(pclt, db_num, db_num, sizeof(veh_comm_packet_t);
	}
	for (i = 0; i < MAX_PLATOON_SIZE; i++) {
		int db_num = DB_COMM_BASE_VAR+i;
		veh_comm_packet_t tmp;
		tmp.my_pip = -1;
		db_clt_update(pclt, db_num, sizeof(veh_comm_packet_t), &tmp);
	}

	if (setjmp(exit_env) != 0) {
		printf("Sent %d messages\n", msg_count);
		for (i = 0; i < MAX_PLATOON_SIZE; i++) {
			clt_destroy(pclt, DB_COMM_BASE_VAR+i);
		}
		db_list_done(pclt, db_vars_list, num_db_vars, NULL, 0);		
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

	while (1) {
		db_clt_read(pclt, DB_GPS_PT_LCL_VAR, 
			sizeof(path_gps_point_t), &self_gps);
		for (i = 0; i < MAX_PLATOON_SIZE; i++) {
			int db_num = DB_COMM_BASE_VAR+i;
			veh_comm_packet_t tmp;
			// cwh_exc muar dill these DB variables with
			// entries with unique vehicle strings, caching
			// the strings
			db_clt_read(pclt, db_num, sizeof(veh_comm_packet_t), 
				&tmp);
			if (tmp.my_pip > 0) 
				insert_if_neighbor(&tmp, &neighbors[0]);
			// insert_if_neighbor checks vehicle string
			// for uniqueness and closeness if entire
			// array is filled with entries with recent
			// timestamp
		}
		
		
		TIMER_WAIT(ptmr);
	}
	longjmp(exit_env,1);	/* go to exit code when loop terminates */
}
