/* 
 *	Writes an 8-bit register at an ioport address
 *	Usage: wriop <ioport_addr> <value>
 *	Must be root to run this
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/io.h> /* for glibc */


int main (int argc, char **argv)
{
#if defined(linux)
	unsigned short reg_addr;
	unsigned char value;

	sscanf(argv[1],"%hi",&reg_addr);	
	if(reg_addr < 0x400) {
		if( (ioperm( (unsigned long)reg_addr, 1, 0666)) < 0) {
			perror("ioperm");
			exit(EXIT_FAILURE);
		}
	} else {
		if(iopl(3) < 0) {
			perror("iopl");
			exit(EXIT_FAILURE);
		}
	} 
	if(argc == 3) {
		sscanf(argv[2],"%hhi",&value);	
		outb(value, reg_addr);
		printf("%#04.4hx: wrote %#02.2hhx\n", reg_addr, value);
	} else {
		value = inb(reg_addr);
		printf("%#04.4hx: read %#02.2hhx\n", reg_addr, value);
	}	
	
#endif
	return 0;
}

