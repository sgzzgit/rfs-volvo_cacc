/**\file
 *	Definitions needed to generate and then read in
 *	an input file containing columns of data that
 *	will be read by msg_send.
 */
#include <sys_os.h>
#include "datastructs.h"
	
typedef struct {
	uint32_t identifier;	/// CAN identifier to use
	uint8_t field_num;	/// number of columns to read for this message
	char *msg_name;		/// for debugging
} msg_descriptor_t;

extern msg_descriptor_t msg_descriptors[];
extern int num_message_types;
