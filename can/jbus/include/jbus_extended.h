/**\file	
 *      Database variable definitions for J1939 and J1587 buses
 *	 Note that first field for all structures is the time stamp,
 *	 so it can be altered by routines that do not know the type
 * 	 of the message.
 *
 *
 * Copyright (c) 2001   Regents of the University of California
 *
 *
 *	static char rcsid[] = "$Id";
 *
 *
 *	$Log: jbus.h,v $
 *	Revision 1.2  2003/05/23 17:33:34  dickey
 *	fixed problems do to changed comments
 *	
 *	Revision 1.1.1.1  2003/01/22 18:31:46  dickey
 *	SAE J1939 and J1587 code
 *	
 *	Revision 2.3  2002/12/21 00:15:09  dickey
 *	Add new brake messages, new CNG bus messages.
 *
 *	Revision 2.2  2002/08/23 23:56:24  dickey
 *	Added J1587 variable types.
 *
 *	Revision 2.1  2002/07/24 21:39:46  dickey
 *	Check in of San Diego test code
 *
 *	Revision 1.5  2002/05/22 22:54:39  dickey
 *	fixed some field names
 *
 *	Revision 1.4  2002/04/08 16:19:34  dickey
 *	Changed timestamp handling, added more messages.
 *
 *	Revision 1.2  2002/03/15 20:37:55  dickey
 *	Structures for jbus database variables
 *
 *	Revision 1.1  2002/03/13 00:18:12  dickey
 *	Initial revision
 */


/** PDU - generic database variable type for storing information about
 *	 Protocol Data Units that have been received with a correct sum,
 *	 with no field translation.
 */
typedef struct {
	timestamp_t timestamp;
	unsigned char priority;	
	unsigned char pdu_format;	/// Protocol Data Unit Format (PF) 
	unsigned char pdu_specific;	/// PDU Specific (PS) 
	unsigned char src_address;	/// Source address 
	unsigned char data_field[8];	/// 64 bits maximum 
	unsigned char numbytes;		/// number of bytes in data_field 
} IS_PACKED j1939_pdu_typ;

/** PDU TSC1 (Torque/Speed Control) doc. in J1939 - 71, p149 */

typedef struct {
	timestamp_t timestamp;
	unsigned char EnOvrdCtrlMPr;
	unsigned char EnRSpdCtrlC;
	unsigned char EnOvrdCtrlM;
	float EnRSpdSpdLm;		/// RPM 
	float EnRTrqTrqLm;		/// percent reference torque 
	int destination_address;	/// engine or retarder 
	int src_address;		/// sent in header, important for logging 
} IS_PACKED j1939_tsc1_typ;


/** PDU TSC1 (Torque/Speed Control) doc. in J1939 - 71, p149 */
typedef struct {
	timestamp_t timestamp;
	unsigned char EnOvrdCtrlMPr;
	unsigned char EnRSpdCtrlC;
	unsigned char EnOvrdCtrlM;
	float EnRSpdSpdLm;		/// RPM 
	float EnRTrqTrqLm;		/// percent reference torque 
	int destination_address;	/// engine or retarder 
	int src_address;		/// sent in header, important for logging 
} IS_PACKED j1939_tsc1_e_acc_typ;

/** PDU TSC1 (Torque/Speed Control) doc. in J1939 - 71, p149 */
typedef struct {
	timestamp_t timestamp;
	unsigned char EnOvrdCtrlMPr;
	unsigned char EnRSpdCtrlC;
	unsigned char EnOvrdCtrlM;
	float EnRSpdSpdLm;		/// RPM 
	float EnRTrqTrqLm;		/// percent reference torque 
	int destination_address;	/// engine or retarder 
	int src_address;		/// sent in header, important for logging 
} IS_PACKED j1939_tsc1_er_acc_typ;

