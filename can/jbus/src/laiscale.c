/***\file
 * Routines that scale input byte(s) to units, as specified by 
 * Lane Assist Interface documentation.
 *
 * Copyright (c) 2005 Regents of the University of California
 *
 */

#include "old_include/std_jbus.h"

/** Routines return out of range values to indicate errors.
 *  Application programs are responsible for checking that values
 *  are in range.
 */

/** Values from 251 to 255 in high byte indicate errors, returned as increment
 *  to out-of-range value 1500.0
 *  Range from 0 to 2,880,000  corresponds to -1440.000 to +1440.000.
 *  Other values should never be sent.
 */
float scale_m1440_to_p1440(unsigned int data)
{
        if (BYTE3(data) <= 250) {
                return ((data/1000.0) - 1440.000);
        } else
                return(9999.999); 
}

/* Scale from -2.000 to 2.000 (used for ymeas and ycar)
 */
float scale_m2_to_p2(unsigned int data)
{
        if (BYTE3(data) <= 250) {
                return ((data/1000.0) - 2.000);
        } else
                return(9.999); 
}

/* Scale from -1.00000 to 1.00000 (used for the gyro rate)
 */
float scale_m1_to_p1(unsigned int data)
{
        if (BYTE3(data) <= 250)
                return ((data/100000.0) -1.00000);
        else
                return(9.99999);
}

/* Scale from -200 to 200 (used for lat_pos)
 */
int scale_m200_to_p200(unsigned int data)
{
        if (HIBYTE(data) <= 250)
                return (data - 200);
        else
                return(999);
}

/* Scale from -720 to 720 (used for lat_est)
 */
int scale_m720_to_p720(unsigned int data)
{
        if (HIBYTE(data) <= 250)
                return (data - 720);
        else
                return(999);
}

/* Range from 5000 to 45000 corresponds to 0.5 to 4.5 volts. 
 * Hibyte values from 251 to 255 indicate errors, returned as negative.
 */
float scale_10000_to_volt(unsigned int data)
{
        if (HIBYTE(data) <= 250)
                return ((float)(data/10000.0));
        else
                return(0.0 - HIBYTE(data));
}

/* Range from 0 to 1000 corresponds to 0 to 10 volts. 
 * Hibyte values from 251 to 255 indicate errors, returned as negative.
 */
float scale_100_to_volt(unsigned int data)
{
        if (HIBYTE(data) <= 250)
                return ((float)(data/100.0));
        else
                return(0.0 - HIBYTE(data));
}

/* Scale by 100, used for mag_dist
 */
float scale_by_100(unsigned int data)
{
        if (HIBYTE(data) <= 250)
                return (data * 0.01);
        else
                return(0.0 - HIBYTE(data));
}

/* Scale from msec/4 to sec (used for delta_timer_obs)
 */
float scale_to_sec(unsigned int data)
{
        if (BYTE3(data) <= 250) {
                return (data*4.0/1000.0);
        } else
                return(9.999); 
}
