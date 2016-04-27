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
#include "veh_lib.h"

#include "asn_application.h"
#include "asn_internal.h"       /* for _ASN_DEFAULT_STACK_MAX */
#include "asn_SEQUENCE_OF.h" 
#include "BSMCACC.h"
 
 
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
	int udp_port = 5050;

	veh_comm_packet_t comm_pkt;
	BSMCACC_t *BSMCACC;
	BSMCACC_t *BSMCACC_decode;
#define BSMCACCSIZE	3000
	char BSMCACC_buf[BSMCACCSIZE];

	asn_enc_rval_t erv;
	asn_dec_rval_t rval;

        int bytes_sent;     		/// received from a call to sendto
	int verbose = 0;
	int debug = 0;
	short msg_count = 0;
	char *remote_ipaddr = "10.0.1.9";	/// address of UDP destination
	char *local_ipaddr = "127.0.0.1";	/// address of UDP destination
	struct sockaddr_in dst_addr;
	char *vehicle_str = "Blue";
	posix_timer_typ *ptmr;
	int interval = 20;	/// milliseconds
	int do_broadcast = 0;	/// by default do unicast
	int ret = -1;
	int counter = 0;
	float fcounter = 10.0;
	int i;
	BSMblob_t *my_blob;
//	CaccData_t *my_caccdata;
//	DDateTime_t *my_datetime;
//	VehicleSize_t *my_vehiclesize;

        //Allocate memory for J2735 BSMCACC message
        BSMCACC = (BSMCACC_t *)calloc(1, sizeof(BSMCACC_t));
	my_blob = (BSMblob_t *)calloc(1, sizeof(BSMblob_t));
//	my_caccdata = (CaccData_t *)calloc(1, sizeof(CaccData_t));
//	my_datetime = (DDateTime_t *)calloc(1, sizeof(DDateTime_t));
//	my_vehiclesize = (VehicleSize_t *)calloc(1, sizeof(VehicleSize_t));

	//Now add the parts of BSMCACC to it
//        asn_sequence_add(my_blob, my_vehiclesize);
//        asn_sequence_add(&BSMCACC->caccData, my_datetime);
        asn_sequence_add(&BSMCACC->blob1, my_blob);
