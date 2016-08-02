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
#include "../avcs/veh_trk.h"
#include "../avcs/clt_vars.h"
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

static db_id_t db_vars_list[] = {
	{DB_COMM_LEAD_TRK_VAR, sizeof(veh_comm_packet_t)},
	{DB_COMM_SECOND_TRK_VAR, sizeof(veh_comm_packet_t)},
	{DB_COMM_THIRD_TRK_VAR, sizeof(veh_comm_packet_t)},
	{DB_COMM_TX_VAR, sizeof(veh_comm_packet_t)},
};

#define NUM_DB_VARS     sizeof(db_vars_list)/sizeof(db_id_t)

int db_trig_list[] = {
	DB_COMM_LEAD_TRK_VAR,
	DB_COMM_SECOND_TRK_VAR,
	DB_COMM_THIRD_TRK_VAR,
	DB_COMM_TX_VAR,
	DB_DVI_RCV_VAR,
};

#define NUM_TRIG_VARS     sizeof(db_trig_list)/sizeof(int)

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
	trig_info_typ trig_info;
	int recv_type;

	int sd;				/// socket descriptor
	int sd2;				/// socket descriptor
	struct SeretUdpStruct dvi_out;
	struct ExtraDataCACCStruct egodata;
	char db_dvi_rcv = -1;
	veh_comm_packet_t comm_pkt1;
	veh_comm_packet_t comm_pkt2;
	veh_comm_packet_t comm_pkt3;
	veh_comm_packet_t self_comm_pkt;
	timestamp_t comm_pkt1_ts_sav = {0};
	timestamp_t comm_pkt2_ts_sav = {0};
	timestamp_t comm_pkt3_ts_sav = {0};

        int bytes_sent;     		/// received from a call to sendto
	int verbose = 0;
	int debug = 0;
	short msg_count = 0;
	char *remote_ipaddr = "172.16.5.77";	/// address of UDP destination
	char *local_ipaddr = "172.16.5.77";	/// address of UDP destination
	short unsigned remote_port = 10007;
	short unsigned remote_port2 = 10005;
	struct sockaddr_in dst_addr;
	struct sockaddr_in dst_addr2;
	char *vehicle_str = "VNL475";
	posix_timer_typ *ptmr;
	int interval = 50;	/// milliseconds
	int counter = 0;
	int send_test = 0;
	char *send_test_str;
	int no_send1 = 1;
	char *send_test_str2;
	int no_send2 = 1;
	int create_db_vars = 0;
	long_output_typ long_output;

	memset(&dvi_out, 0, sizeof(struct SeretUdpStruct));
	memset(&egodata, 0, sizeof(struct ExtraDataCACCStruct));

        while ((ch = getopt(argc, argv, "A:a:i:t:C:cr:R:vP:E:d")) != EOF) {
                switch (ch) {
		case 'A': local_ipaddr = strdup(optarg);
			  break;
		case 'a': remote_ipaddr= strdup(optarg);
			  break;
		case 'i': interval = atoi(optarg);
			  break;
		case 't': vehicle_str = strdup(optarg);
			  break;
		case 'C': counter = atoi(optarg); 
			  break;
		case 'c': create_db_vars = 1; 
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
                default:  printf("Usage: %s -A <local IP address, def. 127.0.0.1> -a <remote IP address, def. 10.0.1.9> -P <platooning data> -E <Egodata test string> -C <test counter> -c (Create db vars) -r <remote UDP port, def. 10007> -R <remote UDP port2, def. 10005>  -t <vehicle string, def. Blue> -i <interval, def. 20 ms>\n",argv[0]);
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

	if(create_db_vars)
		pclt = db_list_init(argv[0], hostname, domain, xport, db_vars_list, NUM_DB_VARS, db_trig_list,  NUM_TRIG_VARS); 
	else
		pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, db_trig_list, NUM_TRIG_VARS); 

	if (setjmp(exit_env) != 0) {
		printf("Sent %d messages\n", msg_count);
		if(create_db_vars)
			db_list_done(pclt, db_vars_list, NUM_DB_VARS, db_trig_list,  NUM_TRIG_VARS);		
		else
			db_list_done(pclt, NULL, 0, NULL, 0);		
		if(sd > 0){
			memset(&dvi_out, 0, sizeof(dvi_out));
	                bytes_sent = sendto(sd, &dvi_out, sizeof(dvi_out),
				 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));
			close(sd);
		}
		if(sd2 > 0){
			memset(&egodata, 0, sizeof(egodata));
                	bytes_sent = sendto(sd2, &egodata, sizeof(egodata),
				 0, (struct sockaddr *) &dst_addr2, sizeof(dst_addr2));
			close(sd2);
		}
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

	get_current_timestamp(&comm_pkt1_ts_sav);
	get_current_timestamp(&comm_pkt2_ts_sav);
	get_current_timestamp(&comm_pkt3_ts_sav);

	while (1) {
		recv_type= clt_ipc_receive(pclt, &trig_info, sizeof(trig_info));

		if(DB_TRIG_VAR(&trig_info) == DB_COMM_TX_VAR) {
	                db_clt_read(pclt, DB_COMM_TX_VAR, sizeof(veh_comm_packet_t), &self_comm_pkt);
			db_clt_read(pclt, DB_LONG_OUTPUT_VAR, sizeof(long_output_typ), &long_output);

			char drive_mode_2_CACCState[] = {0, 0, 4, 4, 2}; //DVI CACCState: 0:nothing, 1:CACC Enabled, 2:CACC Active, 3: ACC enabled, 4:ACC active
									//comm_pkt drive_mode (aka user_ushort_2): 0:stay, 1:manual, 2:CC, 3:ACC, 4:CACC
			egodata.CACCState = drive_mode_2_CACCState[self_comm_pkt.user_ushort_2];
			if( (long_output.selected_gap_level > 0) && (long_output.selected_gap_level <= 5) ) 
				long_output.selected_gap_level--;
			else
				long_output.selected_gap_level = 4;
    			egodata.CACCTimeGap = long_output.selected_gap_level;//0-4
    			egodata.ACCTimeGap = long_output.selected_gap_level;//0-4


			dvi_out.platooningState = 2; //0=standby, 2=platooning NOTE:Must be set to 2 to get rid of "Please switch VEC stalk to ON"
			dvi_out.position = self_comm_pkt.my_pip - 1;       // 1-3: 0=Not available 1=First position 2=Second position 3=Third position
			printf("Got self_pkt! CACCState %d my_pip %d db_dvi_rcv %hhu user_ushort_2 %d\n",
				egodata.CACCState, 
				dvi_out.position,
				db_dvi_rcv,
				self_comm_pkt.user_ushort_2
			);
			if(self_comm_pkt.my_pip == 1) {
				dvi_out.vehicles[0].type = 1;	// 0=nothing 1=truck 2=truck with communication error
				if ( (self_comm_pkt.user_bit_3 == 1) || (self_comm_pkt.user_float > 0.1) )
					dvi_out.vehicles[0].isBraking = 1;  // 0:false, 1:braking, 2:hard braking (PATH: same red indication for both 1 & 2)
				else
					dvi_out.vehicles[0].isBraking = 0;
				dvi_out.vehicles[0].hasIntruder = ((self_comm_pkt.maneuver_des_2 & 0x03) == 1) ? 1 : 0;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out

				if( (dvi_out.vehicles[1].type == 1) || (dvi_out.vehicles[2].type == 1))
					egodata.CACCActiveConnectionFromFollower = 1;
				else
					egodata.CACCActiveConnectionFromFollower = 0;

				egodata.CACCActiveConnectionToTarget = 0;
			}
			if(self_comm_pkt.my_pip == 2) {
				dvi_out.vehicles[1].type = 1;	// 0=nothing 1=truck 2=truck with communication error
				if ( (self_comm_pkt.user_bit_3 == 1) || (self_comm_pkt.user_float > 0.1) )
					dvi_out.vehicles[1].isBraking = 1;  // 0:false, 1:braking, 2:hard braking (PATH: same red indication for both 1 & 2)
				else
					dvi_out.vehicles[1].isBraking = 0;
				dvi_out.vehicles[1].hasIntruder = ((self_comm_pkt.maneuver_des_2 & 0x03) == 1) ? 1 : 0;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out

				if(dvi_out.vehicles[0].type == 1)
					egodata.CACCActiveConnectionToTarget = 1;
				else
					egodata.CACCActiveConnectionToTarget = 0;
				if(dvi_out.vehicles[2].type == 1)
					egodata.CACCActiveConnectionFromFollower = 1;
				else
					egodata.CACCActiveConnectionFromFollower = 0;
			}
			if(self_comm_pkt.my_pip == 3) {
				dvi_out.vehicles[2].type = 1;	// 0=nothing 1=truck 2=truck with communication error
				if ( (self_comm_pkt.user_bit_3 == 1) || (self_comm_pkt.user_float > 0.1) )
					dvi_out.vehicles[2].isBraking = 1;  // 0:false, 1:braking, 2:hard braking (PATH: same red indication for both 1 & 2)
				else
					dvi_out.vehicles[2].isBraking = 0;
				dvi_out.vehicles[2].hasIntruder = ((self_comm_pkt.maneuver_des_2 & 0x03) == 1) ? 1 : 0;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out

				if( (dvi_out.vehicles[0].type == 1) || (dvi_out.vehicles[1].type == 1))
					egodata.CACCActiveConnectionToTarget = 1;
				else
					egodata.CACCActiveConnectionToTarget = 0;

				egodata.CACCActiveConnectionFromFollower = 0;
			}
		}

		if(DB_TRIG_VAR(&trig_info) == DB_COMM_LEAD_TRK_VAR) {

			db_clt_read(pclt, DB_COMM_LEAD_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt1);

			if( ts2_is_later_than_ts1(&comm_pkt1_ts_sav, &comm_pkt1.ts) )
				dvi_out.vehicles[0].type = 1;	// 0=nothing 1=truck 2=truck with communication error
			else
				dvi_out.vehicles[0].type = 2;	// 0=nothing 1=truck 2=truck with communication error
			comm_pkt1_ts_sav = comm_pkt1.ts;

			if(dvi_out.vehicles[0].type == 1) {
				if ( (comm_pkt1.user_bit_3 == 1) || (comm_pkt1.user_float > 0.1) )
					dvi_out.vehicles[0].isBraking = 1;  // 0:false, 1:braking, 2:hard braking (PATH: same red indication for both 1 & 2)
				else
					dvi_out.vehicles[0].isBraking = 0;
				dvi_out.vehicles[0].hasIntruder = comm_pkt1.maneuver_des_2 & 0x03;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out
			}
		}

		if(DB_TRIG_VAR(&trig_info) == DB_COMM_SECOND_TRK_VAR) {

			db_clt_read(pclt, DB_COMM_SECOND_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt2);

			if( ts2_is_later_than_ts1(&comm_pkt2_ts_sav, &comm_pkt2.ts) )
				dvi_out.vehicles[1].type = 1;	// 0=nothing 1=truck 2=truck with communication error
			else
				dvi_out.vehicles[1].type = 2;	// 0=nothing 1=truck 2=truck with communication error
			comm_pkt2_ts_sav = comm_pkt2.ts;

			if(dvi_out.vehicles[1].type == 1) {
				if ( (comm_pkt2.user_bit_3 == 1) || (comm_pkt2. user_float > 0.1) )
					dvi_out.vehicles[1].isBraking = 1;  // 0:false, 1:braking, 2:hard braking (PATH: same red indication for both 1 & 2)
				else
					dvi_out.vehicles[1].isBraking = 0;
				dvi_out.vehicles[1].hasIntruder = comm_pkt2.maneuver_des_2 & 0x03;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out
			}
		}

		if(DB_TRIG_VAR(&trig_info) == DB_COMM_THIRD_TRK_VAR) {

			db_clt_read(pclt, DB_COMM_THIRD_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt3);

			if( ts2_is_later_than_ts1(&comm_pkt3_ts_sav, &comm_pkt3.ts) )
				dvi_out.vehicles[2].type = 1;	// 0=nothing 1=truck 2=truck with communication error
			else
				dvi_out.vehicles[2].type = 2;	// 0=nothing 1=truck 2=truck with communication error
			comm_pkt3_ts_sav = comm_pkt3.ts;

			if(dvi_out.vehicles[2].type == 1) {
				if ( (comm_pkt3.user_bit_3 == 1) || (comm_pkt3. user_float > 0.1) )
					dvi_out.vehicles[2].isBraking = 1;  // 0:false, 1:braking, 2:hard braking (PATH: same red indication for both 1 & 2)
				else
					dvi_out.vehicles[2].isBraking = 0;
				dvi_out.vehicles[2].hasIntruder = comm_pkt3.maneuver_des_2 & 0x03;	// 0-2: 0=Not available 1=Cut-in 2=Cut-out
			}
		}

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
			printf("EgoPlatooningState %d EgoVehiclePosition %d EgoCACCState1 %d Type1 %d Braking1 %d CutIn1 %d Type2 %d Braking2 %d CutIn2 %d Type3 %d Braking3 %d CutIn3 %d db_dvi_rcv %hhu\n",
						dvi_out.platooningState, 
						dvi_out.position,
						egodata.CACCState, 
						dvi_out.vehicles[0].type, 
						dvi_out.vehicles[0].isBraking, 
						dvi_out.vehicles[0].hasIntruder, 
						dvi_out.vehicles[1].type, 
						dvi_out.vehicles[1].isBraking, 
						dvi_out.vehicles[1].hasIntruder, 
						dvi_out.vehicles[2].type, 
						dvi_out.vehicles[2].isBraking, 
						dvi_out.vehicles[2].hasIntruder,
						db_dvi_rcv
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
