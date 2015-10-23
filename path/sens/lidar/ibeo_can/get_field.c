#ifdef __QNXNTO__
#include <sys_qnx6.h>
#include <gulliver.h>
#endif

#include "ibeo.h"

int main(int argc, char **argv)
{
	unsigned char t; 
	int hi = atoi(argv[2]);
	int low = atoi(argv[3]);
	sscanf(argv[1], "%i", &t);

	printf("mask %d,%d 0x%02x\n", hi, low, GETFIELD(t, hi, low));
}
	
