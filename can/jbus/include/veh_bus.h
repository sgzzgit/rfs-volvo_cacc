/*	FILE
 *  veh_bus.h    Version for Demo 2003 buses
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

#define DEFAULT_CONFIG_FILE		"realtime.ini"


/* Following is DB_LAT_INPUT_SENSORS_TYPE, DB_LAT_INPUT_SENSORS_VAR in database */
typedef struct
{   
	float steer_angle;      /* Measured steering angle in degrees of roadwheel. */
	float motor_cond;       /* Motor condition */
	int countx;             /* X-counter from the encoder card */
	short int clutch_state; /* Current state of clutch (0=OFF, 1=ON) */
	short int power;        /* Steering actuator on (1=ON, 0=OFF) */
	float long_accel;       /* Accelerometer x-axis (longitudinal) */
	float lat_acc;          /* Accelerometer y-axis (lateral) */
	short int auto_steer;   /* Automatic steering ON (1=ON, 0=OFF) */
	short int auto_brake;   /* Automatic brake ON (1=ON, 0=OFF) */
	short int auto_throt;   /* Automatic throttle ON (1=ON, 0=OFF) */
	float accel_temp;       /* Accelerometer temperature */
	short int manual_trans; /* Manual transition (1=ON, 0=OFF) */
	short int auto_trans;   /* Automatic transition (1=ON, 0=OFF */
	short int dc1;          /* DC1 (input for Han-Shue's demo code */
	short int dc2;          /* DC2 (input for Han-Shue's demo code */
} lat_input_sensors_typ;

/* Following is DB_LAT_OUTPUT_TYPE, DB_LAT_OUTPUT_VAR in database */
typedef struct
{
	float torque;       /* Desired steering torque */
	short int clutch;   /* Desired clutch state (0=OFF, 1=ON) */
} lat_output_typ;

/* Following is DB_LAT_HEARTBEAT_OUTPUT_TYPE, DB_LAT_HEARTBEAT_OUTPUT_VAR in
 * database (written by lateral controller, read by veh_iobl). */
typedef struct
{
	int heartbeat;      /* Heartbeat of control conputer (either 0 or 1) */
} lat_heartbeat_output_typ;

/* Following is DB_LAT_INPUT_MAG_TYPE, DB_LAT_INPUT_MAG_VAR in database */
typedef struct
{
	float mag_fll_lat;  // Voltage of front left left magnetometer's lateral axis
	float mag_fll_vert; // Voltage of front left left magnetometer's vertival axis
	float mag_fl_lat;   // Voltage of front left magnetometer's lateral axis
	float mag_fl_vert;  // Voltage of front left magnetometer's vertical axis
	float mag_fcl_lat;  // Voltage of front center left magnetometer's lateral axis
	float mag_fcl_vert; // Voltage of front center left magnetometer's vertical axis
	float mag_fc_lat;   // Voltage of front center magnetometer's lateral axis
	float mag_fc_vert;  // Voltage of front center magnetometer's vertical axis
	float mag_fcr_lat;  // Voltage of front center right magnetometer's lateral axis
	float mag_fcr_vert; // Voltage of front center right magnetometer's vertical axis
	float mag_fr_lat;   // Voltage of front right magnetometer's lateral axis
	float mag_fr_vert;  // Voltage of front right magnetometer's vertical axis
	float mag_frr_lat;  // Voltage of front right right magnetometer's lateral axis
	float mag_frr_vert; // Voltage of front right right magnetometer's vertical axis

	float mag_rll_lat;  // Voltage of rear left magnetometer's lateral axis
	float mag_rll_vert; // Voltage of rear left left magnetometer's vertival axis
	float mag_rl_lat;   // Voltage of rear left magnetometer's lateral axis
	float mag_rl_vert;  // Voltage of rear left magnetometer's vertical axis
	float mag_rcl_lat;  // Voltage of rear center left magnetometer's lateral axis
	float mag_rcl_vert; // Voltage of rear center left magnetometer's vertical axis
	float mag_rc_lat;   // Voltage of rear center magnetometer's lateral axis
	float mag_rc_vert;  // Voltage of rear center magnetometer's vertical axis
	float mag_rcr_lat;  // Voltage of rear center right magnetometer's lateral axis
	float mag_rcr_vert; // Voltage of rear center right magnetometer's vertical axis
	float mag_rr_lat;   // Voltage of rear right magnetometer's lateral axis
	float mag_rr_vert;  // Voltage of rear right magnetometer's vertical axis
	float mag_rrr_lat;  // Voltage of rear right right magnetometer's lateral axis
	float mag_rrr_vert; // Voltage of rear right right magnetometer's vertical axis

	short int health;   // Magnetometer health signals (1=OK, 0=fault)
	                    // bit 0 = front left left; bit 1 = front left;
	                    // bit 2 = front center left; bit 3 = front center;
	                    // bit 4 = front center right; bit 5 = front right;
	                    // bit 6 = front right right; bit 7 = rear left left;
	                    // bit 8 = rear left; bit 9 = rear center left;
	                    // bit 10 = rear center; bit 11 = rear center right;
	                    // bit 12 = rear right; bit 13 = rear right right
} lat_input_mag_typ;

