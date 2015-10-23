/* dummy main for it to compile at the moment.
   will get rid of it once it is more mature */
#include <sys_os.h>
#include "msg_packer.h"

int main()
{
    sensor_state ss;
    uint64_t a = 0;
    int x = 0;
    uint8_t* p = (uint8_t*)&a;

    ss.operation_code = 1;
    ss.fault_message = 2;
    ss.sensor_health[0] = 3;
    ss.sensor_health[1] = 4;
    ss.sensor_health[2] = 5;
    ss.output_type = 6; 

    pack_sensor_state(&ss, &a);

    for (; x < 8; x++)
	printf("%X ", *(p++));
    
    return 0;
}
