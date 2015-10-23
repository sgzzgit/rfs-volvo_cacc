/**\file:  wvtrnx.h 
 *
 * Copyright 2009 Regents of the University of California
 *
 * Contains structure and database variable definitions for Wavetronix radar
 */
#ifndef WVTRNX_H
#define WVTRNX_H

#define MAX_WVTRNX_MSG_LENGTH	76
#define WVTRNX_MAX_TRACKS	25

#define WVTRNX_STATUS_APPROACHING_SENSOR_MASK	1<<4
#define WVTRNX_STATUS_CORRECT_DIRECTION_MASK 	1<<3
#define WVTRNX_STATUS_READY_TO_READ_MASK	1<<2
#define WVTRNX_STATUS_NEWLY_DISCOVERED_MASK	1<<1
#define WVTRNX_STATUS_ACTIVE_MASK		1<<0


typedef struct{
	unsigned char	status;		/// indicates if the vehicle is approaching or departing from the sensor, also indicates if the vehicle is in the correct direction
	unsigned char	distance;	/// indicates the distance from the sensor
	unsigned char	speed;		/// indicates the speed of the vehicle
} wvtrnx_track_t;

typedef struct{
	timestamp_t ts;
	wvtrnx_track_t tracks[WVTRNX_MAX_TRACKS];	/// maxium of 25 tracks in a field
} wvtrnx_msg_t;

/**
 *  Definitions for variable numbers using the PATH DB data server
 */
#define DB_WVTRNX_BASE		5500

#define DB_WVTRNX_RADAR0_VAR	DB_WVTRNX_BASE
#define DB_WVTRNX_RADAR1_VAR	DB_WVTRNX_BASE+1
#define DB_WVTRNX_RADAR2_VAR	DB_WVTRNX_BASE+2
#define DB_WVTRNX_RADAR3_VAR	DB_WVTRNX_BASE+3

#define DB_WVTRNX_RADAR0_TYPE	DB_WVTRNX_RADAR0_VAR
#define DB_WVTRNX_RADAR1_TYPE	DB_WVTRNX_RADAR1_VAR
#define DB_WVTRNX_RADAR2_TYPE	DB_WVTRNX_RADAR2_VAR
#define DB_WVTRNX_RADAR3_TYPE	DB_WVTRNX_RADAR3_VAR
#endif

/** Macro to convert and scale Wavetronix data
 */
#define WVTRNX_STATUS(val, mask) ((val)&(mask))	? (1):(0)	/// Return 1 if the condition is TRUE, 0 if the condition is FALSE
#define WVTRNX_CVT_DISTANCE(fvft) (((fvft) * 5.0) * 0.3048)	/// change 5 feet increments to meters
#define WVTRNX_CVT_SPEED(mph) ((mph) * 0.44704)			/// miles per hour to meters per second
