/* \file:  evt300.h 
 *
 * Copyright 2007 Regents of the University of California
 *
 * Contains structure and database variable definitions for evt300
 * Also contains definitions of message types
 */
#ifndef EVT300_H
#define EVT300_H

#ifdef __QNXNTO__
/// Cogent datahub implementation has limitations to <1100 variable ID
#define EVT300_BASE 1000
#else
#define EVT300_BASE 5000
#endif

#define DB_EVT300_RADAR1_TYPE         EVT300_BASE + 1  /* evt300_mess_typ */
#define DB_EVT300_RADAR2_TYPE         EVT300_BASE + 2  /* evt300_mess_typ */
#define DB_EVT300_RADAR3_TYPE         EVT300_BASE + 3  /* evt300_mess_typ */
#define DB_EVT300_RADAR4_TYPE         EVT300_BASE + 4  /* evt300_mess_typ */

#define DB_EVT300_RADAR1_VAR         DB_EVT300_RADAR1_TYPE 
#define DB_EVT300_RADAR2_VAR         DB_EVT300_RADAR2_TYPE 
#define DB_EVT300_RADAR3_VAR         DB_EVT300_RADAR3_TYPE 
#define DB_EVT300_RADAR4_VAR         DB_EVT300_RADAR4_TYPE 

#define EVT300_MAX_TARGETS	7
#define EVT300_MAX_BYTES	134

/** Structure used by parsing code. Note that the EVT300 sends binary
 *  data over a serial port; evt_300_mess_type defines the fields of
 *  this binary data as the message comes from the radar; the union
 *  type gen_mess_type allows the bytes of the message to be referred
 *  to by byte number instead of field name, which is more convenient
 *  for reading in and check sum calculation. Note that the binary
 *  data message coming from the radar is actually variable sized,
 *  with the targ_count field indicating how many tarets there are. 
 */
typedef struct
{
	char          msgID;       /* 82 for front end targer report message */
	char          FFTframe;         /* LS 8 bits of FFT Frame Number */
	char          targ_count;       /* Number of targets (0-7) */
	unsigned char target_1_id;      /* Target ID number (1-255) */
	short int     target_1_range;   /* Range, LSB = 0.1 ft */
	short int     target_1_rate;   /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_1_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_1_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_1_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                         * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7. */
	unsigned char target_2_id;      /* Target ID number (1-255) */
	short int     target_2_range;   /* Range, LSB = 0.1 ft */
	short int     target_2_rate;   /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_2_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_2_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_2_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                         * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7. */
	unsigned char target_3_id;      /* Target ID number (1-255) */
	short int     target_3_range;   /* Range, LSB = 0.1 ft */
	short int     target_3_rate;  /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_3_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_3_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_3_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                         * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	unsigned char target_4_id;      /* Target ID number (1-255) */
	short int     target_4_range;   /* Range, LSB = 0.1 ft */
	short int     target_4_rate;  /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_4_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_4_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_4_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                         * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	unsigned char target_5_id;      /* Target ID number (1-255) */
	short int     target_5_range;   /* Range, LSB = 0.1 ft */
	short int     target_5_rate;  /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_5_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_5_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_5_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                         * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	unsigned char target_6_id;      /* Target ID number (1-255) */
	short int     target_6_range;   /* Range, LSB = 0.1 ft */
	short int     target_6_rate;   /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_6_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_6_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_6_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                         * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	unsigned char target_7_id;      /* Target ID number (1-255) */
	short int     target_7_range;   /* Range, LSB = 0.1 ft */
	short int     target_7_rate;   /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_7_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_7_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_7_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                         * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	char      checksum;
} IS_PACKED evt300_mess_typ;

typedef struct
{
	unsigned char    data[EVT300_MAX_BYTES];	
} IS_PACKED evt300_gen_mess_typ;

typedef union
{
	evt300_mess_typ        evt300_mess;
	evt300_gen_mess_typ           gen_mess;
} IS_PACKED evt300_mess_union_typ;

