#ifndef MSG_UNPACK_H
#define MSG_UNPACK_H
#include "messages.h"

void unpack_track_measurement(track_measurement_t* msg, uint32_t* in);
void unpack_sensor_status(sensor_status_t* msg, uint32_t* in);
void unpack_raw_data(raw_data_t* msg, uint32_t* in);
void unpack_cc_sensor_comand(cc_sensor_command_t* msg, uint32_t* in);
void unpack_hmi_primary(hmi_primary_t* msg, uint32_t* in);
void unpack_cc_hmi_primary(cc_hmi_primary_t* msg, uint32_t* in);
int unpack_vaa_received_msg(can_received_msg_t *pr, can_basic_msg_t *pb);

#endif
