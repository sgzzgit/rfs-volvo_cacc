#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "vaa_msg_struct.h"
#include "msg_pack.h"
#include "msg_unpack.h"

int main()
{
    int i;
    srand(time(NULL));
    
    for (i = 0; i < 100; i++)
    {
	track_measurement_t start;
	track_measurement_t end;
	cc_hmi_primary_t start2, end2;
	can_basic_msg_t packed;
	can_received_msg_t received;

	// fill in random data
	signed char* fsarray = track_measurement_descr.pfield_size;
	start.peak_detection = rand() % (1 << abs(fsarray[0]));
	start.low_speed = rand() % (1 << abs(fsarray[1]));
	start.track_detected = rand() % (1 << abs(fsarray[2]));
	start.peak_sensor_id = rand() % (1 << abs(fsarray[3]));
	start.track_polarity = rand() % (1 << abs(fsarray[4]));
	start.message_counter = rand() % (1 << abs(fsarray[5]));
	start.time_counter = rand() % (1 << abs(fsarray[6]));

	// pack and unpack
	packed = pack_track_measurement(&start);
	unpack_vaa_received_msg(&received, &packed);
	end = received.front_track1_measurement;

	// check if start and end is the same
	if (start.peak_detection != (end.peak_detection & ((1 << abs(fsarray[0])) - 1)))
	    printf("1 %d\t%d\n", start.peak_detection, end.peak_detection);
	if (start.low_speed != (end.low_speed & ((1 << abs(fsarray[1])) - 1)))
	    printf("2 %d\t%d\n", start.low_speed, end.low_speed);
	if (start.track_detected != (end.track_detected & ((1 << abs(fsarray[2])) - 1)))
	    printf("3 %d\t%d\n", start.track_detected, end.track_detected);
	if (start.peak_sensor_id != (end.peak_sensor_id & ((1 << abs(fsarray[3])) - 1)))
	    printf("4 %d\t%d\n", start.peak_sensor_id, end.peak_sensor_id);
	if (start.track_polarity != (end.track_polarity & ((1 << abs(fsarray[4])) - 1)))
	    printf("5 %d\t%d\n", start.track_polarity, end.track_polarity);
	if (start.message_counter != (end.message_counter & ((1 << abs(fsarray[5])) - 1)))
	    printf("6 %d\t%d\n", start.message_counter, end.message_counter);
	if (start.time_counter != (end.time_counter & ((1 << abs(fsarray[6])) - 1)))
	    printf("7 %d\t%d\n", start.time_counter, end.time_counter);

	// test negative and sign extension
	if (start.peak_detection & (1 << (abs(fsarray[0]) - 1)) && end.peak_detection >= 0)
	    printf("0 %d\t%d\n", start.peak_detection, end.peak_detection);

/*
	// fill in random data for 2
	fsarray = cc_hmi_primary_descr.pfield_size;
	memset(&start2, 0, sizeof(start2));
	start2.cc.transition_state = rand() % (1 << fsarray[0]);
	start2.cc.mission_state = rand() % (1 << fsarray[1]);
	start2.cc.controller_state = rand() % (1 << fsarray[2]);
	start2.cc.controller_mode = rand() % (1 << fsarray[3]);
	start2.cc.current_role = rand() % (1 << fsarray[4]);
	start2.cc.requested_role = rand() % (1 << fsarray[5]);
	start2.cc.notify_driver = rand() % (1 << fsarray[6]);
	start2.cc.actuator_vibration = rand() % (1 << fsarray[7]);
	start2.cc.actuator_mode = rand() % (1 << fsarray[8]);
	start2.cc.steer_d = rand() % (1 << fsarray[9]);
	start2.cc.counter = rand() % (1 << fsarray[10]);

	start2.cc_fault.severity = rand() % (1 << fsarray[11]);
	start2.cc_fault.fbar = rand() % (1 << fsarray[12]);
	start2.cc_fault.rbar = rand() % (1 << fsarray[13]);
	start2.cc_fault.gps = rand() % (1 << fsarray[14]);
	start2.cc_fault.yawr1 = rand() % (1 << fsarray[15]);
	start2.cc_fault.speed = rand() % (1 << fsarray[16]);
	start2.cc_fault.yawr2 = rand() % (1 << fsarray[17]);
	start2.cc_fault.actuator = rand() % (1 << fsarray[18]);
	start2.cc_fault.hmi1 = rand() % (1 << fsarray[19]);
	start2.cc_fault.hmi2 = rand() % (1 << fsarray[20]);
	start2.cc_fault.selfcc = rand() % (1 << fsarray[21]);
	start2.cc_fault.peercc = rand() % (1 << fsarray[22]);
	start2.cc_fault.CAN1 = rand() % (1 << fsarray[23]);
	start2.cc_fault.CAN2 = rand() % (1 << fsarray[24]);
	start2.cc_fault.CAN3 = rand() % (1 << fsarray[25]);
	start2.cc_fault.CAN4 = rand() % (1 << fsarray[26]);
	start2.cc_fault.RS232_1 = rand() % (1 << fsarray[27]);
	start2.cc_fault.RS232_2 = rand() % (1 << fsarray[28]);
	start2.cc_fault.RS232_3 = rand() % (1 << fsarray[29]);
	start2.cc_fault.ethernet = rand() % (1 << fsarray[30]);
	start2.cc_fault.hmi_differ = rand() % (1 << fsarray[31]);
	start2.cc_fault.cc_differ = rand() % (1 << fsarray[32]);

	packed = pack_cc_hmi_primary(&start2, CC1_HMI_PRIMARY_ID);
	memset(&received, 0, sizeof(received));
	unpack_vaa_received_msg(&received, &packed);
	end2 = received.cc_hmi_primary_msg;
	if (start2.cc.transition_state != end2.cc.transition_state)
	    printf("1 %d, %d\n", start2.cc.transition_state, end2.cc.transition_state);
	if (start2.cc.mission_state != end2.cc.mission_state)
	    printf("2 %d, %d\n", start2.cc.mission_state, end2.cc.mission_state);
	if (start2.cc.controller_state != end2.cc.controller_state)
	    printf("3 %d, %d\n", start2.cc.controller_state, end2.cc.controller_state);
	if (start2.cc.controller_mode != end2.cc.controller_mode)
	    printf("4 %d, %d\n", start2.cc.controller_mode, end2.cc.controller_mode);
	if (start2.cc.current_role != end2.cc.current_role)
	    printf("5 %d, %d\n", start2.cc.current_role, end2.cc.current_role);
	if (start2.cc.requested_role != end2.cc.requested_role)
	    printf("6 %d, %d\n", start2.cc.requested_role, end2.cc.requested_role);
	if (start2.cc.notify_driver != end2.cc.notify_driver)
	    printf("7 %d, %d\n", start2.cc.notify_driver, end2.cc.notify_driver);
	if (start2.cc.actuator_vibration != end2.cc.actuator_vibration)
	    printf("8 %d, %d\n", start2.cc.actuator_vibration, end2.cc.actuator_vibration);
	if (start2.cc.actuator_mode != end2.cc.actuator_mode)
	    printf("9 %d, %d\n", start2.cc.actuator_mode, end2.cc.actuator_mode);
	if (start2.cc.steer_d != end2.cc.steer_d)
	    printf("10 %d, %d\n", start2.cc.steer_d, end2.cc.steer_d);
	if (start2.cc.counter != end2.cc.counter)
	    printf("11 %d, %d\n", start2.cc.counter, end2.cc.counter);

	if (start2.cc_fault.severity != end2.cc_fault.severity)
	    printf("12 %d, %d\n", start2.cc_fault.severity, end2.cc_fault.severity);
	if (start2.cc_fault.fbar != end2.cc_fault.fbar)
	    printf("13 %d, %d\n", start2.cc_fault.fbar, end2.cc_fault.fbar);	
	if (start2.cc_fault.rbar != end2.cc_fault.rbar)
	    printf("14 %d, %d\n", start2.cc_fault.rbar, end2.cc_fault.rbar);
	if (start2.cc_fault.gps != end2.cc_fault.gps)
	    printf("15 %d, %d\n", start2.cc_fault.gps, end2.cc_fault.gps);
	if (start2.cc_fault.yawr1 != end2.cc_fault.yawr1)
	    printf("16 %d, %d\n", start2.cc_fault.yawr1, end2.cc_fault.yawr1);
	if (start2.cc_fault.speed != end2.cc_fault.speed)
	    printf("17 %d, %d\n", start2.cc_fault.speed, end2.cc_fault.speed);
	if (start2.cc_fault.yawr2 != end2.cc_fault.yawr2)
	    printf("18 %d, %d\n", start2.cc_fault.yawr2, end2.cc_fault.yawr2);
	if (start2.cc_fault.actuator != end2.cc_fault.actuator)
	    printf("19 %d, %d\n", start2.cc_fault.actuator, end2.cc_fault.actuator);
	if (start2.cc_fault.hmi1 != end2.cc_fault.hmi1)
	    printf("20 %d, %d\n", start2.cc_fault.hmi1, end2.cc_fault.hmi1);
	if (start2.cc_fault.hmi2 != end2.cc_fault.hmi2)
	    printf("21 %d, %d\n", start2.cc_fault.hmi2, end2.cc_fault.hmi2);
	if (start2.cc_fault.selfcc != end2.cc_fault.selfcc)
	    printf("22 %d, %d\n", start2.cc_fault.selfcc, end2.cc_fault.selfcc);
	if (start2.cc_fault.peercc != end2.cc_fault.peercc)
	    printf("23 %d, %d\n", start2.cc_fault.CAN1, end2.cc_fault.CAN1);
	if (start2.cc_fault.CAN1 != end2.cc_fault.CAN1)
	    printf("24 %d, %d\n", start2.cc_fault.CAN1, end2.cc_fault.CAN1);
	if (start2.cc_fault.CAN3 != end2.cc_fault.CAN3)
	    printf("25 %d, %d\n", start2.cc_fault.CAN3, end2.cc_fault.CAN3);
	if (start2.cc_fault.CAN4 != end2.cc_fault.CAN4)
	    printf("26 %d, %d\n", start2.cc_fault.CAN4, end2.cc_fault.CAN4);
	if (start2.cc_fault.RS232_1 != end2.cc_fault.RS232_1)
	    printf("27 %d, %d\n", start2.cc_fault.RS232_1, end2.cc_fault.RS232_1);
	if (start2.cc_fault.RS232_2 != end2.cc_fault.RS232_2)
	    printf("28 %d, %d\n", start2.cc_fault.RS232_2, end2.cc_fault.RS232_2);
	if (start2.cc_fault.RS232_3 != end2.cc_fault.RS232_3)
	    printf("29 %d, %d\n", start2.cc_fault.RS232_3, end2.cc_fault.RS232_3);
	if (start2.cc_fault.ethernet != end2.cc_fault.ethernet)
	    printf("30 %d, %d\n", start2.cc_fault.ethernet, end2.cc_fault.ethernet);
	if (start2.cc_fault.hmi_differ != end2.cc_fault.hmi_differ)
	    printf("31 %d, %d\n", start2.cc_fault.hmi_differ, end2.cc_fault.hmi_differ);
	if (start2.cc_fault.cc_differ != end2.cc_fault.cc_differ)
	    printf("32 %d, %d\n", start2.cc_fault.cc_differ, end2.cc_fault.cc_differ);
*/
    }
    

    return 0;
}
