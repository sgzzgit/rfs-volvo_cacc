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
#include <udp_utils.h>
#include <timestamp.h>
#include <local.h>
#include <sys_rt.h>
#include <sys_ini.h>
#include "path_gps_lib.h"
#include "long_comm.h"
#include "veh_lib.h"
//#include "dvi.h"

#define quint8 unsigned char
#define quint32 unsigned int
#define qint8 char


//Vehicle -> DVI - port 10007 (string data and popups)
struct VehicleStruct {
  quint8 type; // 0=nothing 1=truck 2=truck with communication error
  quint8 hasIntruder; //0:false, 1:truck, 2:car, 3:MC (PATH: The graphical indication is the same for all intruders)
  quint8 isBraking; //0:false, 1:braking, 2:hard braking (PATH: same red indication for both 1 & 2)
};

 

struct SeretUdpStruct{
    quint8 platooningState; //0=standby, 1=joining, 2=platooning, 3=leaving, 4=dissolve (PATH: I guess only 0 and 3 is used?)
    qint8 position; //-1:nothing (follower with no platoon), 0:leader, >0 Follower (Ego position of vehicle)
    quint8 TBD; //Not used for the moment
    quint8 popup;//0:no popup, 1:Platoon found - join?
    quint32 exitDistance; //value/10.0 km (PATH: Not currently used)
    struct VehicleStruct vehicles[3];
} IS_PACKED;
//};

 

//Vehicle -> DVI - port 10005 (ACC/CACC information)
struct ExtraDataCACCStruct{
    quint8 CACCState; //0:nothing, 1:CACC Enabled, 2:CACC Active, 3: ACC enabled, 4:ACC active
    quint8 CACCTargetActive; //0:false, 1:true (also used for target in ACC)
    quint8 CACCDegraded;//0: false, 1:Overheated brakes (I guess you don't need this one)
    quint8 CACCActiveConnectionToTarget;//0:no connection 1:connection (if this or ...fromFollower equals 1 the WIFI icon will appear)
    quint8 CACCActiveConnectionFromFollower;//0:no connection, 1:connection
    quint8 CACCTimeGap;//0-4
    quint8 ACCTimeGap;//0-4
    quint8 CACCEvents;//0:"No popup", 1:"FCW",2:"Brake Capacity",3:"LC Left",4:"LC Right",5:"Obstacle ahead",6:"Connection lost"
    quint8 platooningState;//0:"Platooning",1:"Joining",2:"Leaving",3:"Left",4:"Dissolving",5:"Dissolved" (NOT CURRENTLY USED!)
    quint8 counter;//Counter for dissolving, not implemented for the moment
};
 