/* Following is DB_LAT_CONTROL_OUTPUT_TYPE, DB_LAT_CONTROL_OUTPUT_VAR
 * in database. */
typedef struct
{
	short int actuator_mode;      /* 0=manual, 1=auto */
	float steer_ctrl;             /* Handwheel angle in degrees */
	short int steer_mode;         /* TBD */
} lat_control_output_typ;

/* Following is DB_LAT_STEER_INPUT_TYPE, DB_LAT_STEER_INPUT_VAR in database */
typedef struct
{   
	float steer_angle;      /* Measured steering angle in degrees of roadwheel. */
	int counta;             /* A-counter from the encoder card */
	int driver_status;      /* Driver status (0=NOT READY, 1=READY) */
	int failure_code;       /* Failure mode (0=None; 1=Motor failure,
	                         * 2=Motor power off; 3=command failure) */
} lat_steer_input_typ;

/* Following is DB_LAT_STEER_OUTPUT_TYPE, DB_LAT_STEER_OUTPUT_VAR in database */
typedef struct
{
	float torque;       /* Desired steering torque */
	short mode;         /* Desired (0=OFF, 1=Start, 2=Shutdown, 3=Normal) */
} lat_steer_output_typ;


/* Following is DB_LAT_CONTROL_INPUT_TYPE, DB_LAT_CONTROL_INPUT_VAR in
 * database. */
typedef struct
{
	float steer_angle;          /* Calibrated, in hadnwheel degrees */
	float torque;               /* Desired steering torque */
	short int ready;            /* 0=NOT READY, 1=READY */
	short int actuator_status;  /* 0=manual, 1=auto */
	short int fault_code;       /* Failure mode (0=None; 1=Motor failure,
	                             * 2=Motor power off; 3=command failure) */
} lat_control_input_typ;

/* Following is the definition for DB_GYRO_TYPE, DB_GYRO_VAR,
 * DB_GYRO2_TYPE, DB_GYRO2_VAR in the
 * database. */
typedef struct
{
	float gyro_rate;            /* Gyro rate in degrees/sec for KVH E-Core
	                             * 2000 Gyro */
} gyro_typ;

/* Following is DB_MARKER_POS_TYPE, DB_MARKER_POS_VAR in database (output
 * from lateral controller). */
typedef struct
{
	int marker_number;     /* This is the most recently seen marker, number
	                        * will vary from 4558 to 16456 in San Diego */
	int lane_number;       /* This is a number from 0-15 indicating which
	                        * lane we're on. */
	bool_typ direction;        /* South = 0; North = 1 */
	float lateral_error;   /* Lateral error in cm */
	int perform_maneuver;  /* This is the current maneuver lateral controller
	                        * is performing.  */
	double timestamp;      /* Time at marker position */
	int marker_counter;    /* Counter of number of markers */
} marker_pos_typ;

/* Following is the definition for DB_LONG_INPUT_TYPE, DB_LONG_INPUT_VAR
 * in the database. */
typedef struct
{
	int platoon_pos;             /* Platoon position */
	float fb_applied;            /* Front brake applied transducer */
	float rb_applied;            /* Rear brake applied transducer */
	float fb_monitor;            /* Front brake monitor transducer */
	float rb_monitor;            /* Rear brake monitor transducer */
	float fb_axle;	             /* Front brake axle transducer */
	float mb_axle;               /* Middle brake axle transducer */
	float rb_axle;               /* Rear brake axle transducer */
	float acc_pedal;             /* Accelerator pedal */
	float trans_retarder;        /* Transmission retarder */
	int engine_fan;              /* Engine desired fan state (truck only) */
} long_input_typ;

