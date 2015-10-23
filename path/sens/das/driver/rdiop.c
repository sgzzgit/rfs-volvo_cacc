/* 
 *	Reads an 8-bit register at an ioport address
 *	Usage: read_iop <ioport_addr>
 */

#include <sys_qnx6.h>
#include <sys/mman.h>
#include <hw/inout.h>

int
main(int argc, char **argv)
{
	unsigned short reg_addr;

	ThreadCtl(_NTO_TCTL_IO, NULL); //required to access I/O ports

	sscanf(argv[1],"%hi", &reg_addr);	
	printf("0x%04hx: 0x%02hhx\n",  reg_addr, in8(reg_addr));
	return 0;
}