//        asn_sequence_add(&BSMCACC->caccData, my_caccdata);

        BSMCACC_decode = (BSMCACC_t *)calloc(1, sizeof(BSMCACC_t));

        while ((ch = getopt(argc, argv, "A:a:bi:t:u:vd")) != EOF) {
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
		case 'u': udp_port = atoi(optarg); 
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

	if ( (sd = udp_unicast_init(&dst_addr, remote_ipaddr, local_ipaddr, udp_port)) < 0) {
 
		printf("Failure to initialize socket from %s to %s on port %d\n",
			remote_ipaddr, local_ipaddr, udp_port);
		longjmp(exit_env, 2);
	}
	while (1) {
		db_clt_read(pclt, DB_COMM_TX_VAR, sizeof(comm_pkt), &comm_pkt);

		set_vehicle_string(&comm_pkt, vehicle_str);

                msg_count++;
		comm_pkt.sequence_no = msg_count;
		if(debug) {
			fcounter += 0.02;
			counter = (int)(fcounter * 100);
			comm_pkt.global_time = fcounter;
			comm_pkt.user_float = fcounter;
			comm_pkt.user_float1 = fcounter;
			comm_pkt.user_ushort_1 = (short)(fcounter * 100);
			comm_pkt.user_ushort_2 = (short)(fcounter * 100);
			comm_pkt.my_pip = counter; 
			comm_pkt.maneuver_id = counter;
			comm_pkt.fault_mode = counter;
			comm_pkt.maneuver_des_1 = (short)(fcounter * 100);
			comm_pkt.maneuver_des_2 = (short)(fcounter * 100);
			comm_pkt.pltn_size = counter;
			comm_pkt.sequence_no = msg_count;
			comm_pkt.user_bit_1 = counter;
			comm_pkt.user_bit_2 = counter;
			comm_pkt.user_bit_3 = counter;
			comm_pkt.user_bit_4 = counter;
			comm_pkt.acc_traj = fcounter;
			comm_pkt.vel_traj = fcounter;
			comm_pkt.velocity = fcounter;
			comm_pkt.accel = fcounter;
			comm_pkt.range = fcounter;
			comm_pkt.rate = fcounter;
			comm_pkt.object_id[GPS_OBJECT_ID_SIZE + 1] = "barf";
		}

		if(verbose)
printf("Got to 2 msgID %#x seq_no %d global_time %f userbit1 %d userbit2 %d userbit3 %d userbit4 %d my_pip %d maneuver_id %d fault_mode %d maneuver_des_1 %d pltn_size %d acc_traj %f veh_traj %f range %f rate %f velocity %f acceleration %f\n", 
		BSMCACC->msgID,
		comm_pkt.sequence_no,
		comm_pkt.global_time,
		comm_pkt.user_bit_1,
		comm_pkt.user_bit_2,
		comm_pkt.user_bit_3,
		comm_pkt.user_bit_4,
		comm_pkt.my_pip,
		comm_pkt.maneuver_id,
		comm_pkt.fault_mode,
		comm_pkt.maneuver_des_1,
		comm_pkt.pltn_size,
		comm_pkt.acc_traj,
		comm_pkt.vel_traj,
		comm_pkt.range,
		comm_pkt.rate,
		comm_pkt.velocity,
		comm_pkt.accel
		);

		get_current_timestamp(&comm_pkt.ts);
		memset(BSMCACC, 0, sizeof(BSMCACC_t));
		memset(BSMCACC_buf, 0, BSMCACCSIZE);

		ret = vehcomm2BSM(BSMCACC, &comm_pkt);
		erv = der_encode_to_buffer(&asn_DEF_BSMCACC, BSMCACC, BSMCACC_buf, sizeof(BSMCACC_t));

		if(erv.encoded < 0) {
			fprintf(stderr, "%s: Cannot convert into DER\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	
                bytes_sent = sendto(sd, &BSMCACC_buf, erv.encoded,
			 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));
		if (verbose) {
                BSMCACC_decode = 0;
                rval = ber_decode(0, &asn_DEF_BSMCACC,(void **)&BSMCACC_decode, &BSMCACC_buf[0], BSMCACCSIZE);
                if(rval.code != RC_OK) {
                        fprintf(stderr, "%s:Cannot decode received message. Bytes consumed %d\n", argv[0], rval.consumed);
                        exit(EXIT_FAILURE);
                }
                ret = BSM2vehcomm(BSMCACC_decode, &comm_pkt);

                //Print out BSMCACC if desired
                if(verbose) {
                        xer_fprint(stdout, &asn_DEF_BSMCACC, BSMCACC_decode);
                        printf("BSMCACC->msgID %d\n", (int)BSMCACC_decode->msgID);
                        printf("\n");
                        for(i=0; i < rval.consumed; i++)
                                printf("%02hhx ", BSMCACC_buf[i]);
                        printf("\n");
                }

/*
			printf("%s %d: %f\n", comm_pkt.object_id,
				comm_pkt.my_pip,
				comm_pkt.global_time);
		BSMCACC_decode = 0;
			rval = ber_decode(0, &asn_DEF_BSMCACC,(void **)&BSMCACC_decode, &BSMCACC_buf[0], erv.encoded);
			ret = BSM2vehcomm(BSMCACC_decode, &comm_pkt_decode);
printf("Got to 3 msgID %#x seq_no %d global_time %f userbit1 %d userbit2 %d userbit3 %d userbit4 %d my_pip %d maneuver_id %d fault_mode %d maneuver_des_1 %d pltn_size %d acc_traj %f veh_traj %f range %f rate %f velocity %f acceleration %f\n", 
		BSMCACC_decode->msgID,
		comm_pkt_decode.sequence_no,
		comm_pkt_decode.global_time,
		comm_pkt_decode.user_bit_1,
		comm_pkt_decode.user_bit_2,
		comm_pkt_decode.user_bit_3,
		comm_pkt_decode.user_bit_4,
		comm_pkt_decode.my_pip,
		comm_pkt_decode.maneuver_id,
		comm_pkt_decode.fault_mode,
		comm_pkt_decode.maneuver_des_1,
		comm_pkt_decode.pltn_size,
		comm_pkt_decode.acc_traj,
		comm_pkt_decode.vel_traj,
		comm_pkt_decode.range,
		comm_pkt_decode.rate,
		comm_pkt_decode.velocity,
		comm_pkt_decode.accel
		);
			ret = xer_fprint(stdout, &asn_DEF_BSMCACC, &BSMCACC_decode);
			if(ret >= 0) {
printf("Got to 3 BSMCACC_decode.msgID %#x msgCnt %d lat %d\n", 
	(unsigned int)BSMCACC_decode->msgID,
	(unsigned int)BSMCACC_decode->blob1.msgCnt,
	(unsigned int)BSMCACC_decode->blob1.lat);
//::printf("BSMCACC_decode->msgID %d globalTime %f\n", BSMCACC_decode->msgID, BSMCACC->caccData.globalTime/50.0);      // From long_ctl or trk_comm_mgr
			}
			else {
				printf("xer_fprint returned an error\n");
			}
//			printf("\n");
//			for(i=0; i < erv.encoded ; i++) 
//				printf("%02hhx ", BSMCACC_buf[i]);
//			printf("\n");
*/
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