/** PDU TC1 (Torque/Speed Control) doc. in J1939 - 71, p149 */
typedef struct {
	timestamp_t timestamp;
	unsigned char disengage_driveline_request;
	unsigned char torque_converter_lockup_disable_request;
	unsigned char gear_shift_inhibit_request;
	float requested_percent_clutch_slip;
	char requested_gear;
	unsigned char disengage_differential_lock_rear_axle_2;
	unsigned char disengage_differential_lock_rear_axle_1;
	unsigned char disengage_differential_lock_front_axle_2;
	unsigned char disengage_differential_lock_front_axle_1;
	unsigned char disengage_differential_lock_central_rear;
	unsigned char disengage_differential_lock_central_front;
	unsigned char disengage_differential_lock_central;
} IS_PACKED j1939_tc1_typ;

/** PDU ERC1 (Electronic Retarder Controller #1) doc. in J1939 - 71, p150 */
typedef struct {
	timestamp_t timestamp;
	unsigned char ERC1ERRetarderEnableShiftAssistSw;
	unsigned char ERC1ERRtdrEnablBrakeAssistSwitch;
	unsigned char ERC1ERRetarderTorqueMode;
	float ERC1ERActualEngineRetPercentTrq;
	float ERC1ERIntendedRtdrPercentTorque;	
	unsigned char ERC1ERRetarderRqingBrakeLight;	
	unsigned char ERC1ERSrcAddrOfCtrlDvcFrRtdrCtrl;	
	int ERC1ERDrvrsDmandRtdrPerctTorque;
	int ERC1ERRetarderSelectionNonEng;
	int ERC1ERActlMxAvlbRtdrtPerctTorque;
} IS_PACKED j1939_erc1_typ;

/** PDU VBRK (Volvo brake type, Bendix brakes) */
typedef struct {
	timestamp_t timestamp;
        float VBRK_BrkAppPressure;
        float VBRK_BrkPrimaryPressure;
        float VBRK_BrkSecondaryPressure;
        unsigned char VBRK_BrkStatParkBrkActuator;
        unsigned char VBRK_ParkBrkRedWarningSignal;
        unsigned char VBRK_ParkBrkReleaseInhibitStat;
} IS_PACKED j1939_volvo_brk_t;

/** PDU VP15 (Volvo brake type, Bendix brakes) */
typedef struct {
	timestamp_t timestamp;
	unsigned char VP15_EcoRollStatus;
	unsigned char VP15_AutomaticHSARequest;
	unsigned char VP15_EngineShutdownRequest;
	float VP15_RoadInclinationVP15;
	float VP15_PermittedHillHolderP;
	unsigned char VP15_RecommendedGearshift;
	unsigned char VP15_EcoRollActiveStatus;
	unsigned char VP15_ClutchOverloadStatus;
	unsigned char VP15_PowerDownAcknowledge;
	unsigned char VP15_DirectionGearAllowed;
	float VP15_VehicleWeightVP15;
} IS_PACKED j1939_volvo_vp15_t;

/** PDU EBC1 (Electronic Brake Controller #1) doc. in J1939 - 71, p151 */
typedef struct {
	timestamp_t timestamp;
	unsigned char EBSBrakeSwitch;
	unsigned char EBC1_AntiLockBrakingActive;
	unsigned char EBC1_ASRBrakeControlActive;
	unsigned char EBC1_ASREngineControlActive;
	float EBC1_BrakePedalPosition;
	unsigned char traction_control_override_switch_status;
	unsigned char asr_hill_holder_switch_status;
	unsigned char EBC1_ASROffroadSwitch;
	unsigned char EBC1_RemoteAccelEnableSwitch;
	unsigned char auxiliary_engine_shutdown_switch_status;
	unsigned char engine_derate_switch_status;
	unsigned char accelerator_interlock_switch_status;
	float percent_engine_retarder_torque_selected;
	float EBC1_EngRetarderSelection;
	unsigned char ABSEBSAmberWarningSignal;
	unsigned char EBC1_ABSFullyOperational;
	unsigned char EBC1_EBSRedWarningSignal;
	unsigned char EBC1_SrcAddrssOfCtrllngDvcFrBrkCntrl;
	float total_brake_demand;
} IS_PACKED j1939_ebc1_typ;

