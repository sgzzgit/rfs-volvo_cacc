#include <stdarg.h>
#include "vaa_msg_struct.h"
#include "msg_unpack.h"

/* takes in a pointer to some data, and the number of variables
   to unpack the data into. Then for each variable, it takes
   a pair of paramaters specifying the variable's size-in-bits
   and the variable's address */
void bit_unpack(uint32_t* in, unsigned numargs, signed char* pfield_size, ...)
{
    va_list arglist;
    unsigned i = 0;
    unsigned bit_position = 0;

    va_start(arglist, pfield_size);
    for (; i < numargs; i++)
    {
	signed char bitsize = pfield_size[i];
	uint32_t* ptr = va_arg(arglist, uint32_t*);
	int neg = 0;
	unsigned mask = 0;

	if (bitsize < 0) {
	    neg = 1;
	    bitsize = -bitsize;
	}

	mask = (1 << bitsize) - 1;

	if (bit_position < 32)
	{
	    *ptr = (in[0] >> bit_position) & mask;
	    if (bit_position + bitsize > 32)
	    {
		unsigned mask2 = (1 << (bit_position + bitsize - 32)) - 1;
		*ptr |= (in[1] & mask2) << (32 - bit_position);
	    }
	}
	else if (bit_position >= 32)
	{
	    *ptr = (in[1] >> (bit_position - 32)) & mask;
	}

	if (neg && *ptr & (1 << (bitsize - 1)))
	    *ptr |= ~((1 << bitsize) - 1);

	bit_position += bitsize;
    }
    va_end(arglist);
}

void unpack_track_measurement(track_measurement_t* msg, uint32_t* in)
{
    signed char* fsarray = track_measurement_descr.pfield_size;
    bit_unpack(in, track_measurement_descr.num_fields, fsarray,
	       &msg->peak_detection,
	       &msg->low_speed,
	       &msg->track_detected,
	       &msg->peak_sensor_id,
	       &msg->track_polarity,
	       &msg->message_counter,
	       &msg->time_counter);
}

void unpack_sensor_status(sensor_status_t* msg, uint32_t* in)
{
    signed char* fsarray = sensor_status_descr.pfield_size;
    bit_unpack(in, sensor_status_descr.num_fields, fsarray,
	       &msg->bar_status,
	       &msg->sensor_fault,
	       &msg->fault_sensor,
	       &msg->degraded_mode,
	       &msg->can_reception_fault,
	       &msg->secondary_fault_cond,
	       &msg->other_status,
	       &msg->message_counter,
	       &msg->vehicle_speed);
}

void unpack_raw_data(raw_data_t* msg, uint32_t* in)
{
    signed char* fsarray = raw_data_descr.pfield_size;
    bit_unpack(in, raw_data_descr.num_fields, fsarray,
	       &msg->longitudinal,
	       &msg->lateral,
	       &msg->vertical,
	       &msg->message_counter,
	       &msg->speed);
}

void unpack_cc_sensor_command(cc_sensor_command_t* msg, uint32_t* in)
{
    signed char* fsarray = cc_sensor_command_descr.pfield_size;
    bit_unpack(in, cc_sensor_command_descr.num_fields, fsarray,
	       &msg->command,
	       &msg->command_validation,
	       &msg->command_addresses,
	       &msg->speed,
	       &msg->message_counter);
}

