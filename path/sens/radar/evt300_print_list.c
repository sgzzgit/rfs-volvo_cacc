/**\file
 *	Prints all the known messages of the EVT300 VBUS radar system
 */

#include <sys_os.h>
#include "local.h"
#include "timestamp.h"
#include "evt300.h"

int main (int argc, char **argv)
{
	int i;
	evt300_msg_info_t *pm;
	for (i = 0; i < 256; i++) {
		pm = &evt300_msg_info[i];
		if (pm->msg_id != 0) {
			printf("%2d\t %-45s %3d\t %3d\n",
			 i, pm->msg_name, pm->msg_id, (signed) pm->num_bytes);
		}
	}
	return 0;
} 

