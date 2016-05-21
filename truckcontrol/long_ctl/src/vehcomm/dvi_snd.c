/**\file
 *	sdi_snd.c 
 *		Sends a message to the Secondary Display Interface, filling in
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
#include "veh_lib.h"
#include "dvi.h"

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

int OpenTCPConnection(char *local_ip, char* remote_ip, unsigned short local_port, unsigned short remote_port);
int CloseTCPConnection(int sockfd);

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
	controller_to_DVI_t dvi_out;
	controller_to_DVI_t dvi_out_decode;
	veh_comm_packet_t comm_pkt1;
	veh_comm_packet_t comm_pkt2;
	veh_comm_packet_t comm_pkt3;

        int bytes_sent;     		/// received from a call to sendto
	int verbose = 0;
	int debug = 0;
	short msg_count = 0;
	char *remote_ipaddr = "10.0.1.9";	/// address of UDP destination
	char *local_ipaddr = "127.0.0.1";	/// address of UDP destination
	short unsigned local_port = 0;
	short unsigned remote_port = 0;
	struct sockaddr_in dst_addr;
	char *vehicle_str = "VNL475";
	posix_timer_typ *ptmr;
	int interval = 20;	/// milliseconds
	int do_broadcast = 0;	/// by default do unicast
	int ret = -1;
	int counter = 0;
	float fcounter = 10.0;
	int i;

        while ((ch = getopt(argc, argv, "A:a:bi:t:u:vdl:r:")) != EOF) {
                switch (ch) {
		case 'A': local_ipaddr = strdup(optarg);
			  break;
		case 'a': remote_ipaddr= strdup(optarg);
			  break;
		case 'b': do_broadcast = 1; 
			  break;
		case 'i': interval = atoi(optarg);
			  break;
		case 't': vehicle_str = strdup(optarg);
			  break;
		case 'l': local_port = atoi(optarg); 
			  break;
		case 'r': remote_port = atoi(optarg); 
			  break;
		case 'v': verbose = 1; 
			  break;
		case 'd': debug = 1; 
			  verbose = 1; 
			  break;
                default:  printf("Usage: %s [-v (verbose)] -A <local IP address, def. 127.0.0.1> -a <remote IP address, def. 10.0.1.9> -u <UDP port, def. 5050> -t <vehicle string, def. Blue> -i <interval, def. 20 ms>\n",argv[0]);
			  exit(EXIT_FAILURE);
                          break;
                }
        }
printf("Got to 1\n");
	if( ( sd = OpenTCPConnection(local_ipaddr, remote_ipaddr, local_port, remote_port)) < 0) {
		perror("OpenTCPConnection");
		printf("Failure %d to initialize socket from %s:%hd to %s:%d\n",
			sd, local_ipaddr, local_port, remote_ipaddr, remote_port);
		longjmp(exit_env, 2);
	}
       if ((ptmr = timer_init(interval, ChannelCreate(0))) == NULL) {
                printf("timer_init failed\n");
                exit(EXIT_FAILURE);
        }

printf("Got to 2\n");
        get_local_name(hostname, MAXHOSTNAMELEN);

	/**  assumes DB_COMM variables were aleady created by another process
	 */
	pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0); 
	if (setjmp(exit_env) != 0) {
		printf("Sent %d messages\n", msg_count);
		db_list_done(pclt, NULL, 0, NULL, 0);		
		if(sd > 0)
			CloseTCPConnection(sd);
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);
	while (1) {
printf("Got to 3\n");
		db_clt_read(pclt, DB_COMM_LEAD_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt1);
		db_clt_read(pclt, DB_COMM_SECOND_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt2);
		db_clt_read(pclt, DB_COMM_THIRD_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt3);

		dvi_out.sec_past_midnight = sec_past_midnight_float();
               
		dvi_out.msg_count = counter++;

/* man_des partial code list
         *      40  : Adaptive Cruise Control (in a platoon with at least one radar but no communication )
         *      41  : Cooperative Adaptive Cruise Control (in a platoon with at least one radar and communication )
         *      45  : manual control (including all the maneuvers)
*/
			dvi_out.Vehicle1State = comm_pkt1.maneuver_des_1; // 0, 40, 41, 45: 0=Not available 45=Off 40=ACC 41=CACC
			dvi_out.Vehicle1Braking = 0;          // 0-3: 0=Not available 1=Not braking 2=Braking 3=Hard braking
			dvi_out.Vehicle1CutIn = comm_pkt1.maneuver_des_2;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out
			dvi_out.Vehicle1LaneChange = 0;       // 0-3: 0=Not available 1=No lane change 2=Lane change to left 3=Lane change to right
			dvi_out.Vehicle1Communication = 0;    // 0-2: 0=Not available 1=Not communicating 2=Communicating
			dvi_out.Vehicle1Malfunction = comm_pkt1.fault_mode;      // 0-2: 0=Not available 1=Functioning 2=Malfunctioning
			dvi_out.Vehicle2State = comm_pkt2.maneuver_des_1; // 0, 40, 41, 45: 0=Not available 45=Off 40=ACC 41=CACC
			dvi_out.Vehicle2Braking = 0;          // 0-3: 0=Not available 1=Not braking 2=Braking 3=Hard braking
			dvi_out.Vehicle2CutIn = comm_pkt2.maneuver_des_2;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out
			dvi_out.Vehicle2LaneChange = 0;       // 0-3: 0=Not available 1=No lane change 2=Lane change to left 3=Lane change to right
			dvi_out.Vehicle2Communication = 0;    // 0-2: 0=Not available 1=Not communicating 2=Communicating
			dvi_out.Vehicle2Malfunction = comm_pkt2.fault_mode;      // 0-2: 0=Not available 1=Functioning 2=Malfunctioning
			dvi_out.Vehicle3State = comm_pkt3.maneuver_des_1; // 0, 40, 41, 45: 0=Not available 45=Off 40=ACC 41=CACC
			dvi_out.Vehicle3Braking = 0;          // 0-3: 0=Not available 1=Not braking 2=Braking 3=Hard braking
			dvi_out.Vehicle3CutIn = comm_pkt3.maneuver_des_2;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out
			dvi_out.Vehicle3LaneChange = 0;       // 0-3: 0=Not available 1=No lane change 2=Lane change to left 3=Lane change to right
			dvi_out.Vehicle3Communication = 0;    // 0-2: 0=Not available 1=Not communicating 2=Communicating
			dvi_out.Vehicle3Malfunction = comm_pkt3.fault_mode;      // 0-2: 0=Not available 1=Functioning 2=Malfunctioning
			dvi_out.EgoVehiclePosition = 0;       // 1-3: 0=Not available 1=First position 2=Second position 3=Third position

		if(verbose)
printf("Got to 2 Vehicle1State %d Vehicle1Braking %d Vehicle1CutIn %d Vehicle1LaneChange %d Vehicle1Communication %d Vehicle1Malfunction %d Vehicle2State %d Vehicle2Braking %d Vehicle2CutIn %d Vehicle2LaneChange %d Vehicle2Communication %d Vehicle2Malfunction %d Vehicle3State %d Vehicle3Braking %d Vehicle3CutIn %d Vehicle3LaneChange %d Vehicle3Communication %d Vehicle3Malfunction %d EgoVehiclePosition %d  \n",
			dvi_out.Vehicle1State,
			dvi_out.Vehicle1Braking, 
			dvi_out.Vehicle1CutIn,
			dvi_out.Vehicle1LaneChange,
			dvi_out.Vehicle1Communication,
			dvi_out.Vehicle1Malfunction,
			dvi_out.Vehicle2State,
			dvi_out.Vehicle2Braking,
			dvi_out.Vehicle2CutIn,
			dvi_out.Vehicle2LaneChange,
			dvi_out.Vehicle2Communication,
			dvi_out.Vehicle2Malfunction,
			dvi_out.Vehicle3State,
			dvi_out.Vehicle3Braking,
			dvi_out.Vehicle3CutIn,
			dvi_out.Vehicle3LaneChange,
			dvi_out.Vehicle3Communication,
			dvi_out.Vehicle3Malfunction, 
			dvi_out.EgoVehiclePosition 
		);

                bytes_sent = sendto(sd, &dvi_out, sizeof(dvi_out),
			 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));

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

