#ifndef _MSG_UNPACKER_H
#define _MSG_UNPACKER_H
#include "datastructs.h"


void unpack_sensor_config(uint64_t msg, sensor_config* data);
void unpack_sensor_state(uint64_t msg, sensor_state* data);
void unpack_message_status(uint64_t msg, message_status* data);
void unpack_position_data_normal(uint64_t* msg, position_data_normal* data);
void unpack_position_data_raw(uint64_t* msg, position_data_raw* data);
void unpack_position_data_calibration(uint64_t* msg, 
				     position_data_calibration* data);
void unpack_control_computer_status(uint64_t msg,
				   control_computer_status* data);
void unpack_system_command(uint64_t msg, system_command* data);
void unpack_data_inputs(uint64_t msg, data_inputs* data);
void unpack_HMI_state(uint64_t msg, HMI_state* data);
void unpack_HMI_device_state(uint64_t msg, HMI_device_state* data);
void unpack_HMI_optional_data(uint64_t msg, HMI_optional_data* data);
void unpack_CC_state(uint64_t msg, CC_state* data);
void unpack_CC_operation_state(uint64_t msg, CC_operation_state* data);
void unpack_CC_optional_data(uint64_t msg, CC_optional_data* data);


#endif