/* Following is the definition for DB_LONG_OUTPUT_TYPE, DB_LONG_OUTPUT_VAR
 * in the database. */
typedef struct
{
	float engine_speed;
	float engine_torque;
	float engine_retarder_torque;
	unsigned char engine_command_mode;         /* 0=disable, 1=speed, 2=torque,
	                                            * 3=speed/torque limit */
	unsigned char engine_retarder_command_mode; /* 0=disable, 2=torque,
	                                             * 3=torque limit */
	float acc_pedal_control;                   /* Accelerator pedal control */
	float ebs_deceleration;
	unsigned char brake_command_mode;          /* 0=not active, 1=active */
	float trans_retarder_value;                /* Percent of maximum */
	unsigned char trans_retarder_command_mode; /* 0=disable, 2=enable */
	float fb_control;                          /* Front brake control (bus only) */
	float rb_control;                          /* Rear brake control (bus only) */
	int fan_override;                          /* Engine fan override (truck only) */
	int fan_control;                           /* Engine fan ON/OFF (truck only) */
	int fault_status;                          /* 0=OK, 1=fault */
} long_output_typ;

/* Following is the definition for DB_GPS_GGA_TYPE, DB_GPS_GGA_VAR in
 * the database. */
typedef struct
{
	float utc_time;           /* Current UTC time of fix in hours, minutes,
	                           * and seconds (hhmmss.ss) */
	float latitude;           /* Latitude component of position in degrees
	                           * and decimal minutes (ddmm.mmmmm) */
	float longitude;          /* Longitude component of position in degrees
	                           * and decimal minutes (ddmm.mmmmm) */
	float altitude;           /* Altitude in meters above the ellipsoid */
} gps_gga_typ;

/* Following is the definition for DB_GPS_VTG_TYPE, DB_GPS_VTG_VAR in
 * the database. */
typedef struct
{
	float speed_over_ground;  /* Speed over ground in km/h */
} gps_vtg_typ;



