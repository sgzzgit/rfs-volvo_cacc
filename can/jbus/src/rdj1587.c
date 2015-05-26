/**\file
 * 	Process to read J1587 data from serial port
 *	and write to database. Any database variables
 *	whose fields have been updated are written to
 *	the database at the end of the message.
 *
 * Copyright (c) 2005   Regents of the University of California
 *	Ported to QNX6 May 2005.
 *
 */
#include "old_include/std_jbus.h"

jmp_buf env;

static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	ERROR,
};

static void sig_hand( int sig)
{
	longjmp( env, 1);
}

static struct avcs_timing tmg;
extern int current_mid;

/** List of Known MID's */ 
unsigned char msg_j1587[] = {
   128,  
   130, 
   136, 
   140,
   142, 
   179,  
   219  
};

#define N_MIDS sizeof(msg_j1587)/sizeof(unsigned char)

/* local update variables, used to save most recently received values
 * for database writes
 */

j1587_enga_typ enga_update = {{0, 0, 0, 0}, 0.0, 0.0, 0.0, 0.0};
j1587_engb_typ engb_update = {{0, 0, 0, 0}, 0, 0, 0.0, 0.0};
j1587_engc_typ engc_update = {{0, 0, 0, 0}, 0, 0, 0, 0, 0.0, 0.0};
j1587_engd_typ engd_update = {{0, 0, 0, 0}, 0.0, 0.0, 0.0, 0.0, 0.0};
j1587_enge_typ enge_update = {{0, 0, 0, 0}, 0.0};
j1587_engf_typ engf_update = {{0, 0, 0, 0}, 0.0, 0.0, 0.0};
j1587_engg_typ engg_update = {{0, 0, 0, 0}, 0.0, 0.0};
j1587_trans_typ trans_update = {{0, 0, 0, 0}, 0.0, 0.0, 0.0, 0, 0, 0, 0};   

struct j1587_update update_list[] = {
	{&enga_update, DB_J1587_ENGA_TYPE, 0},
	{&engb_update, DB_J1587_ENGB_TYPE, 0},
	{&engc_update, DB_J1587_ENGC_TYPE, 0},
	{&engd_update, DB_J1587_ENGD_TYPE, 0},
	{&enge_update, DB_J1587_ENGE_TYPE, 0},
	{&engf_update, DB_J1587_ENGF_TYPE, 0},
	{&engg_update, DB_J1587_ENGG_TYPE, 0},	
	{&trans_update, DB_J1587_TRANS_TYPE, 0},
};

#define N_DBVS sizeof(update_list)/sizeof(struct j1587_update)

/* assumes update list is in order by consecutive database variable numbers */ 
#define UPDATE_NUM(dbn) ((dbn) - DB_J1587_ENGA_TYPE)

