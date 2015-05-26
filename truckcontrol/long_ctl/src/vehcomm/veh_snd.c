/**\file
 *	veh_snd.c 
 *		Sends a message to another vehicle, filling in
 *		the fields by reading the appropriate database variable.
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

/** Open UDP socket for sending to a unicast or broadcast IPv4 address
 *  and initialize sock_addrin structure and set sock_opt if necessary
 */
static int udp_init(char *ipaddr, short port, struct sockaddr_in *paddr,
		int do_broadcast)
{
        int sockfd;

	if (do_broadcast)
		sockfd = udp_broadcast();
	else
		sockfd = udp_unicast();

        memset(paddr, 0, sizeof(struct sockaddr_in));
        paddr->sin_family = AF_INET;       // host byte order
        paddr->sin_port = htons(port);     // short, network byte order
        paddr->sin_addr.s_addr = inet_addr(ipaddr);
        memset(&(paddr->sin_zero), '\0', 8); // zero the rest of the struct

        return sockfd;
}

/* Set the vehicle string as the object identifier in the
 * GPS point structure within the packet being sent.
 */ 
void set_vehicle_string(veh_comm_packet_t *pvcp, char * vehicle_str)
{
	/** GPS_OBJECT_ID_SIZE is only 6, "Blue" and "Gold" will
	 *  work, "Silver" will be shortened 
	 */
	strncpy(&pvcp->object_id[0], vehicle_str, GPS_OBJECT_ID_SIZE);
	
	/// make sure string is terminated
	pvcp->object_id[GPS_OBJECT_ID_SIZE - 1] = '\0';
}


int main(int argc, char *argv[])
{
	int ch;		
        db_clt_typ *pclt;  		/// data bucket pointer	
        char *domain=DEFAULT_SERVICE;
        char hostname[MAXHOSTNAMELEN+1];
        int xport = COMM_OS_XPORT;
	int sd;				/// socket descriptor
	int udp_port;
	veh_comm_packet_t comm_pkt;
        int bytes_sent;     		/// received from a call to sendto
	int verbose = 0;
	short msg_count = 0;
	char *ipaddr = "10.0.1.9";	/// address of UDP destination
	struct sockaddr_in dst_addr;
	char *vehicle_str = "Blue";
	posix_timer_typ *ptmr;
	int interval = 20;	/// milliseconds
	int do_broadcast = 0;	/// by default do unicast

        while ((ch = getopt(argc, argv, "a:bi:t:u:v")) != EOF) {
                switch (ch) {
		case 'a': ipaddr = strdup(optarg);
			  break;
		case 'b': do_broadcast = 1; 
			  break;
		case 'i': interval = atoi(optarg);
			  break;
		case 't': vehicle_str = strdup(optarg);
			  break;
		case 'u': udp_port = atoi(optarg); 
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

	/**  assumes DB_COMM variables were aleady created by another process
	 */
	pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0); 
	if (setjmp(exit_env) != 0) {
		printf("Sent %d messages\n", msg_count);
		db_list_done(pclt, NULL, 0, NULL, 0);		
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

	if ( (sd = udp_init(ipaddr, udp_port, &dst_addr, do_broadcast)) < 0) {
		printf("Failure to initialize socket for %s port %d\n",
			ipaddr, udp_port);
		longjmp(exit_env, 2);
	}

	while (1) {
		db_clt_read(pclt, DB_COMM_TX_VAR, sizeof(comm_pkt), &comm_pkt);

		set_vehicle_string(&comm_pkt, vehicle_str);

                msg_count++;
		comm_pkt.sequence_no = msg_count;

		get_current_timestamp(&comm_pkt.ts);

                bytes_sent = sendto(sd, &comm_pkt, sizeof(veh_comm_packet_t),
			 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));

		if (verbose) {
			printf("%s %d: %f\n", comm_pkt.object_id,
				comm_pkt.my_pip,
				comm_pkt.global_time);
			fflush(stdout);
		}
                if (bytes_sent < 0) {
                        perror("UDP sendto ");
                        printf("port %d addr 0x%08x\n",
                                ntohs(dst_addr.sin_port),
                                ntohl(dst_addr.sin_addr.s_addr));
                        fflush(stdout);
                }
		TIMER_WAIT(ptmr);
	}
	longjmp(exit_env,1);	/* go to exit code when loop terminates */
}
