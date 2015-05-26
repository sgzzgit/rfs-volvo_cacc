/**\file
 *	Extern definitions for j1587scale.c
 */
#ifndef J1587SCALE_H
#define J1587SCALE_H
extern void convert_transmitter_system_status
   (int *param, void *pdbv); /* PID # 2 */
extern void convert_attention_warning_status
   (int *param, void *pdbv); /* PID # 44 */
extern void convert_trans_retarder_status
   (int *param, void *pdbv); /* PID # 47 */
extern void convert_extended_range_barometric_press
   (int *param, void *pdbv); /* PID # 48 */
extern void convert_ABS_control_status
   (int *param, void *pdbv); /* PID # 49 */
extern void convert_percent_throttle_valve_position
   (int *param, void *pdbv); /* PID # 51 */
extern void convert_brake_switch_status
   (int *param, void *pdbv); /* PID # 65 */
extern void convert_torque_limiting_factor
   (int *param, void *pdbv); /* PID # 68 */
extern void convert_parking_brake_switch_status
   (int *param, void *pdbv); /* PID # 70 */
extern void convert_idle_shutdown_timer_status
   (int *param, void *pdbv); /* PID # 71 */
extern void convert_forward_rear_drive_axle_temp
   (int *param, void *pdbv); /* PID # 77 */
extern void convert_rear_rear_drive_axle_temp
   (int *param, void *pdbv); /* PID # 78 */
extern void convert_road_speed_limit_status
   (int *param, void *pdbv); /* PID # 83 */
extern void convert_road_speed
   (int *param, void *pdbv); /* PID # 84 */
extern void convert_cruise_cont_status
   (int *param, void *pdbv); /* PID # 85 */
extern void convert_cruise_cont_set_kph
   (int *param, void *pdbv); /* PID # 86 */
extern void convert_power_takeoff_status
   (int *param, void *pdbv); /* PID # 89 */
extern void convert_percent_throttle
   (int *param, void *pdbv); /* PID # 91 */
extern void convert_percent_engine_load
   (int *param, void *pdbv); /* PID # 92 */
extern void convert_output_torque
   (int *param, void *pdbv); /* PID # 93 */
extern void convert_fuel_level
   (int *param, void *pdbv); /* PID # 96 */
extern void convert_engine_oil_pressure
   (int *param, void *pdbv); /* PID # 100 */
extern void convert_boost_pressure
   (int *param, void *pdbv); /* PID # 102 */
extern void convert_turbo_speed
   (int *param, void *pdbv); /* PID # 103 */
extern void convert_intake_manifold_air_temp
   (int *param, void *pdbv); /* PID # 105 */
extern void convert_air_Inlet_press
   (int *param, void *pdbv); /* PID # 106 */
extern void convert_barometric_press
   (int *param, void *pdbv); /* PID # 108 */
extern void convert_engine_coolant_temp
   (int *param, void *pdbv); /* PID # 110 */
extern void convert_coolant_level
   (int *param, void *pdbv); /* PID # 111 */
extern void convert_hydraulic_retarder_oil_temp
   (int *param, void *pdbv); /* PID # 120 */
extern void convert_engine_retarder_status
   (int *param, void *pdbv); /* PID # 121 */
extern void convert_comp_specific_para_req
   (int *param, void *pdbv); /* PID # 128 */
extern void convert_ATC_control_status
   (int *param, void *pdbv); /* PID # 151 */
extern void convert_switched_battery_potential
   (int *param, void *pdbv); /* PID # 158 */
extern void convert_transmission_range_sel
   (int *param, void *pdbv); /* PID # 162 */
extern void convert_transmission_range_att
   (int *param, void *pdbv); /* PID # 163 */
extern void convert_battery_volts
   (int *param, void *pdbv); /* PID # 168 */
extern void convert_ambient_air_temp
   (int *param, void *pdbv); /* PID # 171 */
extern void convert_air_inlet_temp
   (int *param, void *pdbv); /* PID # 172 */
extern void convert_exhaust_gas_temp
   (int *param, void *pdbv); /* PID # 173 */
extern void convert_fuel_temp
   (int *param, void *pdbv); /* PID # 174 */
extern void convert_engine_oil_temp
   (int *param, void *pdbv); /* PID # 175 */
extern void convert_transmission_oil_temp
   (int *param, void *pdbv); /* PID # 177 */
extern void convert_trip_fuel
   (int *param, void *pdbv); /* PID # 182 */
extern void convert_fuel_rate
   (int *param, void *pdbv); /* PID # 183 */
extern void convert_instantaneous_MPG
   (int *param, void *pdbv); /* PID # 184 */
extern void convert_avg_MPG
   (int *param, void *pdbv); /* PID # 185 */
extern void convert_power_takeoff_set_speed
   (int *param, void *pdbv); /* PID # 187 */
extern void convert_engine_speed
   (int *param, void *pdbv); /* PID # 190 */
extern void convert_transmission_output_shaft_speed
   (int *param, void *pdbv); /* PID # 191 */
extern void convert_trans_sys_diag_code
   (int *param, void *pdbv); /* PID # 194 */
extern void convert_speed_sensor_calib
   (int *param, void *pdbv); /* PID # 228 */
extern void convert_total_idle_hours
   (int *param, void *pdbv); /* PID # 235 */
extern void convert_total_idle_fuel_used
   (int *param, void *pdbv); /* PID # 236 */
extern void convert_trip_distance
   (int *param, void *pdbv); /* PID # 244 */
extern void convert_total_vehicle_km
   (int *param, void *pdbv); /* PID # 245 */
extern void convert_total_engine_hours
   (int *param, void *pdbv); /* PID # 247 */
extern void convert_total_PTO_hours
   (int *param, void *pdbv); /* PID # 248 */
extern void convert_total_fuel_used
   (int *param, void *pdbv); /* PID # 250 */
extern void convert_data_link_escape
   (int *param, void *pdbv); /* PID # 254 */
extern void no_op
   (int *param, void *pdbv); /* PID # 255 */

/* print routines used in j1939_dbv_info */

extern void print_enga
	(void *pdbv, FILE *fp, int numeric);
extern void print_engb
	(void *pdbv, FILE *fp, int numeric);
extern void print_engc
	(void *pdbv, FILE *fp, int numeric);
extern void print_engd
	(void *pdbv, FILE *fp, int numeric);
extern void print_enge
	(void *pdbv, FILE *fp, int numeric);
extern void print_engf
	(void *pdbv, FILE *fp, int numeric);
extern void print_engg
	(void *pdbv, FILE *fp, int numeric);
extern void print_trans
	(void *pdbv, FILE *fp, int numeric);
#endif
