/***\file
 * Routines that scale input byte(s) to units, as specified by 
 * Lane Assist Interface documentation.
 *
 * Copyright (c) 2005 Regents of the University of California
 *
 */

//extern float steer_angle_m1440_to_p1440(unsigned int data);
//extern float tenth_mvolt_to_volt(unsigned short data);

extern float scale_m1440_to_p1440(unsigned int data);
extern float scale_m2_to_p2(unsigned int data);
extern float scale_m1_to_p1(unsigned int data);
extern int scale_m200_to_p200(unsigned int data);
extern int scale_m720_to_p720(unsigned int data);
extern float scale_10000_to_volt(unsigned int data);
extern float scale_100_to_volt(unsigned int data);
extern float scale_by_100(unsigned int data);
extern float scale_to_sec(unsigned int data);