/* Following is DB_LONG_EVRADAR_TYPE, DB_LONG_EVRADAR_VAR in database. */
typedef struct
{
	unsigned char target_1_id;   /* ID number of nearest object */
	short int target_1_range;    /* Range of object, LSB=0.1 ft */
	short int target_1_rate;     /* Rate of object approach, LSB=0.1 ft/s */
	signed char target_1_azimuth;/* Target azimuth, LSB=0.002 radians */
	unsigned char target_1_mag;  /* Magnitude, LSB=-0.543 dB */
	unsigned char target_1_lock; /* Bit mapped, 1=locked, 0=not locked
	                              * bit 0 is current FFT frame (n), bit 1 is
	                              * FFT frame n-1,...,bit 7 is FFT frame n-7 */
	unsigned char target_2_id;   /* ID number of nearest object */
	short int target_2_range;    /* Range of object, LSB=0.1 ft */
	short int target_2_rate;     /* Rate of object approach, LSB=0.1 ft/s */
	signed char target_2_azimuth;/* Target azimuth, LSB=0.002 radians */
	unsigned char target_2_mag;  /* Magnitude, LSB=-0.543 dB */
	unsigned char target_2_lock; /* Bit mapped, 1=locked, 0=not locked
	                              * bit 0 is current FFT frame (n), bit 1 is
	                              * FFT frame n-1,...,bit 7 is FFT frame n-7 */
	unsigned char target_3_id;   /* ID number of nearest object */
	short int target_3_range;    /* Range of object, LSB=0.1 ft */
	short int target_3_rate;     /* Rate of object approach, LSB=0.1 ft/s */
	signed char target_3_azimuth;/* Target azimuth, LSB=0.002 radians */
	unsigned char target_3_mag;  /* Magnitude, LSB=-0.543 dB */
	unsigned char target_3_lock; /* Bit mapped, 1=locked, 0=not locked
	                              * bit 0 is current FFT frame (n), bit 1 is
	                              * FFT frame n-1,...,bit 7 is FFT frame n-7 */
	unsigned char target_4_id;   /* ID number of nearest object */
	short int target_4_range;    /* Range of object, LSB=0.1 ft */
	short int target_4_rate;     /* Rate of object approach, LSB=0.1 ft/s */
	signed char target_4_azimuth;/* Target azimuth, LSB=0.002 radians */
	unsigned char target_4_mag;  /* Magnitude, LSB=-0.543 dB */
	unsigned char target_4_lock; /* Bit mapped, 1=locked, 0=not locked
	                              * bit 0 is current FFT frame (n), bit 1 is
	                              * FFT frame n-1,...,bit 7 is FFT frame n-7 */
	unsigned char target_5_id;   /* ID number of nearest object */
	short int target_5_range;    /* Range of object, LSB=0.1 ft */
	short int target_5_rate;     /* Rate of object approach, LSB=0.1 ft/s */
	signed char target_5_azimuth;/* Target azimuth, LSB=0.002 radians */
	unsigned char target_5_mag;  /* Magnitude, LSB=-0.543 dB */
	unsigned char target_5_lock; /* Bit mapped, 1=locked, 0=not locked
	                              * bit 0 is current FFT frame (n), bit 1 is
	                              * FFT frame n-1,...,bit 7 is FFT frame n-7 */
	unsigned char target_6_id;   /* ID number of nearest object */
	short int target_6_range;    /* Range of object, LSB=0.1 ft */
	short int target_6_rate;     /* Rate of object approach, LSB=0.1 ft/s */
	signed char target_6_azimuth;/* Target azimuth, LSB=0.002 radians */
	unsigned char target_6_mag;  /* Magnitude, LSB=-0.543 dB */
	unsigned char target_6_lock; /* Bit mapped, 1=locked, 0=not locked
	                              * bit 0 is current FFT frame (n), bit 1 is
	                              * FFT frame n-1,...,bit 7 is FFT frame n-7 */
	unsigned char target_7_id;   /* ID number of nearest object */
	short int target_7_range;    /* Range of object, LSB=0.1 ft */
	short int target_7_rate;     /* Rate of object approach, LSB=0.1 ft/s */
	signed char target_7_azimuth;/* Target azimuth, LSB=0.002 radians */
	unsigned char target_7_mag;  /* Magnitude, LSB=-0.543 dB */
	unsigned char target_7_lock; /* Bit mapped, 1=locked, 0=not locked
	                              * bit 0 is current FFT frame (n), bit 1 is
	                              * FFT frame n-1,...,bit 7 is FFT frame n-7 */
	char hour;                   /* Time of day of last message, hours */
	char min;                    /* Time of day of last message, minutes */
	char sec;                    /* Time of day of last message, seconds */
	short int millisec;          /* Time of day of last message, milliseconds */
	unsigned char mess_ID;       /* Message ID */
} long_evradar_typ;



/* Following is the definition for DB_LONG_LIDARA_TYPE, DB_LONG_LIDARA_VAR
 * in the database.  */