typedef struct
{
	unsigned char id;   	/// ID number of nearest object 
	short int range;    	/// Range of object, LSB=0.1 ft 
	short int rate;     	/// Rate of object approach, LSB=0.1 ft/s 
	signed char azimuth;	/// Target azimuth, LSB=0.002 radians */
	unsigned char mag;  	/// Magnitude, LSB=-0.543 dB */
	unsigned char lock; 	/// Bit mapped, 1=locked, 0=not locked
	                        /// bit 0 is current FFT frame (n),
				/// bit 1 is FFT frame n-1,...,
				/// bit 7 is FFT frame n-7 
} evt300_target_t;
	
/** Structure written by evt300 driver to data server
 */
typedef struct
{
	timestamp_t ts;			/// time read from driver	
	char target_count;       	/// Number of targets (0-7) 
	evt300_target_t target[EVT300_MAX_TARGETS];
	unsigned char mess_ID;          /// Message ID 
} IS_PACKED  evt300_radar_typ;

/** Structure used for identification and error reporting information
 *  passed between main program and libary routines.
 */
typedef struct
{
	char *id;		// string used to identify radar
	int error_count;	// used to count checksum errors
	int verbose;		// option to print error messages
} IS_PACKED evt300_radar_info_t;

/** Codes for the msgID field of evt300_mess_typ
 *  The EVT300 consists of the CPU, the FE (Front End) and the
 *  DDU (Driver Display Unit.) The "Test Equipment" is the
 *  computer reading the data. 
 */
#define UNUSED	0

/** Messages set to the DDU are assigned ID numbers from 1 through 31
 *  Messages sent to the DDU from the CPU are assigned ID numbers 1 through 15
 */
#define DDU_ID_REQUEST_MESSAGE	1
#define DDU_DISPLAY_UPDATE_MESSAGE	2
#define DDU_TONE_DATA_MESSAGE	3
#define DDU_RESET_MESSAGE	4
#define DDU_DRIVER_ID_DATA_REQUEST_MESSAGE	5

/** Messages sent to the DDU from the Test Equipment are assigned
 *	 ID numbers 16 through 31
 */
#define DDU_DATA_REQUEST_MESSAGE	16

/** Messages sent to the FE are assigned ID numbers 32 through 63
 *  Messages sent to the FE from the CPU are assigned ID numbers 32 through 47
 */

#define FE_ID_REQUEST_MESSAGE	32
#define FE_TRANSMITTER_CONTROL_MESSAGE	33
#define FE_CRITICAL_TARGET_MESSAGE	34
#define FE_CORRECTION_VALUES_MESSAGE	35
#define FE_VEHICLE_DATA_MESSAGE	36
#define FE_SOFTWARE_UPDATE_MESSAGE	37
#define FE_RESET_MESSAGE	38
#define FE_BORESIGH_DATA_MESSAGE	39

/** Messages set to the FE from the Test Eqipment are assigned 
 *	ID numbers 56 through 63
 */
#define FE_CALIBRATION_MODE_REQUEST_MESSAGE	56
#define FE_FREQUENCY_CALIBRATION_MESSAGE	57
#define FE_ASIMUTH_CALIBRATION_MESSAGE	58
#define FE_SERIAL_NUMBER_ASSIGNMENT_MESSAGE	59
#define FE_TEMPERATURE_SENSOR_DATA_REQUEST_MESSAGE	60
#define FE_DATA_REQUEST_MESSAGE	61

/** Messages sent to the CPU are assinged ID numbers 64 trough 127
 *  Messages sent to the CPU from the DDU are assigned ID numbers 64 through 79
 */
#define DDU_POWER_ON_COMPLETE_MESSAGE	64
#define DDU_ID_REPORT_MESSAGE	65
#define DDU_STATUS_MESSAGE	66
#define DDU_DRIVER_ID_DATA_MESSAGE	67

