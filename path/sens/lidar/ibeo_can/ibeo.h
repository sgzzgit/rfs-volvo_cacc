/**\file
 *
 *	ibeo.h	Header file for IBEO Automobile Sensor Laserscanner
 *
 *	Definitions in this file are based on "Specification of the
 *	CAN Message Protocol for IBEO Automobile Sensor GmbH Laserscanners",
 *	authors Roland Krzikalla, Mario Brumm, Volker Willhoeft, Version 1.9.0,
 *	Date: 22.04.02. See that document for interpretation of fields.
 */

#ifndef IBEO_H
#define IBEO_H

#ifdef __QNX__
#ifdef __QNXNTO__
#include "ibeo_qnx6.h"
#else
#include "ibeo_qnx4.h"
#endif
#else
#include <sys_os.h>
#include <local.h>
#include <db_clt.h>
#include <timestamp.h>
#endif

#include "ibeo_vars.h"

#define IBEO_ALASCA_OBJECT_DATA_TYPE	1
#define IBEO_ALASCA_COMPRESSED_SCAN_DATA_TYPE	15
/// The following type was present but I have not seen it documented
#define IBEO_ALASCA_255_DATA_TYPE	255

/**
 *  	For the IBEO ALASCA, Ethernet messages have a 16 byte header
 */
typedef struct {
	int magic_word;		/// always 0xaffec0c0
	int size;
	int message_type;	/// 1 object data, 15 compressed scan
	int milliseconds;	/// timestamp in milliseconds 
} ibeo_alasca_header_t;

/**
 *  The IBEO Laserscanners use CAN v2.0A (11-big identifiers) for communication
 *  to the host.
 */
typedef unsigned short can_std_id_t; 

/** The CAN base value will be constant for each activation of the driver,
 *  corresponding to a single laserscanner.
 */

extern can_std_id_t ibeo_can_base;

#define IBEO_COMMAND_ID		ibeo_can_base	// ID of commands to scanner
#define IBEO_DATA_ID		(ibeo_can_base + 1) // ID of data from scanner
#define IBEO_SYNC_ID		0x0f0
#define IBEO_VELOCITY_ID	0x0f1
#define IBEO_YAWRATE_ID		0x0f2
#define IBEO_STEERINGANGLE_ID	0x0f3
#define IBEO_CALIBRATION_ID	0x0fa

#define IBEO_DEFAULT_SENSOR_ID	0x4f0

/* Gets a mask for a field within an 8-bit byte value */ 
#define GETMASK(hi,low) (((1<<(hi+1))-1) & (~((1<<(low))-1))) 

/* Gets the field, as a standalone value, from an unsigned char */
#define GETFIELD(val,hi,low) (GETMASK(hi,low) & (val))>>(low) 

typedef struct {
	char *name;
	unsigned short id;
	unsigned short length; 	// of mesage in bytes
	unsigned short interval; // between transmissions, in millisec
} ibeo_instruction_t;

/* Value for the Message Type field in a Sensor Data message
 */
#define IBEO_DATA_LIST_ID	1	// list header
#define IBEO_DATA_OBJ_ID	2	// basic object header
#define IBEO_DATA_EXT1_ID	3	// object deviation info 
#define IBEO_DATA_EXT2_ID	4	// object classification and age 
#define IBEO_DATA_EXT3_ID	5	// extended object info
#define IBEO_DATA_PT_ID		6	// object point
#define IBEO_DATA_END_ID	7	// list end

typedef struct {
	unsigned char object_style;
	unsigned char object_count;
	unsigned char sensor_status;	// 0 error, 1 OK
	unsigned char calibration_flag; // 0 not calibrated, 1 calibrated
	unsigned char cycle_counter;
	unsigned int timestamp; // beginning of measurement, millisecs
} ibeo_list_header_t; // basic list header type

