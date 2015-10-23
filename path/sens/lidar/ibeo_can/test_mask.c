#include <sys_qnx6.h>
#include <gulliver.h>
#include "ibeo.h"

int main(int argc, char **argv)
{
	unsigned char t;
	int hi = atoi(argv[1]);
	int low = atoi(argv[2]);
	t = GETMASK(hi,low);
	printf("mask %d,%d 0x%02x\n", hi, low, t);
}
	
