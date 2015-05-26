#include "can_mm.h"
#include <sys/neutrino.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "defs.h"

canregs_t *can_base_addr;

/* Dumep Phillips SJA1000 control registers 
 *	Assumes chip is using memory mapped register addressing
 */
int
main(int argc, char **argv)
{
	unsigned int base_addr;
	unsigned char reg_addr;

	sscanf(argv[1],"%i", &base_addr);	

	if( ThreadCtl(_NTO_TCTL_IO, NULL) == -1) {
		perror("_NTO_TCTL_IO");
		exit(EXIT_FAILURE);
	}

	if( (can_base_addr = (canregs_t *) mmap_device_memory(NULL, 
		SJA1000_MAP_SIZE,
		PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, base_addr ))
		== (canregs_t *) MAP_DEVICE_FAILED ) {
			perror("MAP_DEVICE_FAILED");
			exit(EXIT_FAILURE);
	}
	printf("Base address 0x%04x\n", (unsigned int) can_base_addr);
	printf("canmode: 0x%02x\n",  CANin(0,canmode));
	printf("cancmd: 0x%02x\n",  CANin(0,cancmd));
	printf("canstat: 0x%02x\n",  CANin(0,canstat));
	printf("canirq: 0x%02x\n",  CANin(0,canirq));
	printf("canirq_enable: 0x%02x\n",  CANin(0,canirq_enable));
	printf("cantim0: 0x%02x\n",  CANin(0,cantim0));
	printf("cantim1: 0x%02x\n",  CANin(0,cantim1));
	printf("canoutc: 0x%02x\n",  CANin(0,canoutc));
	printf("cantest: 0x%02x\n",  CANin(0,cantest));
	printf("arbitrationlost: 0x%02x\n",  CANin(0,arbitrationlost));
	printf("errorcode: 0x%02x\n",  CANin(0,errorcode));
	printf("errorwarninglimit: 0x%02x\n",  CANin(0,errorwarninglimit));
	printf("rxerror: 0x%02x\n",  CANin(0,rxerror));
	printf("txerror: 0x%02x\n",  CANin(0,txerror));
	printf("frameinfo: 0x%02x\n",  CANin(0,frameinfo));
	printf("canrxbufferadr: 0x%02x\n",  CANin(0,canrxbufferadr));
	printf("canrxmsgcntr: 0x%02x\n",  CANin(0,reserved3));
	printf("canclk: 0x%02x\n",  CANin(0,canclk));
	return (0);
}
