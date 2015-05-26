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

char *usage = "-a <ip_addr (127.0.0.1)> \n\t\t\t-b broadcast message \n\t\t\t-t <vehicle (Blue)> \n\t\t\t-u <udp port (5051)> \n\t\t\t-r <comm reply, user_ushort_1 (1)> \n\t\t\t-w <sw_pt->auto_sw, user_ushort_2 (1)> \n\t\t\t-s <global time (0.0)> \n\t\t\t-z <platoon size (1)> \n\t\t\t-v verbose\n";
#ifdef __USAGE
veh_snd_test 	-a <ip_addr (127.0.0.1)>
		-b broadcast message
		-t <vehicle (Blue)>
		-u <udp port (5051)>
		-r <comm reply, user_ushort_1 (1)> 
		-w <sw_pt->auto_sw, user_ushort_2 (1)> 
		-s <global time (0.0)>
		-z <platoon size (1)> 
		-v verbose
#endif

int main(int argc, char *argv[])
{
	int ch;		
        db_clt_typ *pclt;  		/// data bucket pointer	
        char *domain=DEFAULT_SERVICE;
        char hostname[MAXHOSTNAMELEN+1];
        int xport = COMM_OS_XPORT;
	int sd;				/// socket descriptor
	int udp_port = 5051;
	veh_comm_packet_t comm_pkt;
        int bytes_sent;     		/// received from a call to sendto
	int verbose = 0;
	short msg_count = 0;
	char *ipaddr = "127.0.0.1";	/// address of UDP destination
	struct sockaddr_in dst_addr;
	char *vehicle_str = "Blue";
	int do_broadcast = 0;	/// by default do unicast
	float time_input = 0;
	int pltn_size = 1;
	int comm_reply = 1;// Check for all other trucks' position-in-platoon
                           // field before setting comm_reply to 1. (The pip
                           // field is set from "PositionInPlatoon" in realtime.ini 
			   // during a truck's initialization.)
                           // Check for all other trucks' user_ushort_1 field
                           // before setting handshake_start to ON. (The
                           // user_ushort_1 field in the send message is set
                           // to the comm_reply value, calculated above. So
                           // for handshake_start to be set to ON, all trucks
                           // must receive all other trucks' position-in-platoon,
                           // set their comm_reply's, and send those out.)
	int auto_sw = 1;   // Check for all other trucks' manual/auto switch 
			   // setting.


	
        while ((ch = getopt(argc, argv, "a:bt:u:vs:r:hz:w:")) != EOF) {
                switch (ch) {
		case 'a': ipaddr = strdup(optarg);
			  break;
		case 'b': do_broadcast = 1; 
			  break;
		case 't': vehicle_str = strdup(optarg);
			  break;
		case 'u': udp_port = atoi(optarg); 
			  break;
		case 'r': comm_reply = atoi(optarg); 
			  break;
		case 'w': auto_sw = atoi(optarg); 
			  break;
		case 's': time_input = atof(optarg); 
			  break;
		case 'z': pltn_size = atoi(optarg); 
			  break;
		case 'v': verbose = 1; 
			  break;
		case 'h':
                default:  printf("\nUsage: %s %s", argv[0], usage);
			  exit(EXIT_FAILURE);
                          break;
                }
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

		db_clt_read(pclt, DB_COMM_TX_VAR, sizeof(comm_pkt), &comm_pkt);

		set_vehicle_string(&comm_pkt, vehicle_str);
		if( !strcmp(vehicle_str, "Blue") ) {
			comm_pkt.my_pip = 1;
			comm_pkt.vel_traj = 100.0;
			comm_pkt.acc_traj = 10.0;
		}
		if( !strcmp(vehicle_str, "Gold") ) {
			comm_pkt.my_pip = 2;
			comm_pkt.vel_traj = 200.0;
			comm_pkt.acc_traj = 20.0;
		}
		if( !strcmp(vehicle_str, "Silvr") ) {
			comm_pkt.my_pip = 3;
			comm_pkt.vel_traj = 300.0;
			comm_pkt.acc_traj = 30.0;
		}
		comm_pkt.global_time = time_input;
		comm_pkt.user_ushort_1 = comm_reply;
		comm_pkt.user_ushort_2 = auto_sw;
		comm_pkt.pltn_size = pltn_size;

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
	longjmp(exit_env,1);	/* go to exit code when loop terminates */
}
