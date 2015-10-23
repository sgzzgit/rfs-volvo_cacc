/**\file
 *
 *	ibeo.c	Driver to send commands and read data from
 *		a single IBEO laserscanner. For multiple scanners,
 *		run multiple instances of the driver.
 */

can_std_id_t ibeo_can_base = IBEO_DEFAULT_SENSOR_ID;

