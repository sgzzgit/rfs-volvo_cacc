/**\file	
 *  veh_truck.h   Onboard Monitoring System for trucks 
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 */

#define DEFAULT_CONFIG_FILE		"realtime.ini"


/* Following is DB_LAT_INPUT_SENSORS_TYPE, DB_LAT_INPUT_SENSORS_VAR in database */
typedef struct
{   
	float fb_pressure;	/* Brake pressure */
	float steer_angle;      /* Measured steering angle in degrees of roadwheel. */
	float long_accel;       /* Accelerometer x-axis (longitudinal) */
	float lat_accel;        /* Accelerometer y-axis (lateral) */
	int   turn_signal_left;	/* Left turn signal */
	int   turn_signal_right;/* Right turn signal */
	int   mirror_left;	/* Left mirror */
	int   mirror_right;	/* Right mirror */
	int   side_sensor_right;/* Right side sensor */
	int   ign_sig;  	/* Ignition signal */
	int   seatbelt_used;	/* Seatbelt use */
	int   zero_accel;
	char hour;		/* When written to database */
	char min;
	char sec;
	short millisec;
} IS_PACKED lat_input_sensors_typ;


/* Following is DB_LAT_STEER_INPUT_TYPE, DB_LAT_STEER_INPUT_VAR in database */
typedef struct
{   
	float steer_angle;      /* Measured steering angle in degrees of roadwheel. */
	float motor_cond;       /* Motor condition */
	int counta;             /* A-counter from the encoder card */
	int countx;             /* X-counter from the encoder card */
	int driver_status;      /* Driver status (0=NOT READY, 1=READY) */
	int failure_code;       /* Failure mode (0=None; 1=Motor failure,
	                         * 2=Motor power off; 3=command failure) */
} IS_PACKED lat_steer_input_typ;

/* Following is the definition for DB_GYRO_TYPE, DB_GYRO_VAR in the
 * database. */
typedef struct
{
	char hour;                   /* Time of day of last message, hours */
	char min;                    /* Time of day of last message, minutes */
	char sec;                    /* Time of day of last message, seconds */
	short int millisec;          /* Time of day of last message, milliseconds */
	float roll;            /* Roll in deg */
	float roll_rate;       /* Roll rate in deg/sec */
	float pitch;           /* Pitch in deg */
	float pitch_rate;      /* Pitch rate in deg/sec */
	float yaw_rate;        /* Yaw rate in deg/sec */
	float xaccel;          /* X-axis acceleration in G's */
	float yaccel;          /* Y-axis acceleration in G's */
	float zaccel;          /* Z-axid acceleration in G's */
	float temperature;     /* Internal temperature in C */
	float time;            /* free running clock  */
} IS_PACKED gyro_typ;

/* Following is the definition for DB_ROAD_SURFACE_TYPE, DB_ROAD_SURFACE_VAR in the
 * database. */
typedef struct
{
	int		short_voltage;	/* Short voltage */
	int		long_voltage;	/* Long voltage */
	int		temperature;	/* Temperature */
	float	ratio;			/* Short / Long ratio */
	short	disp_cond_code;	/* Displayed condition code */
	short	meas_cond_code;	/* Measured condition code */
	char	disp_cond_mnemonic[4];	/* Displayed condition mnemonic */
	char	meas_cond_mnemonic[4];	/* Measured condition mnemonic */
} IS_PACKED roadsurface_typ;


/* Following is the definition for DB_LONG_INPUT_TYPE, DB_LONG_INPUT_VAR
 * in the database. */
typedef struct
{
	int   platoon_pos;           /* Platoon position */
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
} IS_PACKED long_input_typ;


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
} IS_PACKED gps_gga_typ;

/* Following is the definition for DB_GPS_VTG_TYPE, DB_GPS_VTG_VAR in
 * the database. */
typedef struct
{
	float speed_over_ground;  /* Speed over ground in km/h */
} IS_PACKED gps_vtg_typ;



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
} IS_PACKED long_evradar_typ;

