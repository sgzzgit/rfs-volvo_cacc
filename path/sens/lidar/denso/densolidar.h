/* FILE:  densolidar.h  Header file for the second generation of Denso lidar
 *
 * Copyright (c) 2009  Regents of the University of California
 *
 */
#ifndef DENSOLIDAR_H
#define DENSOLIDAR_H


#include <sys_os.h>

#define LIDAR1_SERIAL_DEVICE_NAME "/dev/ser3"
#define DB_LONG_LIDARA_TYPE	206  /* long_lidarA_typ         */
#define DB_LONG_LIDARB_TYPE	207  /* long_lidarB_typ         */
#define DB_LONG_LIDARA_VAR	DB_LONG_LIDARA_TYPE
#define DB_LONG_LIDARB_VAR	DB_LONG_LIDARB_TYPE


typedef struct
{
	unsigned char start1;       /* Start of message (always 0xff) */
	unsigned char start2;       /* Start of message (always 0xff) */
	unsigned char function;     /* Function code (always 0xfd) */
	unsigned char dummy1;       /* Dummy data (always 0x00) */
	unsigned char dummy2;       /* Dummy data (always 0x00) */
	unsigned char dummy3;       /* Dummy data (always 0x00) */
	unsigned char dummy4;       /* Dummy data (always 0x00) */

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

	char      checksum;         /* Sum of all preceding bytes (if it is 0xff,
	                             * it changes to 0x00) */
} IS_PACKED lidar2_mess_typ;

typedef struct
{
	unsigned char start1;     /* Start byte 1 (always 0xff) */
	unsigned char start2;     /* Start byte 2 (always 0xff) */
	unsigned char function;   /* Function code (always 0xfd) */
	unsigned char cruise_stat;/* Cruise status (always 0x00) */
	unsigned char velocity;   /* Velocity, LSB=1 km/h */
	unsigned char curve_h;    /* Horizontal road curvature, LSB=100 m */
	unsigned char curve_l;    /* Lateral road curvature, LSB=20 m */
	unsigned char dummy;      /* Dummy data, (always 0x00) */
	unsigned char checksum;   /* Sum of all preceding bytes (if it is 0xff,
	                           * it changes to 0x00) */
} IS_PACKED tolidar2_mess_typ;

typedef struct
{
	char    data[150];
} IS_PACKED gen_mess2_typ;

typedef union
{
	lidar2_mess_typ   lidar2_mess;
	tolidar2_mess_typ tolidar2_mess;
	gen_mess2_typ     gen_mess2;
} IS_PACKED mess_union2_typ;


/* Following is the definition for DB_LONG_LIDARA_TYPE, DB_LONG_LIDARA_VAR
 * in the database.  */
typedef struct
{
        char hour;                  /* Time of day of last message, hours */
        char min;                   /* Time of day of last message, minutes */
        char sec;                   /* Time of day of last message, seconds */
        short int millisec;         /* Time of day of last message, milliseconds
 */

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
                                                                  
        signed char curve_h;        /* Horizontal curve radius received from PC
*/                                                                
        signed char curve_l;        /* Lateral curve radius received from PC */
} IS_PACKED long_lidarB_typ;                                      
#endif
