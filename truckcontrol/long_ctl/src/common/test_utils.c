/**\file	test_utils.c
 *
 *	Test code for utilities.
 *	So far just a test for the circular buffer code.
 */
#include "long_ctl.h"

typedef struct {
	int one;
	int two;
} buffer_item;

int main(int argc, char **argv)
{
	cbuff_typ dbuff;
	buffer_item *pdata; 
	buffer_item current;
	int current_index;
	int i;
	(void) init_circular_buffer(&dbuff, 10, sizeof(buffer_item));
	pdata = (buffer_item *) dbuff.data_array;
	printf(" array starts at 0x%x\n", (unsigned int)pdata);
	for (i = 0; i < 20; i++) {
		int index = get_circular_index(&dbuff);
		current.one = i;
		current.two = i+1;
		pdata[index] = current;
	}
	current_index = dbuff.data_start + 1;
	if (current_index == dbuff.data_size) current_index = 0;
	printf("data_start is index %d\n", current_index);	
	for (i = 0; i < dbuff.data_count; i++) {
		printf("%d: %d %d\n", i, pdata[current_index].one,
					pdata[current_index].two);
		current_index++;
		if (current_index == dbuff.data_size)
			current_index = 0;
	} 
	return 0;
}
