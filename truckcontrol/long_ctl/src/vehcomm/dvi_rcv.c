/**\file
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
        char *remote_ipaddr = "172.16.0.1";       /// address of UDP destination
        char *local_ipaddr = "172.16.0.75";       /// address of UDP destination
        struct sockaddr_in dst_addr;
	int create_db_vars = 0;

	int sd;				/// socket descriptor
	int udp_port = 8003;

        char buf;

        int bytes_received;     // received from a call to recv
	int verbose = 0;
	short msg_count = 0;
	int socklen = sizeof(src_addr);

        while ((ch = getopt(argc, argv, "A:a:cu:v")) != EOF) {
                switch (ch) {
                case 'A': local_ipaddr = strdup(optarg);
                          break;
                case 'a': remote_ipaddr= strdup(optarg);
                          break;
		case 'c': create_db_vars = 1; 
			  break;
		case 'u': udp_port = atoi(optarg); 
			  break;
		case 'v': verbose = 1; 
			  break;
                default:  printf( "Usage: %s -v (verbose) -A <local ip, def. 172.16.0.75> -a <remote ip, def. 172.16.0.1> -u <UDP port, def. 8003> ", argv[0]);
			  exit(EXIT_FAILURE);
                          break;
                }
        }
        get_local_name(hostname, MAXHOSTNAMELEN);

        if(create_db_vars)
                pclt = db_list_init(argv[0], hostname, domain, xport, db_vars_list, 1, NULL,  0); 
        else
                pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0); 

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

		db_clt_write(pclt, DB_DVI_RCV_VAR, sizeof(char), &buf);
		if(buf != 0) 
		{
			printf("Byte received %hhu\n", buf);
		}
	}
	longjmp(exit_env,1);	/* go to exit code when loop terminates */
}