/** PDU ETC1 (Elec. Transmission Controller #1) doc. in J1939 - 71, p151 */
typedef struct {
	timestamp_t timestamp;
	unsigned char ETC1_TransmissionDrivelineEngaged;
	unsigned char ETC1_TorqueConverterLockupEngaged;
	unsigned char ETC1_TransmissionShiftInProcess;
	float ETC1_TransmissionOutputShaftSpeed;
	float ETC1_PercentClutchSlip;
	unsigned char ETC1_MomentaryEngineOverspeedEnable;
	unsigned char ETC1_ProgressiveShiftDisable;
	float ETC1_TransInputShaftSpeed;
	unsigned char ETC1_SrcAddrssOfCtrllngDvcFrTrnsCtrl;
} IS_PACKED j1939_etc1_typ;

/** PDU EEC1 (Electronic Engine Controller #1) doc. in J1939 - 71, p152 */
typedef struct {
	timestamp_t timestamp;
	char EEC1_EngineTorqueMode;
	float EEC1_DrvrDemandEngPercentTorque;
	float EEC1_ActualEnginePercentTorque;	
	float EEC1_EngDemandPercentTorque;	
	float EEC1_EngineSpeed;	
	unsigned char source_address;	/// not supported by Cummins? 
} IS_PACKED j1939_eec1_typ;

/** PDU EEC2 (Electronic Engine Controller #2) doc. in J1939 - 71, p152 */
typedef struct {
	timestamp_t timestamp;
	unsigned char EEC2_RoadSpeedLimitStatus;
	unsigned char EEC2_AccelPedalKickdownSwitch;
	unsigned char EEC2_AccelPedal2LowIdleSwitch;
	unsigned char EEC2_AccelPedal1LowIdleSwitch;
	float EEC2_AccelPedalPos1;
	float EEC2_AccelPedalPos2;
	float EEC2_ActMaxAvailEngPercentTorque;
	float EEC2_EnginePercentLoadAtCurrentSpd;
                                                                  
} IS_PACKED j1939_eec2_typ;
 
/** PDU ETC2 (Electronic Transmission Controller #2, source transmission) doc. in J1939 - 71, p152 */
typedef struct {
	timestamp_t timestamp;
	char ETC2_TransmissionSelectedGear;
	float ETC2_TransmissionActualGearRatio;
	char ETC2_TransmissionCurrentGear;
	int ETC2_TransmissionRangeSelected;
	int ETC2_TransmissionRangeAttained;
} IS_PACKED j1939_etc2_typ;
 
/** PDU ETC2_E (Electronic Transmission Controller #2, source engine) doc. in J1939 - 71, p152 */
typedef struct {
	timestamp_t timestamp;
	char ETC2_E_TransmissionSelectedGear;
	float ETC2_E_TransmissionActualGearRatio;
	char ETC2_E_TransmissionCurrentGear;
	int ETC2_E_TransmissionRangeSelected;
	int ETC2_E_TransmissionRangeAttained;
} IS_PACKED j1939_etc2_e_typ;
 
/** PDU TURBO (Turbocharger) doc. in J1939 - 71, p153 */
typedef struct {
	timestamp_t timestamp;
	float turbocharger_lube_oil_pressure;
	float turbocharger_speed;
} IS_PACKED j1939_turbo_typ;
 
/** PDU EEC3 (Electronic Engine Controller #3) doc. in J1939 - 71, p154 */
typedef struct {
	timestamp_t timestamp;
	float EEC3_NominalFrictionPercentTorque;
	float EEC3_EstEngPrsticLossesPercentTorque;
	float EEC3_EngsDesiredOperatingSpeed;
	char operating_speed_asymmetry_adjust;
} IS_PACKED j1939_eec3_typ;

