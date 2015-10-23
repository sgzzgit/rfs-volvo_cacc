/**\file
 *
 * 	vaa_db_utils.c
 *
 *	Contains array that specifies which functions to use
 *	to send, receive and print DB data server variables.
 *
 *	Also contains generic send and receive functions.
 */

#include <sys_os.h>
#include "db_clt.h"
#include "db_utils.h"
#include "sys_rt.h"
#include "timestamp.h"
#include "can_defs.h"
#include "can_client.h"
#include "vaa_msg.h"
#include "vaa_clt_vars.h"

// Generic send for data server variable on CAN bus
void snd_g(db_clt_typ *pclt, int can_fd, int extended, void *psnd_msg_info)
{
	vaa_msg_descr_t *p = (vaa_msg_descr_t *) psnd_msg_info;
	unsigned char vaa_msg_id = p->id;
	int vaa_msg_size = p->size;

	unsigned char buf[MAX_DATA_SIZE]; // DB server MAX_DATA_SIZE is now 128
	unsigned char data[8]; 	//CAN messages have 8 bytes data payload

	memset(buf, 0, sizeof(buf));
	memset(data, 0, sizeof(data));

	if (vaa_msg_size > MAX_DATA_SIZE) { 
		vaa_msg_size = MAX_DATA_SIZE;
		fprintf(stderr, "Data server variable size too big!\n");
	}

	// get variable from data server
	db_clt_read(pclt, DB_VAA_BASE + vaa_msg_id, vaa_msg_size,
		       		(void *) &buf[0]);

	if (vaa_msg_size <= 8) 
		can_write(can_fd, vaa_msg_id, extended, buf, vaa_msg_size);
	else {
		int i;		// count CAN packets in VAA message
		int j = 0;	// index into buf

		// first byte of multi-packet message is sequence number
		// sequence numbers start at 1, will be less than 
		// (MAX_DATA_SIZE-1)/7 + 1
		for (i = 1; i <= (vaa_msg_size-1)/7 + 1; i++) {
			int k;	// index into data bytes of CAN message
			data[0] = i;
			for (k = 1; k < 7; k++) {
				data[k] = buf[j]; 
				j++;
			}
			can_write(can_fd, vaa_msg_id, extended, data, 8);
		}
	}
}

// Generic rcv from CAN bus, write to data server
// Returns 1 on success, 0 on error.

int rcv_g(db_clt_typ *pclt, int can_fd, void *pmsg_funcs_array)
{
	vaa_msg_descr_t *parray = (vaa_msg_descr_t *)pmsg_funcs_array;
	unsigned char vaa_msg_id; 
	int vaa_msg_size;
	int size;	// for return value from can_read

	unsigned char buf[MAX_DATA_SIZE]; // DB server MAX_DATA_SIZE is now 128
	unsigned char data[8];

	memset(buf, 0, sizeof(buf));

	size = can_read(can_fd, &vaa_msg_id, (char *)NULL, data, 8);

	// Add consistency check on vaa_msg_id
	vaa_msg_size = parray[vaa_msg_id].size;

	if (vaa_msg_size > MAX_DATA_SIZE) { 
		vaa_msg_size = MAX_DATA_SIZE;
		fprintf(stderr, "Data server variable size too big!\n");
		return 0;
	}

	if (vaa_msg_size <= 8) { 
		db_clt_write(pclt, DB_VAA_BASE + vaa_msg_id, vaa_msg_size,
			 data);
		return 1;
	} else {
		int i;	// count of received packets
		int j = 0;	// index of buf to place data byte
		int k;	// index into CAN message data array
		if (data[0] != 1) {
			fprintf(stderr, "Not first packet\n");
			return 0;
		}
		for (k = 1; k < 8; k++) {
			buf[j] = data[k];
			j++;
		}
		for (i = 2; i <= (vaa_msg_size-1)/7 + 1; i++) {
			unsigned long id;
			size = can_read(can_fd, &id, (char *)NULL, data, 8);
			if (id != vaa_msg_id) {
				fprintf(stderr, "Got id %lu, looking for %lu\n",
					id, vaa_msg_id);
				return 0;
			}
			if (data[0] != i) {
				fprintf(stderr, "Got seq %d, looking for %d\n",
					data[0], i);
				return 0;
			} 
			for (k = 1; k < 8; k++) {
				buf[j] = data[k];
				j++;
			}
		}
		db_clt_write(pclt, DB_VAA_BASE + vaa_msg_id, vaa_msg_size, buf);
		return 1;
	}
}

