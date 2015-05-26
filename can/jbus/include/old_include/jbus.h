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
	unsigned char override_control_mode_priority;
	unsigned char requested_speed_control_conditions;
	unsigned char override_control_modes;
	float requested_speed_or_limit;		/// RPM 
	float requested_torque_or_limit;	/// percent reference torque 
	int destination_address;	/// engine or retarder 
	int src_address;	/// sent in header, important for logging 
} IS_PACKED j1939_tsc1_typ;

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
	unsigned char enable_shift_assist_status;
	unsigned char enable_brake_assist_status;
	unsigned char engine_retarder_torque_mode;
	float actual_retarder_percent_torque;
	float intended_retarder_percent_torque;	
	unsigned char coolant_load_increase;	
	unsigned char source_address;	
} IS_PACKED j1939_erc1_typ;

/** PDU EBC1 (Electronic Brake Controller #1) doc. in J1939 - 71, p151 */
typedef struct {
	timestamp_t timestamp;
	unsigned char ebs_brake_switch_status;
	unsigned char abs_active_status;
	unsigned char asr_brake_control_active_status;
	unsigned char asr_engine_control_active_status;
	float brake_pedal_position;
	unsigned char traction_control_override_switch_status;
	unsigned char asr_hill_holder_switch_status;
	unsigned char asr_off_road_switch_status;
	unsigned char abs_off_road_switch_status;
	unsigned char remote_accelerator_enable_switch_status;
	unsigned char auxiliary_engine_shutdown_switch_status;
	unsigned char engine_derate_switch_status;
	unsigned char accelerator_interlock_switch_status;
	float percent_engine_retarder_torque_selected;
	unsigned char abs_ebs_amber_warning_state;
	unsigned char ebs_red_warning_state;
	unsigned char abs_fully_operational;
	unsigned char source_address;
	float total_brake_demand;
} IS_PACKED j1939_ebc1_typ;

/** PDU ETC1 (Elec. Transmission Controller #1) doc. in J1939 - 71, p151 */
typedef struct {
	timestamp_t timestamp;
	float output_shaft_speed;
	float percent_clutch_slip;	
	float input_shaft_speed;
	unsigned char shift_in_progress;
	unsigned char torque_converter_lockup_engaged;
	unsigned char driveline_engaged;
	unsigned char progressive_shift_disable;
	unsigned char momentary_engine_overspeed_enable;
	unsigned char source_address;
} IS_PACKED j1939_etc1_typ;

/** PDU EEC1 (Electronic Engine Controller #1) doc. in J1939 - 71, p152 */
typedef struct {
	timestamp_t timestamp;
	char engine_retarder_torque_mode;
	float driver_demand_percent_torque;
	float actual_engine_percent_torque;	
	float engine_speed;	
	unsigned char source_address;	/// not supported by Cummins? 
} IS_PACKED j1939_eec1_typ;

/** PDU EEC2 (Electronic Engine Controller #2) doc. in J1939 - 71, p152 */
typedef struct {
	timestamp_t timestamp;
	unsigned char road_speed_limit_inactive;
	unsigned char kickdown_active;
	unsigned char low_idle;
	float accelerator_pedal_position;
	float percent_load_current_speed;
	float remote_accelerator_position;
} IS_PACKED j1939_eec2_typ;
 
/** PDU ETC2 (Electronic Transmission Controller #2) doc. in J1939 - 71, p152 */
typedef struct {
	timestamp_t timestamp;
	char selected_gear;
	float actual_gear_ratio;
	char current_gear;
	char transmission_requested_range[2];
	char transmission_current_range[2];
} IS_PACKED j1939_etc2_typ;
 
/** PDU TURBO (Turbocharger) doc. in J1939 - 71, p153 */
typedef struct {
	timestamp_t timestamp;
	float turbocharger_lube_oil_pressure;
	float turbocharger_speed;
} IS_PACKED j1939_turbo_typ;
 
/** PDU EEC3 (Electronic Engine Controller #3) doc. in J1939 - 71, p154 */
typedef struct {
	timestamp_t timestamp;
	float nominal_friction_percent_torque;
	float engine_desired_operating_speed;
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
	unsigned char parking_brake;
	unsigned char two_speed_axle; 
	float wheel_based_vehicle_speed;
	unsigned char clutch_switch;
	unsigned char brake_switch;
	unsigned char cruise_control_enable;
	unsigned char cruise_control_active;
	unsigned char cruise_control_accelerate;
	unsigned char cruise_control_resume;
	unsigned char cruise_control_coast;
	unsigned char cruise_control_set;
	float cruise_control_set_speed;
	unsigned char cruise_control_state;
	unsigned char pto_state;
	unsigned char engine_shutdown_override;
	unsigned char engine_test_mode;
	unsigned char idle_decrement;
	unsigned char idle_increment;
} IS_PACKED j1939_ccvs_typ;

/** PDU LFE (Fuel Economy) doc. in J1939 - 71, p162 */
typedef struct {
	timestamp_t timestamp;
	float fuel_rate;
	float instantaneous_fuel_economy;
	float average_fuel_economy;
	float throttle_position;
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
	float front_axle_speed;
	float front_left_wheel_relative;
	float front_right_wheel_relative;
	float rear1_left_wheel_relative;
	float rear1_right_wheel_relative;
	float rear2_left_wheel_relative;
	float rear2_right_wheel_relative;
} IS_PACKED j1939_ebc2_typ;

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

/** PDU EI (Engine Information), J1939-71, sec 5.3.105 */
typedef struct {
	timestamp_t timestamp;
	float pre_filter_oil_pressure;	/// 0 to 1000 kPa 
	float exhaust_gas_pressure;	/// -250 to 251.99 kPa
	float rack_position;		/// 0 to 100% 
	float natural_gas_mass_flow;	/// 0 to 3212.75 kg/h 
	float instantaneous_estimated_brake_power;	/// 0 to 32127.5 kW 
} IS_PACKED j1939_ei_typ;

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

