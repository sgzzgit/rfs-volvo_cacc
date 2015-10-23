/* 
 *	Writes an 8-bit register at an ioport address
 *	Usage: wriop <ioport_addr> <value>
 *	Must be root to run this
 */

#include <sys_qnx6.h>
#include <sys/mman.h>
#include <hw/inout.h>

int main (int argc, char **argv)
{
	unsigned short reg_addr;
	unsigned char value;
	ThreadCtl(_NTO_TCTL_IO, NULL); //required to access I/O ports

	sscanf(argv[1],"%hi",&reg_addr);	
	sscanf(argv[2],"%hhi",&value);	
	out8(reg_addr, value);
	printf("0x%04hx: wrote 0x%02hhx\n", reg_addr, value);
	return 0;
}
