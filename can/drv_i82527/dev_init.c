#include "can_man.h"
#include "i82527.h"
#include "can_mm.h"

/* function : write Intel 82527 registers with initialization values
 *		using memory mapped addressing 
 *		(as with ECAN 527)
 */

typedef struct {
	unsigned char addr;
	unsigned char value;
} init_array_t;

init_array_t init_array[] =
{
	{CONTROL_REG, 0x41},
	{CPU_IF_REG, 0x60},
	{CLKOUT_REG, 0x0},
	{BUS_CON_REG, 0x0},
	{STATUS_REG, 0x0},
	{BIT_TIMING_REG0, 0x41},	// values for 250Kbps
	{BIT_TIMING_REG1, 0xD8},
	{MSG_1+CTRL_0_REG, 0x55},
	{MSG_2+CTRL_0_REG, 0x55},
	{MSG_3+CTRL_0_REG, 0x55},
	{MSG_4+CTRL_0_REG, 0x55},
	{MSG_5+CTRL_0_REG, 0x55},
	{MSG_6+CTRL_0_REG, 0x55},
	{MSG_7+CTRL_0_REG, 0x55},
	{MSG_8+CTRL_0_REG, 0x55},
	{MSG_9+CTRL_0_REG, 0x55},
	{MSG_10+CTRL_0_REG, 0x55},
	{MSG_11+CTRL_0_REG, 0x55},
	{MSG_12+CTRL_0_REG, 0x55},
	{MSG_13+CTRL_0_REG, 0x55},
	{MSG_14+CTRL_0_REG, 0x55},
	{MSG_15+CTRL_0_REG, 0x55},
	{G_MASK_S_REG0, 0xFF},
	{G_MASK_S_REG1, 0xFF},
	{G_MASK_E_REG0, 0xFF},
};

int array_size = sizeof(init_array) / sizeof(init_array_t);


int main(int argc, char **argv)
{
	unsigned int base_addr = 0xd8000;	// truck CAN 1 address
	unsigned char reg_addr;
        unsigned char value;
	unsigned int pbase;
	unsigned int mask = 0xffffffff;	// print all by default
	unsigned char data_mask = 0xff; // use complete value by default
	volatile unsigned char *preg;
	int i;

	ThreadCtl(_NTO_TCTL_IO, NULL); //required to access I/O ports

	printf("array size %d\n", array_size);

	if (argc > 1) 
		sscanf(argv[1],"%i",&base_addr);	
	if (argc > 2) 
		sscanf(argv[2], "%i", &mask);
	if (argc > 3)
		sscanf(argv[3], "%hhi", &data_mask);

	printf("base 0x%08x, mask 0x%08x, data_mask 0x%02x\n", 
		base_addr, mask, data_mask);

	pbase = (unsigned int) mmap_device_memory(NULL,
		I82527_MAP_SIZE,
		PROT_READ | PROT_WRITE | PROT_NOCACHE,
		0, base_addr);

	printf("Mapped address 0x%08x\n", pbase);

	for (i = 0; i < array_size; i++) {
		if ((mask & (1 << i)) == 0)
			continue;
		reg_addr = init_array[i].addr;
		value = init_array[i].value & data_mask;
		preg = (volatile unsigned char *) (pbase + reg_addr);
		*preg = value;

		printf("0x%02x: wrote 0x%02x\n", reg_addr, value);
	}
	return 0;
}
