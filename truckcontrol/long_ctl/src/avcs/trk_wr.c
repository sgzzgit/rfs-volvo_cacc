/* \file 
 *
 * 	Copyright (c) 2008   Regents of the University of California
 *
 *	This process use the long_read_vehicle_state function
 *	in ../common/long_utils.c to read all the input data server variables
 *	to the long_trk process, and in addition read's the long_trk's
 *	processes output variables. It save this data in a circular
 *	buffer, and prints the data to a file when it exits.
 *
 *	Assumes that trk_cr has been run to create all variables before
 *	trk_wr is started.
 */

#include <sys_os.h>
#include <timestamp.h>
#include <path_gps_lib.h>
#include <sys_rt.h>
#include <sys_list.h>
#include <db_clt.h>
#include <clt_vars.h>	
#include <db_utils.h>
#include <jbus_vars.h>
#include <path_gps_lib.h>
#include <long_ctl.h>
#include <evt300.h>
#include <mdl.h>
#include <densolidar.h>
#include <long_comm.h>
#include <veh_trk.h>

static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else
		longjmp(exit_env, code);
}

/** uses long_ctrl type from long_ctl/src/common directory,
 * as the structure for keeping cbuff_typ,
 * but constructs buffer_item differently than in
 * long_ctl/src/template/tasks.h
 * long_vehicle_state includes all inputs to the long_trk process
 * the long_output_typ, long_dig_out_typ and veh_comm_packet_t fields
 * are the outputs from that process 
 */
typedef struct {
	double process_time;
	timestamp_t ts;
	long_vehicle_state state;
	long_output_typ cmd;
	long_dig_out_typ dig_out;
	veh_comm_packet_t comm_tx;
} buffer_item;

static void gather_data(long_ctrl *pctrl, long_output_typ *pout,
		long_dig_out_typ *pdig_out, veh_comm_packet_t *pcomm_tx)
{
        long_vehicle_state *pv = &pctrl->vehicle_state;
        cbuff_typ *pbuff = &pctrl->buff;
        buffer_item *pdata = (buffer_item *) pbuff->data_array;
        struct timeb current_time;
        buffer_item current_item;
	int index = get_circular_index(pbuff);
	struct timeb start_time = pctrl->start_time;

	ftime(&current_time);
	current_item.state = *pv;
	current_item.cmd = *pout;
	current_item.dig_out= *pdig_out;
	current_item.comm_tx= *pcomm_tx;
	current_item.process_time =
		TIMEB_SUBTRACT(&start_time, &current_time);
	get_current_timestamp(&current_item.ts);
	pdata[index] = current_item;
}

/**
 *	 Prints out stored data to stdout.
 *	 (Take care of file names in script.)
 */