/** Echoes raw bytes in variable to file as 2 digit hex numbers.
 */
void print_g(FILE *fp, unsigned char vaa_msg_id, void *dbv, int dbv_size)
{
	int i;
	char *buf = (char *) dbv;
	for (i = 0; i < dbv_size; i++) 
		fprintf(fp, "%02x ", buf[i]); 
	fprintf(fp, "\n");
}

char * vaa_msg_name[] = {
	"null_id",
	"sensor_config", 
	"sensor_state", 
	"message_status",
	"position_normal", 
	"position_raw", 
	"position_calibration", 
	"com_status", 
	"system_command", 
	"data_inputs", 
	"hmi_state", 
	"hmi_device_state", 
	"hmi_optional_data", 
	"cc_state", 
	"cc_operation_state", 
	"cc_optional_data", 
};

/** First field of vaa_msg_descr_t structure is the same as the index
 *  into the array.
 */
vaa_msg_descr_t vaa_msg[] = {
	{0, NULL, NULL, NULL, 0, 0, "null_msg"},
	{DB_SENSOR_CONFIG_ID, rcv_g, snd_g, print_g, 
		sizeof(sensor_config_t), 3, "sensor_config"}, 
	{DB_SENSOR_STATE_ID, rcv_g, snd_g, print_g, 
		sizeof(sensor_state_t), 6, "sensor_state"}, 
	{DB_MESSAGE_STATUS_ID, rcv_g, snd_g, print_g,
		 sizeof(message_status_t), 1, "message_status"},
	{DB_POSITION_NORMAL_ID, rcv_g, snd_g, print_g, 
		sizeof(position_normal_t), 26, "position_normal"}, 
	{DB_POSITION_RAW_ID, rcv_g, snd_g, print_g, 
		sizeof(position_raw_t), 31, "position_raw"}, 
	{DB_POSITION_CALIBRATION_ID, rcv_g, snd_g, print_g, 
		sizeof(position_calibration_t), 30, "position_calibration"}, 
	{DB_CC_STATUS_ID, rcv_g, snd_g, print_g, 
		sizeof(cc_status_t), 2, "CC_status"}, 
	{DB_SYSTEM_COMMAND_ID, rcv_g, snd_g, print_g,
		sizeof(system_command_t), 2, "system_command"}, 
	{DB_DATA_INPUTS_ID, rcv_g, snd_g, print_g, 
		sizeof(data_inputs_t), 2, "data_inputs"}, 
	{DB_HMI_STATE_ID, rcv_g, snd_g, print_g,
		sizeof(hmi_state_t), 4, "HMI_state"}, 
	{DB_HMI_DEVICE_STATE_ID, rcv_g, snd_g, print_g, 
		sizeof(hmi_device_state_t), 2, "HMI_device_state"}, 
	{DB_HMI_OPTIONAL_DATA_ID, rcv_g, snd_g, print_g, 
		sizeof(hmi_optional_data_t), 1, "HMI_optional_data"}, 
	{DB_CC_STATE_ID, rcv_g, snd_g, print_g, 
		sizeof(cc_state_t), 4, "CC_state"}, 
	{DB_CC_OPERATION_STATE_ID, rcv_g, snd_g, print_g, 
		sizeof(cc_operation_state_t), 4, "CC_operation_state"}, 
	{DB_CC_OPTIONAL_DATA_ID, rcv_g, snd_g, print_g, 
		sizeof(cc_optional_data_t), 1, "CC_optional_data"}, 
};

int num_vaa_msg_ids = sizeof(vaa_msg)/sizeof(vaa_msg_descr_t);

/** Array containing index of messages that test data is generated for
 *  To change what messages we are generating, just put different
 *  indexes in the array. This description is also read by both 
 *  vaa_input_gen.c and vaa_db_writer.c. 
 */
unsigned char vaa_msg_indexes[] = {
	DB_SENSOR_CONFIG_ID, 
	DB_SENSOR_STATE_ID, 
	DB_POSITION_NORMAL_ID, 
	DB_HMI_STATE_ID
};

int vaa_num_generated_msgs = sizeof(vaa_msg_indexes)/sizeof(int);
