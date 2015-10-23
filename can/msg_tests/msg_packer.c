#include <sys_os.h>
//#include <stdlib.h>
#include <stdarg.h>
#include "msg_packer.h"

/* packs integers into 8 bytes with arbitrary placement.
   The 1st argument is how many integers to pack.
   Then for each integer, it must be preceded by a 32-bit integer
   signifying how many bits that integer should occupy.

   For example, bit_packer(2, 5, 29, 15, 64) will produce
   an 8-byte integer such that the number 29 will occupy the 1st 5 bytes,
   and the number 64 will occupy the next 15 bytes.
*/
uint64_t bit_pack(size_t numargs, ...)
{
    uint64_t value = 0;
    va_list arglist;
    size_t i = 0;
    size_t bit_position = 0;
    
    va_start(arglist, numargs);
    for (; i < numargs; i++)
    {
	size_t bitsize = va_arg(arglist, size_t);

	uint64_t item = va_arg(arglist, uint32_t);
	
	value |= (item << bit_position);
	
	bit_position += bitsize;
    }
    
    va_end(arglist);
    
    return value;
}

int pack_sensor_config(sensor_config* data, uint64_t* out)
{
    *out = bit_pack(3, 
		    8, data->serial,
		    8, data->type,
		    8, data->configuration);
    return 0;
}

int pack_sensor_state(sensor_state* data, uint64_t* out)
{
    *out = bit_pack(6,
		    5, data->operation_code,
		    8, data->fault_message,
		    10, data->sensor_health[0],
		    10, data->sensor_health[1],
		    10, data->sensor_health[2],
		    5, data->output_type);
    return 0;
}

int pack_message_status(message_status* data, uint64_t* out)
{
    *out = bit_pack(1, 8, data->heartbeat);
    return 0;
}

int pack_position_data_normal(position_data_normal* data,
			      uint64_t** out, size_t* count)
{
    *out = (uint64_t*)malloc(sizeof(uint64_t*) * 5);
    *count = 5;
    (*out)[0] = bit_pack(3,
			 16, data->lateral_pos[0],
			 16, data->lateral_pos[1],
			 16, data->lateral_pos[2]);
    (*out)[1] = bit_pack(3,
			 16, data->lateral_pos[3],
			 16, data->lateral_pos[4],
			 16, data->lateral_pos[5]);
    (*out)[2] = bit_pack(3,
			 16, data->time_stamp[1],
			 16, data->time_stamp[2],
			 16, data->time_stamp[3]);
    (*out)[3] = bit_pack(3,
			 16, data->time_stamp[3],
			 16, data->time_stamp[4],
			 16, data->time_stamp[5]);
    (*out)[4] = bit_pack(8,
			 4, data->track_number[0],
			 4, data->track_number[1],
			 4, data->track_number[2],
			 16, data->estimated_speed,
			 16, data->real_speed,
			 1, data->missing_magnet_flag[0],
			 1, data->missing_magnet_flag[1],
			 1, data->missing_magnet_flag[2]);
    return 0;
}

int pack_position_data_raw(position_data_raw* data, 
			   uint64_t** out, size_t* count)
{
    size_t i;

    *out = (uint64_t*)malloc(sizeof(uint64_t*) * 11);
    *count = 11;
    for (i = 0; i < 10; ++i)
    {
	(*out)[i] = bit_pack(3,
			     20, data->magnetic_strengths[i * 3],
			     20, data->magnetic_strengths[i * 3 + 1],
			     20, data->magnetic_strengths[i * 3 + 2]);
    }
    
    (*out)[10] = bit_pack(1,
			  20, data->vehicle_speed);
    
    return 0;
}

int pack_position_data_calibration(position_data_calibration* data,
                                   uint64_t** out, size_t* count)
{
    size_t i;

    *out = (uint64_t*)malloc(sizeof(uint64_t*) * 10);
    *count = 10;
    for (i = 0; i < 10; ++i)
    {
	(*out)[i] = bit_pack(3,
			     20, data->magnetic_strengths[i * 3],
			     20, data->magnetic_strengths[i * 3 + 1],
			     20, data->magnetic_strengths[i * 3 + 2]);
    }

    return 0;
}

int pack_control_computer_status(control_computer_status* data,
                                 uint64_t* out)
{
    *out = bit_pack(2, 
		    8, data->id,
		    8, data->status);
    return 0;
}

int pack_system_command(system_command* data, uint64_t* out)
{
    *out = bit_pack(2,
		    8, data->command,
		    8, data->target);
    return 0;
}

int pack_data_inputs(data_inputs* data, uint64_t* out)
{
    *out = bit_pack(2, 
		    32, data->vehicle_speed,
		    32, data->magnet_information);
    return 0;
}

int pack_HMI_state(HMI_state* data, uint64_t* out)
{
    *out = bit_pack(4,
		    16, data->id,
		    5, data->operation_state,
		    4, data->heartbeat,
		    8, data->fault_message);
    return 0;
}

int pack_HMI_device_state(HMI_device_state* data, uint64_t* out)
{
    *out = bit_pack(2,
		    8, data->state,
		    12, data->devices);
    return 0;
}

int pack_HMI_optional_data(HMI_optional_data* data, uint64_t* out)
{
    *out = bit_pack(1,
		    24, data->recommended_action);
    return 0;
}

int pack_CC_state(CC_state* data, uint64_t* out)
{
    *out = bit_pack(4,
		    8, data->id,
		    4, data->state,
		    4, data->heartbeat,
		    24, data->fault_message);
    return 0;
}

int pack_CC_operation_state(CC_operation_state* data, uint64_t* out)
{
    *out = bit_pack(4,
		    4, data->controller_state,
		    4, data->transition_state,
		    4, data->coordination_state,
		    4, data->reserved_state);
    return 0;
}

int pack_CC_optional_data(CC_optional_data* data, uint64_t* out)
{
    *out = bit_pack(1,
		    16, data->steering_command);
    return 0;
}

