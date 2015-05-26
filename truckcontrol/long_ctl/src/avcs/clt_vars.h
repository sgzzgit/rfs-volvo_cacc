/* FILE
 * clt_vars.h  Version for Demo 2003 truck.
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 * As a convention, the variable name/type space is partitioned as
 * follows:
 *
 *	0	to	99		Used by the system.
 *	100	to	199		Reserved.
 *	200	to	299		Permanent longitudinal variables.
 *	300	to	399		Permanent lateral variables.
 *	400	to	499		Permanent communications variables.
 *	1000 to	1099	Temporary variables.
 */

#ifndef CLT_VARS_H
#define CLT_VARS_H

/*	Longitudinal variables
 */

#define DB_LONG_INPUT_TYPE	200  /* long_input_typ          */
#define DB_LONG_OUTPUT_TYPE	201  /* long_output_typ         */
#define DB_SELF_GPS_POINT_TYPE	202  /* path_gps_point_t	    */  
//#define DB_GPS_VTG_TYPE	203  / no longer usedp             */
//#define DB_LONG_EVRADAR_TYPE	204  /* long_evradar_typ        */
#define DB_LONG_DIG_IN_TYPE		205  /* dig_in_typ */	
#define DB_LONG_DIG_OUT_TYPE		206  /* dig_out_typ */

#define DB_LONG_INPUT_VAR	DB_LONG_INPUT_TYPE
#define DB_LONG_OUTPUT_VAR	DB_LONG_OUTPUT_TYPE
#define DB_SELF_GPS_POINT_VAR	DB_SELF_GPS_POINT_TYPE
//#define DB_GPS_VTG_VAR	DB_GPS_VTG_TYPE
//#define DB_LONG_EVRADAR_VAR	DB_LONG_EVRADAR_TYPE
#define DB_LONG_DIG_IN_VAR		DB_LONG_DIG_IN_TYPE
#define DB_LONG_DIG_OUT_VAR		DB_LONG_DIG_OUT_TYPE


/*	Lateral variables.
 */

#define DB_LAT_INPUT_MAG_TYPE       300  /* lat_input_mag_typ		*/
#define DB_LAT_INPUT_SENSORS_TYPE   301  /* lat_input_sensors_typ	*/
#define DB_LAT_OUTPUT_TYPE          302  /* lat_output_typ			*/
#define DB_LAT_STEER_INPUT_TYPE     303  /* lat_steer_input_typ     */
#define DB_LAT_STEER_OUTPUT_TYPE    304  /* lat_steer_output_typ    */
#define DB_MARKER_POS_TYPE          305  /* marker_pos_typ          */
#define DB_LAT_CONTROL_OUTPUT_TYPE  306  /* lat_control_output_typ  */
#define DB_LAT_CONTROL_INPUT_TYPE   307  /* lat_control_input_typ   */
#define DB_GYRO_TYPE                308  /* gyro_typ                */

#define DB_LAT_INPUT_MAG_VAR        300
#define DB_LAT_INPUT_SENSORS_VAR    301
#define DB_LAT_OUTPUT_VAR           302
#define DB_LAT_STEER_INPUT_VAR      303
#define DB_LAT_STEER_OUTPUT_VAR     304
#define DB_MARKER_POS_VAR           305
#define DB_LAT_CONTROL_OUTPUT_VAR   306
#define DB_LAT_CONTROL_INPUT_VAR    307
#define DB_GYRO_VAR                 308

/* Communication variables.
 */

#define DB_MANEUVER_DESIRED_TYPE      400
#define DB_MANEUVER_FEEDBACK_TYPE     401

#define DB_MANEUVER_DESIRED_VAR       400
#define DB_MANEUVER_FEEDBACK_VAR      401

#endif /* CLT_VARS_H */
