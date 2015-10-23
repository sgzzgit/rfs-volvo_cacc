#ifndef VAA_MSG_STRUCT_H
#define VAA_MSG_STRUCT_H
#include <stdint.h>
#include "messages.h"

/** These are just placeholders for the real IDs, which may be changed
 *  to adjust priorities.
 *
 *  ALL OTHER CODE THAT USES MESSAGE IDS SHOULD INCLUDE THIS FILE AND
 *  REFERENCE IDS BY NAME.
 */

#define FRONT_TRACK1_MEASUREMENT_ID	10
#define FRONT_TRACK2_MEASUREMENT_ID	20
#define REAR_TRACK1_MEASUREMENT_ID	30
#define REAR_TRACK2_MEASUREMENT_ID	40
#define SENSOR_STATUS_ID	50
#define CC_SENSOR_COMMAND_ID	60
#define HMI1_PRIMARY_ID         70
#define HMI2_PRIMARY_ID         80
#define CC1_HMI_PRIMARY_ID      90
#define CC2_HMI_PRIMARY_ID      100

/* macro to convert from the can IDS (above) to db IDS */
#define TO_DBID(id) id

/** Following definitions assume we will be gathering raw data from
 *  one sensor bar at a time
 */
#define RAW_DATA_ID_BASE	500
#define NUM_MAGNETIC_SENSORS 10

typedef struct {
	int id;
	unsigned char num_fields;
	signed char *pfield_size;
} vaa_msg_descr_t;

extern vaa_msg_descr_t track_measurement_descr; 
extern vaa_msg_descr_t sensor_status_descr; 
extern vaa_msg_descr_t raw_data_descr; 
extern vaa_msg_descr_t cc_sensor_command_descr;
extern vaa_msg_descr_t hmi_primary_descr;
extern vaa_msg_descr_t hmi_primary_descr2;
extern vaa_msg_descr_t cc_hmi_primary_descr;

/** This structure, holding the data actually sent on the CAN bus,
 *  can be easily copied into the structure used by the driver send and
 *  receiveon either the ARM or the QNX PC104s.
 */
typedef struct {
    uint32_t id;
    uint32_t d[2];
} can_basic_msg_t;

typedef struct {
    uint32_t id;	// for raw data, indicates sensor number as well as type
    union {
	track_measurement_t front_track1_measurement;	
	track_measurement_t front_track2_measurement;	
	track_measurement_t rear_track1_measurement;	
	track_measurement_t rear_track2_measurement;	
	sensor_status_t sensor_status;	
	cc_sensor_command_t cc_sensor_command;	
	raw_data_t raw_data;	// only one sensor received per message
	hmi_primary_t hmi_primary_msg;
	cc_hmi_primary_t cc_hmi_primary_msg;
    };
} can_received_msg_t;
#endif    
