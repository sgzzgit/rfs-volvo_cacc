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

#include "asn_application.h"
#include "asn_internal.h"       /* for _ASN_DEFAULT_STACK_MAX */
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

int vehcomm2BSM(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt);
static int write_out(const void *buffer, size_t size, void *key);

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
	int udp_port = 5050;

	veh_comm_packet_t comm_pkt;
	BSMCACC_t *BSMCACC;
#define BSMCACCSIZE	10000
	char BSMCACC_buf[BSMCACCSIZE];

	static asn_TYPE_descriptor_t PDU_Type;
	static asn_TYPE_descriptor_t *pduType = &PDU_Type;
	asn_enc_rval_t erv;
	void *structure;    /* Decoded structure */

        int bytes_sent;     		/// received from a call to sendto
	int verbose = 0;
	short msg_count = 0;
	char *remote_ipaddr = "10.0.1.9";	/// address of UDP destination
	char *local_ipaddr = "127.0.0.1";	/// address of UDP destination
	struct sockaddr_in dst_addr;
	char *vehicle_str = "Blue";
	posix_timer_typ *ptmr;
	int interval = 20;	/// milliseconds
	int do_broadcast = 0;	/// by default do unicast
	int ret = -1;

        BSMCACC = (BSMCACC_t *)calloc(1, sizeof(BSMCACC_t));

        while ((ch = getopt(argc, argv, "A:a:bi:t:u:v")) != EOF) {
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

//	if ( (sd = udp_init(ipaddr, udp_port, &dst_addr, do_broadcast)) < 0) {
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

		get_current_timestamp(&comm_pkt.ts);

		ret = vehcomm2BSM(BSMCACC, &comm_pkt);
//printf("Got to 1\n");
		erv = der_encode_to_buffer(&asn_DEF_BSMCACC, BSMCACC, &BSMCACC_buf, sizeof(BSMCACC_t));

/*

ssize_t AsnJ2735Lib::encode_bsm_payload(const BSM_element_t* ps_bsm,char* ptr,const size_t size,bool withHeader) const
{
        asn_enc_rval_t rval;    // Encoder return value

        // BasicSafetyMessage::msgID (INTEGER_t)(size 1)
        asn_long2INTEGER(&(pbsm->msgID),DSRCmsgID_basicSafetyMessage);
        // fill mpBsmEncode
        u_char blob1[BSMBLOB1SIZE] = {};
        int offset = encode_bsmblob1_payload(&(ps_bsm->bolb1_element),blob1);
        // BasicSafetyMessage::blob1 (OCTET_STRING_t)
        OCTET_STRING_fromBuf(&(pbsm->blob1),(char*)blob1,offset);
        // BasicSafetyMessage::*safetyExt       (OPTIONAL)
        // BasicSafetyMessage*status (OPTIONAL)

        // encode BSM
        rval = der_encode_to_buffer(&asn_DEF_BasicSafetyMessage, pbsm, ptr, size);
        // free pbsm
        SEQUENCE_free(&asn_DEF_BasicSafetyMessage, pbsm, 0);
*/
printf("Got to 2 BSMCACC.msgID %#x %d\n", BSMCACC->msgID, erv.encoded) ;
		if(erv.encoded < 0) {
			fprintf(stderr, "%s: Cannot convert %s into DER\n", argv[0], pduType->name);
			exit(EXIT_FAILURE);
		}
	
//printf("Got to 3\n");
                bytes_sent = sendto(sd, &BSMCACC_buf, erv.encoded,
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


int vehcomm2BSM(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt) {

	BSMCACC->msgID = 0x20;
//	BSMCACC-> = comm_pkt->node;               // Node number of packet origin
//	BSMCACC-> = comm_pkt->rcv_ts;     // When message is received, from veh_recv
//	BSMCACC-> = comm_pkt->ts;         // When message is sent, from veh_send
//	BSMCACC-> = comm_pkt->global_time;      // From long_ctl or trk_comm_mgr
//	BSMCACC-> = comm_pkt->user_float;
//	BSMCACC-> = comm_pkt->user_float1;
//	BSMCACC-> = comm_pkt->char user_ushort_1;
//	BSMCACC-> = comm_pkt->char user_ushort_2;
//	BSMCACC-> = comm_pkt->char my_pip;  // My position-in-platoon (i.e. 1, 2, or 3)
//	BSMCACC-> = comm_pkt->char maneuver_id;
//	BSMCACC-> = comm_pkt->char fault_mode;
//	BSMCACC->caccData = comm_pkt->char maneuver_des_1;
//	BSMCACC-> = comm_pkt->char maneuver_des_2;
//	BSMCACC-> = comm_pkt->char pltn_size;
	BSMCACC->blob1.msgCnt = comm_pkt->sequence_no;

//	BSMCACC-> = comm_pkt->user_bit_1 : 1;
//	BSMCACC-> = comm_pkt->user_bit_2 : 1;
//	BSMCACC-> = comm_pkt->user_bit_3 : 1;
//	BSMCACC-> = comm_pkt->user_bit_4 : 1;
//	BSMCACC-> = comm_pkt->acc_traj;         //Desired acceleration from profile (m/s^2)
//	BSMCACC-> = comm_pkt->vel_traj;         //Desired velocity from profile (m/s)
//	BSMCACC-> = comm_pkt->velocity;         //Current velocity (m/s)
//	BSMCACC-> = comm_pkt->accel;            //Current acceleration (m/s^2)
	BSMCACC->caccData.distToPVeh = comm_pkt->range;            //Range from *dar (m)
	BSMCACC->caccData.relSpdPVeh = comm_pkt->rate;             //Relative velocity from *dar (m/s)
//	BSMCACC->blob1.id = "1"; //comm_pkt->object_id[GPS_OBJECT_ID_SIZE + 1];


	return 0;
}

/* Dump the buffer out to the specified FILE */
static int write_out(const void *buffer, size_t size, void *key) {
	FILE *fp = (FILE *)key;
	return (fwrite(buffer, 1, size, fp) == size) ? 0 : -1;
}
