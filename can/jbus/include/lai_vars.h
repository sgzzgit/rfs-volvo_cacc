/*	FILE
 *	clt_vars.h  Version for Demo 2003 bus.
 *
 * Copyright (c) 2002   Regents of the University of California
 *
 *
 *	static char rcsid[] = "$Id$";
 *
 *	$Log$
 *
 *
 *	As a convention, the variable name/type space is partitioned as
 *	follows:
 *
 *	0	to	99		Used by the system.
 *	100	to	199		Reserved.
 *	200	to	299		Permanent longitudinal variables.
 *	300	to	399		Permanent lateral variables.
 *	400	to	499		Permanent communications variables.
 *	500	to	599		Jbus variables
 *	600	to	699		Longitudinal communications variables
 *	1000 to	1099	Temporary variables.
 */

/*	Longitudinal variables
 */

#define DB_LONG_INPUT_TYPE           200  /* long_input_typ          */
#define DB_LONG_OUTPUT_TYPE          201  /* long_output_typ         */
#define DB_GPS_GGA_TYPE              202  /* gps_gga_typ             */
#define DB_GPS_VTG_TYPE              203  /* gps_vtg_typ             */
#define DB_LONG_EVRADAR_TYPE         204  /* long_evradar_typ        */
#define DB_LONG_LIDARA_TYPE          205  /* long_lidarA_typ         */
#define DB_LONG_LIDARB_TYPE          206  /* long_lidarB_typ         */
#define DB_LONG_COORD_OUTPUT_TYPE    207  /* long_coord_output_typ   */
#define DB_LONG_DVI_OUTPUT_TYPE      208  /* long_dvi_output_typ     */
#define DB_LONG_FCW_WARNING_TYPE     209  /* long_fcw_warning_typ    */


#define DB_LONG_INPUT_VAR            200
#define DB_LONG_OUTPUT_VAR           201
#define DB_GPS_GGA_VAR               202
#define DB_GPS_VTG_VAR               203
#define DB_LONG_EVRADAR_VAR          204
#define DB_LONG_LIDARA_VAR           205
#define DB_LONG_LIDARB_VAR           206
#define DB_LONG_COORD_OUTPUT_VAR     207
#define DB_LONG_DVI_OUTPUT_VAR       208
#define DB_LONG_FCW_WARNING_VAR      209

/*	Lateral variables.
 */

#define DB_LAT_INPUT_MAG_TYPE        300  /* lat_input_mag_typ		  */
#define DB_LAT_INPUT_SENSORS_TYPE    301  /* lat_input_sensors_typ	  */
#define DB_LAT_OUTPUT_TYPE           302  /* lat_output_typ			  */
#define DB_LAT_STEER_INPUT_TYPE      303  /* lat_steer_input_typ      */
#define DB_LAT_STEER_OUTPUT_TYPE     304  /* lat_steer_output_typ     */
#define DB_MARKER_POS_TYPE           305  /* marker_pos_typ           */
#define DB_LAT_CONTROL_OUTPUT_TYPE   306  /* lat_control_output_typ   */
#define DB_LAT_CONTROL_INPUT_TYPE    307  /* lat_control_input_typ    */
#define DB_GYRO_TYPE                 308  /* gyro_typ                 */
#define DB_LAT_COORD_OUTPUT_TYPE     309  /* lat_coord_output_typ     */
#define DB_GYRO2_TYPE                310  /* gyro_typ                 */
#define DB_LAT_HEARTBEAT_OUTPUT_TYPE 311  /* lat_heartbeat_output_typ */
#define DB_LAT_DVI_OUTPUT_TYPE       312  /* lat_dvi_output_typ       */
#define DB_SIGNALPROC_OUTPUT_TYPE    313  /* signalproc_output_typ    */

#define DB_LAT_INPUT_MAG_VAR         300
#define DB_LAT_INPUT_SENSORS_VAR     301
#define DB_LAT_OUTPUT_VAR            302
#define DB_LAT_STEER_INPUT_VAR       303
#define DB_LAT_STEER_OUTPUT_VAR      304
#define DB_MARKER_POS_VAR            305
#define DB_LAT_CONTROL_OUTPUT_VAR    306
#define DB_LAT_CONTROL_INPUT_VAR     307
#define DB_GYRO_VAR                  308
#define DB_LAT_COORD_OUTPUT_VAR      309
#define DB_GYRO2_VAR                 310
#define DB_LAT_HEARTBEAT_OUTPUT_VAR  311
#define DB_LAT_DVI_OUTPUT_VAR        312
#define DB_SIGNALPROC_OUTPUT_VAR     313

/* Communication variables. Also used for serial port communication to DVI.
 * Some types only used by DVI are included here for ease in writing
 * diagnostics that simulate the DVI operation on the control computer.
 */

#define DB_MANEUVER_DESIRED_TYPE     400
#define DB_MANEUVER_FEEDBACK_TYPE    401
#define DB_DVI_LEDS_TYPE             402  /* dvi_leds_typ           */
#define DB_COMM_DVI_OUTPUT_TYPE      403  /* comm_dvi_output_typ    */
#define DB_DVI_INPUT_TYPE            404  /* dvi_mess_typ    */
#define DB_DVI_OUTPUT_TYPE           405  /* dvi_mode_typ    */
#define DB_SERIAL_TIMEOUT_TYPE       406  /* dvi_serial_timeout_typ    */
#define DB_DVI_BUTTON_TYPE           407  /* dvi_button_typ    */
#define DB_DVI_MONITOR_TYPE           408  /* dvi_monitor_typ    */

#define DB_MANEUVER_DESIRED_VAR      400
#define DB_MANEUVER_FEEDBACK_VAR     401
#define DB_DVI_LEDS_VAR              402
#define DB_COMM_DVI_OUTPUT_VAR       403
#define DB_DVI_INPUT_VAR             404  /* dvi_mess_typ    */
#define DB_DVI_OUTPUT_VAR            405  /* dvi_mode_typ    */
#define DB_SERIAL_TIMEOUT_VAR        406  /* dvi_serial_timeout_typ    */
#define DB_DVI_BUTTON_VAR           407  /* dvi_button_typ    */
#define DB_DVI_MONITOR_VAR           408  /* dvi_monitor_typ    */
