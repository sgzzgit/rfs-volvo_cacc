/*	FILE
 *
 *  mag.h    Version for communicating the data between QNX4 and QNX6
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 *
 *	static char rcsid[] = "$Id";
 *
 *
 *	$Log$
 *
 *
 */

/* Following is DB_SIGNALPROC_OUTPUT_TYPE, DB_SIGNALPROC_OUTPUT_VAR in database 
   (to be passed from magnetometer signal processing to lateral process) */
typedef struct
{
	short int mag_health;	/* Magnetometer health signals (1=OK, 0=fault)  */
	float mag_dist;		/* Calculated marker spacing in [m] 		*/
	float delta_timer_obs;	/* Difference between detection at the REAR and the FRONT */

	bool_typ f_mark_flag;	/* TRUE when a marker is detected, set in get_marker() */
   	int f_sensor;		/* Magnetometer which has the strongest signal	*/
	int f_polarity;		/* Magnet polarity (0 or 1)			*/
	float f_ymeas;		/* Actual lateral measurement in [m]            */
	float f_ycar;		/* Lateral error in [m] after trajectory planning */

	bool_typ r_mark_flag;   /* TRUE when a marker is detected, set in get_marker() */
   	int r_sensor;		/* Magnetometer which has the strongest signal	*/
	int r_polarity;		/* Magnet polarity (0 or 1)			*/
	float r_ymeas;		/* Actual lateral measurement in [m]            */
	float r_ycar;		/* Lateral error in [m] after trajectory planning */
} signalproc_output_typ;