struct j1587_pid pid_j1587[256] = {
        {0, 0, NULL, -1, NULL, 0, NULL},
        {1, 0, NULL, -1, NULL, 0, NULL},
        {2, 1, convert_transmitter_system_status, DB_J1587_ENGC_TYPE,
		&engc_update.transmitter_system_status,
		TYPE_IS_UCHAR,
		&update_list[UPDATE_NUM(DB_J1587_ENGC_TYPE)].changed},
        {3, 0, NULL, -1, NULL, 0, NULL},
        {4, 0, NULL, -1, NULL, 0, NULL},
        {5, 0, NULL, -1, NULL, 0, NULL},
        {6, 0, NULL, -1, NULL, 0, NULL},
        {7, 0, NULL, -1, NULL, 0, NULL},
        {8, 0, NULL, -1, NULL, 0, NULL},
        {9, 0, NULL, -1, NULL, 0, NULL},
        {10, 0, NULL, -1, NULL, 0, NULL},
        {11, 0, NULL, -1, NULL, 0, NULL},
        {12, 0, NULL, -1, NULL, 0, NULL},
        {13, 0, NULL, -1, NULL, 0, NULL},
        {14, 0, NULL, -1, NULL, 0, NULL},
        {15, 0, NULL, -1, NULL, 0, NULL},
        {16, 0, NULL, -1, NULL, 0, NULL},
        {17, 0, NULL, -1, NULL, 0, NULL},
        {18, 0, NULL, -1, NULL, 0, NULL},
        {19, 0, NULL, -1, NULL, 0, NULL},
        {20, 0, NULL, -1, NULL, 0, NULL},
        {21, 0, NULL, -1, NULL, 0, NULL},
        {22, 0, NULL, -1, NULL, 0, NULL},
        {23, 0, NULL, -1, NULL, 0, NULL},
        {24, 0, NULL, -1, NULL, 0, NULL},
        {25, 0, NULL, -1, NULL, 0, NULL},
        {26, 0, NULL, -1, NULL, 0, NULL},
        {27, 0, NULL, -1, NULL, 0, NULL},
        {28, 0, NULL, -1, NULL, 0, NULL},
        {29, 0, NULL, -1, NULL, 0, NULL},
        {30, 0, NULL, -1, NULL, 0, NULL},
        {31, 0, NULL, -1, NULL, 0, NULL},
        {32, 0, NULL, -1, NULL, 0, NULL},
        {33, 0, NULL, -1, NULL, 0, NULL},
        {34, 0, NULL, -1, NULL, 0, NULL},
        {35, 0, NULL, -1, NULL, 0, NULL},
        {36, 0, NULL, -1, NULL, 0, NULL},
        {37, 0, NULL, -1, NULL, 0, NULL},
        {38, 0, NULL, -1, NULL, 0, NULL},
        {39, 0, NULL, -1, NULL, 0, NULL},
        {40, 0, NULL, -1, NULL, 0, NULL},
        {41, 0, NULL, -1, NULL, 0, NULL},
        {42, 0, NULL, -1, NULL, 0, NULL},
        {43, 0, NULL, -1, NULL, 0, NULL},
        {44, 1, convert_attention_warning_status, DB_J1587_TRANS_TYPE,
		 &trans_update.attention_warning_status,
		 TYPE_IS_UCHAR, 
		 &update_list[UPDATE_NUM(DB_J1587_TRANS_TYPE)].changed},
        {45, 0, NULL, -1, NULL, 0, NULL},
        {46, 0, NULL, -1, NULL, 0, NULL},
        {47, 1, convert_trans_retarder_status, DB_J1587_TRANS_TYPE,
		 &trans_update.transmission_retarder_status,
		 TYPE_IS_UCHAR, 
		 &update_list[UPDATE_NUM(DB_J1587_TRANS_TYPE)].changed},
        {48, 1, convert_extended_range_barometric_press, -1, NULL, 0, NULL},
        {49, 1, convert_ABS_control_status, -1, NULL, 0, NULL},
        {50, 0, NULL, -1, NULL, 0, NULL},
        {51, 1, convert_percent_throttle_valve_position, -1, NULL, 0, NULL},
        {52, 0, NULL, -1, NULL, 0, NULL},
        {53, 0, NULL, -1, NULL, 0, NULL},
        {54, 0, NULL, -1, NULL, 0, NULL},
        {55, 0, NULL, -1, NULL, 0, NULL},
        {56, 0, NULL, -1, NULL, 0, NULL},
        {57, 0, NULL, -1, NULL, 0, NULL},
        {58, 0, NULL, -1, NULL, 0, NULL},
        {59, 0, NULL, -1, NULL, 0, NULL},
        {60, 0, NULL, -1, NULL, 0, NULL},
        {61, 0, NULL, -1, NULL, 0, NULL},
        {62, 0, NULL, -1, NULL, 0, NULL},
        {63, 0, NULL, -1, NULL, 0, NULL},
        {64, 0, NULL, -1, NULL, 0, NULL},
        {65, 1, convert_brake_switch_status, -1, NULL, 0, NULL},
        {66, 0, NULL, -1, NULL, 0, NULL},
        {67, 0, NULL, -1, NULL, 0, NULL},
        {68, 1, convert_torque_limiting_factor, -1, NULL, 0, NULL},
        {69, 0, NULL, -1, NULL, 0, NULL},
        {70, 1, convert_parking_brake_switch_status, -1, NULL, 0, NULL},
        {71, 1, convert_idle_shutdown_timer_status, DB_J1587_ENGC_TYPE, 
		&engc_update.idle_shutdown_timer_status,
		TYPE_IS_UCHAR,
		&update_list[UPDATE_NUM(DB_J1587_ENGC_TYPE)].changed},
        {72, 0, NULL, -1, NULL, 0, NULL},
        {73, 0, NULL, -1, NULL, 0, NULL},
        {74, 0, NULL, -1, NULL, 0, NULL},
        {75, 0, NULL, -1, NULL, 0, NULL},
        {76, 0, NULL, -1, NULL, 0, NULL},
        {77, 1, convert_forward_rear_drive_axle_temp, -1, NULL, 0, NULL},
        {78, 1, convert_rear_rear_drive_axle_temp, -1, NULL, 0, NULL},
        {79, 0, NULL, -1, NULL, 0, NULL},
        {80, 0, NULL, -1, NULL, 0, NULL},
        {81, 0, NULL, -1, NULL, 0, NULL},
        {82, 0, NULL, -1, NULL, 0, NULL},
        {83, 1, convert_road_speed_limit_status, DB_J1587_ENGC_TYPE,
		&engc_update.road_speed_limit_status,
		TYPE_IS_UCHAR,
		&update_list[UPDATE_NUM(DB_J1587_ENGC_TYPE)].changed},
        {84, 1, convert_road_speed, DB_J1587_ENGA_TYPE,
		 &enga_update.road_speed,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_ENGA_TYPE)].changed},
        {85, 1, convert_cruise_cont_status, DB_J1587_ENGB_TYPE,
	 	 &engb_update.cruise_control_status,
	 	 TYPE_IS_UCHAR,
	 	 &update_list[UPDATE_NUM(DB_J1587_ENGB_TYPE)].changed},
        {86, 1, convert_cruise_cont_set_kph, DB_J1587_ENGF_TYPE,
		 &engf_update.cruise_control_set_MPH,
		 TYPE_IS_FLOAT,
		 &update_list[UPDATE_NUM(DB_J1587_ENGF_TYPE)].changed},
        {87, 0, NULL, -1, NULL, 0, NULL},
        {88, 0, NULL, -1, NULL, 0, NULL},
        {89, 1, convert_power_takeoff_status, DB_J1587_ENGC_TYPE,
		&engc_update.power_takeoff_status,
		TYPE_IS_UCHAR,
		&update_list[UPDATE_NUM(DB_J1587_ENGC_TYPE)].changed},
        {90, 0, NULL, -1, NULL, 0, NULL},
        {91, 1, convert_percent_throttle, DB_J1587_ENGA_TYPE,
		 &enga_update.percent_throttle,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_ENGA_TYPE)].changed},
        {92, 1, convert_percent_engine_load, DB_J1587_ENGA_TYPE,
		 &enga_update.percent_engine_load,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_ENGA_TYPE)].changed},
        {93, 1, convert_output_torque, -1, NULL, 0, NULL},
        {94, 0, NULL, -1, NULL, 0, NULL},
        {95, 0, NULL, -1, NULL, 0, NULL},
        {96, 1, convert_fuel_level, -1, NULL, 0, NULL},
        {97, 0, NULL, -1, NULL, 0, NULL},
        {98, 0, NULL, -1, NULL, 0, NULL},
        {99, 0, NULL, -1, NULL, 0, NULL},
        {100, 1, convert_engine_oil_pressure, DB_J1587_ENGC_TYPE,
		 &engc_update.oil_pressure,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_ENGC_TYPE)].changed},
        {101, 0, NULL, -1, NULL, 0, NULL},
        {102, 1, convert_boost_pressure, DB_J1587_ENGC_TYPE,
		 &engc_update.boost_pressure,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_ENGC_TYPE)].changed},
        {103, 1, convert_turbo_speed, -1, NULL, 0, NULL},
        {104, 0, NULL, -1, NULL, 0, NULL},
        {105, 1, convert_intake_manifold_air_temp, DB_J1587_ENGD_TYPE,
		 &engd_update.intake_air_temperature,
		 TYPE_IS_FLOAT,
		 &update_list[UPDATE_NUM(DB_J1587_ENGD_TYPE)].changed},
        {106, 1, convert_air_Inlet_press, -1, NULL, 0, NULL},
        {107, 0, NULL, -1, NULL, 0, NULL},
        {108, 1, convert_barometric_press, DB_J1587_ENGD_TYPE,
		 &engd_update.barometric_pressure,
		 TYPE_IS_FLOAT,
		 &update_list[UPDATE_NUM(DB_J1587_ENGD_TYPE)].changed},
        {109, 0, NULL, -1, NULL, 0, NULL},
        {110, 1, convert_engine_coolant_temp, DB_J1587_ENGD_TYPE,
		 &engd_update.engine_coolant_temperature,
		 TYPE_IS_FLOAT,
                 &update_list[UPDATE_NUM(DB_J1587_ENGD_TYPE)].changed},
        {111, 1, convert_coolant_level, -1, NULL, 0, NULL},
        {112, 0, NULL, -1, NULL, 0, NULL},
        {113, 0, NULL, -1, NULL, 0, NULL},
        {114, 0, NULL, -1, NULL, 0, NULL},
        {115, 0, NULL, -1, NULL, 0, NULL},
        {116, 0, NULL, -1, NULL, 0, NULL},
        {117, 0, NULL, -1, NULL, 0, NULL},
        {118, 0, NULL, -1, NULL, 0, NULL},
        {119, 0, NULL, -1, NULL, 0, NULL},
        {120, 1, convert_hydraulic_retarder_oil_temp, DB_J1587_TRANS_TYPE, 
		 &trans_update.hydraulic_retarder_oil_temp,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_TRANS_TYPE)].changed},
        {121, 1, convert_engine_retarder_status, DB_J1587_ENGB_TYPE,
		 &engb_update.engine_retarder_status,
		 TYPE_IS_UCHAR,
		 &update_list[UPDATE_NUM(DB_J1587_ENGB_TYPE)].changed}, 
        {122, 0, NULL, -1, NULL, 0, NULL},
        {123, 0, NULL, -1, NULL, 0, NULL},
        {124, 0, NULL, -1, NULL, 0, NULL},
        {125, 0, NULL, -1, NULL, 0, NULL},
        {126, 0, NULL, -1, NULL, 0, NULL},
        {127, 0, NULL, -1, NULL, 0, NULL},
        {128, 2, convert_comp_specific_para_req, -1, NULL, 0, NULL},
        {129, 0, NULL, -1, NULL, 0, NULL},
        {130, 0, NULL, -1, NULL, 0, NULL},
        {131, 0, NULL, -1, NULL, 0, NULL},
        {132, 0, NULL, -1, NULL, 0, NULL},
        {133, 0, NULL, -1, NULL, 0, NULL},
        {134, 0, NULL, -1, NULL, 0, NULL},
        {135, 0, NULL, -1, NULL, 0, NULL},
        {136, 0, NULL, -1, NULL, 0, NULL},
        {137, 0, NULL, -1, NULL, 0, NULL},
        {138, 0, NULL, -1, NULL, 0, NULL},
        {139, 0, NULL, -1, NULL, 0, NULL},
        {140, 0, NULL, -1, NULL, 0, NULL},
        {141, 0, NULL, -1, NULL, 0, NULL},
        {142, 0, NULL, -1, NULL, 0, NULL},
        {143, 0, NULL, -1, NULL, 0, NULL},
        {144, 0, NULL, -1, NULL, 0, NULL},
        {145, 0, NULL, -1, NULL, 0, NULL},
        {146, 0, NULL, -1, NULL, 0, NULL},
        {147, 0, NULL, -1, NULL, 0, NULL},
        {148, 0, NULL, -1, NULL, 0, NULL},
        {149, 0, NULL, -1, NULL, 0, NULL},
        {150, 0, NULL, -1, NULL, 0, NULL},
        {151, 2, convert_ATC_control_status, -1, NULL, 0, NULL},
        {152, 0, NULL, -1, NULL, 0, NULL},
        {153, 0, NULL, -1, NULL, 0, NULL},
        {154, 0, NULL, -1, NULL, 0, NULL},
        {155, 0, NULL, -1, NULL, 0, NULL},
        {156, 0, NULL, -1, NULL, 0, NULL},
        {157, 0, NULL, -1, NULL, 0, NULL},
        {158, 2, convert_switched_battery_potential, -1, NULL, 0, NULL},
        {159, 0, NULL, -1, NULL, 0, NULL},
        {160, 0, NULL, -1, NULL, 0, NULL},
        {161, 0, NULL, -1, NULL, 0, NULL},
        {162, 2, convert_transmission_range_sel, DB_J1587_TRANS_TYPE,
		 &trans_update.transmission_range_selected,
		 TYPE_IS_INT, 
		 &update_list[UPDATE_NUM(DB_J1587_TRANS_TYPE)].changed},
        {163, 2, convert_transmission_range_att, DB_J1587_TRANS_TYPE,
		 &trans_update.transmission_range_attained,
		 TYPE_IS_INT, 
		 &update_list[UPDATE_NUM(DB_J1587_TRANS_TYPE)].changed},
        {164, 0, NULL, -1, NULL, 0, NULL},
        {165, 0, NULL, -1, NULL, 0, NULL},
        {166, 0, NULL, -1, NULL, 0, NULL},
        {167, 0, NULL, -1, NULL, 0, NULL},
        {168, 2, convert_battery_volts, DB_J1587_ENGD_TYPE,
		 &engd_update.volts,
		 TYPE_IS_FLOAT,
                 &update_list[UPDATE_NUM(DB_J1587_ENGD_TYPE)].changed},
        {169, 0, NULL, -1, NULL, 0, NULL},
        {170, 0, NULL, -1, NULL, 0, NULL},
        {171, 2, convert_ambient_air_temp, -1, NULL, 0, NULL},
        {172, 2, convert_air_inlet_temp, -1, NULL, 0, NULL},
        {173, 2, convert_exhaust_gas_temp, -1, NULL, 0, NULL},
        {174, 2, convert_fuel_temp, -1, NULL, 0, NULL},
        {175, 2, convert_engine_oil_temp, DB_J1587_ENGD_TYPE,
		 &engd_update.oil_temperature,
		 TYPE_IS_FLOAT,
                 &update_list[UPDATE_NUM(DB_J1587_ENGD_TYPE)].changed},
        {176, 0, NULL, -1, NULL, 0, NULL},
        {177, 2, convert_transmission_oil_temp, DB_J1587_TRANS_TYPE,
		 &trans_update.transmission_oil_temp,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_TRANS_TYPE)].changed},
        {178, 0, NULL, -1, NULL, 0, NULL},
        {179, 0, NULL, -1, NULL, 0, NULL},
        {180, 0, NULL, -1, NULL, 0, NULL},
        {181, 0, NULL, -1, NULL, 0, NULL},
        {182, 2, convert_trip_fuel, DB_J1587_ENGF_TYPE,
                 &engf_update.trip_fuel,
                 TYPE_IS_FLOAT,
		 &update_list[UPDATE_NUM(DB_J1587_ENGF_TYPE)].changed},
        {183, 2, convert_fuel_rate, DB_J1587_ENGB_TYPE,
		 &engb_update.fuel_rate,
		 TYPE_IS_FLOAT,
		 &update_list[UPDATE_NUM(DB_J1587_ENGB_TYPE)].changed}, 
        {184, 2, convert_instantaneous_MPG, DB_J1587_ENGB_TYPE,
		 &engb_update.instantaneous_mpg,
		 TYPE_IS_FLOAT,
		 &update_list[UPDATE_NUM(DB_J1587_ENGB_TYPE)].changed}, 
        {185, 2, convert_avg_MPG, DB_J1587_ENGF_TYPE,
                 &engf_update.average_MPG,
                 TYPE_IS_FLOAT,
                 &update_list[UPDATE_NUM(DB_J1587_ENGF_TYPE)].changed},
        {186, 0, NULL, -1, NULL, 0, NULL},
        {187, 2, convert_power_takeoff_set_speed, -1, NULL, 0, NULL},
        {188, 0, NULL, -1, NULL, 0, NULL},
        {189, 0, NULL, -1, NULL, 0, NULL},
        {190, 2, convert_engine_speed, DB_J1587_ENGA_TYPE,
		 &enga_update.engine_speed,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_ENGA_TYPE)].changed},
        {191, 2, convert_transmission_output_shaft_speed, DB_J1587_TRANS_TYPE,
		 &trans_update.transmission_output_shaft_speed,
		 TYPE_IS_FLOAT, 
		 &update_list[UPDATE_NUM(DB_J1587_TRANS_TYPE)].changed},
        {192, 0, NULL, -1, NULL, 0, NULL},
        {193, 0, NULL, -1, NULL, 0, NULL},
        {194, 1, convert_trans_sys_diag_code, DB_J1587_ENGE_TYPE,
		 &enge_update.trans_sys_diag_code,
		 TYPE_IS_FLOAT,
                 &update_list[UPDATE_NUM(DB_J1587_ENGE_TYPE)].changed},
        {195, 0, NULL, -1, NULL, 0, NULL},
        {196, 0, NULL, -1, NULL, 0, NULL},
        {197, 0, NULL, -1, NULL, 0, NULL},
        {198, 0, NULL, -1, NULL, 0, NULL},
        {199, 0, NULL, -1, NULL, 0, NULL},
        {200, 0, NULL, -1, NULL, 0, NULL},
        {201, 0, NULL, -1, NULL, 0, NULL},
        {202, 0, NULL, -1, NULL, 0, NULL},
        {203, 0, NULL, -1, NULL, 0, NULL},
        {204, 0, NULL, -1, NULL, 0, NULL},
        {205, 0, NULL, -1, NULL, 0, NULL},
        {206, 0, NULL, -1, NULL, 0, NULL},
        {207, 0, NULL, -1, NULL, 0, NULL},
        {208, 0, NULL, -1, NULL, 0, NULL},
        {209, 0, NULL, -1, NULL, 0, NULL},
        {210, 0, NULL, -1, NULL, 0, NULL},
        {211, 0, NULL, -1, NULL, 0, NULL},
        {212, 0, NULL, -1, NULL, 0, NULL},
        {213, 0, NULL, -1, NULL, 0, NULL},
        {214, 0, NULL, -1, NULL, 0, NULL},
        {215, 0, NULL, -1, NULL, 0, NULL},
        {216, 0, NULL, -1, NULL, 0, NULL},
        {217, 0, NULL, -1, NULL, 0, NULL},
        {218, 0, NULL, -1, NULL, 0, NULL},
        {219, 0, NULL, -1, NULL, 0, NULL},
        {220, 0, NULL, -1, NULL, 0, NULL},
        {221, 0, NULL, -1, NULL, 0, NULL},
        {222, 0, NULL, -1, NULL, 0, NULL},
        {223, 0, NULL, -1, NULL, 0, NULL},
        {224, 0, NULL, -1, NULL, 0, NULL},
        {225, 0, NULL, -1, NULL, 0, NULL},
        {226, 0, NULL, -1, NULL, 0, NULL},
        {227, 0, NULL, -1, NULL, 0, NULL},
        {228, 5, convert_speed_sensor_calib, -1, NULL, 0, NULL},
        {229, 0, NULL, -1, NULL, 0, NULL},
        {230, 0, NULL, -1, NULL, 0, NULL},
        {231, 0, NULL, -1, NULL, 0, NULL},
        {232, 0, NULL, -1, NULL, 0, NULL},
        {233, 0, NULL, -1, NULL, 0, NULL},
        {234, 0, NULL, -1, NULL, 0, NULL},
        {235, 5, convert_total_idle_hours, -1, NULL, 0, NULL},
        {236, 5, convert_total_idle_fuel_used, -1, NULL, 0, NULL},
        {237, 0, NULL, -1, NULL, 0, NULL},
        {238, 0, NULL, -1, NULL, 0, NULL},
        {239, 0, NULL, -1, NULL, 0, NULL},
        {240, 0, NULL, -1, NULL, 0, NULL},
        {241, 0, NULL, -1, NULL, 0, NULL},
        {242, 0, NULL, -1, NULL, 0, NULL},
        {243, 0, NULL, -1, NULL, 0, NULL},
        {244, 5, convert_trip_distance, DB_J1587_ENGG_TYPE,
                 &engg_update.trip_distance,
                 TYPE_IS_FLOAT,
                 &update_list[UPDATE_NUM(DB_J1587_ENGG_TYPE)].changed},
        {245, 5, convert_total_vehicle_km, DB_J1587_ENGG_TYPE,
                 &engg_update.total_vehicle_miles,
                 TYPE_IS_FLOAT,
                 &update_list[UPDATE_NUM(DB_J1587_ENGG_TYPE)].changed},
        {246, 0, NULL, -1, NULL, 0, NULL},
        {247, 5, convert_total_engine_hours, -1, NULL, 0, NULL},
        {248, 5, convert_total_PTO_hours, -1, NULL, 0, NULL},
        {249, 0, NULL, -1, NULL, 0, NULL},
        {250, 5, convert_total_fuel_used, -1, NULL, 0, NULL},
        {251, 0, NULL, -1, NULL, 0, NULL},
        {252, 0, NULL, -1, NULL, 0, NULL},
        {253, 0, NULL, -1, NULL, 0, NULL},
        {254, 5, convert_data_link_escape, -1, NULL, 0, NULL},
        {255, 1, no_op, -1, NULL, 0, NULL}
};


