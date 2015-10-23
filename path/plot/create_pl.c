#include <stdio.h>
#include <unistd.h>

/**
 * Reads a variable name and a column number from standard input
 * Creates a gnuplot input file for four graphs per page on standard output 
 */

int
main(int argc, char **argv)
{
	char buffer[80];
	char field_name[80];
	int column;
	int count = 0;
	char *filename = "test.dat";

	if(argv[1] != NULL)
		filename = argv[1];
	printf("# File generated from list of field names and columns\
 from file %s\n", filename);
	printf("set grid\n");
	printf("set size 0.5,0.5\n");
	printf("\n");
	while (fgets(buffer, 80, stdin)){
		float xloc = (count % 2)/2.0;
		float yloc = ((3-count) / 2)/2.0;
		sscanf(buffer, "%s %d", field_name, &column);
		if (count == 0) {
			printf("set multiplot\n");
			printf("set xlabel \"Tick # [each is 75 msec]\"\n");
		}
		printf("set origin %.2f,%.2f\n", xloc, yloc);
		printf("set title \"%s\"\n", field_name);
		printf("plot \"%s\" using %d\n", filename, column);
		if (count == 3) {
			printf("set nomultiplot\n");
			printf("pause -1 \"Press Enter to continue.\"\n");
			printf("clear\n");
			printf("\n");
		}
		count++;
		if (count == 4) count = 0;
	}
	printf("set nomultiplot\n");
	printf("pause -1 \"Press Enter to continue.\"\n");
}