/** Messages sent to the CPU from the FE are assigned ID numbers 80 through 95 
 */
#define FE_POWER_ON_MESSAGE	80
#define FE_ID_REPORT_MESAGE	81
#define FE_TARGET_REPORT_MESSAGE	82	 
#define FE_STATUS_AND_BIST_REPORT_MESSAGE	83
#define FE_CORRECTION_VALUE_REQUEST_MESSAGE	84
#define FE_CORRECTION_VALUE_UPDATE_MESSAGE	85
#define FE_SOFTWARE_UPDATE_ACKNOWLEDGE_MESAGE	87
#define FE_BORESIGHT_UPDATE_MESSAGE	88
#define FE_TARGET_REPORT_2_MESSAGE	89

/** Messages sent to the CPU from the Test Equipmenet are assigned
 *	 ID numbers 96 through 111
 */
#define CPU_BIST_REPORT_REQUEST_MESSAGE	96
#define CPU_MONITOR_REQUEST_MESSAGE	97
#define CPU_SERIALNUMBER_ASSIGNMENT_MESSAGE	98
#define CPU_SOFTWARE_UPDATE_MESSAGE	99
#define CPU_DATA_REQUEST_MESSAGE	100
#define CPU_CLEAR_FAULT_LOG_MESSAGE	101
#define CPU_TEST_CONTROL_MESSAGE	102
#define CPU_RESET_MESSAGE	103
#define CPU_BLOCK_TRANSFER_ACKNOWLEDGE_MESSAGE	105
#define CPU_ID_REQUEST_MESSAGE	105
#define CPU_CONFIGURATION_DATA_REQUREST_MESAGE	106

/** Message ID numbers 128 through 191 are reserved.
 *  Messages sent to the Test Equipment are assigned ID numbers 192 through 240.
 *  Messages set to the Test Equipment from the DDU are assigned ID numbers
 *	192 through 199.
 */
#define DUU_DATA_TRANSFER_MESSAGE	192

/** Messages sent to the Test Equipment from the FE are assigned
 *	ID numbers 218 through 231
 */
#define CPU_SOFTWARE_UPDATE_ACKNOWLEDGE_MESSAGE	216
#define CPU_DATA_TRANSFER_MESSAGE	217
#define CPU_BIST_STATUS_MESSAGE	218
#define CPU_ID_REPORT_MESAGE	219
#define CPU_BLOCK_TRANSFER_MESSAGE	220
#define CPU_CONFIGURATION_DATA_MESSAGE	221
#define CPU_DEBUG_MESSAGE	225

/** Messages send to all units are assigned ID numbers 240 through 255.
 */
#define CPU_POWER_ON_MESSAGE	240
#define VBUS_SHUTDOWN_MESSAGE	255

/** Structure for use in static array in evt300_lib.c that holds information
 * 	about each message type. The index into the array is
 *	the same as the msg_id for all defined messages, 0 for unused.
 */
typedef struct {
	char * msg_name;
	unsigned char msg_id;
	short num_bytes;
} evt300_msg_info_t;

extern bool_typ evt300_ser_driver_read(int fpin, 
	evt300_radar_typ *p300, evt300_radar_info_t *pinfo);
extern void evt300_print_radar(FILE *f_radar,
	evt300_radar_typ *pradar, timestamp_t write_time,
	evt300_radar_info_t *pinfo);
extern int evt300_sprint_radar(char *evt300_buffer, evt300_radar_typ *pradar,
                        timestamp_t write_time, evt300_radar_info_t *pinfo);
extern int evt300_sscan_radar(char *strbuf, evt300_radar_typ *pradar, 
			timestamp_t *pwrite_time, evt300_radar_info_t *pinfo);
extern void evt300_mysql_save_radar(evt300_radar_typ *pradar,
	evt300_radar_info_t *pinfo);
extern evt300_msg_info_t evt300_msg_info[256];

#endif