int byte_position = 0;
int read_error = 0;
int num_messages = 0;

/** Look for the messages most frequently sent, from transmission */
int sync_on_known_message(int fp)
{
 /* 130 191	 0	 0 191												 */
 /* or																					*/
 /* 130 191	 0	 0 162	82	 5 163	82	67 142 */

	unsigned char buffer[1];
	unsigned short byte_count = 0;
	unsigned int byte_sum = 0;
	unsigned int checksum = 0;
	int init = FALSE;

	while(TRUE) {
		tmg.exec_num++;
		if (read(fp,&buffer[0],1)) {
		}
		else {
			/* EOF, Terminate Program */
			longjmp(env, 1);
		}

		++byte_position;
		if (buffer[0] == 130) {
		/* Found the First 130 */
			byte_sum = byte_sum + buffer[0];
			++byte_count; 
			while(TRUE) {
				if (read(fp,&buffer[0],1)) {
				} else {
					/* EOF, Terminate Program */
					longjmp(env, 1);
				}
				++byte_position;
				if (!init) {
				/* Look for 191 */
					if (buffer[0] == 191) {
						init = TRUE;
					} else {
						byte_count = 0;
						byte_sum = 0;
						break;
					}
				}
				++byte_count;
				/* Look for checksum with a correct length */
				if (checksum == buffer[0] &&
				 ((byte_count == 5) || (byte_count == 11)))
					 break;	/* valid message found */
				if (byte_count > J1587_MSG_MAX)
					 return(0); /* start over */	
				byte_sum = byte_sum + buffer[0];
				/* 1 Byte 2's Complement of byte_sum */
				checksum = (~byte_sum & 0xff)+0x01;
			} /*  end of loop to check MID 130 message */
			if (byte_count == 0) {	
				/* No message was found, continue looking */
			} else {
				break;
			}
		}
	}	/* end of loop looking for MID 130 */
#ifdef VERBOSE
	printf("Messages Synchronized @ byte_position = %d\n",byte_position);
#endif
	return(1);
}