static void print_data(FILE *fp, long_ctrl *pctrl)
{
	cbuff_typ *pbuff = &pctrl->buff;
	buffer_item *pdata = (buffer_item *) pbuff->data_array;
	int i, j;
	int current_index;

	if ((pdata == NULL) || (pbuff->data_count == 0))
		return;

	current_index = pbuff->data_start;
	
	for (i = 0; i < pbuff->data_count; i++) {
		buffer_item *p = &pdata[current_index]; 
		long_vehicle_state *pv = &p->state;
		long_output_typ *pcmd = &p->cmd;
		long_dig_out_typ *pdig_out = &p->dig_out;
		veh_comm_packet_t *pcomm_tx = &p->comm_tx;

	print_timestamp(fp, &p->ts);					//1
	fprintf(fp, " %.3f ", p->process_time);

	///EEC1
	fprintf(fp, "%.3f ", pv->EEC1_EngineSpeed);
	fprintf(fp, "%.3f ", pv->EEC1_DrvrDemandEngPercentTorque);
	fprintf(fp, "%.3f ", pv->EEC1_ActualEnginePercentTorque);	//5
	fprintf(fp, "%.3f ", pv->EEC1_EngDemandPercentTorque);
	fprintf(fp, "%d ", pv->EEC1_EngineTorqueMode);

	///EEC2
	fprintf(fp, "%d ", pv->EEC2_AccelPedal1LowIdleSwitch);	   
	fprintf(fp, "%d ", pv->EEC2_AccelPedal2LowIdleSwitch);	   
	fprintf(fp, "%d ", pv->EEC2_AccelPedalKickdownSwitch);		//10 
	fprintf(fp, "%.3f ", pv->EEC2_AccelPedalPos1);	   
	fprintf(fp, "%.3f ", pv->EEC2_AccelPedalPos2);	   
	fprintf(fp, "%d ", pv->EEC2_ActMaxAvailEngPercentTorque);	   
	fprintf(fp, "%.3f ", pv->EEC2_EnginePercentLoadAtCurrentSpd);
	fprintf(fp, "%d ", pv->EEC2_RoadSpeedLimitStatus);		//15 

	///EEC3
	fprintf(fp, "%.3f ", pv->EEC3_NominalFrictionPercentTorque);
	fprintf(fp, "%.3f ", pv->EEC3_EstEngPrsticLossesPercentTorque);
	fprintf(fp, "%.3f ", pv->EEC3_EngsDesiredOperatingSpeed);


	///EBC1
	fprintf(fp, "%.3f ", pv->EBC1_BrakePedalPosition);
	fprintf(fp, "%d ", pv->EBSBrakeSwitch); 			//20
	fprintf(fp, "%d ", pv->ABSEBSAmberWarningSignal);
	fprintf(fp, "%d ", pv->EBC1_ABSFullyOperational);

	fprintf(fp, "%d ", pv->EBC1_AntiLockBrakingActive);	  
	fprintf(fp, "%d ", pv->EBC1_ASRBrakeControlActive);	   
	fprintf(fp, "%d ", pv->EBC1_ASREngineControlActive);	    	//25
	fprintf(fp, "%d ", pv->EBC1_ASROffroadSwitch);	   

	fprintf(fp, "%d ", pv->EBC1_EBSRedWarningSignal);	   
	fprintf(fp, "%.2f ", pv->EBC1_EngRetarderSelection);	   
	fprintf(fp, "%d ", pv->EBC1_RemoteAccelEnableSwitch);	   
	fprintf(fp, "%d ", pv->EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl);	//30	 
	///EBC2
	fprintf(fp, "%.3f ", pv->EBC2_FrontAxleSpeed);	
	fprintf(fp, "%.3f ", pv->EBC2_RelativeSpeedFrontAxleLeftWheel);
	fprintf(fp, "%.3f ", pv->EBC2_RlativeSpeedFrontAxleRightWheel);
	fprintf(fp, "%.3f ", pv->EBC2_RelativeSpeedRearAxle1LeftWheel);
	fprintf(fp, "%.3f ", pv->EBC2_RlativeSpeedRearAxle1RightWheel);	//35
	fprintf(fp, "%.3f ", pv->EBC2_RelativeSpeedRearAxle2LeftWheel);
	fprintf(fp, "%.3f ", pv->EBC2_RlativeSpeedRearAxle2RightWheel);

	///CCVS
	fprintf(fp, "%.3f ", pv->CCVS_VehicleSpeed);
	fprintf(fp, "%.3f ", pv->CCVS_CruiseControlSetSpeed);
	fprintf(fp, "%d ", pv->CCVS_TwoSpeedAxleSwitch);		//40
	fprintf(fp, "%d ", pv->CCVS_ParkingBrakeSwitch);
	fprintf(fp, "%d ", pv->CCVS_CruiseCtrlPauseSwitch);
	fprintf(fp, "%d ", pv->CCVS_ParkBrakeReleaseInhibitRq);
	fprintf(fp, "%d ", pv->CCVS_CruiseControlActive);
	fprintf(fp, "%d ", pv->CCVS_CruiseControlEnableSwitch);		//45
	fprintf(fp, "%d ", pv->CCVS_BrakeSwitch);
	fprintf(fp, "%d ", pv->CCVS_ClutchSwitch);
	fprintf(fp, "%d ", pv->CCVS_CruiseControlSetSwitch);
	fprintf(fp, "%d ", pv->CCVS_CruiseControlCoastSwitch);
	fprintf(fp, "%d ", pv->CCVS_CruiseControlResumeSwitch);		//50
	fprintf(fp, "%d ", pv->CCVS_CruiseControlAccelerateSwitch);
	fprintf(fp, "%d ", pv->CCVS_PtoState);
	fprintf(fp, "%d ", pv->CCVS_CruiseControlState);
	fprintf(fp, "%d ", pv->CCVS_EngIdleIncrementSwitch);
	fprintf(fp, "%d ", pv->CCVS_EngIdleDecrementSwitch);		//55
	fprintf(fp, "%d ", pv->CCVS_EngTestModeSwitch);
	fprintf(fp, "%d ", pv->CCVS_EngShutdownOverrideSwitch);

	///ETC1
	fprintf(fp, "%d ", pv->ETC1_TransmissionShiftInProcess);
	fprintf(fp, "%.3f ", pv->ETC1_TransInputShaftSpeed);
	fprintf(fp, "%.3f ", pv->ETC1_TransmissionOutputShaftSpeed);	//60
	fprintf(fp, "%d ", pv->ETC1_TorqueConverterLockupEngaged);
	fprintf(fp, "%d ", pv->ETC1_ProgressiveShiftDisable);
	fprintf(fp, "%d ", pv->ETC1_TransmissionDrivelineEngaged);
	fprintf(fp, "%d ", pv->ETC1_MomentaryEngineOverspeedEnable);
	fprintf(fp, "%.3f ", pv->ETC1_PercentClutchSlip);		//65
	fprintf(fp, "%d ", pv->ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl);

	///ETC2
	fprintf(fp, "%d ", pv->ETC2_TransmissionSelectedGear);
	fprintf(fp, "%.3f ", pv->ETC2_TransmissionActualGearRatio);
	fprintf(fp, "%d ", pv->ETC2_TransmissionCurrentGear);
	fprintf(fp, "%d ", pv->ETC2_TransmissionSelectedGear); 		//70
	fprintf(fp, "%d ", pv->ETC2_TransmissionRangeAttained);
 
	///LFE
	fprintf(fp, "%.3f ", pv->LFE_EngineAverageFuelEconomy);
	fprintf(fp, "%.3f ", pv->LFE_EngineFuelRate);
	fprintf(fp, "%.3f ", pv->LFE_EngineInstantaneousFuelEconomy);	   
	fprintf(fp, "%.3f ", pv->LFE_EngineThrottleValve1Position);	//75

		///FD
		fprintf(fp, "%d ", pv->fan_drive_state);
		fprintf(fp, "%.3f ", pv->estimated_percent_fan_speed);

	///ERC1 (TRANS)
	fprintf(fp, "%d ", pv->ERC1ERRetarderEnableShiftAssistSw);
	fprintf(fp, "%d ", pv->ERC1ERRtdrEnablBrakeAssistSwitch);
	fprintf(fp, "%d ", pv->ERC1ERRetarderTorqueMode);		//80
	fprintf(fp, "%.2f ", pv->ERC1ERActualEngineRetPercentTrq);
	fprintf(fp, "%.2f ", pv->ERC1ERIntendedRtdrPercentTorque);
	fprintf(fp, "%d ", pv->ERC1ERRetarderRqingBrakeLight);
	fprintf(fp, "%d ", pv->ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl);
	fprintf(fp, "%hhd ", pv->ERC1ERDrvrsDmandRtdrPerctTorque);	//85
	fprintf(fp, "%d ", pv->ERC1ERRetarderSelectionNonEng);
	fprintf(fp, "%hhd ", pv->ERC1ERActlMxAvlbRtdrtPerctTorque);
		
	// Transmission control of engine
	fprintf(fp, "%d ", pv->TSC1_E_T_EnOvrdCtrlM);
	fprintf(fp, "%d ", pv->TSC1_E_T_EnRSpdCtrlC);
	fprintf(fp, "%d ", pv->TSC1_E_T_EnRSpdSpdLm);	//90
	fprintf(fp, "%d ", pv->TSC1_E_T_EnRTrqTrqLm);
	fprintf(fp, "%d ", pv->TSC1_E_T_OvrdCtrlMPr);

	// Transmission control of engine retarder
	fprintf(fp, "%d ", pv->TSC1_ER_T_EnOvrdCtrlM);
	fprintf(fp, "%d ", pv->TSC1_ER_T_EnRSpdCtrlC);
	fprintf(fp, "%d ", pv->TSC1_ER_T_EnRSpdSpdLm);	//95
	fprintf(fp, "%d ", pv->TSC1_ER_T_EnRTrqTrqLm);
	fprintf(fp, "%d ", pv->TSC1_ER_T_OvrdCtrlMPr);

	// CC control of engine retarder                                
	fprintf(fp, "%d ", pv->TSC1_ER_V_EnOvrdCtrlM);
	fprintf(fp, "%d ", pv->TSC1_ER_V_EnRSpdCtrlC);
	fprintf(fp, "%d ", pv->TSC1_ER_V_EnRSpdSpdLm);		//100
	fprintf(fp, "%d ", pv->TSC1_ER_V_EnRTrqTrqLm);
	fprintf(fp, "%d ", pv->TSC1_ER_V_OvrdCtrlMPr);

	fprintf(fp, "%.3f ", pv->VDC2_LateralAcceleration);
	fprintf(fp, "%.3f ", pv->VDC2_LongitudinalAcceleration);
	fprintf(fp, "%.3f ", pv->VDC2_SteeringWheelAngle);		//105
	fprintf(fp, "%d ", pv->VDC2_SteeringWheelTurnCounter);
	fprintf(fp, "%.3f ", pv->VDC2_YawRate);
      
	fprintf(fp, "%d ", pv->EBC5_FoundationBrakeUse);
	fprintf(fp, "%d ", pv->EBC5_HaltBrakeMode);
	fprintf(fp, "%d ", pv->EBC5_XBRAccelerationLimit);		//110
	fprintf(fp, "%d ", pv->EBC5_XBRActiveControlMode);
	   
	fprintf(fp, "%.3f ", pv->MVS_X_E_AppliedVehicleSpeedLimit_BB1_X_E);
 
	fprintf(fp, "%.3f ", pv->VP_X_TGW_Latitude_BB1_X_TGW);
	fprintf(fp, "%.3f ", pv->VP_X_TGW_Longitude_BB1_X_TGW);
	   
	fprintf(fp, "%.3f ", pv->IC1_EngAirIntakePress);		//115
	fprintf(fp, "%.3f ", pv->IC1_EngDslPrtclateFilterIntakePress);
	fprintf(fp, "%.3f ", pv->IC1_EngIntakeManifold1Press);
	fprintf(fp, "%.3f ", pv->IC1_EngIntakeManifold1Temp);

	fprintf(fp, "%d ", pv->CAN1_BusLoad);
	fprintf(fp, "%d ", pv->CAN1_ExtData);		//120
	fprintf(fp, "%d ", pv->CAN1_StdData);

	fprintf(fp, "%d ", pv->CAN2_BusLoad);
	   
		/// long_output_typ
                fprintf(fp, "%.3f ", pcmd->engine_speed);
                fprintf(fp, "%.3f ", pcmd->engine_torque);
                fprintf(fp, "%d ", pcmd->engine_command_mode);		//125
                fprintf(fp, "%.3f ", pcmd->engine_retarder_torque);
                fprintf(fp, "%d ", pcmd->engine_retarder_command_mode);
                fprintf(fp, "%.3f ", pcmd->ebs_deceleration);
                fprintf(fp, "%d ", pcmd->brake_command_mode);
                fprintf(fp, "%.3f ", pcmd->trans_retarder_value);	//130
                fprintf(fp, "%d ", pcmd->trans_retarder_command_mode);

		/// GPS and COMM
		fprintf(fp, "%.7f ", pv->self_gps.latitude);
		fprintf(fp, "%.7f ", pv->self_gps.longitude);
		fprintf(fp, "%.3f ", pv->self_gps.speed);
		fprintf(fp, "%.3f ", pv->lead_trk.global_time);		//135
		// We removed the transmission of GPS over the comm link 
		// because the packet was >128 bytes
//		fprintf(fp, "%.7f ", pv->lead_trk.gps.latitude);
//		fprintf(fp, "%.7f ", pv->lead_trk.gps.longitude);
//		fprintf(fp, "%.3f ", pv->lead_trk.gps.speed);
		fprintf(fp, "%.3f ", pv->lead_trk.velocity);		//136
		fprintf(fp, "%.3f ", pv->second_trk.global_time);	//137
//		fprintf(fp, "%.7f ", pv->second_trk.gps.latitude);
//		fprintf(fp, "%.7f ", pv->second_trk.gps.longitude);
//		fprintf(fp, "%.3f ", pv->second_trk.gps.speed);
		fprintf(fp, "%.3f ", pv->second_trk.velocity);		//138
		fprintf(fp, "%.3f ", pv->third_trk.global_time);	//139
//		fprintf(fp, "%.7f ", pv->third_trk.gps.latitude);
//		fprintf(fp, "%.7f ", pv->third_trk.gps.longitude);
//		fprintf(fp, "%.3f ", pv->third_trk.gps.speed);
		fprintf(fp, "%.3f ", pv->third_trk.velocity);		//140 



		// Formerly used for MDL lidar
		fprintf(fp, "%.3f ", pv->mdl_lidar.range);	//183

		// Denso lidar
		fprintf(fp,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
                        pv->lidarA.h_latpos_1,pv->lidarA.l_latpos_1,	//185
                        pv->lidarA.vert_pos_1, pv->lidarA.h_dist_1,
                        pv->lidarA.l_dist_1,pv->lidarA.lanerate_1,
                        pv->lidarA.veh_rate_1,pv->lidarA.targ_stat_1,	//191
                        pv->lidarA.lat_vel_1, pv->lidarA.h_velocity_1,
                        pv->lidarA.l_velocity_1,pv->lidarA.h_width_1,
                        pv->lidarA.l_width_1,pv->lidarA.height_1, 
                        pv->lidarA.h_depth_1, pv->lidarA.l_depth_1,
                        pv->lidarA.rel_acc_1);				//200


		// Digital inputs and outputs
		fprintf(fp, "%02hhx %02hhx %02hhx %02hhx %02hhx ", 
			pv->dig_in.manualctl,
			pv->dig_in.autoctl,
			pv->dig_in.brakesw,
			pdig_out->outchar,				
			pdig_out->amber_flash);				//205

		// Platoon communication; add remaining fields later
		fprintf(fp, "%.3f ", pcomm_tx->global_time);
		fprintf(fp, "%hd ", pcomm_tx->user_ushort_1);		
		fprintf(fp, "%hd ", pcomm_tx->user_ushort_2);
		fprintf(fp, "%.3f ", pcomm_tx->user_float);	
		fprintf(fp, "%.3f ", pv->lead_trk.global_time);		//210
		fprintf(fp, "%hd ", pv->lead_trk.user_ushort_1);
		fprintf(fp, "%hd ", pv->lead_trk.user_ushort_2);
		fprintf(fp, "%.3f ", pv->lead_trk.user_float);
		fprintf(fp, "%.3f ", pv->second_trk.global_time);
		fprintf(fp, "%hd ", pv->second_trk.user_ushort_1);	//215
		fprintf(fp, "%hd ", pv->second_trk.user_ushort_2);
		fprintf(fp, "%.3f ", pv->second_trk.user_float);
		fprintf(fp, "%.3f ", pv->third_trk.global_time);
		fprintf(fp, "%hd ", pv->third_trk.user_ushort_1);
		fprintf(fp, "%hd ", pv->third_trk.user_ushort_2);	//220
		fprintf(fp, "%.3f ", pv->third_trk.user_float);		//221

		fprintf(fp, "\n");

		current_index++;
		if (current_index == pbuff->data_size)
			current_index = 0;
	}
	return;
}

