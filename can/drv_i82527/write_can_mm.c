#include "can_man.h"
#include "i82527.h"
#include "can_mm.h"

/* function : write Intel 82527 registers 
 *		using memory mapped addressing 
 *		(as with ECAN 527)
 */

int main(int argc, char **argv)
{
	unsigned int base_addr;
	unsigned char reg_addr;
        unsigned char value;
	unsigned int pbase;
	volatile unsigned char *preg;

	ThreadCtl(_NTO_TCTL_IO, NULL); //required to access I/O ports

	sscanf(argv[1],"%i",&base_addr);	
	sscanf(argv[2],"%hhi",&reg_addr);	
	sscanf(argv[3],"%hhi",&value);	

	pbase = (unsigned int) mmap_device_memory(NULL,
		I82527_MAP_SIZE,
		PROT_READ | PROT_WRITE | PROT_NOCACHE,
		0, base_addr);

	printf("Base address 0x%04x\n", pbase);

	preg = (volatile unsigned char *) (pbase + reg_addr);
	*preg = value;

	printf("0x%02x: wrote 0x%02x\n", reg_addr, value);
	return 0;
}
