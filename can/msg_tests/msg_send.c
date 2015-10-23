/*

msg.send

Takes in a standard input in the form of columns of data (represented as floats),
packages these data into structs,
and calls functions on these packages

Then sends these packages periodically

*/

//#include "datastructs.h"
//#include "msg_packer.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
//#include "sys_os.h"


int count, i, bc; nbc, j;  //bc = buffer counter and nbc = new buffer counter


int main(int argc, char  **argv){

	struct timespec rqtp;
	rqtp.tv_sec = 0;
    rqtp.tv_nsec = 1000000;


	unsigned char buffer[1000];				//string holding stdin
	float new_buffer[100];		//stores unsigned floats
	unsigned char * pch;

	for(j=0; j < 100; j++){
		new_buffer[j] = NULL;
	}

	memset(buffer, '\0', 1000);
	//double * output;		//need to change because not hardware compatible
	

	/*fgets will store stdin as a string (read into buffer 1000 bytes, 8000 bits)
	assuming there are spaces between each float
	*/

	if(fgets(buffer, 1000, stdin) == NULL){
		printf("Input is null");
		fflush(0);
		exit(0);
	}
	
	
	bc = 0;
	nbc = 0;
	
	//if we have a whitespace, we have reached end of float
	pch = strtok(buffer, " ,.-");
		
	while(pch != NULL){
		new_buffer[nbc] = atof(pch);
		printf ("%f\n", new_buffer[nbc]);
		fflush(0);

		pch = strtok (NULL, " ,.-");
		nbc++;
	}

	
	/* iterate through newbuffer and store*/

	
	count = 0;
	sensor_config.serial = (uint8_t) new_buffer[count];
	sensor_config.type = (uint8_t) new_buffer[count + 1];
	sensor_config.configuration = (uint8_t) new_buffer[count + 2];

	count = count + 3;

	sensor_state.operation_code = (uint8_t) new_buffer[count];
	sensor_state.fault_message = (uint8_t) new_buffer[count + 1];
	sensor_state.sensor_health[0] = (uint16_t) new_buffer[count + 2];
	sensor_state.sensor_health[1] = (uint16_t) new_buffer[count + 3];
	sensor_state.sensor_health[2] = (uint16_t) new_buffer[count + 4];
	sensor_state.output_type = (uint8_t) new_buffer[count + 5];

	count = count + 6;

	message_status.heartbeat = (uint8_t) new_buffer[count];

	count = count++;

	position_data_normal.lateral_pos = (uint16_t) new_buffer[count];
	position_data_normal.time_stamp = (uint16_t) new_buffer[count + 1];
	position_data_normal.polarity = (uint8_t) new_buffer[count + 2];
	position_data_normal.track_number = (uint8_t) new_buffer[count + 3];
	position_data_normal.estimated_speed = (uint16_t) new_buffer[count + 4];
	position_data_normal.real_speed = (uint16_t) new_buffer[count + 5];
	position_data_normal.missing_magnet_flag[0] = (uint8_t) new_buffer[count + 6];
	position_data_normal.missing_magnet_flag[1] = (uint8_t) new_buffer[count + 7];
	position_data_normal.missing_magnet_flag[2] = (uint8_t) new_buffer[count + 8];

	count = count + 9;
	
	for(i=0; i<30; i++){
		position_data_raw.magnetic_strengths[i] = (uint32_t) new_buffer[count];
		count++;
	}

	count++;

	position_data_raw.vehicle_speed = (uint32_t) new_buffer[count];

	count++;

	for(i=0; i<30; i++){
		position_data_calibration.magnetic_strengths[i] = (uint32_t) new_buffer[count];
		count++;
	}

	count++;
	
	control_computer_status.id = (uint8_t) new_buffer[count];
	control_computer_status.status = (uint8_t) new_buffer[count + 1];

	count = count + 2;

	system_command.command = (uint8_t) new_buffer[count];
	system_command.target = (uint8_t) new_buffer[count + 1];

	count = count + 2;

	data_inputs.vehicle_speed = (uint32_t) new_buffer[count];
	data_inputs.magnet_information = (uint32_t) new_buffer[count + 1];

	count = count + 2;

	HMI_state.id = (uint16_t) new_buffer[count];
	HMI_state.operation_state = (uint8_t) new_buffer[count + 1];
	HMI_state.heartbeat = (uint8_t) new_buffer[count + 2];
	HMI_state.fault_message = (uint8_t) new_buffer[count + 3];

	count = count + 4;

	HMI_device_state.state = (uint8_t) new_buffer[count];
	HMI_device_state.devices = (uint16_t) new_buffer[count + 1];

	count = count + 2;

	HMI_optional_data.recommended_action = (uint32_t) new_buffer[count];

	count++;

	CC_state.id = (uint8_t) new_buffer[count];
	CC_state.state = (uint8_t) new_buffer[count + 1];
	CC_state.heartbeat = (uint8_t) new_buffer[count + 2];
	CC_state.fault_message = (uint32_t) new_buffer[count + 3];

	count = count + 4;

	CC_operation_state.controller_state = (uint8_t) new_buffer[count];
	CC_operation_state.transition_state = (uint8_t) new_buffer[count + 1];
	CC_operation_state.coordination_state = (uint8_t) new_buffer[count + 2];
	CC_operation_state.reserved_state = (uint8_t) new_buffer[count + 3];

	count = count + 4;

	CC_optional_data.steering_commands = (uint32_t) new_buffer[count];

	count++;

	/* now calling on functions from msg_packer.h */

	sensor_config * s_c = &sensor_config;
	pack_sensor_config(s_c, output);

	sensor_state * s_s = &sensor_state;
	pack_sensor_state(s_s, output);

	message_status * m_s = &message_status;
	pack_message_status(m_s, output);

	position_data_normal * p_d_n = &position_data_normal;
	pack_position_data_normal(p_d_n, output);

	position_data_raw * p_d_r = &position_data_raw;
	pack_position_data_raw(p_d_r, output);

	position_data_calibration * p_d_c = &position_data_calibration;
	pack_position_data_calibration(p_d_c, output);
	
	control_computer_status * c_c_s = &control_computer_status;
	pack_control_computer_status(c_c_s, output);

	system_command * s_cmd = &system_command;
	pack_system_command(s_cmd, output);

	data_inputs * d_i = &data_inputs;
	pack_data_inputs(d_i, output);

	HMI_state * hmi_s = &HMI_state;
	pack_HMI_state(hmi_s, output);

	HMI_device_state * hmi_d_s = &HMI_device_state;
	pack_HMI_device_state(hmi_d_s, output);

	HMI_optional_data * hmi_o_d = &HMO_optional_data;
	pack_HMI_optional_data(hmi_o_d, output);

	CC_state * cc_s = &CC_state;
	pack_CC_state(cc_s, output);

	CC_operation_state * cc_o_s = &CC_operation_state;
	pack_CC_operation_state(cc_o_s, output);

	CC_optional_data * cc_o_d = &CC_optional_data;
	pack_CC_optional_data(cc_o_d, output);

	
	nanosleep(&rqtp, NULL);
	




	return 0;





}


