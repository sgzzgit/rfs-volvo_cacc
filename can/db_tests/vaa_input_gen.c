/**\file
 *
 *	Program to generate an input stream containing columns of fake
 *	data representing CAN messages that will be read by other programs
 * 	to write the data server or send messages directly on the CAN bus. 
 *	Data is written to stdout. 
 *
 *      Copyright (c) 2010 Regents of the University of California
 *
 *      Author: Sue Dickey
 */
#include <sys_os.h>
#include "db_clt.h"
#include "vaa_msg.h"

/** Gets information what messages are on a line of the input file from array
 *  in vaa_db_utils.c, which is also used by the programs that read  
 *
 *  To change what messages we are generating, just put different
 *  indexes in the array. 
 */
extern int vaa_msg_indexes[];		// Contains IDs of messages to generate
extern int vaa_num_generated_msgs;	// Number of messages per line

int main(int argc, char **argv)
{
	int i,j, k;
	int opt;
	int num_lines = 10;	// by default generates 10 lines
	unsigned int seed = 13;		// for the random number generator

	while ((opt = getopt(argc, argv, "n:s:")) != -1) {
		switch (opt) {
		case 'n':
			num_lines = atoi(optarg);
			break;
		case 's':
			seed = atoi(optarg);
			break;
		default:
			printf("Usage: %s -n <number of lines>\n", argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}
	srand(seed);
	for (i = 0; i < num_lines; i++) {
		printf("%d ", vaa_num_generated_msgs); 
		for (j = 0; j < vaa_num_generated_msgs; j++) {
			vaa_msg_descr_t *p = &vaa_msg[vaa_msg_indexes[j]];
			if (i == 0) {
				fprintf(stderr, "%s\n", p->name);
				fprintf(stderr, "%d\n", p->fields);
			}

			printf(" %d ", p->id);
			for (k = 0; k < p->fields; k++)
				printf(" %d ", rand());			
		}
		printf("\n");
	}
	return 0;
}