/** Parses J1587 message.
 * It is assumed that a valid MID will be the first character encountered 
 * since sync_on_known_message has been called prior this		
 */ 
int get_j1587(
	int fp,			/// file descriptor for input
	unsigned char *msg_buf,	/// buffer to hold message
	int *msg_byte_count,	/// number of bytes in message
 
	int *pid_ptrs	/// pointers to PIDs within the message buffer 
)
{
	int i,j,num_data_chars;
	int mid = -1;
	unsigned int byte_sum = 0;
	unsigned int checksum = 0;
	int data_byte = FALSE;

	struct j1587_pid *pid_struct;
	unsigned char buffer[1];
	unsigned char *msg = &msg_buf[0];

	*msg_byte_count = 0;

	/* Look for a vaild MID */
	if (read(fp,&buffer[0],1)) {
	} else {
		/* EOF, Terminate Program */
		longjmp(env,1);
	}

	++byte_position;

	for(i=0;i<N_MIDS;++i) {
		if (buffer[0] == msg_j1587[i]) {
			mid = buffer[0];
			*(msg+(*msg_byte_count)) = mid;
			++(*msg_byte_count);
			byte_sum = byte_sum + buffer[0];
			break;
		}
	}

	if (mid == -1) {
		while(!sync_on_known_message(fp)) {
		}
		return 0; 
	}

	/* 1 Byte 2's Complement of byte_sum */
	/* Calculated Here Since Next Byte May be Zero and
	 * checksum is Currently = 0 */
	checksum = (~byte_sum & 0xff)+0x01;

	j = 0;

	/* We have a Valid MID, Look for checksum at a Position
	 * that is NOT PID Data */

	/* Make Sure the Next Char is a Known PID */

	while(TRUE) {
		if (read(fp,&buffer[0],1)) {
			if (data_byte) {
				--j;
			}
			*(msg+(*msg_byte_count)) = buffer[0];
			++byte_position;
			++(*msg_byte_count);
			if ((*msg_byte_count)>J1587_MSG_MAX) {
				/* Too Many Chars ReSync */
				while(!sync_on_known_message(fp)) {
				}
				return 0;
			} 
			if (checksum == buffer[0] &&
				 (*msg_byte_count != 19) && !data_byte) {
						break; /* valid message */
			}
			byte_sum = byte_sum + buffer[0];
			/* 1 Byte 2's Complement of byte_sum */
			checksum = (~byte_sum & 0xff)+0x01;

			if (!data_byte) {
				pid_struct = &pid_j1587[buffer[0]];
				num_data_chars = pid_struct->num_chars;

				/* Make Sure the Char is a Known PID,
				 * if num_data_chars = 0, It	
				 * is Not defined in pid_j1587 */
				if (num_data_chars == 0) {
					while(!sync_on_known_message(fp)) {
					}
					return 0;
				}
				/* Start Data Char Count */
				data_byte = TRUE;
				j = num_data_chars;
			}
			if (j==0) data_byte = FALSE;
		} else {
			/* at EOF */
			longjmp(env,1);
		}
	}

	return 1;
}

