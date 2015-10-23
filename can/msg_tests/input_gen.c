/**\file
 *	Program to generate an input stream containing columns of data that
 *	will be read by msg_send. Data is written to stdout. 
 *
 *      Copyright (c) 2008 Regents of the University of California
 *
 *      Author: Sue Dickey
 */
#include <sys_os.h>
#include "datastructs.h"
#include "msg_descriptors.h"

/** Array containing index of messages that data is generated for
 *  To change what messages we are generating, just put different
 *  indexes in the array. 
 */
int msg_indexes[] = {0, 1, 2, 6};

int num_msgs = sizeof(msg_indexes)/sizeof(int);

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
		printf("%d ", num_msgs); 
		for (j = 0; j < num_msgs; j++) {
			msg_descriptor_t *p = &msg_descriptors[msg_indexes[j]];
			if (i == 0)
			{
			    fprintf(stderr, "%s\n", p->msg_name);
			    fprintf(stderr, "%d\n", p->field_num);
			}

			printf(" %d ", p->identifier);
			for (k = 0; k < p->field_num; k++)
				printf(" %d ", rand());			
		}
		printf("\n");
	}
	return 0;
}
