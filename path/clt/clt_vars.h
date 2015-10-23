/*	FILE
 *	clt_vars.h  This version is intended for Intersection Decidion
 *              Support (IDS) project
 *
 *	As a convention, the variable name/type space is partitioned as
 *	follows:
 *
 *  0    to 99     Used by the system.
 *  100  to 199    Reserved.
 *  200  to 299    Permanent variables.
 *  1000 to 1099   Temporary variables.
 */

/*  Permanent variables
 */

#define DB_DII_OUT_TYPE             200  /* dii_out_typ */
#define DB_LONG_RADAR1_TYPE         201  /* long_radar_typ */
#define DB_LONG_RADAR2_TYPE         202  /* long_radar_typ */
#define DB_LONG_LIDAR1A_TYPE        203  /* long_lidarA_typ */
#define DB_LONG_LIDAR2A_TYPE        204  /* long_lidarA_typ */
#define DB_LONG_LIDAR1B_TYPE        205  /* long_lidarB_typ */
#define DB_LONG_LIDAR2B_TYPE        206  /* long_lidarB_typ */
#define DB_GPS1_GGA_TYPE            207  /* gps_gga_typ */
#define DB_GPS1_VTG_TYPE            208  /* gps_vtg_typ */

#define DB_DII_OUT_VAR              200
#define DB_LONG_RADAR1_VAR          201
#define DB_LONG_RADAR2_VAR          202
#define DB_LONG_LIDAR1A_VAR         203
#define DB_LONG_LIDAR2A_VAR         204
#define DB_LONG_LIDAR1B_VAR         205
#define DB_LONG_LIDAR2B_VAR         206
#define DB_GPS1_GGA_VAR             207
#define DB_GPS1_VTG_VAR             208