//DVI -> Vehicle - port 8003 (button pressed). Is being sent with an interval of 50ms. Triggers when a button is released and is kept high for 200ms.
struct PathButtonStruct{
    quint8 buttonPressed;
    //0: no button pressed
    //1: joinpopup_JOIN
    //2: joinpopup_IGONORE
    //3: LEAVE
    //4: DISSOLVE
    //5: ACC_BUTTON
    //6: CACC_BUTTON
    //7: TIMEGAP_MINUS
    //8: TIMEGAP_PLUS
};

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
	int sd2;				/// socket descriptor
	struct SeretUdpStruct dvi_out;
	struct ExtraDataCACCStruct egodata;
	veh_comm_packet_t comm_pkt1;
	veh_comm_packet_t comm_pkt2;
	veh_comm_packet_t comm_pkt3;

        int bytes_sent;     		/// received from a call to sendto
	int verbose = 0;
	int debug = 0;
	short msg_count = 0;
	char *remote_ipaddr = "192.168.1.111";	/// address of UDP destination
	char *local_ipaddr = "192.168.1.68";	/// address of UDP destination
	short unsigned remote_port = 10007;
	short unsigned remote_port2 = 10007;
	struct sockaddr_in dst_addr;
	struct sockaddr_in dst_addr2;
	char *vehicle_str = "VNL475";
	posix_timer_typ *ptmr;
	int interval = 50;	/// milliseconds
	int counter = 100;
	int send_test = 0;
	char *send_test_str;
	int no_send1 = 1;
	char *send_test_str2;
	int no_send2 = 1;

	memset(&dvi_out, 0, sizeof(struct SeretUdpStruct));
	memset(&egodata, 0, sizeof(struct ExtraDataCACCStruct));

        while ((ch = getopt(argc, argv, "A:a:i:t:c:r:R:vP:E:d")) != EOF) {
                switch (ch) {
		case 'A': local_ipaddr = strdup(optarg);
			  break;
		case 'a': remote_ipaddr= strdup(optarg);
			  break;
		case 'i': interval = atoi(optarg);
			  break;
		case 't': vehicle_str = strdup(optarg);
			  break;
		case 'c': counter = atoi(optarg); 
			  break;
		case 'r': remote_port = atoi(optarg); 
			  break;
		case 'R': remote_port2 = atoi(optarg); 
			  break;
		case 'v': verbose = 1; 
			  break;
		case 'P': send_test = 1; 
			  send_test_str = strdup(optarg);
			  no_send1 = 0;
			  sscanf(send_test_str, "%hhu %hhu %hhu %hhu %u %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu ", 
				&dvi_out.platooningState, //0=standby, 1=joining, 2=platooning, 3=leaving, 4=dissolve 
							 //(PATH: I guess only 0 and 3 is used?)
				&dvi_out.position, //-1:nothing (follower with no platoon), 0:leader, >0 Follower (Ego position of vehicle)
				&dvi_out.TBD, //Not used for the moment
				&dvi_out.popup,//0:no popup, 1:Platoon found - join?
				&dvi_out.exitDistance, //value/10.0 km (PATH: Not currently used)
				&dvi_out.vehicles[0].type, // 0=nothing 1=truck 2=truck with communication error
				&dvi_out.vehicles[0].hasIntruder, // 0:false, 1:truck, 2:car, 3:MC 
								  // (PATH: The graphical indication is the same for all intruders)
				&dvi_out.vehicles[0].isBraking, // 0:false, 1:braking, 2:hard braking 
								// (PATH: same red indication for both 1 & 2)
				&dvi_out.vehicles[1].type, // 0=nothing 1=truck 2=truck with communication error
				&dvi_out.vehicles[1].hasIntruder, // 0:false, 1:truck, 2:car, 3:MC 
								  // (PATH: The graphical indication is the same for all intruders)
				&dvi_out.vehicles[1].isBraking, // 0:false, 1:braking, 2:hard braking 
								// (PATH: same red indication for both 1 & 2)
				&dvi_out.vehicles[2].type, // 0=nothing 1=truck 2=truck with communication error
				&dvi_out.vehicles[2].hasIntruder, // 0:false, 1:truck, 2:car, 3:MC 
								  // (PATH: The graphical indication is the same for all intruders)
				&dvi_out.vehicles[2].isBraking // 0:false, 1:braking, 2:hard braking 
								// (PATH: same red indication for both 1 & 2)
			  );
			  break;
		case 'E': send_test = 1; 
			  send_test_str2 = strdup(optarg);
			  no_send2 = 0;
			  sscanf(send_test_str2, "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu", 
    				&egodata.CACCState, //0:nothing, 1:CACC Enabled, 2:CACC Active, 3: ACC enabled, 4:ACC active
    				&egodata.CACCTargetActive, //0:false, 1:true (also used for target in ACC)
    				&egodata.CACCDegraded,//0: false, 1:Overheated brakes (I guess you don't need this one)
    				&egodata.CACCActiveConnectionToTarget,//0:no connection 1:connection (if this or ...fromFollower equals 1 the WIFI icon will appear)
    				&egodata.CACCActiveConnectionFromFollower,//0:no connection, 1:connection
    				&egodata.CACCTimeGap,//0-4
    				&egodata.ACCTimeGap,//0-4
    				&egodata.CACCEvents,//0:"No popup", 1:"FCW",2:"Brake Capacity",3:"LC Left",4:"LC Right",5:"Obstacle ahead",6:"Connection lost"
    				&egodata.platooningState,//0:"Platooning",1:"Joining",2:"Leaving",3:"Left",4:"Dissolving",5:"Dissolved" (NOT CURRENTLY USED!)
    				&egodata.counter //Counter for dissolving, not implemented for the moment
			  );
			  break;
		case 'd': debug = 1; 
			  verbose = 1; 
			  break;
                default:  printf("Usage: %s -A <local IP address, def. 127.0.0.1> -a <remote IP address, def. 10.0.1.9> -P <platooning data> -E <Egodata test string> -c <test counter> -r <remote UDP port, def. 5050> -R <remote UDP port2, def. 10007>  -t <vehicle string, def. Blue> -i <interval, def. 20 ms>\n",argv[0]);
			  exit(EXIT_FAILURE);
                          break;
                }
        }

	if ( (sd = udp_peer2peer_init(&dst_addr, remote_ipaddr, local_ipaddr, remote_port, 0)) < 0) {
		printf("Failure create unicast socket1 from %s to %s:%d\n",
		local_ipaddr, remote_ipaddr, remote_port);
		longjmp(exit_env, 2);
	}

        if ( (sd2 = udp_peer2peer_init(&dst_addr2, remote_ipaddr, local_ipaddr, remote_port2, 0)) < 0) {
		printf("Failure create unicast socket2 from %s to %s:%d\n",
			local_ipaddr, remote_ipaddr, remote_port2);
		longjmp(exit_env, 2);
        }

       if ((ptmr = timer_init(interval, ChannelCreate(0))) == NULL) {
                printf("timer_init failed\n");
                exit(EXIT_FAILURE);
        }

	if(send_test) {
		while(counter-- >= 0) {
		
			if(!no_send1) {
				bytes_sent = sendto(sd, &dvi_out, sizeof(dvi_out),
				0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));

				if (bytes_sent < 0) {
					perror("UDP sendto ");
					printf("port %d addr 0x%08x\n",
					ntohs(dst_addr.sin_port),
					ntohl(dst_addr.sin_addr.s_addr));
					fflush(stdout);
				}
printf("bytes_sent1 %d\n", bytes_sent);
			}

			if(!no_send2) {
				bytes_sent = sendto(sd2, &egodata, sizeof(egodata),
				0, (struct sockaddr *) &dst_addr2, sizeof(dst_addr2));

				if (bytes_sent < 0) {
					perror("UDP sendto ");
					printf("port %d addr 0x%08x\n",
					ntohs(dst_addr2.sin_port),
					ntohl(dst_addr2.sin_addr.s_addr));
					fflush(stdout);
				}
printf("bytes_sent2 %d\n", bytes_sent);
			}
			TIMER_WAIT(ptmr);
		}
			exit(EXIT_SUCCESS);
	}

        get_local_name(hostname, MAXHOSTNAMELEN);

	/**  assumes DB_COMM variables were aleady created by another process
	 */
	pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0); 
	if (setjmp(exit_env) != 0) {
		printf("Sent %d messages\n", msg_count);
		db_list_done(pclt, NULL, 0, NULL, 0);		
		if(sd > 0)
			close(sd);
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);
	while (1) {
		db_clt_read(pclt, DB_COMM_LEAD_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt1);
		db_clt_read(pclt, DB_COMM_SECOND_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt2);
		db_clt_read(pclt, DB_COMM_THIRD_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt3);

/* man_des partial code list
         *      40  : Adaptive Cruise Control (in a platoon with at least one radar but no communication )
         *      41  : Cooperative Adaptive Cruise Control (in a platoon with at least one radar and communication )
         *      45  : manual control (including all the maneuvers)
*/
			egodata.CACCState = comm_pkt1.maneuver_des_1; // comm_pkt:  0:Not available, 45:Off, 40:ACC, 41:CACC
									 // CACCState: 0:nothing, 1:CACC Enabled, 2:CACC Active, 3: ACC enabled, 4:ACC active

			dvi_out.vehicles[0].isBraking = 0;          // 0-3: 0=Not available 1=Not braking 2=Braking 3=Hard braking

			dvi_out.vehicles[0].hasIntruder = comm_pkt1.maneuver_des_2;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out

//			dvi_out.Vehicle1Communication = 0;    // 0-2: 0=Not available 1=Not communicating 2=Communicating

//			dvi_out.Vehicle1Malfunction = comm_pkt1.fault_mode;      // 0-2: 0=Not available 1=Functioning 2=Malfunctioning

//			egodata[1].CACCState = comm_pkt2.maneuver_des_1; // comm_pkt:  0:Not available, 45:Off, 40:ACC, 41:CACC
									 // CACCState: 0:nothing, 1:CACC Enabled, 2:CACC Active, 3: ACC enabled, 4:ACC active

			dvi_out.vehicles[1].isBraking = 0;          // 0-3: 0=Not available 1=Not braking 2=Braking 3=Hard braking

			dvi_out.vehicles[1].hasIntruder = comm_pkt2.maneuver_des_2;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out

//			dvi_out.Vehicle2Communication = 0;    // 0-2: 0=Not available 1=Not communicating 2=Communicating

//			dvi_out.Vehicle2Malfunction = comm_pkt2.fault_mode;      // 0-2: 0=Not available 1=Functioning 2=Malfunctioning

//			egodata[2].CACCState = comm_pkt3.maneuver_des_1; // comm_pkt:  0:Not available, 45:Off, 40:ACC, 41:CACC
									 // CACCState: 0:nothing, 1:CACC Enabled, 2:CACC Active, 3: ACC enabled, 4:ACC active

			dvi_out.vehicles[2].isBraking = 0;          // 0-3: 0=Not available 1=Not braking 2=Braking 3=Hard braking

			dvi_out.vehicles[2].hasIntruder = comm_pkt3.maneuver_des_2;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out

//			dvi_out.Vehicle3Communication = 0;    // 0-2: 0=Not available 1=Not communicating 2=Communicating

//			dvi_out.Vehicle3Malfunction = comm_pkt3.fault_mode;      // 0-2: 0=Not available 1=Functioning 2=Malfunctioning

			dvi_out.position = comm_pkt1.my_pip;       // 1-3: 0=Not available 1=First position 2=Second position 3=Third position

		if(debug) {
				egodata.CACCState = 1; 
				dvi_out.vehicles[0].isBraking = 1; 
				dvi_out.vehicles[0].hasIntruder = 1; 
				dvi_out.vehicles[1].isBraking = 1; 
				dvi_out.vehicles[1].hasIntruder = 1; 
				dvi_out.vehicles[2].isBraking = 1; 
				dvi_out.vehicles[2].hasIntruder = 1; 
				dvi_out.position = 1;
		}

		if(verbose)
printf("Got to 2 CACCState1 %d Braking1 %d CutIn1 %d Braking2 %d CutIn2 %d Braking3 %d CutIn3 %d EgoVehiclePosition %d\n",
			egodata.CACCState, 
			dvi_out.vehicles[0].isBraking, 
			dvi_out.vehicles[0].hasIntruder, 
			dvi_out.vehicles[1].isBraking, 
			dvi_out.vehicles[1].hasIntruder, 
			dvi_out.vehicles[2].isBraking, 
			dvi_out.vehicles[2].hasIntruder, 
			dvi_out.position
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

                bytes_sent = sendto(sd2, &egodata, sizeof(egodata),
			 0, (struct sockaddr *) &dst_addr2, sizeof(dst_addr2));

                if (bytes_sent < 0) {
                        perror("UDP sendto ");
                        printf("port %d addr 0x%08x\n",
                                ntohs(dst_addr2.sin_port),
                                ntohl(dst_addr2.sin_addr.s_addr));
                        fflush(stdout);
                }
		TIMER_WAIT(ptmr);
	}
	longjmp(exit_env,1);	/* go to exit code when loop terminates */
}
