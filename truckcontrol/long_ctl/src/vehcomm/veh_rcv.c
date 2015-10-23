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
#include "path_gps_lib.h"
#include "long_comm.h"
#include <local.h>
#include <sys_rt.h>
#include <sys_ini.h>
#include <udp_utils.h>

#include "asn_application.h"
#include "asn_internal.h"       /* for _ASN_DEFAULT_STACK_MAX */
#include "BSMCACC.h"

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
	(-1)
};

static jmp_buf exit_env;

int BSM2vehcomm(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt);

static void sig_hand(int code)
{
        if (code == SIGALRM)
                return;
        else
                longjmp(exit_env, code);
}

// Sets up a UDP socket for reception on a port from any address
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

/**  By default (if no ini file entry) sets up as head car
 */
static int get_ids(FILE *fpin, int *pself, int *plead, int *psecond, int *pthird) 
{
	*plead = get_ini_long(fpin, "LeadVehicleID", 1);   
	*pself = get_ini_long(fpin, "PositionInPlatoon", 1);   
	*psecond = get_ini_long(fpin, "SecondVehicleID", 0);   
	*pthird = get_ini_long(fpin, "ThirdVehicleID", 0);   
	printf("lead %d self %d second %d third %d\n", *plead, *pself, *psecond, *pthird);
	fflush(stdout);
        return 1;
}

#define BSMCACCSIZE     300

int main( int argc, char *argv[] )
{
	int ch;		
        db_clt_typ *pclt;  		/// data bucket pointer	
        char *domain=DEFAULT_SERVICE;
        char hostname[MAXHOSTNAMELEN+1];
        int xport = COMM_OS_XPORT;
	int self_vehicle_id;
	int lead_vehicle_id;
	int second_vehicle_id;
	int third_vehicle_id;
        struct sockaddr_in src_addr;
	int sd;				/// socket descriptor
	int udp_port = 5052;

	veh_comm_packet_t comm_pkt;
        BSMCACC_t *BSMCACC;
        char BSMCACC_buf[BSMCACCSIZE];

        static asn_TYPE_descriptor_t PDU_Type;
        static asn_TYPE_descriptor_t *pduType = &PDU_Type;
        asn_dec_rval_t rval;
	int ret = -1;

        void *structure;    /* Decoded structure */

        int bytes_received;     // received from a call to recv
	FILE *fpin;			/// file pointer for ini file
	char *vehicle_str="Blue";		/// e.g., Blue, Gold, Silver
	int verbose = 0;
	short msg_count = 0;
	char *ini_fname = "realtime.ini";
	int socklen = sizeof(src_addr);

        short rcvd_sn = 0;////////////////////////////////
printf("VEH_RCV.C: sizeof veh_comm_packet_t %d\n", sizeof(veh_comm_packet_t));

	BSMCACC = (BSMCACC_t *)calloc(1, sizeof(BSMCACC_t));

        while ((ch = getopt(argc, argv, "t:u:vf:")) != EOF) {
                switch (ch) {
		case 't': vehicle_str = strdup(optarg);
			  break;
		case 'f': ini_fname = strdup(optarg);
			  break;
		case 'u': udp_port = atoi(optarg); 
			  break;
		case 'v': verbose = 1; 
			  break;
                default:  printf( "Usage: %s [-v (verbose)] -t <vehicle string, def. Blue> -u <UDP port, def. 5052> -f <config file name, def. realtime.ini>", argv[0]);
			  exit(EXIT_FAILURE);
                          break;
                }
        }
        if ((fpin = get_ini_section(ini_fname, vehicle_str)) == NULL) {
                printf("%s: can't get ini file %s, section %s\n",
			   argv[0], ini_fname, vehicle_str);
                fflush(stdout);
                exit(EXIT_FAILURE);
        }

	/**  Read in .ini file and set vehicle IDs
	 */

	printf("vehicle_str %s:", vehicle_str);
	get_ids(fpin, &self_vehicle_id, &lead_vehicle_id, 
			&second_vehicle_id, &third_vehicle_id); 

        get_local_name(hostname, MAXHOSTNAMELEN);

	/**  assumes DB_COMM variables were aleady created by another process
	 */
	pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0); 
	if( setjmp( exit_env ) != 0 ) {
		printf("Received %d messages\n", msg_count);
		db_list_done(pclt, NULL, 0, NULL, 0);		
		exit( EXIT_SUCCESS );
	} else
		sig_ign( sig_list, sig_hand );

	sd = udp_init(udp_port);

	while (1) {
		/// recvfrom
//                if ((bytes_received = recvfrom(sd, &comm_pkt,
//			 sizeof(long_comm_pkt), 0,
                if ((bytes_received = recvfrom(sd, BSMCACC_buf,
			 BSMCACCSIZE, 0,
                        (struct sockaddr *) &src_addr,
                                (socklen_t *) &socklen))
                                 <= 0) {
                        perror("recvfrom failed\n");
                        break;
                }

		//Print out BSMCACC if desired
		if(verbose) {
printf("Got to 1\n");
			memset(BSMCACC, 0, sizeof(BSMCACC_t));
			rval = ber_decode(0, &asn_DEF_BSMCACC,(void **)BSMCACC, &BSMCACC_buf[0], BSMCACCSIZE);
printf("Got to 2 BSMCACC.msgID %#x %d\n", BSMCACC->msgID, rval.consumed) ;
	                if(rval.code != RC_OK) {
	                        fprintf(stderr, "%s:Cannot decode received message. Bytes comsumed %d\n", argv[0], rval.consumed);
	                        exit(EXIT_FAILURE);
	                }
			xer_fprint(stdout, &asn_DEF_BSMCACC, BSMCACC);
			printf("BSMCACC->msgID %d\n", (int)BSMCACC->msgID);
printf("Got to 3\n");
		}

		ret = BSM2vehcomm(BSMCACC, &comm_pkt);

		get_current_timestamp(&comm_pkt.ts);
 
////////////////////Make sure the comm_pkt is received from other trucks/////

                if (comm_pkt.object_id[0] != *vehicle_str) { 
                   msg_count++;

                 if (msg_count == 1)
                      rcvd_sn = comm_pkt.sequence_no; //initial value

                 if ((comm_pkt.sequence_no - rcvd_sn) == 1 )
                      rcvd_sn++;

                   else {
                     //  printf( "%hd packets lost.", (comm_pkt.sequence_no - rcvd_sn - 1) );
                       rcvd_sn = comm_pkt.sequence_no;
                    }

                 //  printf( "\nrcvd_sn: %hd.\n",rcvd_sn ); 
                 } 
////////////////////////////////////////////////////////////////////////////


		/** Check "my_pip" field in packet and write to 
		 *  DB_COMM_LEAD_TRK, DB_COMM_TRK_SECOND, and DB_COMM_TRK_THIRD if relevant 
		 * For second and third vehicles, both may be written from same packet
		 */
		if (comm_pkt.my_pip == lead_vehicle_id)  
			db_clt_write(pclt, DB_COMM_LEAD_TRK_VAR,
				sizeof(comm_pkt), &comm_pkt);
		if (comm_pkt.my_pip == second_vehicle_id)  
			db_clt_write(pclt, DB_COMM_SECOND_TRK_VAR,
				sizeof(comm_pkt), &comm_pkt);
		if (comm_pkt.my_pip == third_vehicle_id)  
			db_clt_write(pclt, DB_COMM_THIRD_TRK_VAR,
				sizeof(comm_pkt), &comm_pkt);
		if (verbose) {
			printf("Vehicle %d received packet from vehicle %d\n",
				self_vehicle_id, comm_pkt.my_pip);
                        printf("%s: %f\n", comm_pkt.object_id,
                               comm_pkt.global_time);
                        fflush(stdout);
                }

	}
		
	longjmp(exit_env,1);	/* go to exit code when loop terminates */
}