/** PDU VD (Vehicle Distance) doc. in J1939 - 71, p154 */
typedef struct {
	timestamp_t timestamp;
	float trip_distance;
	float total_vehicle_distance;
} IS_PACKED j1939_vd_typ;

/** PDU RCFG (Retarder Configuration) doc. in J1939 - 71, p155 */
typedef struct {
	timestamp_t timestamp;
	unsigned char retarder_location;
	unsigned char retarder_type;
	unsigned char retarder_control_steps;
	unsigned char receive_status;
	float retarder_speed[5];
	float percent_torque[5];
	float reference_retarder_torque;
} IS_PACKED j1939_rcfg_typ;

#define MAX_FORWARD_GEARS 16
#define MAX_REVERSE_GEARS 8

/** PDU TCFG (Transmission Configuration) doc. in J1939 - 71, p155 */
typedef struct {
	timestamp_t timestamp;
	char number_reverse_gear_ratios;
	char number_forward_gear_rations;
	float reverse_gear_ratios[MAX_REVERSE_GEARS];
	float forward_gear_rations[MAX_FORWARD_GEARS];
} IS_PACKED j1939_tcfg_typ;

/** PDU ECFG (Engine Configuration) doc. in J1939 - 71, p156 */
typedef struct {
	timestamp_t timestamp;
	float engine_speed[7];
	float percent_torque[5];
	float gain_endspeed_governor;
	float reference_engine_torque;
	float max_momentary_overide_time;
	float speed_control_lower_limit;
	float speed_control_upper_limit;
	float torque_control_lower_limit;
	float torque_control_upper_limit;
	unsigned char receive_status; /// mask of frames received */
} IS_PACKED j1939_ecfg_typ;

/** PDU ETEMP (Engine Temperature) doc. in J1939 - 71, p160 */
typedef struct {
	timestamp_t timestamp;
	float engine_coolant_temperature;
	float fuel_temperature;
	float engine_oil_temperature;
	float turbo_oil_temperature;
	float engine_intercooler_temperature;
	float engine_intercooler_thermostat_opening;
} IS_PACKED j1939_etemp_typ;

/** PDU PTO (Power Takeoff Information) doc. in J1939 - 71, p161 */
typedef struct {
	timestamp_t timestamp;
	float pto_oil_temperature;
	float pto_speed;
	float pto_set_speed;
	unsigned char remote_pto_variable_speed_status;
	unsigned char remote_pto_preprogrammed_status;
	unsigned char pto_enable_switch_status;
	unsigned char pto_accelerate_switch_status;
	unsigned char pto_resume_switch_status;
	unsigned char pto_coast_accelerate_switch_status;
	unsigned char pto_set_switch_status;
} IS_PACKED j1939_pto_typ;

/** PDU CCVS (Cruise Control/Vehicle Speed) doc. in J1939 - 71, p162 */
typedef struct {
	timestamp_t timestamp;
	unsigned char CCVS_ParkingBrakeSwitch;
	unsigned char CCVS_TwoSpeedAxleSwitch; 
	float CCVS_VehicleSpeed;
	unsigned char CCVS_ParkBrakeReleaseInhibitRq ;      
	unsigned char CCVS_ClutchSwitch;                    
	unsigned char CCVS_BrakeSwitch;                     
	unsigned char CCVS_CruiseCtrlPauseSwitch;           
	unsigned char CCVS_CruiseControlEnableSwitch;       
	unsigned char CCVS_CruiseControlActive;             
	unsigned char CCVS_CruiseControlAccelerateSwitch;   
	unsigned char CCVS_CruiseControlResumeSwitch;       
	unsigned char CCVS_CruiseControlCoastSwitch;        
	unsigned char CCVS_CruiseControlSetSwitch;          
	float CCVS_CruiseControlSetSpeed;         
	unsigned char CCVS_CruiseControlState;              
	unsigned char CCVS_PtoState;                        
	unsigned char CCVS_EngShutdownOverrideSwitch;       
	unsigned char CCVS_EngTestModeSwitch;               
	unsigned char CCVS_EngIdleDecrementSwitch;          
	unsigned char CCVS_EngIdleIncrementSwitch;          
} IS_PACKED j1939_ccvs_typ;