/* Following is the definition for DB_DDU_DISPLAY_TYPE, DB_DDU_DISPLAY_VAR, in the database. */
typedef struct
{
	char drv_ID[11];			 /* Driver ID */
	unsigned char status;			 /* Driver ID status */

   /* LED control */
   char power_on;		/* power on */
	char sys_fail;		/* system failure */
	char smart_cruise;/* smart cruise */
	char barcode;		/* barcode */
	char targ_det;		/* target_detect */
	char alert1;		/* Alert 1 */
	char alert2;		/* Alert 2 */

   /* Audio control */
   char audio_tone_sel;     /* unmapped audio tone select */
   char dwnld_tone;     /* downloaded tone */
   char creep_alrt;     /* creep alert */
   char alert1_2_sec;   /* alert 1 (2 second) */
   char alert2_1_sec;   /* alert 2 (1 second) */
   char vol_chg;        /* volume change */
   char drv_id_ok;      /* driver ID read success */
   char drv_id_fail;    /* driver ID read fail */
   char drv_id_gone;    /* driver ID removed */
   char dbg_stat;       /* engineering debug: stationary */
   char dbg_slow;       /* engineering debug: slow moving */

	char volume;		/* volume audio control */
   char start_tone;  /* state transition starts tone */
	char hour;		/* Time of day of last message, hours */
	char min;               /* Time of day of last message, minutes */
	char sec;               /* Time of day of last message, seconds */
	short int millisec;     /* Time of day of last message, milliseconds */
} IS_PACKED ddu_display_typ;



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
} IS_PACKED long_lidarA_typ;



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
} IS_PACKED long_lidarB_typ;

typedef struct 
{
    unsigned short l_target;      // 0: no left target; 1: Left target tracked
    float l_target_long_dist;	  // left target long distance
    float l_target_lat_dist;	  // left target lat distance
    float l_target_spd;	 	  // left target speed
	
    unsigned short target;        // 0: no front target; 1: front target tracked
    float target_long_dist;	  // front target long distance
    float target_lat_dist;	  // front target lat distance
    float target_spd;		  // front target speed
	
    unsigned short r_target;      // 0: no right target; 1: right target tracked
    float r_target_long_dist;	  // right target long distance
    float r_target_lat_dist;	  // right target lat distance
    float r_target_spd;		  // right target speed
} IS_PACKED mon_track_typ;


typedef struct 
{
    float grade;		  // road grade in degree from sensor
    float slip;			  // road slip from sensor estimation
    float curvature;		  // road curvature from NAVTEQ
    float spd_limit;		  // road speed limit from NAVTEQ
    float traffic_speed;          // traffic speed in left and right lanes
    float following_distance;	  // Front traget distance
    unsigned short follow_2_close; // 0: not close; 1: too close - grace period; 				   // 2: too close over grace period;
    unsigned short over_spd;       // 0: not over speed; 
				   // 1: over speed - grace period;
				   // 2: over spd over grace period
    unsigned short roll_over;      // 0: no danger; 1:over speed prediction;
				   // 2: over speed on curve; 3: begin to roll
    unsigned short warning;	   // 0: no warning; 1: orange; 2: red;  
				   // Not used in this phase
} IS_PACKED mon_feedback_typ;


/* Following is the definition for DB_NAVTEQ_POSITION_TYPE, DB_NAVTEQ_POSITION_VAR
 * in the database. */
typedef struct
{
	int  dist_from_start;        /* Distance from start node of root link, in cm */
	int  dist_since_last;        /* Distance travelled since last message, in cm */
	int  latitude;               /* New position, in DMD units */
	int  longitude;              /* New position, in DMD units */
	char hour;                   /* Time of day of last message, hours */
	char min;                    /* Time of day of last message, minutes */
	char sec;                    /* Time of day of last message, seconds */
	short int millisec;          /* Time of day of last message, milliseconds */
} IS_PACKED navteq_pos_typ;

/* Following is the definition for DB_NAVTEQ_SPEEDLIM_TYPE, DB_NAVTEQ_SPEEDLIM_VAR
 * in the database. */
typedef struct
{
	int  speed_limit;            /* Current speed limit */
	char hour;                   /* Time of day of last message, hours */
	char min;                    /* Time of day of last message, minutes */
	char sec;                    /* Time of day of last message, seconds */
	short int millisec;          /* Time of day of last message, milliseconds */
} IS_PACKED navteq_speedlim_typ;

/* Following is the definition for DB_NAVTEQ_CURVE1_TYPE, DB_NAVTEQ_CURVE1_VAR,
 * DB_NAVTEQ_CURVE2_TYPE, DB_NAVTEQ_CURVE2_VAR in the database. */
typedef struct
{
	int distance_ahead;          /* Distance ahead for a shape point */
	int radius;                  /* Radius of curvature at this shape point */
} IS_PACKED shape_point_typ;