typedef struct {
	unsigned char sensor_dirty;	// 0 OK, 1 dirty, 3 unavailable
	unsigned char rain_detection;	// 0 rain, 1 no rain, 3 unavailable
	unsigned char dirt_start;	// start angle = 2 * val
	unsigned char dirt_end;		// end angle = 2 * val
					// 254 means no dirt, 255 unavailable
} ibeo_list_ext1_t;	// list header type, extension 1 

/* list header ext 2 has complicated scaling for values, see specification
 * not supported in IBEO CAN message protocol, v 1.7.0
 */
typedef struct {
	unsigned short left_lane_offset;
	unsigned short right_lane_offset;
	unsigned short heading_angle;
	unsigned short curvature;
	unsigned short confidence;
	unsigned short horizon;
} ibeo_list_ext2_t;	// list header type, extension 2 

/* has "list header" message type with "object number" 2
 */
typedef struct {
	unsigned char parameter;
	unsigned char data_type;
	unsigned int data_value; 
} ibeo_parameter_data_t;	

typedef struct {
	unsigned char tracking_number;	// 0-31
	unsigned char tracking_status;	// 0 known, 1 new
	unsigned char classification;   // 0-15
	unsigned char point_count;	// 1-16 
	unsigned short position_x;
	unsigned short position_y;
	unsigned char velocity_x;
	unsigned char velocity_y;
	unsigned char velocity_x_ext;
	unsigned char velocity_y_ext;
} ibeo_object_header_t;	

typedef struct {
	unsigned char relative_moment; // 0-126 millisec, from start of cycle
	unsigned char position_x_sigma;
	unsigned char position_y_sigma;
	unsigned char velocity_x_sigma;
	unsigned char velocity_y_sigma;
	unsigned char position_cor;
	unsigned char velocity_cor;
} ibeo_object_ext1_t;	

typedef struct {
	unsigned char height;
	unsigned char height_sigma;
	unsigned char class_certainty;
	unsigned char class_age;
	unsigned short object_age;
} ibeo_object_ext2_t;	

typedef struct {
	unsigned short ttc;	/// time to collision, millisecconds
	unsigned char crash_probability;	/// 0 to 100
} ibeo_collision_info_t;	

/// this type is really dopey because it is only reported every
/// other object, making it a real pain to parse into a one object
/// per line (or per data base row) output format
/// not currently in output ibeo_obj_typ, but is parsed and can be printed
/// when debugging; if data is useful we can add to obj array
/// scaling is same as for relative velocity
typedef struct {
	unsigned char x0;	
	unsigned char y0; 
	unsigned char x0_ext;
	unsigned char y0_ext;
	unsigned char x1;	
	unsigned char y1; 
	unsigned char x1_ext;
	unsigned char y1_ext;
} ibeo_abs_velocity_t;

// use to identify info_type 
#define IBEO_EXT3_COLLISION_INFO	0	
#define IBEO_EXT3_ABS_VELOCITY		1	

/// Extended Object Info is different depending on info_type 
typedef struct {
	unsigned char info_type;
	union {
		ibeo_collision_info_t collision_info; /// info type 0
		ibeo_abs_velocity_t abs_velocity;     /// info type 1
		unsigned char data[6];		      /// 6 bytes available
	} info; 
} ibeo_object_ext3_t;	

/** To scale x, y coordinates,
 *	0..8000 : 0,05 * value - 200
 *	8191: not available
 */

typedef struct {
	unsigned char point_number;	// 0-15 
	unsigned short x0;		// first point x, relative coordinate
	unsigned short y0;		// first point y, relative coordinate
	unsigned short x1;		// second point x, relative coordinate
	unsigned short y1;		// second point y, relative coordinate
} ibeo_object_point_t;	

typedef struct {
	unsigned char sensor_status;
	unsigned char sensor_id;
	unsigned char cycle_error;
	unsigned char checksum_error;
} ibeo_list_end_t;	

/* Used to parse a single CAN message from laserscanner,
 * union of possible formats
 */