int main(int argc, char *argv[])
{
	db_clt_typ *pclt;              /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	int option;
	posix_timer_typ *ptimer;
	int delay_ms = 1000;	// wake up once a second
	int verbose = 0;
	timestamp_t current_ts;
	int ksize = 1000;	// maximum size stored data in kilobytes

	long_ctrl control_state;	// read and store this data
	long_output_typ long_out;
	long_dig_out_typ dig_out;
	veh_comm_packet_t comm_tx;

        while ((option = getopt(argc, argv, "s:t:v")) != EOF) {
                switch(option) {
		case 's':
			ksize = atoi(optarg);
			break;
		case 't':
			delay_ms = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "option not recognized\n");
			exit(1);
			break;
                }
	}

	get_local_name(hostname, MAXHOSTNAMELEN);

	/** Call long_database_init to login in to database, initialize 
	 * jbus variables and otherinput variables to long_ctl process 
	 * Initialize control structure first to set vehicle type
	 */

	memset(&control_state, 0, sizeof(control_state));
	ftime(&control_state.start_time);
	init_circular_buffer(&control_state.buff, ksize*100, sizeof(buffer_item));	
	control_state.params.vehicle_type = VEH_TYPE_TRUCK_SILVR;

        if ((pclt = long_database_init(hostname, argv[0], &control_state))
                         == NULL) {
                printf("Database initialization error in %s.\n", argv[0]);
                exit(EXIT_FAILURE);
        }

	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if (setjmp(exit_env) != 0) {
		print_data(stdout, &control_state);
		/* Log out from the database. */
		if (pclt != NULL)
			clt_logout(pclt);
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

        if ((ptimer = timer_init( delay_ms, ChannelCreate(0) )) == NULL)
        {
                fprintf(stderr, "Unable to initialize delay timer\n");
                exit(EXIT_FAILURE);
        }

	// Loop forever gathering data until terminated with signal

	// In verbose mode print timestamp to show the process is alive
	while (TRUE) {
		if (verbose) {
                       get_current_timestamp(&current_ts);
                       print_timestamp(stderr,&current_ts);
                       fprintf(stderr, "%s \n", argv[0]);
		}
		long_read_vehicle_state(pclt, &control_state);
		db_clt_read(pclt, DB_LONG_OUTPUT_VAR,
			sizeof(long_output_typ), &long_out);
		db_clt_read(pclt, DB_LONG_DIG_OUT_VAR,
			sizeof(long_dig_out_typ), &dig_out);
		db_clt_read(pclt, DB_COMM_TX_VAR,
			sizeof(veh_comm_packet_t), &comm_tx);
		gather_data(&control_state, &long_out, &dig_out, &comm_tx);
		TIMER_WAIT (ptimer);
	}
}

