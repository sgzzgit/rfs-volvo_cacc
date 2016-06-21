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
//} IS_PACKED;
};

 

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
	struct SeretUdpStruct dvi_out;
	struct ExtraDataCACCStruct egodata;
	veh_comm_packet_t comm_pkt1;
	veh_comm_packet_t comm_pkt2;
	veh_comm_packet_t comm_pkt3;
	veh_comm_packet_t self_comm_pkt;
	char self_comm_pkt_user_ushort_2;
	char self_comm_pkt_user_bit_3;
	char self_comm_pkt_maneuver_des_2;
	char self_comm_pkt_my_pip;
	char truck1_comm_pkt_user_ushort_2;
	char truck1_comm_pkt_user_bit_3;
	char truck1_comm_pkt_maneuver_des_2;
	char truck1_comm_pkt_my_pip;
	char truck2_comm_pkt_user_ushort_2;
	char truck2_comm_pkt_user_bit_3;
	char truck2_comm_pkt_maneuver_des_2;
	char truck2_comm_pkt_my_pip;
	char truck3_comm_pkt_user_ushort_2;
	char truck3_comm_pkt_user_bit_3;
	char truck3_comm_pkt_maneuver_des_2;
	char truck3_comm_pkt_my_pip;
	unsigned char update_ts1 = 0;
	unsigned char update_ts2 = 0;
	unsigned char update_ts3 = 0;

	char *self_packet_str = NULL;
	char *truck1_packet_str = NULL;
	char *truck2_packet_str = NULL;
	char *truck3_packet_str = NULL;
	short msg_count = 0;

	posix_timer_typ *ptmr;
	int interval = 50;	/// milliseconds

	memset(&dvi_out, 0, sizeof(struct SeretUdpStruct));
	memset(&egodata, 0, sizeof(struct ExtraDataCACCStruct));
	while ((ch = getopt(argc, argv, "S:1:2:3:")) != EOF) {
                switch (ch) {
		case 'S': self_packet_str = strdup(optarg);
			  break;
		case '1': truck1_packet_str = strdup(optarg);
			  break;
		case '2': truck2_packet_str = strdup(optarg);
			  break;
		case '3': truck3_packet_str = strdup(optarg);
			  break;
                default:  printf("Usage: %s -S,1,2,3 \"user_ushort_2(CACCState)=<0-stay, 1-manual,  2-ACC,  3-CACC> user_bit_3(brake switch)=<0,1> maneuver_des_2(Cut in)=<0,1-cut-in,2=cut-out> my_pip(my platoon position)=<1,2,3> update_ts(get_current_timestamp to check comm updates)=<0,1>\"\n",argv[0]);
			  printf("So for instance, the input for the self-vehicle might be:\n\t%s -S \"user_ushort_2=3 user_bit_3=0 maneuver_des_2=1 my_pip=2 update_ts=1\"\n", argv[0]);
			  exit(EXIT_FAILURE);
                          break;
                }
        }

	if(self_packet_str != NULL) {
	    sscanf(self_packet_str, "user_ushort_2=%hhu user_bit_3=%hhu maneuver_des_2=%hhu my_pip=%hhu", 
		&self_comm_pkt_user_ushort_2,
		&self_comm_pkt_user_bit_3,
		&self_comm_pkt_maneuver_des_2,
		&self_comm_pkt_my_pip
	    );
		self_comm_pkt.user_ushort_2= self_comm_pkt_user_ushort_2 & 0x03;
		self_comm_pkt.user_bit_3 = self_comm_pkt_user_bit_3 & 0x01;
		self_comm_pkt.maneuver_des_2 = self_comm_pkt_maneuver_des_2 & 0x03;
		self_comm_pkt.my_pip = self_comm_pkt_my_pip & 0x03;
	}

	if(truck1_packet_str != NULL) {
	    sscanf(truck1_packet_str, "user_ushort_2=%hhu user_bit_3=%hhu maneuver_des_2=%hhu my_pip=%hhu update_ts=%hhu", 
		&truck1_comm_pkt_user_ushort_2,
		&truck1_comm_pkt_user_bit_3,
		&truck1_comm_pkt_maneuver_des_2,
		&truck1_comm_pkt_my_pip,
		&update_ts1 
	    );
		comm_pkt1.user_ushort_2= truck1_comm_pkt_user_ushort_2 & 0x03;
		comm_pkt1.user_bit_3 = truck1_comm_pkt_user_bit_3 & 0x01;
		comm_pkt1.maneuver_des_2 = truck1_comm_pkt_maneuver_des_2 & 0x03;
		comm_pkt1.my_pip = truck1_comm_pkt_my_pip & 0x03;
                printf("comm_pkt1: user_ushort_2(CACCState)=%hhu user_bit_3(brake switch)=%hhu maneuver_des_2(Cut in)=%hhu my_pip(my platoon position)=%hhu update_ts(get_current_timestamp to check comm updates)=%hhu\"\n",
			comm_pkt1.user_ushort_2,
			comm_pkt1.user_bit_3,
			comm_pkt1.maneuver_des_2,
			comm_pkt1.my_pip,
			update_ts1 
		);
	}
	if(update_ts1 != 0)
		get_current_timestamp(&comm_pkt1.ts);

	if(truck2_packet_str != NULL) {
	    sscanf(truck2_packet_str, "user_ushort_2=%hhu user_bit_3=%hhu maneuver_des_2=%hhu my_pip=%hhu update_ts=%hhu", 
		&truck2_comm_pkt_user_ushort_2,
		&truck2_comm_pkt_user_bit_3,
		&truck2_comm_pkt_maneuver_des_2,
		&truck2_comm_pkt_my_pip,
		&update_ts2
	    );
		comm_pkt2.user_ushort_2= truck2_comm_pkt_user_ushort_2 & 0x03;
		comm_pkt2.user_bit_3 = truck2_comm_pkt_user_bit_3 & 0x01;
		comm_pkt2.maneuver_des_2 = truck2_comm_pkt_maneuver_des_2 & 0x03;
		comm_pkt2.my_pip = truck2_comm_pkt_my_pip & 0x03;
                printf("comm_pkt2: user_ushort_2(CACCState)=%d user_bit_3(brake switch)=%d maneuver_des_2(Cut in)=%d my_pip(my platoon position)=%d update_ts(get_current_timestamp to check comm updates)=%d\"\n",
			comm_pkt2.user_ushort_2,
			comm_pkt2.user_bit_3,
			comm_pkt2.maneuver_des_2,
			comm_pkt2.my_pip,
			update_ts2 
		);
	}
	if(update_ts2 != 0)
		get_current_timestamp(&comm_pkt2.ts);

	if(truck3_packet_str != NULL) {
	    sscanf(truck3_packet_str, "user_ushort_2=%hhu user_bit_3=%hhu maneuver_des_2=%hhu my_pip=%hhu update_ts=%hhu", 
		&truck3_comm_pkt_user_ushort_2,
		&truck3_comm_pkt_user_bit_3,
		&truck3_comm_pkt_maneuver_des_2,
		&truck3_comm_pkt_my_pip,
		&update_ts3
	    );
		comm_pkt3.user_ushort_2= truck3_comm_pkt_user_ushort_2 & 0x03;
		comm_pkt3.user_bit_3 = truck3_comm_pkt_user_bit_3 & 0x01;
		comm_pkt3.maneuver_des_2 = truck3_comm_pkt_maneuver_des_2 & 0x03;
		comm_pkt3.my_pip = truck3_comm_pkt_my_pip & 0x03;
                printf("comm_pkt3: user_ushort_2(CACCState)=%d user_bit_3(brake switch)=%d maneuver_des_2(Cut in)=%d my_pip(my platoon position)=%d update_ts(get_current_timestamp to check comm updates)=%d\"\n",
			comm_pkt3.user_ushort_2,
			comm_pkt3.user_bit_3,
			comm_pkt3.maneuver_des_2,
			comm_pkt3.my_pip,
			update_ts3 
		);
	}
	if(update_ts3 != 0)
		get_current_timestamp(&comm_pkt3.ts);

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

		if(truck1_packet_str != NULL)
			db_clt_write(pclt, DB_COMM_LEAD_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt1);
		if(truck2_packet_str != NULL)
			db_clt_write(pclt, DB_COMM_SECOND_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt2);
		if(truck3_packet_str != NULL)
			db_clt_write(pclt, DB_COMM_THIRD_TRK_VAR, sizeof(veh_comm_packet_t), &comm_pkt3);
		if(self_packet_str != NULL)
			db_clt_write(pclt, DB_COMM_TX_VAR, sizeof(veh_comm_packet_t), &self_comm_pkt);
}