typedef struct
{
	char hour;                  /* Time of day of last message, hours */
	char min;                   /* Time of day of last message, minutes */
	char sec;                   /* Time of day of last message, seconds */
	short int millisec;         /* Time of day of last message, milliseconds */

	unsigned char h_latpos_1;   /* Lateral position, high byte */
	unsigned char l_latpos_1;   /* Lateral position, low byte, LSB=0.01 m */
	unsigned char vert_pos_1;   /* Vertical position, LSB=0.5 line */
	unsigned char h_dist_1;     /* Distance, high byte */
	unsigned char l_dist_1;     /* Distance, low byte */
	unsigned char lanerate_1;   /* Lane rate, 0 to 100% */
	unsigned char veh_rate_1;   /* Vehicle rate, 0 to 100% */
	unsigned char targ_stat_1;  /* Target status */
	signed char lat_vel_1;      /* Lateral velocity (LSB=0.03 m/sec) */
	unsigned char h_velocity_1; /* Relative velocity, high byte */
	unsigned char l_velocity_1; /* Relative velocity, low byte */
	unsigned char h_width_1;    /* Width, high byte */
	unsigned char l_width_1;    /* Width, low byte */
	unsigned char height_1;     /* Height, LSB = 1 line */
	unsigned char h_depth_1;    /* Depth, high byte */
	unsigned char l_depth_1;    /* Depth, low byte */
	signed char rel_acc_1;      /* Relative acceleration, LSB=0.05m/s**2) */
	
	unsigned char h_latpos_2;   /* Lateral position, high byte */
	unsigned char l_latpos_2;   /* Lateral position, low byte, LSB=0.01 m */
	unsigned char vert_pos_2;   /* Vertical position, LSB=0.5 line */
	unsigned char h_dist_2;     /* Distance, high byte */
	unsigned char l_dist_2;     /* Distance, low byte */
	unsigned char lanerate_2;   /* Lane rate, 0 to 100% */
	unsigned char veh_rate_2;   /* Vehicle rate, 0 to 100% */
	unsigned char targ_stat_2;  /* Target status */
	signed char lat_vel_2;      /* Lateral velocity (LSB=0.03 m/sec) */
	unsigned char h_velocity_2; /* Relative velocity, high byte */
	unsigned char l_velocity_2; /* Relative velocity, low byte */
	unsigned char h_width_2;    /* Width, high byte */
	unsigned char l_width_2;    /* Width, low byte */
	unsigned char height_2;     /* Height, LSB = 1 line */
	unsigned char h_depth_2;    /* Depth, high byte */
	unsigned char l_depth_2;    /* Depth, low byte */
	signed char rel_acc_2;      /* Relative acceleration, LSB=0.05m/s**2) */

	unsigned char h_latpos_3;   /* Lateral position, high byte */
	unsigned char l_latpos_3;   /* Lateral position, low byte, LSB=0.01 m */
	unsigned char vert_pos_3;   /* Vertical position, LSB=0.5 line */
	unsigned char h_dist_3;     /* Distance, high byte */
	unsigned char l_dist_3;     /* Distance, low byte */
	unsigned char lanerate_3;   /* Lane rate, 0 to 100% */
	unsigned char veh_rate_3;   /* Vehicle rate, 0 to 100% */
	unsigned char targ_stat_3;  /* Target status */
	signed char lat_vel_3;      /* Lateral velocity (LSB=0.03 m/sec) */
	unsigned char h_velocity_3; /* Relative velocity, high byte */
	unsigned char l_velocity_3; /* Relative velocity, low byte */
	unsigned char h_width_3;    /* Width, high byte */
	unsigned char l_width_3;    /* Width, low byte */
	unsigned char height_3;     /* Height, LSB = 1 line */
	unsigned char h_depth_3;    /* Depth, high byte */
	unsigned char l_depth_3;    /* Depth, low byte */
	signed char rel_acc_3;      /* Relative acceleration, LSB=0.05m/s**2) */
	
	unsigned char h_latpos_4;   /* Lateral position, high byte */
	unsigned char l_latpos_4;   /* Lateral position, low byte, LSB=0.01 m */
	unsigned char vert_pos_4;   /* Vertical position, LSB=0.5 line */
	unsigned char h_dist_4;     /* Distance, high byte */
	unsigned char l_dist_4;     /* Distance, low byte */
	unsigned char lanerate_4;   /* Lane rate, 0 to 100% */
	unsigned char veh_rate_4;   /* Vehicle rate, 0 to 100% */
	unsigned char targ_stat_4;  /* Target status */
	signed char lat_vel_4;      /* Lateral velocity (LSB=0.03 m/sec) */
	unsigned char h_velocity_4; /* Relative velocity, high byte */
	unsigned char l_velocity_4; /* Relative velocity, low byte */
	unsigned char h_width_4;    /* Width, high byte */
	unsigned char l_width_4;    /* Width, low byte */
	unsigned char height_4;     /* Height, LSB = 1 line */
	unsigned char h_depth_4;    /* Depth, high byte */
	unsigned char l_depth_4;    /* Depth, low byte */
	signed char rel_acc_4;      /* Relative acceleration, LSB=0.05m/s**2) */
} long_lidarA_typ;



/* Following is the definition for DB_LONG_LIDARB_TYPE, DB_LONG_LIDARB_VAR
 * in the database.  */
