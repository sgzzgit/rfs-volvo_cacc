//#include <sys_os.h>
#include <stdlib.h>
#include <stdarg.h>
#include "msg_unpacker.h"

void bit_unpack(uint64_t msg, size_t numargs, ...)
{
    va_list arglist;
    size_t i = 0;
    size_t bit_position = 0;

    printf ("%X\n", msg);

    va_start(arglist, numargs);
    for (; i < numargs; i++)
    {
	size_t bitsize = va_arg(arglist, size_t);
	uint32_t* ptr = va_arg(arglist, uint32_t*);
	uint32_t mask;
	
	mask = 0xFFFFFFFF;
	mask >>= (32 - bitsize);
	mask <<= bit_position;
	*ptr = (msg & mask) >> bit_position;

	bit_position += bitsize;
    }
    
    va_end(arglist);
}

void unpack_sensor_config(uint64_t msg, sensor_config* data) 
{
    bit_unpack(msg, 3, 
	       8, &data->serial,
	       8, &data->type,
	       8, &data->configuration);
}

void unpack_sensor_state(uint64_t msg, sensor_state* data)
{
    bit_unpack(msg, 6,
	       5, &data->operation_code,
	       8, &data->fault_message,
	       10, &data->sensor_health[0],
	       10, &data->sensor_health[1],
	       10, &data->sensor_health[2],
	       5, &data->output_type);
}

void unpack_message_status(uint64_t msg, message_status* data)
{
    bit_unpack(msg, 1,
	       8, &data->heartbeat);
}

void unpack_position_data_normal(uint64_t* msg,
				 position_data_normal* data)
{
    bit_unpack(msg[0], 3,
	       16, data->lateral_pos[0],
	       16, data->lateral_pos[1],
	       16, data->lateral_pos[2]);
    bit_unpack(msg[1], 3,
	       16, data->lateral_pos[3],
	       16, data->lateral_pos[4],
	       16, data->lateral_pos[5]);
    bit_unpack(msg[2], 3,
	       16, data->time_stamp[1],
	       16, data->time_stamp[2],
	       16, data->time_stamp[3]);
    bit_unpack(msg[3], 3,
	       16, data->time_stamp[3],
	       16, data->time_stamp[4],
	       16, data->time_stamp[5]);
    bit_unpack(msg[4], 8,
	       4, data->track_number[0],
	       4, data->track_number[1],
	       4, data->track_number[2],
	       16, data->estimated_speed,
	       16, data->real_speed,
	       1, data->missing_magnet_flag[0],
	       1, data->missing_magnet_flag[1],
	       1, data->missing_magnet_flag[2]);
    
}

void unpack_position_data_raw(uint64_t* msg, 
			      position_data_raw* data)
{
    size_t i;
    for (i = 0; i < 10; ++i)
    {
	bit_unpack(msg[i], 3,
		   20, &data->magnetic_strengths[i * 3],
		   20, &data->magnetic_strengths[i * 3 + 1],
		   20, &data->magnetic_strengths[i * 3 + 2]);
    }

    bit_unpack(msg[10], 1,
	       20, &data->vehicle_speed);
}

void unpack_position_data_calibration(uint64_t* msg, 
				      position_data_calibration* data)
{
    size_t i;
    for (i = 0; i < 10; ++i)
    {
	bit_unpack(msg[i], 3,
		   20, &data->magnetic_strengths[i * 3],
		   20, &data->magnetic_strengths[i * 3 + 1],
		   20, &data->magnetic_strengths[i * 3 + 2]);
    }
}
void unpack_control_computer_status(uint64_t msg,
				   control_computer_status* data)
{
    bit_unpack(msg, 2,
	       8, &data->id,
	       8, &data->status);
}

void unpack_system_command(uint64_t msg, system_command* data)
{
    bit_unpack(msg, 2,
	       8, &data->command,
	       8, &data->target);
}

void unpack_data_inputs(uint64_t msg, data_inputs* data)
{
    bit_unpack(msg, 2,
	       32, &data->vehicle_speed,
	       32, &data->magnet_information);
}

void unpack_HMI_state(uint64_t msg, HMI_state* data)
{
    bit_unpack(msg, 4,
	       16, &data->id,
	       5, &data->operation_state,
	       4, &data->heartbeat,
	       8, &data->fault_message);
}

void unpack_HMI_device_state(uint64_t msg, HMI_device_state* data)
{
    bit_unpack(msg, 2,
	       8, &data->state,
	       12, &data->devices);
}

void unpack_HMI_optional_data(uint64_t msg, HMI_optional_data* data)
{
    bit_unpack(msg, 1,
	       24, &data->recommended_action);
}

void unpack_CC_state(uint64_t msg, CC_state* data)
{
    bit_unpack(msg, 4,
	       8, &data->id,
	       4, &data->state,
	       4, &data->heartbeat,
	       24, &data->fault_message);
}

void unpack_CC_operation_state(uint64_t msg, CC_operation_state* data)
{
    bit_unpack(msg, 4,
	       4, &data->controller_state,
	       4, &data->transition_state,
	       4, &data->coordination_state,
	       4, &data->reserved_state);
}

void unpack_CC_optional_data(uint64_t msg, CC_optional_data* data)
{
    bit_unpack(msg, 1,
	       16, &data->steering_command);
}

int main()
{
    int i, j, k;
    
    bit_unpack(0xABC, 3, 4, &i, 4, &j, 4, &k);
    printf ("%d, %d, %d\n", i, j, k);

    return 0;
}
