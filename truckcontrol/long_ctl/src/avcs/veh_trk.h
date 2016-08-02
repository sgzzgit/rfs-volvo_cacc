/**\file      
 *  veh_trk.h    Version for Truck Control 2009 
 *		
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *
 */

#ifndef VEH_TRK_H
#define VEH_TRK_H

#define DEFAULT_CONFIG_FILE             "realtime.ini"

#include <timestamp.h>

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
        short int manual_trans; /* Manual transition (1=ON, 0=OFF) */
        short int auto_trans;   /* Automatic transition (1=ON, 0=OFF) */
} lat_input_sensors_typ;

/* Following is the definition for DB_LONG_INPUT_TYPE, DB_LONG_INPUT_VAR
 * in the database. This data was previously filled in by veh_trk, some
 * of it by reading LAT variables, some by reading A/D, etc.; most (all?)
 * of the fields are currently not valid (Sue, Nov 2008)
 */
typedef struct
{
	timestamp_t ts;		     /* Timestamp */
        int platoon_pos;             /* Platoon position */
        float fb_applied;            /* Front brake applied transducer */
        float rb_applied;            /* Rear brake applied transducer */
        float fb_monitor;            /* Front brake monitor transducer */
        float rb_monitor;            /* Rear brake monitor transducer */
        float fb_axle;               /* Front brake axle transducer */
        float mb_axle;               /* Middle brake axle transducer */
        float rb_axle;               /* Rear brake axle transducer */
        float acc_pedal;             /* Accelerator pedal */
        float trans_retarder;        /* Transmission retarder */
        int engine_fan;              /* Engine desired fan state */
} IS_PACKED long_input_typ;

typedef struct {
	timestamp_t ts;
	unsigned char manualctl;
	unsigned char autoctl;
	unsigned char man_autoswerr;
	unsigned char brakesw;
	unsigned char brakeswerr;
} IS_PACKED long_dig_in_typ;

typedef struct {
	timestamp_t ts;
	unsigned char outchar;
	unsigned char amber_flash;
	unsigned char xy_alive;
} IS_PACKED long_dig_out_typ;

/* Following is the definition for DB_LONG_OUTPUT_TYPE, DB_LONG_OUTPUT_VAR
 * in the database. */
typedef struct
{
        float engine_speed;
        float engine_torque;
        float engine_retarder_torque;
        unsigned char engine_command_mode;      /* 0=disable, 1=speed, 2=torque,
                                                    * 3=speed/torque limit */
        unsigned char engine_retarder_command_mode; /* 0=disable, 2=torque,
                                                     * 3=torque limit */
	unsigned char engine_priority;
	unsigned char engine_retarder_priority;
	unsigned char brake_priority;
        float acc_pedal_control;      /* Accelerator pedal control (bus only) */
        float ebs_deceleration;
        unsigned char brake_command_mode;   /* 0=not active, 1=active */
        float trans_retarder_value;         /* Percent of maximum */
        unsigned char trans_retarder_command_mode; /* 0=disable, 2=enable */
        float fb_control;                   /* Front brake control (bus only) */
        float rb_control;                   /* Rear brake control (bus only) */
        int fan_override;                   /* Engine fan override */
        int fan_control;                    /* Engine fan ON/OFF */
	unsigned char selected_gap_level;   // Gap level sent to DVI
} IS_PACKED long_output_typ;

#endif /* VEH_TRK_H */