int OpenTCPConnection(char *local_ip, char* remote_ip, unsigned short local_port, unsigned short remote_port){

	int sockfd = -1;
	int newsockfd;
	int backlog = 1;
	struct sockaddr_in local_addr;
	socklen_t localaddrlen = sizeof(local_addr);
	struct sockaddr_in remote_addr;

	// Open connection to SMS subnetwork controller
	if( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		if(errno != EINTR){
			perror("socket");
			return -2;
		}
	}

	/** set up local socket addressing and port */
	memset(&local_addr, 0, sizeof(struct sockaddr_in));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr =  inet_addr(local_ip);//htonl(INADDR_ANY);//
	local_addr.sin_port = htons(local_port);

	if( (bind(sockfd, (struct sockaddr *)&local_addr,
		localaddrlen) ) < 0) {
		if(errno != EINTR){
			perror("bind");
			printf("sockfd %d\n", sockfd);
			close(sockfd);
			return -3;
		}
	}

	if( (listen(sockfd, backlog )) < 0) {
		if(errno != EINTR){
			perror("listen");
			close(sockfd);
			return -4;
		}
	}

	/** set up remote socket addressing and port */
	memset(&remote_addr, 0, sizeof(struct sockaddr_in));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(remote_ip);//htonl(INADDR_ANY);
	remote_addr.sin_port = htons(remote_port);
	localaddrlen = sizeof(remote_addr);

	if( (newsockfd = accept(sockfd, (struct sockaddr *)&remote_addr,  
	&localaddrlen) ) < 0 ) {
		if(errno != EINTR) {
			perror("accept");
			close(sockfd);
			return -5;
		}
	}
	return newsockfd;
}

int CloseTCPConnection(int sockfd){
	close(sockfd);
	return 0;
}