typedef struct {
	unsigned char raw_data[8];
	unsigned char oid;
	unsigned char onum;
	union {
		ibeo_list_header_t lst;
		ibeo_list_ext1_t lst_ext1;
		ibeo_list_ext2_t lst_ext2;
		ibeo_object_header_t obj;
		ibeo_object_ext1_t obj_ext1;
		ibeo_object_ext2_t obj_ext2;
		ibeo_object_ext3_t obj_ext3;
		ibeo_object_point_t pt;
		ibeo_list_end_t end;
		ibeo_parameter_data_t param;
	} msg;
} ibeo_data_t;

/* Used to hold list information in database.
 * Only info from basic list header seems to be available in current 
 * hardware, but other fields may need to be added in the future. 
 * July 2008: Added fields documented for Environment Info in 1.7.0
 */

typedef struct {
	unsigned char object_style;
	unsigned char object_count;
	unsigned char sensor_status;	// 0 error, 1 OK
	unsigned char calibration_flag; // 0 not calibrated, 1 calibrated
	unsigned char cycle_counter;
	unsigned int timestamp; // beginning of measurement, millisecs
	unsigned char sensor_dirty;	// 0 OK, 1 dirty, 3 unavailable
	unsigned char rain_detection;	// 0 rain, 1 no rain, 3 unavailable
	float dirt_start;	// start angle = 2 * val - 180.0
	float dirt_end;		// end angle = 2 * val - 180.0
} ibeo_list_typ;


typedef struct {
	unsigned short x;
	unsigned short y;
} ibeo_pt_typ;

#define IBEO_MAX_OBJECTS 32
#define IBEO_MAX_POINTS	16

/* Per object point list; stored in variable that has matching object number
 */
typedef struct {
	unsigned char object_number;	/// 0-31	
	unsigned char tracking_number;	/// can be 8 bits
	unsigned char tracking_status;	/// 0 known, 1 new
	unsigned char classification;   /// 0-15
	unsigned char point_count;	/// 1-16  
	float velocity_x;		// relative, range -63.75 to +62.71 m/s
	float velocity_y;		// relative, range -63.75 to +62.71  
	float relative_moment; 		// 0-126 millisec, from start of cycle
	float position_x_sigma;
	float position_y_sigma;
	float velocity_x_sigma;
	float velocity_y_sigma;
	float position_cor;
	float velocity_cor;
	float height;
	float height_sigma;
	unsigned char class_certainty;
	unsigned char class_age;	/// count of tracking scans
	unsigned short object_age;
	unsigned short ttc;		/// time to collision
	unsigned short crash_probability;		
	ibeo_pt_typ point[IBEO_MAX_POINTS];	// 16 possible points
}ibeo_obj_typ; 

/* Function definitions in ibeo_qnx*
 */
extern int init_can(int fd, db_clt_typ *pclt, char **argv);
extern int can_get_message(int fd, int channel_id, unsigned long *pid,
			 unsigned char *data, int length);
extern db_clt_typ *ibeo_database_init(char *progname, char *domain);

/* External function definitions in ibeo_utils
 */

extern int ibeo_parse_message(ibeo_data_t *pcur,	// parsed data returned here
			can_std_id_t id,	// CAN ID 
			unsigned char *data,	// raw data
			unsigned char *pcycle,	//pointer to current cycle 
			int *pchk);		// pointer to running checksum
extern void ibeo_pack_data(ibeo_list_typ *plist, ibeo_obj_typ *obj_array,
			ibeo_data_t *pcur);
extern void ibeo_update_database(db_clt_typ *pclt, ibeo_list_typ *plist,
				 ibeo_obj_typ *obj_array);
extern int get_current_time(int *phour, int *pmin, int *psec, int *pmillisec);
extern void ibeo_print_database(FILE *fp, ibeo_list_typ *plist, 
					ibeo_obj_typ *obj_array, 
					timestamp_t *pts);
#endif
