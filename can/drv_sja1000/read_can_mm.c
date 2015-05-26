#include "can_mm.h"
#include <sys/neutrino.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

/* function : read Phillips SJA1000  registers 
 *	Assumes chip is using memory mapped register addressing
 */
int
main(int argc, char **argv)
{
	unsigned int base_addr;
	unsigned char reg_addr;
	unsigned int pbase;
	volatile unsigned char *preg;

	sscanf(argv[1],"%i", &base_addr);	
	sscanf(argv[2],"%hhi", &reg_addr);	

	if( ThreadCtl(_NTO_TCTL_IO, NULL) == -1) {
		perror("_NTO_TCTL_IO");
		exit(EXIT_FAILURE);
	}

	if( (pbase = (unsigned int) mmap_device_memory( NULL, SJA1000_MAP_SIZE,
		PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, base_addr ))
		== MAP_DEVICE_FAILED ) {
			perror("MAP_DEVICE_FAILED");
			exit(EXIT_FAILURE);
	}
	printf("Base address 0x%04x\n", pbase);
	preg = (volatile unsigned char *) (pbase + reg_addr);

	printf("0x%02x: 0x%02x\n",  reg_addr, *preg);
	return 0;
}