/** PDU LFE (Fuel Economy) doc. in J1939 - 71, p162 */
typedef struct {
	timestamp_t timestamp;
	float LFE_EngineFuelRate;
	float LFE_EngineInstantaneousFuelEconomy;
	float LFE_EngineAverageFuelEconomy;
	float LFE_EngineThrottleValve1Position;
} IS_PACKED j1939_lfe_typ;

/** PDU AMBC (Ambient Conditions) doc. in J1939 - 71, p163 */
typedef struct {
	timestamp_t timestamp;
	float barometric_pressure;
	float cab_interior_temperature;
	float ambient_air_temperature;
	float air_inlet_temperature;
	float road_surface_temperature;
} IS_PACKED j1939_ambc_typ;

/** PDU IEC (Inlet/Exhaust Conditions) doc. in J1939 - 71, p164 */
typedef struct {
	timestamp_t timestamp;
	float particulate_trap_inlet_pressure;
	float boost_pressure;
	float intake_manifold_temperature;
	float air_inlet_pressure;
	float air_filter_differential_pressure;
	float exhaust_gas_temperature;
	float coolant_filter_differential_pressure;
} IS_PACKED j1939_iec_typ;

/** PDU VEP (Vehicle Electrical Power) doc. in J1939 - 71, p164 */
typedef struct {
	timestamp_t timestamp;
	float net_battery_current;
	float alternator_current;
	float alternator_potential;
	float electrical_potential;
	float battery_potential_switched;
} IS_PACKED j1939_vep_typ;

/** PDU TF (Transmission Fluids) doc. in J1939 - 71, p164 */
typedef struct {
	timestamp_t timestamp;
	float clutch_pressure;
	float transmission_oil_level;
	float transmission_filter_differential_pressure;
	float transmission_oil_pressure;
	float transmission_oil_temperature;
} IS_PACKED j1939_tf_typ;

/** PDU RF (Retarder Fluids) doc. in J1939 - 71, p164 */
typedef struct {
	timestamp_t timestamp;
	float hydraulic_retarder_pressure;
	float hydraulic_retarder_oil_temperature;
} IS_PACKED j1939_rf_typ;

/** PDU HRVD (High Resolution Vehicle Distance) doc. in J1939 - 71, p170 */
typedef struct {
	timestamp_t timestamp;
	float vehicle_distance;
	float trip_distance;
} IS_PACKED j1939_hrvd_typ;

/** PDU EBC2 (Electronic Brake Controller 2) doc. in J1939 - 71, p170 */
typedef struct {
	timestamp_t timestamp;
	float EBC2_FrontAxleSpeed;
	float EBC2_RelativeSpeedFrontAxleLeftWheel;
	float EBC2_RlativeSpeedFrontAxleRightWheel;
	float EBC2_RelativeSpeedRearAxle1LeftWheel;
	float EBC2_RlativeSpeedRearAxle1RightWheel;
	float EBC2_RelativeSpeedRearAxle2LeftWheel;
	float EBC2_RlativeSpeedRearAxle2RightWheel;
} IS_PACKED j1939_ebc2_typ;

/** PDU EBC5 (Electronic Brake Controller 5)*/
typedef struct {
	timestamp_t timestamp;
        unsigned char EBC5_FoundationBrakeUse;
        unsigned char EBC5_HaltBrakeMode;
        float EBC5_XBRAccelerationLimit;
        unsigned char EBC5_XBRActiveControlMode;
} IS_PACKED j1939_ebc5_typ;

