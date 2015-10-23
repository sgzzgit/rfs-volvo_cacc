#include <stdarg.h>
#include "msg_pack.h"
#include "vaa_msg_struct.h"

void bit_pack(unsigned numargs, signed char* pfield_size, uint32_t* out, ...)
{
    va_list arglist;
    unsigned i = 0;
    unsigned bit_position = 0;
    
    out[0] = out[1] = 0;
    
    va_start(arglist, out);
    for (; i < numargs; i++)
    {
	signed char bitsize = pfield_size[i];
	uint32_t item = va_arg(arglist, uint32_t);

	if (bitsize < 0)
	     bitsize = -bitsize;
 	item &= (1 << bitsize) - 1;

	if (bit_position < 32)
	{
	    out[0] |= (item << bit_position);
	    if (bit_position + bitsize > 32 && bitsize <= 32)
		out[1] = (item >> (32 - bit_position));
	}
	else
	    out[1] |= (item << (bit_position - 32));

	bit_position += bitsize;
    }
    va_end(arglist);
}

can_basic_msg_t pack_track_measurement(track_measurement_t* msg)
{
    can_basic_msg_t out;
    signed char* fsarray = track_measurement_descr.pfield_size;
    bit_pack(track_measurement_descr.num_fields, fsarray, out.d,
	     msg->peak_detection,
	     msg->low_speed,
	     msg->track_detected,
	     msg->peak_sensor_id,
	     msg->track_polarity,
	     msg->message_counter,
	     msg->time_counter
	);
    if (msg->front_bar && msg->track_number)
	out.id = FRONT_TRACK2_MEASUREMENT_ID;
    else if (msg->front_bar)
	out.id = FRONT_TRACK1_MEASUREMENT_ID;
    else if (msg->track_number)
	out.id = REAR_TRACK2_MEASUREMENT_ID;
    else
	out.id = REAR_TRACK1_MEASUREMENT_ID;

    return out;
}

can_basic_msg_t pack_sensor_status(sensor_status_t* msg)
{
    can_basic_msg_t out;
    signed char* fsarray = sensor_status_descr.pfield_size;
    bit_pack(sensor_status_descr.num_fields, fsarray, out.d,
	     fsarray[0], msg->bar_status,
	     msg->sensor_fault,
	     msg->fault_sensor,
	     msg->degraded_mode, // ?
	     msg->can_reception_fault,
	     msg->secondary_fault_cond,
	     msg->other_status,
	     msg->message_counter,
	     msg->vehicle_speed);
    out.id = sensor_status_descr.id;
    return out;
}

can_basic_msg_t pack_raw_data(raw_data_t* msg)
{
    can_basic_msg_t out;
    signed char* fsarray = raw_data_descr.pfield_size;
    bit_pack(raw_data_descr.num_fields, fsarray, out.d,
	     msg->longitudinal,
	     msg->lateral,
	     msg->vertical,
	     msg->message_counter,
	     msg->speed);
    out.id = RAW_DATA_ID_BASE;
    return out;
}

can_basic_msg_t pack_cc_sensor_comand(cc_sensor_command_t* msg)
{
    can_basic_msg_t out;
    signed char* fsarray = cc_sensor_command_descr.pfield_size;
    bit_pack(cc_sensor_command_descr.num_fields, fsarray, out.d,
	     msg->command,
	     msg->command_validation,
	     msg->command_addresses,
	     msg->speed,
	     msg->message_counter);
    out.id = CC_SENSOR_COMMAND_ID;
    return out;
}

can_basic_msg_t pack_hmi_primary(hmi_primary_t* msg, int id)
{
    can_basic_msg_t out;
    signed char* fsarray = hmi_primary_descr.pfield_size;
    bit_pack(hmi_primary_descr.num_fields, fsarray, out.d,
	     msg->counter,
	     msg->state,
	     
	     msg->amber_led,
	     msg->green_led,
	     msg->blue_led,
	     msg->red_led,
	     msg->sound,
	     msg->on_switch,
	     msg->off_switch,
	     msg->kill_switch,
	     msg->audio,
	     msg->driver_request,
	     msg->command2cc,
	     msg->primary_cc,
	     
	     msg->fault_CAN,
	     msg->fault_RS232,
	     msg->fault_dvi,
	     msg->fault_yawr1,
	     msg->fault_yawr2,
	     msg->fault_gps,
	     msg->fault_fbar,
	     msg->fault_rbar,
	     msg->fault_actuator,
	     msg->fault_hmi2,
	     msg->fault_cc1,
	     msg->fault_cc2,
	     msg->fault_from_peerhmi,
	     msg->fault_from_cc1,
	     msg->fault_from_cc2,
	     msg->fault_ccstate_differ,
	     msg->fault_cccommand_differ,
	     msg->fault_hmistate_differ,
	     msg->fault_hmicommand_differ,
	     msg->fault_severity,
	     msg->fault_TBD,
	     
	     msg->next_state,
	     msg->peerhmi_reboot
	);
    out.id = id;
    return out;
}

can_basic_msg_t pack_cc_hmi_primary(cc_hmi_primary_t* msg, int id)
{
    can_basic_msg_t out;
    signed char *fsarray = cc_hmi_primary_descr.pfield_size;
    bit_pack(cc_hmi_primary_descr.num_fields, fsarray, out.d,
	     msg->transition_state,
	     msg->mission_state,
	     msg->controller_state,
	     msg->controller_mode,
	     msg->current_role,
	     msg->requested_role,
	     msg->notify_driver,
	     msg->actuator_vibration,
	     msg->actuator_mode,
	     msg->steer_d,
	     msg->counter,
	     msg->fault_severity,
	     msg->fault_fbar,
	     msg->fault_rbar,
	     msg->fault_gps,
	     msg->fault_yawr1,
	     msg->fault_speed,
	     msg->fault_yawr2,
	     msg->fault_actuator,
	     msg->fault_hmi1,
	     msg->fault_hmi2,
	     msg->fault_selfcc,
	     msg->fault_peercc,
	     msg->fault_CAN1,
	     msg->fault_CAN1,
	     msg->fault_CAN3,
	     msg->fault_CAN4,
	     msg->fault_RS232_1,
	     msg->fault_RS232_2,
	     msg->fault_RS232_3,
	     msg->fault_ethernet,
	     msg->fault_hmi_differ,
	     msg->fault_cc_differ
	);

    out.id = id;
    return out;
}
