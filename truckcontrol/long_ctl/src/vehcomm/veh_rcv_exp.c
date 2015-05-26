/**\file
 *	veh_rcv.c 
 *
 *	Receives a message from another vehicle.
 *	writes it to the appropriate data server variable,
 *	depending on vehicle string in the message. 
 *	
 *	Keeps track of the data server variable associated
 *	with a vehicle string and the time it was last written.
 *	Writes new messages from the same vehicle string to the
 *	same variable.
 *
 *	If exceeds maximum for unique vehicles, bumps oldest first.
 *
 *
 * Copyright (c) 2008   Regents of the University of California
 *
 */
#include <sys_os.h>
#include <db_clt.h>
#include <db_utils.h>
#include <timestamp.h>
#include "path_gps_lib.h"
#include "long_comm.h"
#include <local.h>
#include <sys_rt.h>
#include <sys_ini.h>

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

typedef struct {
	veh_comm_packet_t comm_pkt;
	timestamp_t ts;
} stored_packet_t;


static int init_stored_packets(stored_packet_t *parray, int array_size)
{
	int i;
	for (i = 0; i < array_size; i++) {
		strncpy(&parray->gps.object_id[0], "", GPS_OBJECT_ID_SIZE);
		get_current_timestamp(&parray->ts);
	}
}

static int find_oldest_stored_packet(stored_packet_t *parray, int array_size)

	int i;
	timestamp_t oldest_ts = parray[0].ts;
	int oldest_index = 0;

	for (i = 0; i < array_size; i++) {
		timestamp_t *pcurrent = &parray[i].ts;
		if (ts2_is_later_than_ts1(pcurrent, &oldest_ts)) {
			oldest_ts = *pcurrent;
			oldest_index = i;
		}
	}
	return oldest_index;
}
	

/** Maximum packets stored in data server is expected to
 *  be about 10, so code does simple pass through the array.
 *  
 *  First check if this vehicle is already stored in data server,
 *  then use the same data server variable number. Otherwise
 *  if there are DB_COMM_BASE variable numbers that
 *  have never been used, use one of them. Assumes that
 *  empty string in object_id field indicates this element
 *  has never been used, and that elements are used in order.
 *  Otherwise find the earliest timestamp and replace that.
 *
 *  Note that an empty string in a received message may cause
 *  it to be overwritten immediately by the next received message.
 */
static int store_packet(db_clt_typ *pclt, veh_comm_packet_t *pvcp,
			stored_packet_t *parray, int array_size)
{
	int match = 0;
	char *current_str = &pvcp->gps.object_id[0];
	int db_index = 0;
	int i;

	for (i = 0; i < array_size; i++) {
		char *element_str = &parray[i].gps.object_id[0];
		if (strncmp(current_str, element_str, GPS_OBJECT_ID_SIZE) == 0) 
			|| (strncmp("", element_str, GPS_OBJECT_ID_SIZE) == 0) {
			match = 1;
			db_index = i;
			break;
		}
	}

	if (!match) 
		db_index = find_oldest_stored_packet(parray, array_size);

	parray[db_index].comm_pkt = *pvcp;
	get_current_timestamp(&parray[db_index].ts);
	db_clt_write(pclt, DB_COMM_BASE_VAR+db_index, 
		sizeof(vehicle_comm_packet_t), pvcp); 
	return db_index;
}

/* Sets up a UDP socket for reception on a port from any address
 */
static int udp_init(short port)
{
        int sockfd;                      // listen on sock_fd
        struct sockaddr_in addr;       // IP info for socket calsl

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                perror("socket");
                return -1;
        }
        set_inet_addr(&addr, INADDR_ANY, port);

        if (bind(sockfd, (struct sockaddr *)&addr,
                                         sizeof(struct sockaddr)) == -1) {
                perror("bind");
                return -2;
        }
        return sockfd;
}

int main(int argc, char **argv)
{
	int ch;		
        db_clt_typ *pclt;  		/// data bucket pointer	
        char *domain=DEFAULT_SERVICE;
        char hostname[MAXHOSTNAMELEN+1];
        int xport = COMM_OS_XPORT;
        struct sockaddr_in src_addr;
	int sd;				/// socket descriptor
	int udp_port;
	veh_comm_packet_t comm_pkt;
        int bytes_received;     	/// received from a call to recv
	FILE *fpin;			/// file pointer for ini file
	char *vehicle_str="Blue";	// maximum 5 usable characters 
	int verbose = 0;
	short msg_count = 0;
	int socklen = sizeof(src_addr);
	stored_packet_t stored_packets[MAX_PLATOON_SIZE];
	int platoon_size = MAX_PLATOON_SIZE;

        while ((ch = getopt(argc, argv, "n:t:u:v")) != EOF) {
                switch (ch) {
		case 'n': platoon_size = atoi(optarg); 
			  if (platoon_size > MAX_PLATOON_SIZE)
				platoon_size = MAX_PLATOON_SIZE;
			  printf("Maximum platoon size %d\n",
				MAX_PLATOON_SIZE);
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
	printf("vehicle_str %s:", vehicle_str);

        get_local_name(hostname, MAXHOSTNAMELEN);

	/**  assumes DB_COMM variables were aleady created by another process
	 */
	pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0); 
	if (setjmp(exit_env) != 0) {
		printf("Received %d messages\n", msg_count);
		db_list_done(pclt, NULL, 0, NULL, 0);		
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

	sd = udp_init(udp_port);

	init_stored_packets(&stored_packets, platoon_size);

	while (1) {
		int db_index = -1;
                if ((bytes_received = recvfrom(sd, &comm_pkt,
			 sizeof(vehicle_comm_pkt_t), 0,
                        (struct sockaddr *) &src_addr,
                        (socklen_t *) &socklen)) <= 0) {

                        perror("recvfrom failed\n");
                        break;
                }
		/// Don't save a self broadcast
                if (comm_pkt.gps.object_id[0] != *vehicle_str) {  
			msg_count++;
			db_index = store_packet(pclt, &comm_pkt, 
					&stored_packets[0]);

                }
		if (verbose) {
			/// in case of error, make sure string is terminated
			comm_pkt.gps.object_id[GPS_OBJECT_ID_SIZE-1] = '\0';
			printf("Received %d bytes from %s, stored %d\n",
				bytes_received, comm_pkt.gps.object_id,
				db_index);

		}
	}
		
	longjmp(exit_env, 1);	/* go to exit code when loop terminates */
}