/** PDU CAN1 */
typedef struct {
	timestamp_t timestamp;
        unsigned char CAN1_ExtData;
        unsigned char CAN1_StdData;
        unsigned char CAN1_BusLoad;
} IS_PACKED j1939_can1_typ;

/** PDU CAN2 */
typedef struct {
	timestamp_t timestamp;
        unsigned char CAN2_BusLoad;
} IS_PACKED j1939_can2_typ;

/** PDU VDC2 */
typedef struct {
	timestamp_t timestamp;
	float VDC2_SteeringWheelAngle;
	float VDC2_YawRate;
	float VDC2_LateralAcceleration;
	float VDC2_LongitudinalAcceleration;
	char VDC2_SteeringWheelTurnCounter;
} IS_PACKED j1939_vdc2_typ;

/** PDU VP_X_TGW */
typedef struct {
	timestamp_t timestamp;
	float VP_X_TGW_Latitude_BB1_X_TGW;
	float VP_X_TGW_Longitude_BB1_X_TGW;
} IS_PACKED j1939_vp_x_typ;

/** PDU FD (Fan Drive) doc. in J1939 - 71, sec. 5.3.58 */
typedef struct {
	timestamp_t timestamp;
	float estimated_percent_fan_speed;
	int fan_drive_state;
} IS_PACKED j1939_fd_typ;

/** PDU EXAC (External Acceleration Control), WABCO proprietary */
typedef struct {
	timestamp_t timestamp;
	unsigned char ebs_override_control_mode_priority;
	unsigned char external_deceleration_control_mode;
	float requested_deceleration_to_ebs;
	unsigned char edc_override_control_mode_priority;
	unsigned char override_control_modes;
	float requested_torque_to_edc;
	unsigned char alive_signal;
	unsigned char acc_internal_status;
	unsigned char undefined;
	unsigned char checksum;
	unsigned char src_address;
} IS_PACKED j1939_exac_typ;


///** PDU VOLVO_XBR(Volvo brake message)*/
typedef struct {
	timestamp_t timestamp;
	float ExternalAccelerationDemand;	///
	unsigned char src_address;
	unsigned char destination_address;
	unsigned char pdu_format;
	unsigned char XBREBIMode;	/// 
	unsigned char XBRPriority;	/// 
	unsigned char XBRControlMode;	/// 
	unsigned char XBRUrgency;	/// 
	unsigned char spare1;		/// 0xFF
	unsigned char spare2;		/// 0xFF
	unsigned char spare3;		/// 0xFF
	unsigned char XBRMessageCounter;/// 
	unsigned char XBRMessageChecksum;/// 
} IS_PACKED j1939_volvo_xbr_typ;

/** PDU VOLVO_XBR_WARN (Volvo brake message)*/
typedef struct {
	timestamp_t timestamp;
	unsigned char src_address;
	unsigned char destination_address;
	unsigned char pdu_format;
	unsigned char byte1;///  0xFF
	unsigned char byte2;///  0x31
	unsigned char byte3;///  0xFF
	unsigned char byte4;///  0xFF
	unsigned char byte5;///  0xFF
	unsigned char byte6;///  0xFF
	unsigned char byte7;///  0xFF
	unsigned char byte8;///  0xFF
} IS_PACKED j1939_volvo_xbr_warn_typ;

/** PDU EBC_ACC (Electronic Brake Control for ACC), WABCO proprietary */
typedef struct {
	timestamp_t timestamp;
	float vehicle_mass;	/// 0 to 100 t 
	float road_slope;	/// -25% to +25% 
} IS_PACKED j1939_ebc_acc_typ;

