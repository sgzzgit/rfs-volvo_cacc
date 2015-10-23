#include <stdio.h>

int main(int argc, char **argv)
{
	int i;	// object number
	int j;	// code for object header with that number 
	int k;	// code for ext1 with that object number 
	int m;	// code for ext2 with that object number 
	int p;	// code for point message with that object number 
	
	printf("onum\thdr\text1\text2\tpt\n");
	for (i = 0; i < 32; i++) {
		j = i << 3 | 2;
		k = i << 3 | 3;
		m = i << 3 | 4;
		p = i << 3 | 6;
		printf("%d\t%d\t%d\t%d\t%d\n",i, j, k, m, p);
	}
}
	
