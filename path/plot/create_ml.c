#include <stdio.h>
#include <unistd.h>

/**
 * Reads a variable name and a column number from standard input
 * Creates a Matlab .m file to assign each column of an array "d"
 * to a vector with the corresponding variable name.
 */

int
main(int argc, char **argv)
{
	char buffer[80];
	char field_name[80];
	int column;
	int count = 0;

	printf("%% File generated from list of field names and columns \n");
	printf("%% load foo.dat\n");
	printf("%% d = foo \n");
	printf("\n");
	while (fgets(buffer, 80, stdin)){
		sscanf(buffer, "%s %d", field_name, &column);
		printf("%s = d(:, %d);\n", field_name, column);
	}
}
