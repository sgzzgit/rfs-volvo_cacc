#ifndef MSG_PACK_H
#define MSG_PACK_H
#include "messages.h"
#include "vaa_msg_struct.h"

can_basic_msg_t pack_track_measurement(track_measurement_t* msg);
can_basic_msg_t pack_sensor_status(sensor_status_t* msg);
can_basic_msg_t pack_raw_data(raw_data_t* msg);
can_basic_msg_t pack_cc_sensor_comand(cc_sensor_command_t* msg);
can_basic_msg_t pack_hmi_primary(hmi_primary_t* msg, int id);
can_basic_msg_t pack_cc_hmi_primary(cc_hmi_primary_t* msg, int id);

#endif