typedef struct
{
	unsigned char h_latpos_5;   /* Lateral position, high byte */
	unsigned char l_latpos_5;   /* Lateral position, low byte, LSB=0.01 m */
	unsigned char vert_pos_5;   /* Vertical position, LSB=0.5 line */
	unsigned char h_dist_5;     /* Distance, high byte */
	unsigned char l_dist_5;     /* Distance, low byte */
	unsigned char lanerate_5;   /* Lane rate, 0 to 100% */
	unsigned char veh_rate_5;   /* Vehicle rate, 0 to 100% */
	unsigned char targ_stat_5;  /* Target status */
	signed char lat_vel_5;      /* Lateral velocity (LSB=0.03 m/sec) */
	unsigned char h_velocity_5; /* Relative velocity, high byte */
	unsigned char l_velocity_5; /* Relative velocity, low byte */
	unsigned char h_width_5;    /* Width, high byte */
	unsigned char l_width_5;    /* Width, low byte */
	unsigned char height_5;     /* Height, LSB = 1 line */
	unsigned char h_depth_5;    /* Depth, high byte */
	unsigned char l_depth_5;    /* Depth, low byte */
	signed char rel_acc_5;      /* Relative acceleration, LSB=0.05m/s**2) */
	
	unsigned char h_latpos_6;   /* Lateral position, high byte */
	unsigned char l_latpos_6;   /* Lateral position, low byte, LSB=0.01 m */
	unsigned char vert_pos_6;   /* Vertical position, LSB=0.5 line */
	unsigned char h_dist_6;     /* Distance, high byte */
	unsigned char l_dist_6;     /* Distance, low byte */
	unsigned char lanerate_6;   /* Lane rate, 0 to 100% */
	unsigned char veh_rate_6;   /* Vehicle rate, 0 to 100% */
	unsigned char targ_stat_6;  /* Target status */
	signed char lat_vel_6;      /* Lateral velocity (LSB=0.03 m/sec) */
	unsigned char h_velocity_6; /* Relative velocity, high byte */
	unsigned char l_velocity_6; /* Relative velocity, low byte */
	unsigned char h_width_6;    /* Width, high byte */
	unsigned char l_width_6;    /* Width, low byte */
	unsigned char height_6;     /* Height, LSB = 1 line */
	unsigned char h_depth_6;    /* Depth, high byte */
	unsigned char l_depth_6;    /* Depth, low byte */
	signed char rel_acc_6;      /* Relative acceleration, LSB=0.05m/s**2) */
	
	unsigned char h_latpos_7;   /* Lateral position, high byte */
	unsigned char l_latpos_7;   /* Lateral position, low byte, LSB=0.01 m */
	unsigned char vert_pos_7;   /* Vertical position, LSB=0.5 line */
	unsigned char h_dist_7;     /* Distance, high byte */
	unsigned char l_dist_7;     /* Distance, low byte */
	unsigned char lanerate_7;   /* Lane rate, 0 to 100% */
	unsigned char veh_rate_7;   /* Vehicle rate, 0 to 100% */
	unsigned char targ_stat_7;  /* Target status */
	signed char lat_vel_7;      /* Lateral velocity (LSB=0.03 m/sec) */
	unsigned char h_velocity_7; /* Relative velocity, high byte */
	unsigned char l_velocity_7; /* Relative velocity, low byte */
	unsigned char h_width_7;    /* Width, high byte */
	unsigned char l_width_7;    /* Width, low byte */
	unsigned char height_7;     /* Height, LSB = 1 line */
	unsigned char h_depth_7;    /* Depth, high byte */
	unsigned char l_depth_7;    /* Depth, low byte */
	signed char rel_acc_7;      /* Relative acceleration, LSB=0.05m/s**2) */
	
	unsigned char h_latpos_8;   /* Lateral position, high byte */
	unsigned char l_latpos_8;   /* Lateral position, low byte, LSB=0.01 m */
	unsigned char vert_pos_8;   /* Vertical position, LSB=0.5 line */
	unsigned char h_dist_8;     /* Distance, high byte */
	unsigned char l_dist_8;     /* Distance, low byte */
	unsigned char lanerate_8;   /* Lane rate, 0 to 100% */
	unsigned char veh_rate_8;   /* Vehicle rate, 0 to 100% */
	unsigned char targ_stat_8;  /* Target status */
	signed char lat_vel_8;      /* Lateral velocity (LSB=0.03 m/sec) */
	unsigned char h_velocity_8; /* Relative velocity, high byte */
	unsigned char l_velocity_8; /* Relative velocity, low byte */
	unsigned char h_width_8;    /* Width, high byte */
	unsigned char l_width_8;    /* Width, low byte */
	unsigned char height_8;     /* Height, LSB = 1 line */
	unsigned char h_depth_8;    /* Depth, high byte */
	unsigned char l_depth_8;    /* Depth, low byte */
	signed char rel_acc_8;      /* Relative acceleration, LSB=0.05m/s**2) */

	signed char curve_h;        /* Horizontal curve radius received from PC */
	signed char curve_l;        /* Lateral curve radius received from PC */
} long_lidarB_typ;


/* Following is the definition for DB_DVI_LEDS_TYPE, DB_DVI_LEDS_VAR
 * in the database.  */