/**  Parse J1587 message and call PID conversion routines.
 *   Start at the First PID and Stop Before the checksum 
 */
int parse_j1587(unsigned char *msg_buf, int msg_byte_count) 
{
	int i,j,num_data_chars;
	int data[5];
	struct j1587_pid *pid;
	current_mid = *msg_buf;
	i=1;
	while(i<msg_byte_count-1) {
		pid = &pid_j1587[*(msg_buf+i)];

		/* Don't try to Parse  PID 255 */
		if (pid->pid == 255) break;
		num_data_chars = pid->num_chars;
		++i;
		for(j=0;j<num_data_chars;++j)
		{
			data[j] = *(msg_buf+i);
			++i;
		}
		if (pid->dbfield != NULL) {
			pid->convert_param(data, pid->dbfield);
			*(pid->changed_flag) = 1;
		}

	}
	return 1;	// error reporting?
}

/** Write J1587 message to database, if any of the parameters in the
 *  database variable have been updated.
 *  J1587 messages may not always have the same grouping of PIDs that
 *  is expected in the database variable structures.
 */
void write_j1587_to_database(db_clt_typ *pclt, struct j1587_update *list)

{
	int i;
	for (i = 0; i < N_DBVS; i++) {
		if (list[i].changed) {
			int db_num = list[i].db_num;
			j1939_dbv_info *info;
			info = &j1587_db_ref[db_num - J1587_DB_OFFSET];
			get_current_timestamp((timestamp_t *) list[i].dbv);
			if (!update_local_database(pclt, db_num,
				 info, list[i].dbv)) 
				printf("Error write %d to database\n", db_num);
			list[i].changed = 0;
		}
	}
}	

