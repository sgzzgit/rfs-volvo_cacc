/**\file
 * dig.c
 *
 *	The CAN driver supports the same devctls for digital I/O
 *	as are supported by the DAS managers, so the same API
 *	functions can be used.
 *
 * This sample file does the following:
 *	 - opens the digital port
 *	 - sets write direction
 *	 - writes a value on the port
 *	 - sets read direction
 *	 - reads back the value from the pins
 *
 * Usage: dig /dev/can1 5
 *
 */

#include <sys_qnx6.h>

#include "local.h"
//#include "sys_das.h"
#include "das_clt.h"

#define PORT0_OUTPUT 	1
#define PORT0_INPUT 	0
#define PORT0		0
#define SET_ALL_BITS	0xFF

int main(int argc, char **argv)
{
	int fd;
	long value;
	
	if(argc<3) {		
		printf("usage: %s dig_port_dev_name value\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDWR);
	if(fd==-1) {
		printf("open %s: %s", argv[1], strerror(errno));
		exit(1);
		
	}

	value = atoi(argv[2]);	// value to write

//	das_digital_dir(fd, 0, PORT0_OUTPUT);
//	das_digital_out(fd, PORT0, value, SET_ALL_BITS, NULL, NULL);
	digital_dir(fd, 0, PORT0_OUTPUT);
	digital_out(fd, PORT0, value, SET_ALL_BITS, NULL, NULL);
	printf("wrote %ld\n", value);

	value = -1;	// to make sure it is really read back in below

//	das_digital_dir(fd, 0,PORT0_INPUT);
//	das_digital_in(fd, PORT0, &value);
	digital_dir(fd, 0,PORT0_INPUT);
	digital_in(fd, PORT0, &value);
	printf("read %ld\n", value);
	
	return 0;
}


