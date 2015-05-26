/* FILE:  dvimsg.h  Define data structures for DVI communication
 *
 * Copyright (c) 2003 Regents of the University of California
 *
 * static char rcsid[] = "$Id$";
 *
 *
 * $Log$
 *
 */

/* Structure for current mode values written by monitor process to
 * database. These are also part of message sent to DVI.
 */

//#ifdef __QNX6___
//#define IS_PACKED __attribute__((__packed__))
//#else
//#define IS_PACKED
//#endif

#define HEARTBEAT_SEQUENCE 255
typedef struct {
	int mode;	/* see mode definitions in dvi_mode.h */
	int lat_mode;	/* from lat_dvi_output_typ */
	int long_mode;	/* from long_dvi_output_typ */
	unsigned char passenger_count;	/* from DVI */	
	unsigned char vehicle_id;	/* from DVI */	
	unsigned char travel_direction;	/* from DVI */	
	unsigned char acc_transition;	/* from DVI */	
	unsigned char acc_speed_set;	/* from DVI */	
	unsigned char acc_distance_set;	/* from DVI */	
} dvi_monitor_typ;   

/* Structure for message ID 1, message from control computer to DVI.
 */
typedef struct {
	char               header1;         /* 0x99 */
	char               header2;         /* 0x44 */
	char               header3;         /* 0x22 */
	char               ID;              /* 0x01 */
	char               length;          /*  64  */
	char		   hour;
	char		   min;
	char		   sec;
	short int	   millisec;
	unsigned char      mag_status;      /* Magnetometer status (0=off, 1=on,
	                                     * 2=fault) */
	unsigned char      mode;     	    /* Current overall mode */
	unsigned char      button_mask;   /* Mask for buttons enabled */
	unsigned char	   fault_code;	    /* Set by monitor process */
	unsigned char	   switch_mask;	    /* Current settings of switches */
	unsigned char      comm_state;      /* Communications state */
	unsigned char      system_fault;    /* set from lat, comm, long faults */
	float              fuel_econ;       /* Average fuel economy (from J-bus)
	                                     * (m/cm**3) */
	unsigned char      platoon_info;    /* Can expand if needed */
	unsigned char      p_current_speed; /* Platoon speed (4 * m/sec) */
	unsigned char      p_target_speed;  /* Target platoon speed (4 * m/sec) */
	unsigned char      p_length;        /* Platoon length */
	unsigned char      p_veh_id;        /* Platoon ID of subject vehicle */
	unsigned char      p_veh_location;  /* Location of subject vehicle
	                                     * in platoon */
	unsigned char      lat_mode;        /* Lateral mode variable  */
	unsigned char      long_mode;       /* Longitudinal mode variable */
	unsigned char      veh_speed;       /* Vehicle speed, round (m/sec * 4) */
	unsigned char      veh_target;      /* Target vehicle speed (m/sec) */
	unsigned char      lane_id;         /* Lane ID (0=unknown, 1=right,
	                                     * 2=left) */
	short int          lat_pos;         /* Lateral position, neg offset right */
	unsigned char      lane_depart_lev; /* Lane departure level */
	unsigned char      lane_depart_dir; /* Lane departure direction */
	unsigned short int long_pos;        /* Longitudinal position */
	unsigned short int distance_to_end; /* Distance to end of magnets */
	unsigned char      rl_status;       /* Radar/lidar status (bit 0 = 1
	                                     * for radar on, bit 1 = 1 for lidar on) */
	unsigned char      track_targ;      /* Tracking known target flag (1=yes) */
	unsigned char      curr_gap;        /* Current gap */
	signed char        gap_rate;        /* Gap closing rate */
	unsigned char      target_gap;      /* Target vehicle gap */
	unsigned char      fcw_level;       /* Forward collision warning level */
	unsigned char 	   serial_timeout;
	unsigned char      checksum;
} IS_PACKED dvi_mess_typ;

/* Structure for message ID 2, message from DVI to control computer.
 */
typedef struct
{
	char               header1;         /* 0x99 */
	char               header2;         /* 0x44 */
	char               header3;         /* 0x22 */
	char               ID;              /* 0x01 */
	char               length;          /*  64  */
	char		   hour;
	char		   min;
	char		   sec;
	short int	   millisec;
	unsigned char      mode;    
	unsigned char	   lat_mode;
	unsigned char      long_mode;    
	unsigned char      button;	/* value 5 to 0 for button, 6 none */
	int		   number;
	unsigned char	   sequence;	/* used to send only once */
	unsigned char 	   serial_timeout;
	unsigned char	   checksum;
} IS_PACKED dvi_mode_typ;

/* button type used only on QNX6; header file is shared with DVI computer*/
typedef struct {
        int                     num0;
        int                     num1;
        int                     num2;
        int                     num3;
        int                     num4;
        int                     num5;
        int                     num6;
        int                     num7;
        int                     num8;
        int                     num9;

        int                     numPound;       /* '#' */
        int                     numAsterisk;    /* '*' */

        int                     selectY;        /* Yellow button */
        int                     selectG;        /* Green button */

        int                     screen[6];        /* Top 5 bottom 0 */

        int                     dpadUp;
        int                     dpadLeft;
        int                     dpadRight;
        int                     dpadDown;
} button_input_typ;

/* Used only on QNX6 by DVI computer, type stored in database for
 * communication with display.
 */
typedef struct {
	int			timed_out;	/* if 1, timed out */
} serial_timeout_typ;

/** Changed by EAV 6/2/2003 from "char" to "unsigned char" in order for
  * comparisons made in the rddvimsg.c file to be valid.  For example,
  * data[0] != 0x99 will ALWAYS evaluate to true if it is signed.
 */
typedef struct
{
	unsigned char    data[100];
} gen_mess_typ;

typedef union
{
	dvi_mess_typ    dvi_mess;
	dvi_mode_typ	dvi_mode;
	gen_mess_typ    gen_mess;
} mess_union_typ;
