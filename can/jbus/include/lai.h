/*\file
 *
 *	lai.h	Lane Assist Interface header file
*/
#ifndef LAI_H
#define LAI_H

/* Messages sent from Sensor Computer to Control Computer
 */
#define LAICTRLSTAT	0xff10
#define LAISTIN		0xff11
#define LAISTOUT	0xff12
#define LAILATOUT	0xff13
#define LAISIGSTAT	0xff14
#define LAISIGOUT	0xff15
#define LAIGYRO		0xff16
#define LAIDVIMON	0xff17
#define LAILATSENS	0xff18
#define LAILONGIN1	0xff19
#define LAILONGIN2	0xff1a

/* Messages sent from Control Computer to Sensor Computer
 */
#define LAIDVISTAT	0xff1b
#define LAIDVISND	0xff1c
#define LAIDVIPOS	0xff1d
#define LAICTRLOUT	0xff1e
#define LAILATHEART	0xff1f
#define LAILONGOUT	0xff20

/***************************************************** 
 * Messages from Sensor Computer to Control Computer *
 *****************************************************/

#ifdef __QNXNTO__
#define TIMESTAMP_T timestamp_t
#else
typedef struct timeb TIMESTAMP_T;
#endif

/* LAI_CTRLSTAT: control status information, input to lateral control */
typedef struct {
	TIMESTAMP_T timestamp;
	float steer_angle;
	unsigned char ready;
	unsigned char actuator_status;
	unsigned char fault_code;
} lai_ctrlstat_typ;

/* LAI_STIN: input from steering servo tdriver */
typedef struct {
	TIMESTAMP_T timestamp;
	float steer_angle;
	unsigned char driver_status;
	unsigned char failure_code;
	short int counta;
} lai_stin_typ;

/* LAI_STOUT: output from tdriver to steerctl (for debugging) */
typedef struct {
	TIMESTAMP_T timestamp;
	float torque;
	unsigned char mode;
} lai_stout_typ;

/* LAI_LATOUT: output from steerctl to hardware (for debugging) */
typedef struct {
	TIMESTAMP_T timestamp;
	float torque;
	unsigned char clutch;
} lai_latout_typ;

/* LAI_SIGSTAT: signal processing status, input to lateral control */
typedef struct {
	TIMESTAMP_T timestamp;
	short int mag_health;
	float mag_dist;
	unsigned char f_sensor;
	unsigned char r_sensor;
} lai_sigstat_typ;

/* LAI_SIGOUT: signal processing outpout, input to lateral control */
typedef struct {
	TIMESTAMP_T timestamp;
	unsigned char delta_timer_obs;
	float f_ymeas;
	float f_ycar;
	unsigned char f_mark_flag; // two bits
	unsigned char f_polarity;  // one bit
	unsigned char r_mark_flag; // two bits
	unsigned char r_polarity; // one bit
	float r_ymeas;
	float r_ycar;
} lai_sigout_typ;

/* LAI_GYRO: gyro rate input */
typedef struct {
	TIMESTAMP_T timestamp;
	float gyro_rate;
} lai_gyro_typ;

/* LAI_DVIMON: input from the DVI computer */
typedef struct {
	TIMESTAMP_T timestamp;
	unsigned char mode;
} lai_dvimon_typ;

/* LAI_LATSENS: lateral sensors input */
typedef struct {
	TIMESTAMP_T timestamp;
	float lat_acc;
	float long_acc;
	// the following are single bit fields
	unsigned char manual_trans;
	unsigned char auto_trans;
	unsigned char auto_steer;
	unsigned char auto_throt;
	unsigned char auto_brake;
	unsigned char dc1;
	unsigned char dc2;
} lai_latsens_typ;

/* LAI_LONGIN1: longitudinal sensors input part 1 */
typedef struct {
	TIMESTAMP_T timestamp;
	float acc_pedal;
	float fb_axle;
	float rb_axle;
} lai_longin1_typ;

/* LAI_LONGIN2: longitudinal sensors input part 2 */
typedef struct {
	TIMESTAMP_T timestamp;
	float fb_applied;
	float rb_applied;
	float fb_monitor;
	float rb_monitor;
} lai_longin2_typ;

/***************************************************** 
 * Messages from Control Computer to Sensor Computer *
 *****************************************************/

/* LAI_DVISTAT: lateral control status for DVI display */
typedef struct {
	TIMESTAMP_T timestamp;
	unsigned char sf_mag_id;
	unsigned char overall_mode;
	unsigned char platoon_info;
	unsigned char lat_status1;
	unsigned char lat_status2;
	unsigned char lat_status3;
	unsigned char lat_status4;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char amber;
} lai_dvistat_typ;	

/* LAI_DVISND: lateral control status for DVI display */
typedef struct {
	TIMESTAMP_T timestamp;
	float sound1;
	float sound2;
	float sound3;
	float sound4;
} lai_dvisnd_typ;	

/* LAI_DVIPOS: lateral position info for DVI display */
typedef struct {
	TIMESTAMP_T timestamp;
	short int lat_pos;
	unsigned short int long_pos;
	unsigned short int distance_end;
	short int lat_est;
} lai_dvipos_typ;	

/* LAI_CTRLOUT: lateral control actuation information, output to tdriver */
typedef struct {
	TIMESTAMP_T timestamp;
	float steer_ctrl;
	unsigned char actuator_mode;
	unsigned char steer_mode;
} lai_ctrlout_typ;	

/* LAI_LATHEART: heartbeat */
typedef struct {
	TIMESTAMP_T timestamp;
	unsigned char is_active;
} lai_latheart_typ;	

/* LAI_LONGOUT: longitudinal control information */
typedef struct {
	TIMESTAMP_T timestamp;
	float acc_pedal_ctrl;
	float fb_ctrl;
	float rb_ctrl;
} lai_longout_typ;	

#endif
