/**\file
 *  ids_io.h    Version for the Intersection Decision Support (IDS) project.
 *
 *  		Out-of-date example version used for sample client programs
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 */


/* Following is the definition for DB_DII_OUT_TYPE, DB_DII_OUT_VAR
 * in the database. */
typedef struct
{
	int dii_flag;                /* Bits to send to DII */
} dii_out_typ;


/* Following is DB_LONG_RADAR1_TYPE, DB_LONG_RADAR1_VAR,
 *              DB_LONG_RADAR2_TYPE, DB_LONG_RADAR2_VAR in database. */
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
} long_radar_typ;


/* Following is the definition for DB_LONG_LIDAR1A_TYPE, DB_LONG_LIDAR1A_VAR,
 * DB_LONG_LIDAR2A_TYPE and DB_LONG_LIDAR2A_VAR in the database.  */
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



/* Following is the definition for DB_LONG_LIDAR1B_TYPE, DB_LONG_LIDAR1B_VAR,
 * DB_LONG_LIDAR2B_TYPE and DB_LONG_LIDAR2B_VAR in the database.  */
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


/* Following is the definition for DB_GPS1_GGA_TYPE, DB_GPS1_GGA_VAR in
 * the database. */
typedef struct
{
	float utc_time;           /* Current UTC time of fix in hours, minutes,
	                           * and seconds (hhmmss.ss) */
	double latitude;          /* Latitude component of position in degrees
	                           * and decimal minutes (ddmm.mmmmm) */
	double longitude;         /* Longitude component of position in degrees
	                           * and decimal minutes (ddmm.mmmmm) */
	float altitude;           /* Altitude in meters above the ellipsoid */
} gps_gga_typ;

/* Following is the definition for DB_GPS1_VTG_TYPE, DB_GPS1_VTG_VAR in
 * the database. */
typedef struct
{
	float speed_over_ground;  /* Speed over ground in km/h */
} gps_vtg_typ;