int main( int argc, char *argv[] )
{
	db_clt_typ *pclt;      /* Database pointer */
	int fpin;		/* file descriptor for serial port */
	char *fname = "/dev/ser7"; 
	char *ftmgname = "rdj1587.tmg"; 
	char *domain = DEFAULT_SERVICE;
	FILE *fptmg = NULL;	//
	int msg_length,pid_location;
	int ch;

        while ((ch = getopt(argc, argv, "a:d:f:")) != EOF) {
                switch (ch) {
		case 'a': ftmgname = strdup(optarg);
			  break;
		case 'd': domain = strdup(optarg);
			  break;
		case 'f': fname = strdup(optarg);
			  break;
                default: printf( "Usage: %s [-f <filename> \n", argv[0]);
			printf("-a <AVCS timing output file>\n");
			printf("-d <data server domain name>\n");
                          break;
                }
        }

	pclt = j1939_database_init(argv);
	printf("rdj1587 opens database 0x%08x\n", (unsigned int) pclt);
	fflush(stdout);

	/* Initialize serial port. */  
	fpin = open(fname, O_RDONLY);
	if ( fpin <= 0 ) {
		printf("Error opening device %s for input\n", fname);
		exit(EXIT_FAILURE);
	}

	/* Open file to hold user and system timing data
	 */
	fptmg = fopen(ftmgname, "w");
	if ( fptmg == NULL ) {
		printf("Error opening file %s for output\n", ftmgname);
		exit(EXIT_FAILURE);
	}

	avcs_start_timing(&tmg);
	/*	set jmp */
	if ( setjmp(env) != 0) {
		close(fpin);
		close_local_database(pclt);
		avcs_end_timing(&tmg);
		avcs_print_timing(fptmg, &tmg);
		printf("byte_position = %d\n",byte_position);
		printf("Total Number of Messages = %d\n",num_messages);
		printf("Number of Read Errors = %d\n",read_error);
		exit( EXIT_SUCCESS);
	}
	else 
		sig_ign( sig_list, sig_hand);
	while(!sync_on_known_message(fpin)) {
	}

	while(TRUE) {
		unsigned char j1587_msg[J1587_MSG_MAX];
		if (get_j1587(fpin,j1587_msg,&msg_length,&pid_location)) {
			++num_messages;
			parse_j1587(j1587_msg,msg_length);
			write_j1587_to_database(pclt, update_list);	
		} else {
			++read_error;
		}
	}
}			
