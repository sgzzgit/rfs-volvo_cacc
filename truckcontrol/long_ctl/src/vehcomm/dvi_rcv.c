/**\fil
 *	veh_rcv.c 
 *		Receives a message from another vehicle and
 *		writes it to the appropriate database variable,
 *		depending on vehicle ID in the message. 
 *
 *		On initialization reads a config file that specifies
 *		the current vehicle ID, the lead vehicle ID,
 *		and the preceding vehicle ID.
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
#include <udp_utils.h>
#include "dvi.h"

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
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
        {DB_DVI_RCV_VAR, sizeof(char)},
};

int main( int argc, char *argv[] )
{
	int ch;		
        db_clt_typ *pclt;  		/// data bucket pointer	
        char *domain=DEFAULT_SERVICE;
        char hostname[MAXHOSTNAMELEN+1];
        int xport = COMM_OS_XPORT;
        struct sockaddr_in src_addr;
        char *remote_ipaddr = "172.16.0.177";       /// address of UDP destination
        char *local_ipaddr = "172.16.0.77";       /// address of UDP destination
        struct sockaddr_in dst_addr;
	int create_db_vars = 0;
	int use_db = 1;
	dvi_out_t dvi_out;
	float ts = 0;
	float ts_sav = 0;
	char gap_level = 5;

	int sd;				/// socket descriptor
	int udp_port = 8003;

        char buf;

        int bytes_received;     // received from a call to recv
	int verbose = 0;
	short msg_count = 0;
	int socklen = sizeof(src_addr);

        while ((ch = getopt(argc, argv, "A:a:cdu:v")) != EOF) {
                switch (ch) {
                case 'A': local_ipaddr = strdup(optarg);
                          break;
                case 'a': remote_ipaddr= strdup(optarg);
                          break;
		case 'c': create_db_vars = 1; 
			  break;
		case 'd': use_db = 0; 
			  break;
		case 'u': udp_port = atoi(optarg); 
			  break;
		case 'v': verbose = 1; 
			  break;
                default:  printf( "Usage: %s -v (verbose) -A <local ip, def. 172.16.0.77> -a <remote ip, def. 172.16.0.177> -u <UDP port, def. 8003> -d (Do NOT use database)", argv[0]);
			  exit(EXIT_FAILURE);
                          break;
                }
        }
        get_local_name(hostname, MAXHOSTNAMELEN);

	if(use_db != 0) {
		if(create_db_vars)
			pclt = db_list_init(argv[0], hostname, domain, xport, db_vars_list, 1, NULL,  0); 
		else
			pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0); 
	}

	if( setjmp( exit_env ) != 0 ) {
		printf("Received %d messages\n", msg_count);
		if(create_db_vars)
			db_list_done(pclt, db_vars_list, 1, NULL, 0);		
		else
			db_list_done(pclt, NULL, 0, NULL, 0);		
		exit( EXIT_SUCCESS );
	} else
		sig_ign( sig_list, sig_hand );

//	if ( (sd = udp_peer2peer_init(&dst_addr, remote_ipaddr, local_ipaddr, udp_port, 0)) < 0) {
	if ( (sd = udp_allow_all(udp_port)) < 0) {
		printf("Failure to initialize socket from %s to %s on port %d\n",
			remote_ipaddr, local_ipaddr, udp_port);
		longjmp(exit_env, 2);
	}


	while (1) {
                if ((bytes_received = recvfrom(sd, &buf, 1, 0, (struct sockaddr *) &src_addr, (socklen_t *) &socklen)) <= 0) {
                        perror("recvfrom failed\n");
                        break;
                }
#define	ACC_REQUESTED		5
#define	CACC_REQUESTED		6
#define	GAP_DECREASE_REQUESTED	7
#define	GAP_INCREASE_REQUESTED	8
		ts = sec_past_midnight_float();
		if(use_db != 0) {
			switch(buf){
				case ACC_REQUESTED:
					dvi_out.acc_cacc_request = 1;
					dvi_out.gap_request = 0;
					break;
				case CACC_REQUESTED:
					dvi_out.acc_cacc_request = 2;
					dvi_out.gap_request = 0;
					break;
				case GAP_DECREASE_REQUESTED:
					if( (ts - ts_sav) > 0.2)
						if( (gap_level--) < 0)
							gap_level = 0;
					ts_sav = ts;
					dvi_out.acc_cacc_request = 0;
					dvi_out.gap_request = gap_level;
					break;
				case GAP_INCREASE_REQUESTED:
					if( (ts - ts_sav) > 0.2)
						if( (gap_level++) > 5)
							gap_level = 5;
					ts_sav = ts;
					dvi_out.acc_cacc_request = 0;
					dvi_out.gap_request = gap_level;
					break;
			}
			db_clt_write(pclt, DB_DVI_OUT_VAR, sizeof(dvi_out_t), &dvi_out);
		}
		if(buf != 0) 
		{
			printf("Byte received: %hhu at %f sec\n", buf, ts);
		}
	}
	longjmp(exit_env,1);	/* go to exit code when loop terminates */
}
