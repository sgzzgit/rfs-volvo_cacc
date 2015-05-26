/**\file
 * Header file with declarations for j1939scale.c
 *
 * static char rcsid[] = "$Id: j1939scale.h,v 1.2 2003/05/23 17:33:34 dickey Exp $";
 *
 * "$Log: j1939scale.h,v $
 * "Revision 1.2  2003/05/23 17:33:34  dickey
 * "fixed problems do to changed comments
 * "
 * "Revision 1.1.1.1  2003/01/22 18:31:46  dickey
 * "SAE J1939 and J1587 code
 * "
 * "Revision 2.3  2002/12/21 00:13:29  dickey
 * "add code_engine_speed, fix brake_demand.
 * "
 * "Revision 2.2  2002/11/21 22:10:01  dickey
 * "Declarations for additional bus and brake messages.
 * "
 * "Revision 2.1  2002/07/24 21:39:39  dickey
 * "Check in of San Diego test code
 * "
 * "Revision 1.1  2002/05/22 22:56:51  dickey
 * "Initial revision
 * "
 */

#ifndef J1939SCALE_H
#define J1939SCALE_H

extern unsigned char code_percent_m125_to_p125(float percent);
extern unsigned short code_engine_speed(float speed);
extern float percent_0_to_100(unsigned char data);
extern float percent_0_to_250(unsigned char data);
extern float percent_m25_to_p25(unsigned char data);
extern float percent_m125_to_p125(unsigned char data);
extern int gear_m125_to_p125(unsigned char data);
extern float gear_ratio(unsigned short data);
extern float pressure_0_to_4000kpa(unsigned char data);
extern float pressure_0_to_1000kpa(unsigned char data);
extern float pressure_0_to_500kpa(unsigned char data);
extern float pressure_0_to_125kpa(unsigned char data);
extern float pressure_0_to_12kpa(unsigned char data);
extern float pressure_m250_to_p252kpa(unsigned short data);
extern float rotor_speed_in_rpm(unsigned short data);
extern float distance_in_km(unsigned int data);
extern float hr_distance_in_km(unsigned int data);
extern float speed_in_rpm_1byte(unsigned char data);
extern float speed_in_rpm_2byte(unsigned short data);
extern float wheel_based_mps(unsigned short data);
extern float wheel_based_mps_relative(unsigned char data);
extern float cruise_control_set_meters_per_sec(unsigned char data);
extern float fuel_rate_cm3_per_sec(unsigned short data);
extern float fuel_economy_meters_per_cm3(unsigned short data);
extern float torque_in_nm(unsigned short data);
extern float time_0_to_25sec(unsigned char data);
extern float gain_in_kp(unsigned short data);
extern float temp_m40_to_p210(unsigned char data);
extern float temp_m273_to_p1735(unsigned short data);
extern float current_m125_to_p125amp(unsigned char data);
extern float current_0_to_250amp(unsigned char data);
extern float voltage(unsigned short data);
extern float mass_0_to_100t(unsigned char data);
extern float brake_demand(unsigned char data);
extern float mass_flow(unsigned short data);
extern float power_in_kw(unsigned short data);
extern float mass_0_to_100t(unsigned char data);
extern short deceleration_to_short(float decel);
extern float short_to_deceleration(short data);

#endif
