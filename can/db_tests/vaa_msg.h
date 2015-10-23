#ifndef _VAA_MSG_H 
#define _VAA_MSG_H

typedef void (*vaa_snd_fptr) (db_clt_typ *pclt, int can_fd, int extended,
	 void *psnd_msg_info);
typedef int (*vaa_rcv_fptr) (db_clt_typ *pclt, int can_fd, void *pmsg_funcs_array);
typedef void (*vaa_print_fptr) (FILE *fp, unsigned long vaa_msg_id, void *dbv, int dbv_size);

/** This structure contains information about the message needed to
 *  process it, for generating test data and reading it from a file
 *  or from the data server as well as for sending and receiving it
 *  on the CAN bus or printing it out.
 */
typedef struct {
	unsigned char id;	/// add to DB_VAA_BASE to get DB VAR number
				/// send as ID in CAN message
	vaa_rcv_fptr rcv;	/// function to receive from CAN bus
	vaa_snd_fptr snd;	/// function to send on CAN bus
	vaa_print_fptr print;	/// function to print the message structure
	unsigned char size;	/// size of data payload in bytes
	unsigned char fields;	/// number of fields in structure
	char *name;		/// name to print (for debugging)
} IS_PACKED  vaa_msg_descr_t;;

extern vaa_msg_descr_t vaa_msg[];
extern int num_vaa_msg_ids;

typedef struct {
	uint8_t serial;
	uint8_t type;
	uint8_t configuration;
} IS_PACKED sensor_config_t;

typedef struct {
	uint8_t operation_code;
	uint8_t fault_message;
	uint16_t sensor_health[3];
	uint8_t output_type;
} IS_PACKED sensor_state_t;

typedef struct {
	uint8_t heartbeat;
} IS_PACKED message_status_t;

typedef struct {
	uint16_t lateral_pos[6];
	uint16_t time_stamp[6];
	uint8_t polarity[6];
	uint8_t track_number[3];
	uint16_t estimated_speed;
	uint16_t real_speed;
	uint8_t missing_magnet_flag[3];
} IS_PACKED position_normal_t;

typedef struct {
	uint32_t magnetic_strengths[30];
	uint32_t vehicle_speed;
} IS_PACKED position_raw_t;

typedef struct {
	uint32_t magnetic_strengths[30];
} IS_PACKED position_calibration_t;

typedef struct {
	uint8_t id;
	uint8_t status;
} IS_PACKED cc_status_t;	// control computer status

typedef struct {
	uint8_t command;
	uint8_t target;
} IS_PACKED system_command_t;

typedef struct {
	uint32_t vehicle_speed;
	uint32_t magnet_information;
} IS_PACKED data_inputs_t;

typedef struct {
	uint16_t id;
	uint8_t operation_state;
	uint8_t heartbeat;
	uint8_t fault_message;
} IS_PACKED hmi_state_t;

typedef struct {
	uint8_t state;
	uint16_t devices;
} IS_PACKED hmi_device_state_t;

typedef struct {
	uint32_t recommended_action;
} IS_PACKED hmi_optional_data_t;

typedef struct {
	uint8_t id;
	uint8_t state;
	uint8_t heartbeat;
	uint32_t fault_message;
} IS_PACKED cc_state_t;

typedef struct {
	uint8_t controller_state;
	uint8_t transition_state;
	uint8_t coordination_state;
	uint8_t reserved_state;
} IS_PACKED cc_operation_state_t;

typedef struct {
	uint16_t steering_command;
} IS_PACKED cc_optional_data_t;

#endif
