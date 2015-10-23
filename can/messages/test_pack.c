#include <stdio.h>
#include "vaa_msg_struct.h"
#include "msg_pack.h"
#include "msg_unpack.h"

int main()
{    
    track_measurement_t tm;
    can_basic_msg_t basic;
    can_received_msg_t received;

    tm.peak_detection = -666;
    tm.low_speed = 555;
    tm.track_detected = 1;
    tm.peak_sensor_id = 9;
    tm.track_polarity = 1;
    tm.message_counter = 44;
    tm.time_counter = 777;

    printf("%d\n", tm.peak_detection);
    printf("%d\n", tm.low_speed);
    printf("%d\n", tm.track_detected);
    printf("%d\n", tm.peak_sensor_id);
    printf("%d\n", tm.track_polarity);
    printf("%d\n", tm.message_counter);
    printf("%d\n", tm.time_counter);
    putchar('\n');
    
    basic = pack_track_measurement(&tm);

    if (unpack_vaa_received_msg(&received, &basic)) {
	tm=received.front_track1_measurement;
	printf("%d\n", tm.peak_detection);
	printf("%d\n", tm.low_speed);
	printf("%d\n", tm.track_detected);
	printf("%d\n", tm.peak_sensor_id);
	printf("%d\n", tm.track_polarity);
	printf("%d\n", tm.message_counter);
	printf("%d\n", tm.time_counter);
    } else
    	printf("Unrecognized message ID %d\n", basic.id);

    return 0;
}
