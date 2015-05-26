#include "can_man.h"
#include "i82527.h"
#include <sys/neutrino.h>
#include <stdint.h>
#include <sys/mman.h>

/* function : read Intel 82527 registers 
 *	Assumes chip is using IO port base addressing
 *	with index written to address register
 *
 */
int
main(int argc, char **argv)
{
	unsigned int base_addr;
	unsigned char reg_addr;
	uintptr_t pbase;

	sscanf(argv[1],"%i", &base_addr);	
	printf("Base address 0x%04x\n", base_addr);
	sscanf(argv[2],"%hhi", &reg_addr);	
	if( ThreadCtl(_NTO_TCTL_IO, NULL) == -1) {
		perror("_NTO_TCTL_IO");
		exit(EXIT_FAILURE);
		}
	if( (pbase = mmap_device_io( 1, base_addr)) == MAP_DEVICE_FAILED ) {
		perror("MAP_DEVICE_FAILED");
		exit(EXIT_FAILURE);
		}

	out8(pbase, reg_addr);
	printf("0x%02x: 0x%02x\n",  reg_addr, in8(pbase + 1));
	return 0;
}
