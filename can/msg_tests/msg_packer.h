#ifndef _MSG_PACKER_H
#define _MSG_PACKER_H
#include "datastructs.h"

int pack_sensor_config(sensor_config* data, uint64_t* out);
int pack_sensor_state(sensor_state* data, uint64_t* out);
int pack_message_status(message_status* data, uint64_t* out);
int pack_position_data_normal(position_data_normal* data, 
			      uint64_t** out, size_t* count);
int pack_position_data_raw(position_data_raw* data, 
			   uint64_t** out, size_t* count);
int pack_position_data_calibration(position_data_calibration* data,
				   uint64_t** out, size_t* count);
int pack_control_computer_status(control_computer_status* data,
				 uint64_t* out);
int pack_system_command(system_command* data, uint64_t* out);
int pack_data_inputs(data_inputs* data, uint64_t* out);
int pack_HMI_state(HMI_state* data, uint64_t* out);
int pack_HMI_device_state(HMI_device_state* data, uint64_t* out);
int pack_HMI_optional_data(HMI_optional_data* data, uint64_t* out);
int pack_CC_state(CC_state* data, uint64_t* out);
int pack_CC_operation_state(CC_operation_state* data, uint64_t* out);
int pack_CC_optional_data(CC_optional_data* data, uint64_t* out);


#endif