void unpack_hmi_primary(hmi_primary_t* msg, uint32_t* in)
{
    signed char* fsarray = hmi_primary_descr.pfield_size;
    bit_unpack(in, hmi_primary_descr.num_fields, fsarray,
	       &msg->counter,
	       &msg->state,
	       
	       &msg->amber_led,
	       &msg->green_led,
	       &msg->blue_led,
	       &msg->red_led,
	       &msg->sound,
	       &msg->on_switch,
	       &msg->off_switch,
	       &msg->kill_switch,
	       &msg->audio,
	       &msg->driver_request,
	       &msg->command2cc,
	       &msg->primary_cc,
	       
	       &msg->fault_CAN,
	       &msg->fault_RS232,
	       &msg->fault_dvi,
	       &msg->fault_yawr1,
	       &msg->fault_yawr2,
	       &msg->fault_gps,
	       &msg->fault_fbar,
	       &msg->fault_rbar,
	       &msg->fault_actuator,
	       &msg->fault_hmi2,
	       &msg->fault_cc1,
	       &msg->fault_cc2,
	       &msg->fault_from_peerhmi,
	       &msg->fault_from_cc1,
	       &msg->fault_from_cc2,
	       &msg->fault_ccstate_differ,
	       &msg->fault_cccommand_differ,
	       &msg->fault_hmistate_differ,
	       &msg->fault_hmicommand_differ,
	       &msg->fault_severity,
	       &msg->fault_TBD,
	       
	       &msg->next_state,
	       &msg->peerhmi_reboot
	);
}

void unpack_cc_hmi_primary(cc_hmi_primary_t* msg, uint32_t* in)
{
    signed char* fsarray = cc_hmi_primary_descr.pfield_size;
    bit_unpack(in, cc_hmi_primary_descr.num_fields, fsarray,
	       &msg->transition_state,
	       &msg->mission_state,
	       &msg->controller_state,
	       &msg->controller_mode,
	       &msg->current_role,
	       &msg->requested_role,
	       &msg->notify_driver,
	       &msg->actuator_vibration,
	       &msg->actuator_mode,
	       &msg->steer_d,
	       &msg->counter,
	       &msg->fault_severity,
	       &msg->fault_fbar,
	       &msg->fault_rbar,
	       &msg->fault_gps,
	       &msg->fault_yawr1,
	       &msg->fault_speed,
	       &msg->fault_yawr2,
	       &msg->fault_actuator,
	       &msg->fault_hmi1,
	       &msg->fault_hmi2,
	       &msg->fault_selfcc,
	       &msg->fault_peercc,
	       &msg->fault_CAN1,
	       &msg->fault_CAN1,
	       &msg->fault_CAN3,
	       &msg->fault_CAN4,
	       &msg->fault_RS232_1,
	       &msg->fault_RS232_2,
	       &msg->fault_RS232_3,
	       &msg->fault_ethernet,
	       &msg->fault_hmi_differ,
	       &msg->fault_cc_differ
	);
}

/** Returns 1 if ID is recognized as a VAA MSG ID, 0 otherwise.
 *  Structure pointed to by pr is filled in with an unpacked version
 *  of the received message, which can then be scaled.
 */
int unpack_vaa_received_msg(can_received_msg_t *pr, can_basic_msg_t *pb)
{
    pr->id = pb->id;
    if ((pb->id >= RAW_DATA_ID_BASE) 
	&& (pb->id < RAW_DATA_ID_BASE + NUM_MAGNETIC_SENSORS))
    {
	unpack_raw_data(&pr->raw_data, pb->d);
    } 
    else 
    {
	switch(pb->id) {
	case FRONT_TRACK1_MEASUREMENT_ID:
	case FRONT_TRACK2_MEASUREMENT_ID:
	case REAR_TRACK1_MEASUREMENT_ID:
	case REAR_TRACK2_MEASUREMENT_ID:
	    unpack_track_measurement(&pr->front_track1_measurement, pb->d);
	    break;
	case SENSOR_STATUS_ID:
	    unpack_sensor_status(&pr->sensor_status, pb->d);
	    break;
	case CC_SENSOR_COMMAND_ID:
	    unpack_cc_sensor_command(&pr->cc_sensor_command, pb->d);
	    break;
	case HMI1_PRIMARY_ID:
	case HMI2_PRIMARY_ID:
	    unpack_hmi_primary(&pr->hmi_primary_msg, pb->d);
	    break;
	case CC1_HMI_PRIMARY_ID:
	case CC2_HMI_PRIMARY_ID:
	    unpack_cc_hmi_primary(&pr->cc_hmi_primary_msg, pb->d);
	    break;
	default:
	    return 0;
	    break;
	}
    }

    return 1;
}
