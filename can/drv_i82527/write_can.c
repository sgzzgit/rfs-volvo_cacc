#include "can_man.h"
#include "i82527.h"

/* function : write Intel 82527 registers 
 *		using IO port address register
 *		(as with SSV CAN)
 */

int main(int argc, char **argv)
{
	unsigned int base_addr;
	unsigned char reg_addr;
        unsigned char value;

	ThreadCtl(_NTO_TCTL_IO, NULL); //required to access I/O ports

	sscanf(argv[1],"%i",&base_addr);	
	printf("Base address 0x%04x\n", base_addr);
	sscanf(argv[2],"%hhi",&reg_addr);	
	sscanf(argv[3],"%hhi",&value);	
	out8(base_addr, reg_addr);
	out8(base_addr + 1, value);
	printf("0x%02x: wrote 0x%02x\n", reg_addr, value);
	return 0;
}
