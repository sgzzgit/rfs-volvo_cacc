/***\file
 * Routines that scale input byte(s) to units, as specified by SAE docs. 
 *
 * Copyright (c) 2005 Regents of the University of California
 *
 * Ported to QNX6 May 2005
 */

#include "std_jbus_extended.h"

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 100% range of return values
 */
float
percent_0_to_100(unsigned char data)

{
        if (data <= 250)
                return (data * 0.004 * 100.0);
        else
                return(0.0 - data);
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 250% range of return values
 */
float
percent_0_to_250(unsigned char data)

{
        if (data <= 250)
                return (data * 1.0);
        else
                return(0.0 - data);
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to -125 to 125% range of return values
 */
float
percent_m25_to_p25(unsigned char data)

{
        if (data <= 250)
                return (data/5 - 25.0);
        else
                return(0.0 - data);
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to -125 to 125% range of return values
 */
float
percent_m125_to_p125(unsigned char data)

{
        if (data <= 250)
                return (data - 125.0);
        else
                return(0.0 - data);
}

/** Reverse routine used when coding up a percent to send
 */
unsigned char
code_percent_m125_to_p125(float percent)
{
	if (percent > 125 || percent < -125)
		return 255;	/** out of range */
	return(ceil(percent - 0.5) + 125);
}

/** Values of 251 indicates park. Values 252 to 255 indicate errors, returned
 * as negative.  Range from 0 to 250 corresponds to -125 to 125 range of
 * return values
 */
int
gear_m125_to_p125(unsigned char data)

{
	int val = data;
        if (val <= 250)
                return (val - 125);
        else if (val == 251)
		return 251;
	else
                return(0 - val);
}

/** Hibyte values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 64255 corresponds to 0 to 64.255 range of return values
 */
float
gear_ratio(unsigned short data)

{
        if (HIBYTE(data) <= 250)
                return (data * 0.001);
        else
                return(0.0 - HIBYTE(data));
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 4000 kilopascals pressure
 */
float
pressure_0_to_4000kpa(unsigned char data)

{
        if (data <= 250)
                return (data * 16.0);
        else
                return(0.0 - data);
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 1000 kilopascals pressure
 */
float
pressure_0_to_1000kpa(unsigned char data)

{
        if (data <= 250)
                return (data * 4.0);
        else
                return(0.0 - data);
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 500 kilopascals pressure
 */
float
pressure_0_to_500kpa(unsigned char data)

{
        if (data <= 250)
                return (data * 2.0);
        else
                return(0.0 - data);
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 125 kilopascals pressure
 */
float
pressure_0_to_125kpa(unsigned char data)

{
        if (data <= 250)
                return (data * 0.5);
        else
                return(0.0 - data);
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 12.5 kilopascals pressure
 */
float
pressure_0_to_12kpa(unsigned char data)

{
        if (data <= 250)
                return (data * 0.05);
        else
                return(0.0 - data);
}

/** Hibyte values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 64255 corresponds to -250 to 251.96 kilopascals pressure
 */
float
pressure_m250_to_p252kpa(unsigned short data)

{
        if (HIBYTE(data) <= 250)
                return (data/128 - 250.0);
        else
                return(0.0 - data);
}

/** Hibyte values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 64255 corresponds to 0 to 257,020 range of return values
 */
float
rotor_speed_in_rpm(unsigned short data)

{
        if (HIBYTE(data) <= 250)
                return (data * 4.0);
        else
                return(0.0 - HIBYTE(data));
}

/** Upper byte values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 4211081215 corresponds to 0 to 526,385,151.875 km
 */
float
distance_in_km(unsigned int data)

{
        if (BYTE3(data) <= 250)
                return (data * 0.125);
        else
                return(0.0 - BYTE3(data));
}

/** Upper byte values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 4,211,081,215 corresponds to 0 to 21,055,406.075 km
 */
float
hr_distance_in_km(unsigned int data)

{
        if (BYTE3(data) <= 250)
                return (data * 0.005);
        else
                return(0.0 - BYTE3(data));
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 2500 range of return values
 */
float
speed_in_rpm_1byte(unsigned char data)

{
        if (data <= 250)
                return (data * 10.0);
        else
                return(0.0 - data);
}

/** Values of upper byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 64255 corresponds to 0 to 8031.875 range of return values
 */
float
speed_in_rpm_2byte(unsigned short data)

{
        if (HIBYTE(data) <= 250)
                return (data * 0.125);
        else
                return(0.0 - HIBYTE(data));
}

/** Reverse routine used to code a float into a 2-byte engine speed for
 * transmission..
 */
unsigned short
code_engine_speed(float speed)
{
	if (speed > 8031.875 || speed < 0)
		return 255 << 8;	/** out of range */
	return (ceil(speed * 8 - 0.5));
}

/** Values of upper byte, 0 to 250, in km/h; values of lower byte, 0 to 255.
 * scaled by 1/256 to give fraction of km/h. Converted to m/sec to agree
 * with units used by AVCS programs. Values of upper byte from 251 to 255
 * indicate errors, returned as negative. Range from 0 to 69.721 meters/sec.
 */

float
wheel_based_mps(unsigned short data)
{
	if (HIBYTE(data) <= 250)
		return((HIBYTE(data) + LOBYTE(data)/256.0) * (1000.0/3600.0)); 
	else
		return(0.0 - HIBYTE(data));
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to -2.170 to +2.170 meters/sec 
 */
float
wheel_based_mps_relative(unsigned char data)

{
        if (data <= 250)
                return ((data * 0.0625 - 7.8125) * (1000.0/3600.0));
        else
                return(0.0 - data);
}
	
/** Values of byte, 0 to 250, in km/h; donverted to m/sec to agree
 * with units used by AVCS programs. Values of byte from 251 to 255
 * indicate errors, returned as negative.
 */

float
cruise_control_set_meters_per_sec(unsigned char data)
{
	if (data <= 250)
		return(data * (1000.0/3600.0)); 
	else
		return(0.0 - data);
}

/** Values of upper byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 64255 corresponds to 0 to 3212.75 liters/hour or
 * 0 to 892.430 cubic centimeters per second..
 */
float
fuel_rate_cm3_per_sec(unsigned short data)
{
        if (HIBYTE(data) <= 250)
                return (data * 0.05 * 1000.0/3600.0);
        else
                return(0.0 - HIBYTE(data));
}

/** Values of upper byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 64555 corresponds to 0 to 128.498 meters/cubic centimeter
 */
float
fuel_economy_meters_per_cm3(unsigned short data)
{
        if (HIBYTE(data) <= 250)
                return (data * (1.0/512.0));
        else
                return(0.0 - HIBYTE(data));
}

/** Values of upper byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 64255 Nm
 */
float
torque_in_nm(unsigned short data)
{
        if (HIBYTE(data) <= 250)
                return (data * (1.0));
        else
                return(0.0 - HIBYTE(data));
}

/** Values of byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 250 scale to 0 to 25 seconds.
 */
float
time_0_to_25sec(unsigned char data)
{
        if (data <= 250)
                return (data * (1.0/10.0));
        else
                return(0.0 - data);
}

/** Values of byte from 251 to 255 indicate errors, returned as negative.
 * Range of 0 to 50.2% reference engine torque/RPM.
 */
float
gain_in_kp(unsigned short data)
{
        if (HIBYTE(data) <= 250)
                return (data * (1.0/1280.0));
        else
                return(0.0 - HIBYTE(data));
}

/** Values of byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 250 scale to -40 to +210 degrees Celsius.
 */
float
temp_m40_to_p210(unsigned char data)
{
        if (data <= 250)
                return (data - 40.0);
        else
                return(0.0 - data);
}

/** Values of byte from 251 to 255 indicate errors, returned as negative.
 * Range of -273 to +1735 degrees Celsius. Since -251 to -255 in allowed
 * range, multiply error indicators by 10 before subtracting from 0.
 */
float
temp_m273_to_p1735(unsigned short data)
{
        if (HIBYTE(data) <= 250)
                return (data * 0.03125 - 273.0);
        else
                return(0.0 - 10.0*HIBYTE(data));
}

/** Values of byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 250 scale to -125 to +125 amperes.
 */
float
current_m125_to_p125amp(unsigned char data)
{
        if (data <= 250)
                return (data - 125.0);
        else
                return(0.0 - data);
}

/** Values of byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 250 in amperes.
 */
float
current_0_to_250amp(unsigned char data)
{
        if (data <= 250)
                return (data - 0.0);
        else
                return(0.0 - data);
}

/** Values of byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 250 scale to 0 to 3212.75 V.
 */
float
voltage(unsigned short data)
{
        if (HIBYTE(data) <= 250)
                return (data * 0.05);
        else
                return(0.0 - HIBYTE(data));
}

/** Values of byte from 251 to 255 indicate errors, returned. 
 * Range from 0 to 250 scale to -10 to 0 m/s^2.
 */
float
brake_demand(unsigned char data)
{
        if (data <= 250)
                return (data * 0.04 - 10.0);
        else
                return(data);
}

/** Values of high byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 64255 scale to 0 to 3212.75 kg/h.
 */
float
mass_flow(unsigned short data)
{
        if (HIBYTE(data) <= 250)
                return (data * 0.05);
        else
                return(0.0 - HIBYTE(data));
}

/** Values of high byte from 251 to 255 indicate errors, returned as negative.
 * Range from 0 to 64255 scale to 0 to 32127.5 kW.
 */
float
power_in_kw(unsigned short data)
{
        if (HIBYTE(data) <= 250)
                return (data * 0.5);
        else
                return(0.0 - HIBYTE(data));
}

/** Values from 251 to 255 indicate errors, stored as negative.
 * range from 0 to 250 corresponds to 0 to 100 metric tons
 */
float
mass_0_to_100t(unsigned char data)

{
        if (data <= 250)
                return (data * 0.4);
        else
                return(0.0 - data);
}

/** Value of 0.0 m/s^2 converts to 25000, value of -10.0 m/s^2 to 0,
 * values above 25000 are ignored, per WABCO EXAC docs 
 */
short
deceleration_to_short(float decel)
{
	decel *= -2500.0;
	return (25000 - (short) decel);
} 

/** for use in printing of EXAC message, decodes two byte value to float m/s^2 */
float
short_to_deceleration(short data)
{
	if (data > 25000)
		return (0xffff);	/** error value */
	else
		return ( (data - 25000) * 0.0004);
}
	


