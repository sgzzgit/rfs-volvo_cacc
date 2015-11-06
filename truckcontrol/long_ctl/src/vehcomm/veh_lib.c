/**\file
 *	veh_lib.c 
 *		Library for interconversion of old "comm_packet_t" with J2735 "BSMCACC_t"
 *
 * Copyright (c) 2015   Regents of the University of California
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
#include "asn_SEQUENCE_OF.h" 
#include "BSMCACC.h"
 
 
int vehcomm2BSM(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt);

int user_Float, user_Float1;

int vehcomm2BSM(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt) {

	short speed;
	TemporaryID_t *myid;
	char myID[4] = "5";

	myid = (TemporaryID_t *)calloc(1, sizeof(TemporaryID_t));
	strcpy(myid, "476");

	BSMCACC->msgID = 2;
//	BSMCACC->blob1.lat =  123000000;
//	BSMCACC-> = comm_pkt->node;               // Node number of packet origin
//	BSMCACC-> = comm_pkt->rcv_ts;     // When message is received, from veh_recv
//	BSMCACC-> = comm_pkt->ts;         // When message is sent, from veh_send


	BSMCACC->caccData.globalTime = (int)(comm_pkt->global_time * 50);      // From long_ctl or trk_comm_mgr

	user_Float = (int)(comm_pkt->user_float * BSM_FLOAT_MULT);
	user_Float = 1234;
//	BSMCACC->caccData.userDF1 = (int *)&user_Float;;

//	user_Float1 = (int)(comm_pkt->user_float1 * BSM_FLOAT_MULT);
	user_Float1 = 5678;
//	BSMCACC->caccData.userDF2 = &user_Float1;

//	BSMCACC->caccData.userDI1 = (int *)&comm_pkt->user_ushort_1;
//	BSMCACC->caccData.userDI2 = (int *)&comm_pkt->user_ushort_2;
	BSMCACC->caccData.vehGrpPos = comm_pkt->my_pip;  // My position-in-platoon (i.e. 1, 2, or 3)
	BSMCACC->caccData.vehManID = comm_pkt->maneuver_id;
	BSMCACC->caccData.vehFltMode = comm_pkt->fault_mode;
	BSMCACC->caccData.vehManDes = comm_pkt->maneuver_des_1;
	BSMCACC->caccData.grpSize = comm_pkt->pltn_size;
	BSMCACC->blob1.msgCnt = comm_pkt->sequence_no;

	BSMCACC->caccData.userBit1 = comm_pkt->user_bit_1;
	BSMCACC->caccData.userBit2 = comm_pkt->user_bit_2;
	BSMCACC->caccData.userBit3 = comm_pkt->user_bit_3;
	BSMCACC->caccData.userBit4 = comm_pkt->user_bit_4;
	BSMCACC->caccData.desAcc = (int)(comm_pkt->acc_traj * BSM_FLOAT_MULT); 		//Desired acceleration from profile (m/s^2)
	BSMCACC->caccData.desSpd = (int)(comm_pkt->vel_traj * BSM_FLOAT_MULT);	//Desired velocity from profile (m/s)
	speed = (short)(comm_pkt->velocity * 50);				//Current velocity (m/s)
//	BSMCACC->blob1.speed =							//Current velocity (m/s)
//		OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING, comm_pkt->velocity, -1);
////	BSMCACC->blob1.accelSet.long = (int)(comm_pkt->accel * BSM_FLOAT_MULT);            //Current acceleration (m/s^2)
//	BSMCACC->blob1.accelSet.long = (int)(comm_pkt->accel * BSM_FLOAT_MULT);            //Current acceleration (m/s^2)
//		OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING, k, -1);
	BSMCACC->caccData.distToPVeh = (int)(comm_pkt->range * BSM_FLOAT_MULT);	//Range from *dar (m)
	BSMCACC->caccData.relSpdPVeh = (int)(comm_pkt->rate * BSM_FLOAT_MULT);	//Relative velocity from *dar (m/s)
//	BSMCACC->blob1.id = "1"; //comm_pkt->object_id[GPS_OBJECT_ID_SIZE + 1];
	BSMCACC->blob1.id = 
//		*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING, &myid, -1);
//		*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING, (TemporaryID_t *)&myID[0], 4);
		*OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING, comm_pkt->object_id, -1);

	return 0;
}

int BSM2vehcomm(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt) {

	if(BSMCACC->msgID == 2) {
//		BSMCACC-> = comm_pkt->node;               // Node number of packet origin
//      	BSMCACC-> = comm_pkt->rcv_ts;     // When message is received, from veh_recv
//      	BSMCACC-> = comm_pkt->ts;         // When message is sent, from veh_send
		comm_pkt->global_time = BSMCACC->caccData.globalTime / 50.0;      // From long_ctl or trk_comm_mgr
//      	BSMCACC-> = comm_pkt->user_float;
//      	BSMCACC-> = comm_pkt->user_float1;
//      	BSMCACC-> = comm_pkt->char user_ushort_1;
//      	BSMCACC-> = comm_pkt->char user_ushort_2;
		comm_pkt->my_pip = BSMCACC->caccData.vehGrpPos;  // My position-in-platoon (i.e. 1, 2, or 3)
		comm_pkt->maneuver_id = BSMCACC->caccData.vehManID;
		comm_pkt->fault_mode = BSMCACC->caccData.vehFltMode;
		comm_pkt->maneuver_des_1 = BSMCACC->caccData.vehManDes;
//      	BSMCACC-> = comm_pkt->char maneuver_des_2;
		comm_pkt->pltn_size = BSMCACC->caccData.grpSize;
        	comm_pkt->sequence_no = BSMCACC->blob1.msgCnt;
	
		comm_pkt->user_bit_1  = BSMCACC->caccData.userBit1;
		comm_pkt->user_bit_2  = BSMCACC->caccData.userBit2;
		comm_pkt->user_bit_3  = BSMCACC->caccData.userBit3;
		comm_pkt->user_bit_4  = BSMCACC->caccData.userBit4;
		comm_pkt->acc_traj = BSMCACC->caccData.desAcc / BSM_FLOAT_MULT; //Desired acceleration from profile (m/s^2)
		comm_pkt->vel_traj = BSMCACC->caccData.desSpd / BSM_FLOAT_MULT;	//Desired velocity from profile (m/s)
//      	BSMCACC-> = comm_pkt->velocity;         //Current velocity (m/s)
//      	BSMCACC-> = comm_pkt->accel;            //Current acceleration (m/s^2)
        	comm_pkt->range = BSMCACC->caccData.distToPVeh / BSM_FLOAT_MULT;	//Range from *dar (m)
        	comm_pkt->rate = BSMCACC->caccData.relSpdPVeh / BSM_FLOAT_MULT;             //Relative velocity from *dar (m/s)
//      	BSMCACC->blob1.id = "1"; //comm_pkt->object_id[GPS_OBJECT_ID_SIZE + 1];

        	return 0;
	}
	else {
		printf("BSMCACC->msgID != 2\n");
		return -1;
	}
}
