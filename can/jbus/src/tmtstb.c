/**\file
 * Transmits internal messages to the B&B Model 1939STB converter.
 * Assumes serial port is on stdout.
 *
 * Copyright (c) 2005 Regents of the University of California
 *
 * Ported to QNX6 May 2005
 */

#include "old_include/std_jbus.h"

int 
main (int argc, char **argv)
{
	int control_code;
	int baud_rate;
	int ch;

        while ((ch = getopt(argc, argv, "c:b:")) != EOF) {
                switch (ch) {
		case 'c': control_code = atoi(optarg);
			  break;
		case 'b': baud_rate = atoi(optarg);
			  break;
                default: printf(
			"Usage: %s -c control code -t baud rate \n",
					 argv[0]);
                          break;
                }
        }
#ifdef DEBUG
	fprintf(stderr, "control code 0x%x \n", control_code);
#endif
	send_stb_internal(STDOUT_FILENO, control_code, baud_rate); 

	return 0;
}			