/** PDU GFI2 (Gaseous Fuel Information 2), J1939-71, sec 5.3.123 */
typedef struct {
	timestamp_t timestamp;
	float fuel_flow_rate1;	/// m3/hour 
	float fuel_flow_rate2;	/// m3/hour
	float fuel_valve1_position;	/// 0 to 100% 
	float fuel_valve2_position;	/// 0 to 100% 
} IS_PACKED j1939_gfi2_typ;

/** PDU VOLVO_TARGET(Volvo target data)*/
typedef struct {
	timestamp_t timestamp;
	float TargetDist;	/// 0-655.35 m
	float TargetVel;	/// 0-655.35 m/sec
	float TargetAcc;	/// -327.68-327.67 m/sec/sec
	unsigned char TargetAvailable; ///0=no target, 1=target
} IS_PACKED j1939_volvo_target_typ;

/** PDU VOLVO_EGO(Volvo self data)*/
typedef struct {
	timestamp_t timestamp;
	float EgoVel;	/// 0-655.35 m/sec
	float EgoAcc;	/// -327.68-327.67 m/sec/sec
} IS_PACKED j1939_volvo_ego_typ;

/** PDU EI (Engine Information), J1939-71, sec 5.3.105 */
typedef struct {
	timestamp_t timestamp;
	float pre_filter_oil_pressure;	/// 0 to 1000 kPa 
	float exhaust_gas_pressure;	/// -250 to 251.99 kPa
	float rack_position;		/// 0 to 100% 
	float natural_gas_mass_flow;	/// 0 to 3212.75 kg/h 
	float instantaneous_estimated_brake_power;	/// 0 to 32127.5 kW 
} IS_PACKED j1939_ei_typ;

typedef struct {
	timestamp_t timestamp;
	float MVS_X_E_AppliedVehicleSpeedLimit_BB1_X_E;
} IS_PACKED j1939_mvs_x_e_typ;

/** Cummins J1587 broadcast group A */
typedef struct {
	timestamp_t timestamp;
	float road_speed;
	float percent_throttle;
	float percent_engine_load;
	float engine_speed;
} IS_PACKED j1587_enga_typ;

/** Cummins J1587 broadcast group B */
typedef struct {
	timestamp_t timestamp;
	unsigned char cruise_control_status;
	unsigned char engine_retarder_status;
	float fuel_rate;
	float instantaneous_mpg;
} IS_PACKED j1587_engb_typ;

/** Cummins J1587 broadcast group C */
typedef struct {
	timestamp_t timestamp;
	unsigned char transmitter_system_status;
	unsigned char idle_shutdown_timer_status;
	unsigned char road_speed_limit_status;
	unsigned char power_takeoff_status;
	float oil_pressure;
	float boost_pressure;
} IS_PACKED j1587_engc_typ;

/** Cummins J1587 broadcast group D */
typedef struct {
 	timestamp_t timestamp;
	float intake_air_temperature;
	float barometric_pressure;
  	float engine_coolant_temperature;
  	float volts;
  	float oil_temperature;
} IS_PACKED j1587_engd_typ;

/** Cummins J1587 broadcast group E */
typedef struct {
  	timestamp_t timestamp;
	float trans_sys_diag_code;  
} IS_PACKED j1587_enge_typ;

/** Cummins J1587 broadcast group F */
typedef struct {
  	timestamp_t timestamp;
	float cruise_control_set_MPH;
	float trip_fuel;
	float average_MPG;
} IS_PACKED j1587_engf_typ;

/** Cummins J1587 broadcast group G */
typedef struct {
  	timestamp_t timestamp;
	float trip_distance;
	float total_vehicle_miles;
} IS_PACKED j1587_engg_typ;

/** Transmission J1587 broadcast group */
typedef struct {
	timestamp_t timestamp;
	float hydraulic_retarder_oil_temp;
	float transmission_oil_temp;
	float transmission_output_shaft_speed;
	unsigned char transmission_retarder_status;
	int transmission_range_selected;
	int transmission_range_attained;
	unsigned char attention_warning_status;
} IS_PACKED j1587_trans_typ;

