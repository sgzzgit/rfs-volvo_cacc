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
#include <data_log.h>

long_vehicle_state pv;
long_output_typ pcmd;
long_dig_out_typ pdig_out;
veh_comm_packet_t pcomm_tx;
timestamp_t timestamp;
double process_time;

data_log_column_spec_t file_spec[] =
{
	{"HH:MM:SS.SSS ", &timestamp, BASE_TIMESTAMP, REPLAY_TIME},             //###1
	{ " %.3f ", &process_time, BASE_FLOAT, REPLAY_USE},

	///EEC1
	{ "%.3f ", &pv.EEC1_EngineSpeed, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.EEC1_DrvrDemandEngPercentTorque, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.EEC1_ActualEnginePercentTorque, BASE_FLOAT, REPLAY_USE},	//5
	{ "%.3f ", &pv.EEC1_EngDemandPercentTorque, BASE_FLOAT, REPLAY_USE},
	{ "%d ", &pv.EEC1_EngineTorqueMode, BASE_CHAR, REPLAY_USE},

	///EEC2
	{ "%d ", &pv.EEC2_AccelPedal1LowIdleSwitch, BASE_CHAR, REPLAY_USE},	   
	{ "%d ", &pv.EEC2_AccelPedal2LowIdleSwitch, BASE_CHAR, REPLAY_USE},	   
	{ "%d ", &pv.EEC2_AccelPedalKickdownSwitch, BASE_CHAR, REPLAY_USE},	//10 
	{ "%.3f ", &pv.EEC2_AccelPedalPos1, BASE_FLOAT, REPLAY_USE},	   
	{ "%.3f ", &pv.EEC2_AccelPedalPos2, BASE_FLOAT, REPLAY_USE},	   
	{ "%d ", &pv.EEC2_ActMaxAvailEngPercentTorque, BASE_CHAR, REPLAY_USE},	   
	{ "%.3f ", &pv.EEC2_EnginePercentLoadAtCurrentSpd, BASE_FLOAT, REPLAY_USE},
	{ "%d ", &pv.EEC2_RoadSpeedLimitStatus, BASE_CHAR, REPLAY_USE},	//15 

	///EEC3
	{ "%.3f ", &pv.EEC3_NominalFrictionPercentTorque, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.EEC3_EstEngPrsticLossesPercentTorque, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.EEC3_EngsDesiredOperatingSpeed, BASE_FLOAT, REPLAY_USE},


	///EBC1
	{ "%.3f ", &pv.EBC1_BrakePedalPosition, BASE_FLOAT, REPLAY_USE},
	{ "%d ", &pv.EBSBrakeSwitch, BASE_CHAR, REPLAY_USE}, 		//20
	{ "%d ", &pv.ABSEBSAmberWarningSignal, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.EBC1_ABSFullyOperational, BASE_CHAR, REPLAY_USE},

	{ "%d ", &pv.EBC1_AntiLockBrakingActive, BASE_CHAR, REPLAY_USE},	  
	{ "%d ", &pv.EBC1_ASRBrakeControlActive, BASE_CHAR, REPLAY_USE},	   
	{ "%d ", &pv.EBC1_ASREngineControlActive, BASE_CHAR, REPLAY_USE},	    	//25
	{ "%d ", &pv.EBC1_ASROffroadSwitch, BASE_CHAR, REPLAY_USE},	   

	{ "%d ", &pv.EBC1_EBSRedWarningSignal, BASE_CHAR, REPLAY_USE},	   
	{ "%.2f ", &pv.EBC1_EngRetarderSelection, BASE_FLOAT, REPLAY_USE},	   
	{ "%d ", &pv.EBC1_RemoteAccelEnableSwitch, BASE_CHAR, REPLAY_USE},	   
	{ "%d ", &pv.EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl, BASE_CHAR, REPLAY_USE},	//30	 
	///EBC2
	{ "%.3f ", &pv.EBC2_FrontAxleSpeed, BASE_FLOAT, REPLAY_USE},	
	{ "%.3f ", &pv.EBC2_RelativeSpeedFrontAxleLeftWheel, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.EBC2_RlativeSpeedFrontAxleRightWheel, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.EBC2_RelativeSpeedRearAxle1LeftWheel, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.EBC2_RlativeSpeedRearAxle1RightWheel, BASE_FLOAT, REPLAY_USE},	//35
	{ "%.3f ", &pv.EBC2_RelativeSpeedRearAxle2LeftWheel, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.EBC2_RlativeSpeedRearAxle2RightWheel, BASE_FLOAT, REPLAY_USE},

	///CCVS
	{ "%.3f ", &pv.CCVS_VehicleSpeed, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.CCVS_CruiseControlSetSpeed, BASE_FLOAT, REPLAY_USE},
	{ "%d ", &pv.CCVS_TwoSpeedAxleSwitch, BASE_CHAR, REPLAY_USE},	//40
	{ "%d ", &pv.CCVS_ParkingBrakeSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_CruiseCtrlPauseSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_ParkBrakeReleaseInhibitRq, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_CruiseControlActive, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_CruiseControlEnableSwitch, BASE_CHAR, REPLAY_USE},	//45
	{ "%d ", &pv.CCVS_BrakeSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_ClutchSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_CruiseControlSetSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_CruiseControlCoastSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_CruiseControlResumeSwitch, BASE_CHAR, REPLAY_USE},	//50
	{ "%d ", &pv.CCVS_CruiseControlAccelerateSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_PtoState, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_CruiseControlState, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_EngIdleIncrementSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_EngIdleDecrementSwitch, BASE_CHAR, REPLAY_USE},	//55
	{ "%d ", &pv.CCVS_EngTestModeSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CCVS_EngShutdownOverrideSwitch, BASE_CHAR, REPLAY_USE},

	///ETC1
	{ "%d ", &pv.ETC1_TransmissionShiftInProcess, BASE_CHAR, REPLAY_USE},
	{ "%.3f ", &pv.ETC1_TransInputShaftSpeed, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.ETC1_TransmissionOutputShaftSpeed, BASE_FLOAT, REPLAY_USE},	//60
	{ "%d ", &pv.ETC1_TorqueConverterLockupEngaged, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.ETC1_ProgressiveShiftDisable, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.ETC1_TransmissionDrivelineEngaged, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.ETC1_MomentaryEngineOverspeedEnable, BASE_CHAR, REPLAY_USE},
	{ "%.3f ", &pv.ETC1_PercentClutchSlip, BASE_FLOAT, REPLAY_USE},	//65
	{ "%d ", &pv.ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl, BASE_CHAR, REPLAY_USE},

	///ETC2
	{ "%d ", &pv.ETC2_TransmissionSelectedGear, BASE_CHAR, REPLAY_USE},
	{ "%.3f ", &pv.ETC2_TransmissionActualGearRatio, BASE_FLOAT, REPLAY_USE},
	{ "%d ", &pv.ETC2_TransmissionCurrentGear, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.ETC2_TransmissionSelectedGear, BASE_CHAR, REPLAY_USE}, 	//70
	{ "%d ", &pv.ETC2_TransmissionRangeAttained, BASE_CHAR, REPLAY_USE},
 
	///LFE
	{ "%.3f ", &pv.LFE_EngineAverageFuelEconomy, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.LFE_EngineFuelRate, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.LFE_EngineInstantaneousFuelEconomy, BASE_FLOAT, REPLAY_USE},	   
	{ "%.3f ", &pv.LFE_EngineThrottleValve1Position, BASE_FLOAT, REPLAY_USE},	//75

	///FD
	{ "%d ", &pv.fan_drive_state, BASE_CHAR, REPLAY_USE},
	{ "%.3f ", &pv.estimated_percent_fan_speed, BASE_FLOAT, REPLAY_USE},

	///ERC1 (TRANS)
	{ "%d ", &pv.ERC1ERRetarderEnableShiftAssistSw, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.ERC1ERRtdrEnablBrakeAssistSwitch, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.ERC1ERRetarderTorqueMode, BASE_CHAR, REPLAY_USE},	//80
	{ "%.2f ", &pv.ERC1ERActualEngineRetPercentTrq, BASE_FLOAT, REPLAY_USE},
	{ "%.2f ", &pv.ERC1ERIntendedRtdrPercentTorque, BASE_FLOAT, REPLAY_USE},
	{ "%d ", &pv.ERC1ERRetarderRqingBrakeLight, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl, BASE_CHAR, REPLAY_USE},
	{ "%hhd ", &pv.ERC1ERDrvrsDmandRtdrPerctTorque, BASE_CHAR, REPLAY_USE},	//85
	{ "%d ", &pv.ERC1ERRetarderSelectionNonEng, BASE_CHAR, REPLAY_USE},
	{ "%hhd ", &pv.ERC1ERActlMxAvlbRtdrtPerctTorque, BASE_CHAR, REPLAY_USE},
	
	// Transmission control of engine
	{ "%d ", &pv.TSC1_EMSTECU_EnOvrdCtrlM, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSTECU_EnRSpdCtrlC, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSTECU_EnRSpdSpdLm, BASE_CHAR, REPLAY_USE},	//90
	{ "%d ", &pv.TSC1_EMSTECU_EnRTrqTrqLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSTECU_OvrdCtrlMPr, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSTECU_src_address, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSTECU_destination_address, BASE_CHAR, REPLAY_USE},

	// Brake control of engine
	{ "%d ", &pv.TSC1_EMSABS_EnOvrdCtrlM, BASE_CHAR, REPLAY_USE},	//95
	{ "%d ", &pv.TSC1_EMSABS_EnRSpdCtrlC, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSABS_EnRSpdSpdLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSABS_EnRTrqTrqLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSABS_OvrdCtrlMPr, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSABS_src_address, BASE_CHAR, REPLAY_USE},	//100
	{ "%d ", &pv.TSC1_EMSABS_destination_address, BASE_CHAR, REPLAY_USE},

	// CC control of engine
	{ "%d ", &pv.TSC1_EMSVMCUes_EnOvrdCtrlM, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSVMCUes_EnRSpdCtrlC, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSVMCUes_EnRSpdSpdLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSVMCUes_EnRTrqTrqLm, BASE_CHAR, REPLAY_USE},	//105
	{ "%d ", &pv.TSC1_EMSVMCUes_OvrdCtrlMPr, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSVMCUes_src_address, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSVMCUes_destination_address, BASE_CHAR, REPLAY_USE},

	// ACC control of engine
	{ "%d ", &pv.TSC1_EMS_ACC_EnOvrdCtrlM, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMS_ACC_EnRSpdCtrlC, BASE_CHAR, REPLAY_USE},	//110
	{ "%d ", &pv.TSC1_EMS_ACC_EnRSpdSpdLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMS_ACC_EnRTrqTrqLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMS_ACC_OvrdCtrlMPr, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMS_ACC_src_address, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMS_ACC_destination_address, BASE_CHAR, REPLAY_USE},	//115

	// Transmission control of engine retarder
	{ "%d ", &pv.TSC1_EMSrTECU_EnOvrdCtrlM, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrTECU_EnRSpdCtrlC, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrTECU_EnRSpdSpdLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrTECU_EnRTrqTrqLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrTECU_OvrdCtrlMPr, BASE_CHAR, REPLAY_USE},	//120
	{ "%d ", &pv.TSC1_EMSrTECU_src_address, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrTECU_destination_address, BASE_CHAR, REPLAY_USE},

	// Brake control of engine retarder
	{ "%d ", &pv.TSC1_EMSrABS_EnOvrdCtrlM, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrABS_EnRSpdCtrlC, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrABS_EnRSpdSpdLm, BASE_CHAR, REPLAY_USE},	//125
	{ "%d ", &pv.TSC1_EMSrABS_EnRTrqTrqLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrABS_OvrdCtrlMPr, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrABS_src_address, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_EMSrABS_destination_address, BASE_CHAR, REPLAY_USE},

	// CC control of engine retarder                                
	{ "%d ", &pv.TSC1_ER_V_EnOvrdCtrlM, BASE_CHAR, REPLAY_USE},	//130
	{ "%d ", &pv.TSC1_ER_V_EnRSpdCtrlC, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_V_EnRSpdSpdLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_V_EnRTrqTrqLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_V_OvrdCtrlMPr, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_V_src_address, BASE_CHAR, REPLAY_USE},	//135
	{ "%d ", &pv.TSC1_ER_V_destination_address, BASE_CHAR, REPLAY_USE},

	// ACC control of engine retarder
	{ "%d ", &pv.TSC1_ER_ACC_EnOvrdCtrlM, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_ACC_EnRSpdCtrlC, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_ACC_EnRSpdSpdLm, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_ACC_EnRTrqTrqLm, BASE_CHAR, REPLAY_USE},	//140
	{ "%d ", &pv.TSC1_ER_ACC_OvrdCtrlMPr, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_ACC_src_address, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.TSC1_ER_ACC_destination_address, BASE_CHAR, REPLAY_USE},

	{ "%.3f ", &pv.VDC2_LateralAcceleration, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.VDC2_LongitudinalAcceleration, BASE_FLOAT, REPLAY_USE},	//145
	{ "%.3f ", &pv.VDC2_SteeringWheelAngle, BASE_FLOAT, REPLAY_USE},
	{ "%d ", &pv.VDC2_SteeringWheelTurnCounter, BASE_CHAR, REPLAY_USE},
	{ "%.3f ", &pv.VDC2_YawRate, BASE_FLOAT, REPLAY_USE},
      
	{ "%d ", &pv.EBC5_FoundationBrakeUse, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.EBC5_HaltBrakeMode, BASE_CHAR, REPLAY_USE},	//150
	{ "%d ", &pv.EBC5_XBRAccelerationLimit, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.EBC5_XBRActiveControlMode, BASE_CHAR, REPLAY_USE},
	   
	{ "%.3f ", &pv.MVS_X_E_AppliedVehicleSpeedLimit_BB1_X_E, BASE_FLOAT, REPLAY_USE},
 
	{ "%.3f ", &pv.VP_X_TGW_Latitude_BB1_X_TGW, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.VP_X_TGW_Longitude_BB1_X_TGW, BASE_FLOAT, REPLAY_USE},	//155
	   
	{ "%.3f ", &pv.IC1_EngAirIntakePress, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.IC1_EngDslPrtclateFilterIntakePress, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.IC1_EngIntakeManifold1Press, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.IC1_EngIntakeManifold1Temp, BASE_FLOAT, REPLAY_USE},

	{ "%d ", &pv.CAN1_BusLoad, BASE_CHAR, REPLAY_USE},		//160
	{ "%d ", &pv.CAN1_ExtData, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.CAN1_StdData, BASE_CHAR, REPLAY_USE},

	{ "%d ", &pv.CAN2_BusLoad, BASE_CHAR, REPLAY_USE},
	   
	/// long_output_typ
	{ "%.3f ", &pcmd.engine_speed, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pcmd.engine_torque, BASE_FLOAT, REPLAY_USE}, 	//165 between 14.0 & 15.0
	{ "%d ", &pcmd.engine_command_mode, BASE_CHAR, REPLAY_USE}, 	//between 0 & 2
	{ "%.3f ", &pcmd.engine_retarder_torque, BASE_FLOAT, REPLAY_USE},	//0->-40.0 until end of file
	{ "%d ", &pcmd.engine_retarder_command_mode, BASE_CHAR, REPLAY_USE}, //0->2 until end of file
	{ "%.3f ", &pcmd.ebs_deceleration, BASE_FLOAT, REPLAY_USE},	//0->ugly negative number til end of file
	{ "%d ", &pcmd.brake_command_mode, BASE_CHAR, REPLAY_USE},	//170 //always 0
	{ "%.3f ", &pcmd.trans_retarder_value, BASE_FLOAT, REPLAY_USE},	//always 0
	{ "%d ", &pcmd.trans_retarder_command_mode, BASE_CHAR, REPLAY_USE},	//always 0
	{ "%d ", &pcmd.engine_priority, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pcmd.engine_retarder_priority, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pcmd.brake_priority, BASE_CHAR, REPLAY_USE},	//175

	/// GPS and COMM
	{ "%.7f ", &pv.self_gps.latitude, BASE_FLOAT, REPLAY_USE},
	{ "%.7f ", &pv.self_gps.longitude, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.self_gps.speed, BASE_FLOAT, REPLAY_USE},	//178
	{ "%.3f ", &pv.lead_trk.global_time, BASE_FLOAT, REPLAY_USE},
	// We removed the transmission of GPS over the comm link 
	// because the packet was >128 bytes
//	{ "%.7f ", &pv.lead_trk.gps.latitude, BASE_FLOAT, REPLAY_USE},
//	{ "%.7f ", &pv.lead_trk.gps.longitude, BASE_FLOAT, REPLAY_USE},
//	{ "%.3f ", &pv.lead_trk.gps.speed, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.lead_trk.velocity, BASE_FLOAT, REPLAY_USE},	//180
	{ "%.3f ", &pv.second_trk.global_time, BASE_FLOAT, REPLAY_USE},
//	{ "%.7f ", &pv.second_trk.gps.latitude, BASE_FLOAT, REPLAY_USE},
//	{ "%.7f ", &pv.second_trk.gps.longitude, BASE_FLOAT, REPLAY_USE},
//	{ "%.3f ", &pv.second_trk.gps.speed, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.second_trk.velocity, BASE_FLOAT, REPLAY_USE},	//181
	{ "%.3f ", &pv.third_trk.global_time, BASE_FLOAT, REPLAY_USE},	//182
//	{ "%.7f ", &pv.third_trk.gps.latitude, BASE_FLOAT, REPLAY_USE},
//	{ "%.7f ", &pv.third_trk.gps.longitude, BASE_FLOAT, REPLAY_USE},
//	{ "%.3f ", &pv.third_trk.gps.speed, BASE_FLOAT, REPLAY_USE}, { "%.3f ", &pv.third_trk.velocity, BASE_CHAR, REPLAY_USE},
	// Digital inputs and outputs
	{ "%02hhx  ", &pv.dig_in.manualctl, BASE_CHAR, REPLAY_USE},	
	{ "%02hhx  ", &pv.dig_in.autoctl, BASE_CHAR, REPLAY_USE},	
	{ "%02hhx  ", &pv.dig_in.brakesw, BASE_CHAR, REPLAY_USE},	
	{ "%02hhx  ", &pdig_out.outchar, BASE_CHAR, REPLAY_USE},			
	{ "%02hhx  ", &pdig_out.amber_flash, BASE_CHAR, REPLAY_USE},	//188

	// Platoon communication; add remaining fields later
	{ "%.3f ", &pcomm_tx.global_time, BASE_FLOAT, REPLAY_USE},
	{ "%hu ", &pcomm_tx.user_ushort_1, BASE_CHAR, REPLAY_USE},	//190	
	{ "%hu ", &pcomm_tx.user_ushort_2, BASE_CHAR, REPLAY_USE},
	{ "%.3f ", &pcomm_tx.user_float, BASE_FLOAT, REPLAY_USE},	
	{ "%.3f ", &pv.lead_trk.global_time, BASE_FLOAT, REPLAY_USE},	
	{ "%hu ", &pv.lead_trk.user_ushort_1, BASE_CHAR, REPLAY_USE},
	{ "%hu ", &pv.lead_trk.user_ushort_2, BASE_CHAR, REPLAY_USE},	//195
	{ "%.3f ", &pv.lead_trk.user_float, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.second_trk.global_time, BASE_FLOAT, REPLAY_USE},
	{ "%hu ", &pv.second_trk.user_ushort_1, BASE_CHAR, REPLAY_USE},
	{ "%hu ", &pv.second_trk.user_ushort_2, BASE_CHAR, REPLAY_USE},
	{ "%.3f ", &pv.second_trk.user_float, BASE_FLOAT, REPLAY_USE},	//200
	{ "%.3f ", &pv.third_trk.global_time, BASE_FLOAT, REPLAY_USE},
	{ "%hu ", &pv.third_trk.user_ushort_1, BASE_CHAR, REPLAY_USE},
	{ "%hu ", &pv.third_trk.user_ushort_2, BASE_CHAR, REPLAY_USE},
	{ "%.3f ", &pv.third_trk.user_float, BASE_FLOAT, REPLAY_USE},	
	{ "%.3f ", &pv.Volvo_TargetDist, BASE_FLOAT, REPLAY_USE},	//205
	{ "%.3f ", &pv.Volvo_TargetVel, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.Volvo_TargetAcc, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.Volvo_TargetAvailable, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.Volvo_EgoVel, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.Volvo_EgoAcc, BASE_FLOAT, REPLAY_USE},		//210
	{ "%.3f ", &pv.Volvo_EgoRoadGrade, BASE_FLOAT, REPLAY_USE},		//210
	{ "%.3f ", &pv.VBRK_BrkAppPressure, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.VBRK_BrkPrimaryPressure, BASE_FLOAT, REPLAY_USE},
	{ "%.3f ", &pv.VBRK_BrkSecondaryPressure, BASE_FLOAT, REPLAY_USE},	
	{ "%.2f ", &pv.VP15_RoadInclinationVP15, BASE_FLOAT, REPLAY_USE},	//215
	{ "%.2f ", &pv.VP15_VehicleWeightVP15, BASE_FLOAT, REPLAY_USE},
	{ "%.1f ", &pv.VP15_PermittedHillHolderP, BASE_FLOAT, REPLAY_USE},
	{ "%d ", &pv.VP15_PowerDownAcknowledge, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.VP15_DirectionGearAllowed, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.VP15_ClutchOverloadStatus, BASE_CHAR, REPLAY_USE},		//220
	{ "%d ", &pv.VP15_EcoRollStatus, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.VP15_RecommendedGearshift, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.VP15_EcoRollActiveStatus, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.VP15_AutomaticHSARequest, BASE_CHAR, REPLAY_USE},
	{ "%d ", &pv.VP15_EngineShutdownRequest, BASE_CHAR, REPLAY_USE},	//225
	{ "%hhu ", &pcomm_tx.maneuver_des_2, BASE_CHAR, REPLAY_USE},
	{ "%hhu ", &pcomm_tx.my_pip, BASE_CHAR, REPLAY_USE},
	{ "%hhu ", &pv.lead_trk.maneuver_des_2, BASE_CHAR, REPLAY_USE},
	{ "%hhu ", &pv.second_trk.maneuver_des_2, BASE_CHAR, REPLAY_USE},
	{ "%hhu ", &pv.third_trk.maneuver_des_2, BASE_CHAR, REPLAY_USE},	//230

};
#define NUM_FILE_COLUMNS sizeof(file_spec)/sizeof(data_log_column_spec_t)
int num_file_columns = sizeof(file_spec)/sizeof(data_log_column_spec_t);
