/** Contains numbers sizes of VAA message types fields.
 *  NO OTHER CODE SHOULD USE THESE CONSTANTS EXCEPT BY ACCESSING THESE
 *  STRUCTURES.
 */
#include "vaa_msg_struct.h"

signed char track_measurement_field_size[] = {
	     -13, ///peak_detection
	     -13, ///low_speed
	     1, ///track_detected
	     4, ///peak_sensor_id
	     1, ///track_polarity
	     8, ///message_counter
	     24, ///time_counter
};

vaa_msg_descr_t track_measurement_descr = {
	FRONT_TRACK1_MEASUREMENT_ID,
	sizeof(track_measurement_field_size)/sizeof(unsigned char),
	&track_measurement_field_size[0],
};

signed char sensor_status_field_size[] = {
	     4, ///bar_status
	     1, ///sensor_fault
	     10, ///fault_sensor
	     1, ///degraded_mode
	     1, ///can_reception_fault
	     4, ///secondary_fault_cond
	     4, ///other_status
	     8, ///message_counter
	     10, ///vehicle_speed
};

vaa_msg_descr_t sensor_status_descr = {
	SENSOR_STATUS_ID,
	sizeof(sensor_status_field_size)/sizeof(unsigned char),
	&sensor_status_field_size[0],
};

signed char raw_data_field_size[] = {
	     -16, ///longitudinal
	     -16, ///lateral
	     -16, ///vertical
	     8, ///message_counter
	     4, ///speed
};

vaa_msg_descr_t raw_data_descr = {
	RAW_DATA_ID_BASE,
	sizeof(raw_data_field_size)/sizeof(unsigned char),
	&raw_data_field_size[0],
};

signed char cc_sensor_command_field_size[] = {
	     4, ///command,
	     4, ///command_validation,
	     2, ///command_addresses,
	     12, ///speed,
	     8, ///message_counter);
};

vaa_msg_descr_t cc_sensor_command_descr = {
	CC_SENSOR_COMMAND_ID,
	sizeof(cc_sensor_command_field_size)/sizeof(unsigned char),
	&cc_sensor_command_field_size[0],
};

signed char hmi_primary_field_size[] = {
    3, // hmi.counter
    5, // hmi.state
    
    1, 1, 1, 1, 1, 1, 1, 1, // dvi
    3, // dvi.audio
    3, // hmi.driver_request
    3, // hmi.command2cc
    1, // hmi.primary_cc

    /* faults */
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1, // hmi_fault_hmi2
    1,
    1,
    1, // hmi_fault_from_peerhmi
    1,
    1,
    1, // hmi_fault_ccstate_differ
    1,
    1,
    1,
    2, // hmi_fault_severity
    3, // TBD
    
    5, // hmi.next_state
    1, // hmi.reboot_mode

};

vaa_msg_descr_t hmi_primary_descr = {
    HMI1_PRIMARY_ID,
    sizeof(hmi_primary_field_size)/sizeof(signed char),
    hmi_primary_field_size
};

signed char cc_hmi_primary_field_size[] = {
    4, 4, 4, 4,
    1, 1,
    4,
    1, 4, 11, 3,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

vaa_msg_descr_t cc_hmi_primary_descr = {
    CC1_HMI_PRIMARY_ID,
    sizeof(cc_hmi_primary_field_size)/sizeof(signed char),
    cc_hmi_primary_field_size
};