#define NUM_SHAPE_POINTS	13
typedef struct
{
	shape_point_typ shape_point[NUM_SHAPE_POINTS];  /* Array of shape points */
	int num_points;              /* Number of total shape points */
	char hour;                   /* Time of day of last message, hours */
	char min;                    /* Time of day of last message, minutes */
	char sec;                    /* Time of day of last message, seconds */
	short int millisec;          /* Time of day of last message, milliseconds */
} IS_PACKED navteq_curve_typ;

 
/* Following is the definition for DB_NAVTEQ_RAIL1_TYPE, DB_NAVTEQ_RAIL1_VAR,
 * DB_NAVTEQ_RAIL2_TYPE, DB_NAVTEQ_RAIL2_VAR in the database. */
typedef struct 
{
        int rail_end_point_typ;      /* Left start = 0x01; left end = 0x11;
                                      * right start = 0x02; right end = 0x12 */
        int distance;                /* Distance in cm in front of vehicle */
} IS_PACKED guard_rail_typ;

typedef struct
{
        guard_rail_typ rail_point[13];  /* Array of guard rail points */
        int num_points;              /* Number of total guard rail points */
        char hour;                   /* Time of day of last message, hours */
        char min;                    /* Time of day of last message, minutes */
        char sec;                    /* Time of day of last message, seconds */
        short int millisec;          /* Time of day of last message, milliseconds */
} IS_PACKED navteq_rail_typ;

/* Following is the definition for DB_SAFETRAC_TYPE, DB_SAFETRAC_VAR in
 * the database. */
typedef struct
{
	int lat_offset;      /* Lateral offset, +/- 250 cm, neg = left of center */
	int lat_velocity;    /* Lateral velocity, +/- 250 cm, neg = towards left */
	float curvature;     /* Road curvature, -0.008 1/m to +0.008 1/m */
	int lane_width;      /* Lane width, 260-480 cm */
	int boundary_type;   /* Boundary type.  Integer between 00 and 33.
	* Tens digit represents left boundary, ones digit
	                      * is right boundary.  Meanings are:
	* 0 Boundary appears to be missing.
	* 1 Boundary appears to be dashed (intermittent).
	* 2 Boundary appears to be solid.
						 * 3 Boundary appears to be missing. */
	int offset_conf;     /* Offset confidence (0-100) */
	int curvature_conf;  /* Confidence in curvature estimate (0-100) */
	int alert_index;     /* Driver alertness index (0-99) */
	int alert_status;    /* Alert system status (see state table) */
   char event;          /* event */
	char lane_change;    /* lane change event */
	char ldw;            /* lane drift warning event */
	char drowsy_alert;   /* drowsy alert event */
	char clean_window;   /* clean window event */
} IS_PACKED safetrac_typ;

/* Following is the definition for DB_INCIDENT_TRIG_TYPE, DB_INCIDENT_TRIG_VAR in
 * the database. */
typedef struct
{
    /* Incident Flags */
    unsigned short close;
    unsigned short over_recommend_speed;
    unsigned short over_limit_speed;
    unsigned short hard_braking;
    unsigned short hard_steering;
    unsigned short fwd_collision_warning;
    unsigned short lane_departure;
    unsigned short seat_belt;
    unsigned short mirror_adjust;
    unsigned short no_signal_lane_change;
    unsigned short roll_over;

    /* Incident variables & parameters */
    float current_speed;
    float speed_limit;
    float recommended_speed;

    float current_following_time_gap;
    float recommended_following_time_gap;
    float recommended_following_dist;

    float long_acc_flt;
    float lat_acc_flt;
    float steering_rate;
    float steering_angle;
    float yaw_rate;
    float roll_angle;
    float roll_rate;
    float pitch_angle;
    float pitch_rate;
    float brake_pressure;

    float warning_time;
    float warning_level;
    int seat_belt_DIO;

    char curve_ahead;
    char camera_icon;
    float HOS_left;
    char hour;                   /* Time of day of last message, hours */
    char min;                    /* Time of day of last message, minutes */
    char sec;                    /* Time of day of last message, seconds */
    short int millisec;          /* Time of day of last message, milliseconds */
} IS_PACKED incident_trig_typ;

/* Following is the definition for DB_INCIDENT_TRIG2_TYPE, DB_INCIDENT_TRIG2_VAR in
 * the database. */
typedef struct
{
    int   tgt_status_l;
    float range_l;
    float lat_l;
    float rate_l;
    int   tgt_status;
    float range;
    float lat;

    float rate;
    int   tgt_status_r;

    float range_r;
    float lat_r;
    float rate_r;

    float warning_time;
    float warning_level;
    int seat_belt_DIO;

    char hour;                   /* Time of day of last message, hours */
    char min;                    /* Time of day of last message, minutes */
    char sec;                    /* Time of day of last message, seconds */
    short int millisec;          /* Time of day of last message, milliseconds */
} IS_PACKED incident_trig2_typ;