int BSM2vehcomm(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt) {

        BSMCACC->msgID = 0x20;
//      BSMCACC-> = comm_pkt->node;               // Node number of packet origin
//      BSMCACC-> = comm_pkt->rcv_ts;     // When message is received, from veh_recv
//      BSMCACC-> = comm_pkt->ts;         // When message is sent, from veh_send
//      BSMCACC-> = comm_pkt->global_time;      // From long_ctl or trk_comm_mgr
//      BSMCACC-> = comm_pkt->user_float;
//      BSMCACC-> = comm_pkt->user_float1;
//      BSMCACC-> = comm_pkt->char user_ushort_1;
//      BSMCACC-> = comm_pkt->char user_ushort_2;
//      BSMCACC-> = comm_pkt->char my_pip;  // My position-in-platoon (i.e. 1, 2, or 3)
//      BSMCACC-> = comm_pkt->char maneuver_id;
//      BSMCACC-> = comm_pkt->char fault_mode;
//      BSMCACC->caccData = comm_pkt->char maneuver_des_1;
//      BSMCACC-> = comm_pkt->char maneuver_des_2;
//      BSMCACC-> = comm_pkt->char pltn_size;
        comm_pkt->sequence_no = BSMCACC->blob1.msgCnt;

//      BSMCACC-> = comm_pkt->user_bit_1 : 1;
//      BSMCACC-> = comm_pkt->user_bit_2 : 1;
//      BSMCACC-> = comm_pkt->user_bit_3 : 1;
//      BSMCACC-> = comm_pkt->user_bit_4 : 1;
//      BSMCACC-> = comm_pkt->acc_traj;         //Desired acceleration from profile (m/s^2)
//      BSMCACC-> = comm_pkt->vel_traj;         //Desired velocity from profile (m/s)
//      BSMCACC-> = comm_pkt->velocity;         //Current velocity (m/s)
//      BSMCACC-> = comm_pkt->accel;            //Current acceleration (m/s^2)
        comm_pkt->range = BSMCACC->caccData.distToPVeh;            //Range from *dar (m)
        comm_pkt->rate = BSMCACC->caccData.relSpdPVeh;             //Relative velocity from *dar (m/s)
//      BSMCACC->blob1.id = "1"; //comm_pkt->object_id[GPS_OBJECT_ID_SIZE + 1];

        return 0;
}

