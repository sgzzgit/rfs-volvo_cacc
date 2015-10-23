#include <sys_qnx6.h>
#include <gulliver.h>

int main(int argc, char **argv)
{
	uint64_t m = 0x123456789abcdef0;
	uint64_t n = 0xaaaaaaaa55555555; 
	uint64_t x = 0x1000000000000000;
	uint64_t y = 0x2000000000000000;
	printf("%llx\n", n << 1);
	printf("%llx\n", n >> 1);
	printf("%llx\n", m);
	n = ENDIAN_SWAP64(&m);
	printf("%llx\n", n);
	printf("%llx\n", m);
	n = x + y;
	printf("%llx\n", n);
}
	
