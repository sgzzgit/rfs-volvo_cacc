#include <sys/neutrino.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>

char *labels[] =
{
	"A/D 7-0",
	"A/D 15-8",
	"A/D low channel",
	"A/D high channel",
	"Auxilary digital input",
	"FD9",
	"FIFO Depth",
	"FIFO Status", 
	"Status",
	"Operation Status",
	"Counter/Timer",
	"Analog Configuration",
	"0",
	"1",
	"2",
	"3",
};

/* function : read Diamond Systems DMM 32 base registers 
 *	Assumes chip is using IO port addressing
 *	Read-back registers are different from write-to.
 *
 */
int
main(int argc, char **argv)
{
	unsigned int base_addr;
	uintptr_t pbase;
	unsigned char i;
	unsigned char page;

	sscanf(argv[1],"%i", &base_addr);	
	printf("Base address 0x%04x\n", base_addr);
	if (ThreadCtl(_NTO_TCTL_IO, NULL) == -1) {
		perror("_NTO_TCTL_IO");
		exit(-1);
		}
	if ((pbase = mmap_device_io(16, base_addr)) == MAP_DEVICE_FAILED) {
		perror("MAP_DEVICE_FAILED");
		exit(-1);
		}
	for (i = 0; i < 12; i++) 
		printf("%2d: 0x%02x %s\n",  i, in8(pbase + i), labels[i]);

	page = in8(pbase+7) & 0x03;

	for (i = 12; i < 16; i++) 
		printf("%2d: 0x%02x page %d.%s\n",  i, in8(pbase + i), 
				page, labels[i]);
	return 0;
}
