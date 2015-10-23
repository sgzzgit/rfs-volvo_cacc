/**\file
 *  This array can be accessed by other code to print error messages
 *  and to skip over messages of fixed length that are not of interest.
 *
 *  Only messages that have been found to actually occur in our radar
 *  set-up should be skipped over, since otherwise you run a greater risk of
 *  skipping the actual start of a message.
 *
 *  First and second field can be used for error messages.
 *  Third field is number of data bytes to messages (does not count checksum),
 *  If the field is variable, and we expect to parse it, the
 *  value is the maximum possible value. If it is variable
 *  and we will not figure out how long it really is, the number is -1.
 */
#include <sys_os.h>
#include "local.h"
#include "timestamp.h"
#include "evt300.h"

evt300_msg_info_t evt300_msg_info[256] = {
	{"UNUSED", 0, 0},
	{"DDU_ID_REQUEST_MESSAGE", 1, 0},
	{"DDU_DISPLAY_UPDATE_MESSAGE", 2, 2},
	{"DDU_TONE_DATA_MESSAGE", 3, 11},
	{"DDU_RESET_MESSAGE", 4, 1},
	{"DDU_DRIVER_ID_DATA_REQUEST_MESSAGE", 5, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//10
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"DDU_DATA_REQUEST_MESSAGE", 16, 4},	/// docs say 1
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//20
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//30
	{"UNUSED", 0, 0},
	{"FE_ID_REQUEST_MESSAGE", 32, 0},
	{"FE_TRANSMITTER_CONTROL_MESSAGE", 33, 1},
	{"FE_CRITICAL_TARGET_MESSAGE", 34, 1},	
	{"FE_CORRECTION_VALUES_MESSAGE", 35, -1}, ///variable, max 66
	{"FE_VEHICLE_DATA_MESSAGE", 36, 5},
	{"FE_SOFTWARE_UPDATE_MESSAGE", 37, -1},	///variable, max 132
	{"FE_RESET_MESSAGE", 38, 1},
	{"FE_BORESIGH_DATA_MESSAGE", 39, 39},
	{"UNUSED", 0, 0},	//40
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//50
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"FE_CALIBRATION_MODE_REQUEST_MESSAGE", 56, 2},
	{"FE_FREQUENCY_CALIBRATION_MESSAGE", 57, 2},
	{"FE_ASIMUTH_CALIBRATION_MESSAGE", 58, -1}, ///variable, max 129
	{"FE_SERIAL_NUMBER_ASSIGNMENT_MESSAGE", 59, 3},
	{"FE_TEMPERATURE_SENSOR_DATA_REQUEST_MESSAGE", 60, 0},
	{"FE_DATA_REQUEST_MESSAGE", 61, 6},
	{"UNUSED", 0, 0},	
	{"FE_EVT300J_CALIBRATION_DATA_MESSAGE", 63, -1}, ///variable, max 34
	{"DDU_POWER_ON_COMPLETE_MESSAGE", 64, 0},
	{"DDU_ID_REPORT_MESSAGE", 65, 6},
	{"DDU_STATUS_MESSAGE", 66, 2},
	{"DDU_DRIVER_ID_DATA_MESSAGE", 67, -1},	/// variable, max 11
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//70
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"FE_POWER_ON_MESSAGE", 80, 1},
	{"FE_ID_REPORT_MESAGE", 81, 34},
	{"FE_TARGET_REPORT_MESSAGE", 82, 58},	///variable, parsed
	{"FE_STATUS_AND_BIST_REPORT_MESSAGE", 83, 2},
	{"FE_CORRECTION_VALUE_REQUEST_MESSAGE", 84, 0},
	{"FE_CORRECTION_VALUE_UPDATE_MESSAGE", 85, -1},	/// variable, max 66
	{"UNUSED", 0, 0},
	{"FE_SOFTWARE_UPDATE_ACKNOWLEDGE_MESAGE", 87, 4},
	{"FE_BORESIGHT_UPDATE_MESSAGE", 88, 39},
	{"FE_TARGET_REPORT_2_MESSAGE", 89, 58},	///variable, parsed
	{"UNUSED", 0, 0},	//90
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"CPU_BIST_REPORT_REQUEST_MESSAGE", 96, 0},
	{"CPU_MONITOR_REQUEST_MESSAGE", 97, 1},
	{"CPU_SERIALNUMBER_ASSIGNMENT_MESSAGE", 98, 3},
	{"CPU_SOFTWARE_UPDATE_MESSAGE", 99, -1},	///variable, max 132
	{"CPU_DATA_REQUEST_MESSAGE", 100, -1},	///variable, max 132
	{"CPU_CLEAR_FAULT_LOG_MESSAGE", 101, -1}, ///variable, max 2
	{"CPU_TEST_CONTROL_MESSAGE", 102, -1},	///variable
	{"CPU_RESET_MESSAGE", 103, 1},
	{"CPU_BLOCK_TRANSFER_ACKNOWLEDGE_MESSAGE", 104, 4},
	{"CPU_ID_REQUEST_MESSAGE", 105, 0},
	{"CPU_CONFIGURATION_DATA_REQUREST_MESAGE", 106, -1},	///variable
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//110
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//120
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"ACP_BIST_REPORT_MESSAGE", 128, -1},	///variable
	{"ACP_CONFIG_DATA_MESSAGE", 129, -1},	///variable
	{"ACP_TEST_MESSAGE", 130, 18},	
	{"ACP_ID_REPORT_MESSAGE", 131, -1},	///variable
	{"UNUSED", 0, 0},
	{"ACP_TEST_DATA_MESSAGE", 133, 0},	///variable, max 128	
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//140
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//150
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"ACP_BIST_REPORT_REQUEST_MESSAGE", 160, 0},
	{"ACP_STATUS_REQUEST_MESSAGE", 161, 0},	
	{"ACP_CONFIG_DATA_REQUEST_MESSAGE", 162, -1},	///variable
	{"ACP_ID_REQUEST_MESSAGE", 163, 0},	
	{"ACP_ANTENNA_TEST_CONTROL_MESSAGE", 164, 1},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//170
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//180
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//190
	{"UNUSED", 0, 0},
	{"DUU_DATA_TRANSFER_MESSAGE", 192, -1},	///variable
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//200
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//210
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"CPU_SOFTWARE_UPDATE_ACKNOWLEDGE_MESSAGE", 216, 4},
	{"CPU_DATA_TRANSFER_MESSAGE", 217, -1},	///variable
	{"CPU_BIST_STATUS_MESSAGE", 218, 12},
	{"CPU_ID_REPORT_MESAGE", 219, 51},
	{"CPU_BLOCK_TRANSFER_MESSAGE", 220, -1},	///variable
	{"CPU_CONFIGURATION_DATA_MESSAGE", 221, -1},	///variable
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"CPU_DEBUG_MESSAGE", 225, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//230
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"CPU_POWER_ON_MESSAGE", 240,1},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},	//250
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"UNUSED", 0, 0},
	{"VBUS_SHUTDOWN_MESSAGE", 255, 1},
};
