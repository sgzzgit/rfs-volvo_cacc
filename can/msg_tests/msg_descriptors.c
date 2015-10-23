/**\file
 *	Definitions needed to generate and then read in
 *	an input file containing columns of data that
 *	will be read by msg_send.
 */
#include <sys_os.h>
#include "datastructs.h"
#include "msg_descriptors.h"

msg_descriptor_t msg_descriptors[] =
{
	{100, 3, "sensor_config"}, 
	{200, 6, " sensor_state"},
	{300, 1, " message_status"},
	{400, 26, " position_data_normal"},
	{500, 31, " position_data_raw"},
	{600, 30, " position_data_calibration"},
	{700, 2, " control_computer_status"},
	{800, 2, " system_command"},
	{900, 2, " data_inputs"},
	{1000, 4, " HMI_state"},
	{1100, 2, " HMI_device_state"},
	{1200, 1, " HMI_optional_data"},
	{1300, 4, " CC_state"},
	{1400, 4, " CC_operation_state"},
	{1500, 1, " CC_optional_data"},
};

int num_message_types = sizeof(msg_descriptors)/sizeof(msg_descriptor_t);