typedef struct
{
	int red;                    /* 1=red LED ON, 0=red LED OFF */
	int green;                  /* 1=green LED ON, 0=green LED OFF */
	int blue;                   /* 1=blue LED ON, 0=blue LED OFF */
	int amber;                  /* 1=amber LED ON, 0=anber LED OFF */
	float sound1;               /* Sound #1 output (0.0 or 9.9 volts */
	float sound2;               /* Sound #2 output (0.0 or 9.9 volts */
	float sound3;               /* Sound #3 output (0.0 or 9.9 volts */
	float sound4;               /* Sound #4 output (0.0 or 9.9 volts */
} dvi_leds_typ;


/* Following is the definition for DB_LAT_DVI_OUTPUT_TYPE,
 * DB_LAT_DVI_OUTPUT_VAR in the database. */
typedef struct
{
	unsigned char      mag_status;      /* Magnetometer status (0=off, 1=on,
	                                     * 2=fault) */
	unsigned char      overall_mode;    /* State change variable  */
	unsigned char      steer_fault;     /* Uses bits 0 and 1 only */
	unsigned char      platoon_info;   /* Platoon info variable 1 */
	unsigned char      lat_mode1;       /* No longer used ? */
	unsigned char      lat_mode2;       /* No longer used ? */
	unsigned char      lat_status1;     /* Lateral status variable 1 */
	unsigned char      lat_status2;     /* Lateral status variable 2 */
	unsigned char      lat_status3;     /* Lateral status variable 3 */
	unsigned char      lat_status4;     /* Lateral status variable 4 */
	unsigned char      lane_id;         /* Lane ID (0=unknown, 1=right,
	                                     * 2=left) */
	short int          lat_pos;         /* Lateral position offset */
	unsigned char      lane_depart_lev; /* No longer used ? */
	unsigned char      lane_depart_dir; /* No longer used ? */
	unsigned short int long_pos;        /* Longitudinal position */
	unsigned short int distance_to_end; /* Distance to end of magnets */
	short int          lat_est;         /* Lateral estimation in lane change */
	short int          add_fault;       /* ? */
} lat_dvi_output_typ;

/* Following is the definition for DB_COMM_DVI_OUTPUT_TYPE,
 * DB_COMM_DVI_OUTPUT_VAR in the database. */
typedef struct
{
	unsigned char      comm_state;      /* Communications state */
	unsigned char      comm_strength;   /* Wireless communications
	                                     * signal strength */
	unsigned char      comm_fault;      /* Uses bits 2 and 3 only */
} comm_dvi_output_typ;


/* Following is the definition for DB_LONG_DVI_OUTPUT_TYPE,
 * DB_LONG_DVI_OUTPUT_VAR in the database. */
typedef struct
{
	unsigned char      long_fault;      /* Uses bits 4 thru 7 only */
	unsigned char      platoon_info;   /* Platoon info variable */
	float		   p_current_speed; /* Current platoon speed (m/sec) */
	float       	   p_target_speed;  /* Target platoon speed (m/sec) */
	unsigned char      p_length;        /* Platoon length */
	unsigned char      p_veh_id;        /* Platoon ID of subject vehicle */
	unsigned char      p_veh_location;  /* Location of subject vehicle
	                                     * in platoon */
	unsigned char	   overall_mode;    /* State change variable */
	unsigned char      long_mode1;      /* Longitudinal mode variable 1 */
	unsigned char      long_mode2;      /* Longitudinal mode variable 2 */
	unsigned char      long_status1;    /* Longitudinal status variable 1 */
	unsigned char      long_status2;    /* Longitudinal status variable 2 */
	unsigned char      long_status3;    /* Longitudinal status variable 3 */
	unsigned char      long_status4;    /* Longitudinal status variable 4 */
	float              veh_speed;       /* Present vehicle speed (m/sec) */
	float              veh_target;      /* Target vehicle speed (m/sec) */
	unsigned char      track_targ;      /* Tracking known target flag (1=yes) */
	unsigned char      curr_gap;        /* Current gap */
	signed char        gap_rate;        /* Gap closing rate */
	unsigned char      target_gap;      /* Target vehicle gap */
} long_dvi_output_typ;


/* Following is the definition for DB_LONG_FCW_WARNING_TYPE,
 * DB_LONG_FCW_WARNING_VAR is the database. */
typedef struct
{
	int                warning;         /* Forward collision warning level */
} long_fcw_warning_typ;

