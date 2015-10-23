/**\file
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *
 *	sys_fifo.c
 *	Buffering routines.
 *
 * Revision 1.2  1996/10/17  23:34:29  path
 * Add copyright notice
 *
 * Revision 1.1  1994/11/16  22:18:01  lchen
 * Initial revision
 *
 */

#include "sys_os.h"

#include "local.h"
#include "sys_mem.h"
#include "sys_list.h"
#include "sys_lib.h"
#include "sys_fifo.h"

typedef struct
{
	int size;
	char *pread;
	char *pdata;
} fifo_item_typ;

/*	fifo_write() will copy the given data, and append a single new 
 *	element to the end of a FIFO queue.  The size and data pointer
 *	of the fifo_item_typ will be set.
 *
 *	RETURN
 *		ERROR 	-	If a memory, size, or list problem is encountered.
 *		0 <= 	-	The item size, if the item is successfully appended.
 */

int fifo_write( dl_head_typ *phead, char *psrc, int size )
{
	fifo_item_typ *pitem;

	if( size == 0 )
		return( 0 );

	if( (phead == NULL) || (size < 0) )
		return( ERROR );

	if( (pitem = MALLOC( sizeof( fifo_item_typ ) )) == NULL )
		return( ERROR );

	if( (pitem->pdata = MALLOC( size )) == NULL )
	{
		FREE( pitem );
		return( ERROR );
	}

	bytecopy( size, psrc, pitem->pdata );
	pitem->size = size;
	pitem->pread = pitem->pdata;

	if( dl_append( pitem, phead ) == FALSE )
	{
		FREE( pitem->pdata );
		FREE( pitem );
		return( ERROR );
	}
	else
		return( size );
}

/*	fifo_read() will copy up to max bytes from the FIFO queue
 *	into the destination buffer.  The number of bytes is 
 *	returned.  All queue elements which have been exhausted are
 *	removed from the queue, and memory is released.
 *	The fifo_item_typ size and data pointers are maintained for
 *	partially exhausted queue elements.
 *
 *	RETURN
 *		ERROR	-	If there is a list problem.
 *		0 <=	-	The number of bytes extracted from the FIFO.
 */

int fifo_read( dl_head_typ *phead, char *pdest, int max )
{
	fifo_item_typ *psrc;
	dl_node_typ *pnode;
	int sum;
	int part;

	if( max == 0 )
		return( 0 );

	if( (phead == NULL) || (max < 0) )
		return( ERROR );

	for( sum = 0; sum < max; )
	{
		/*	Get the head of the queue.	*/

		if( (pnode = phead->pfirst) == NULL )
			break;

		psrc = (fifo_item_typ *) pnode->pitem;

		if( (sum + psrc->size) <= max )
		{
			/*	Read whole packet and remove.			*/

			bytecopy( psrc->size, psrc->pread, pdest );
			pdest += psrc->size;
			sum += psrc->size;
			dl_rm_first( phead );
			FREE( psrc->pdata );
			FREE( psrc );
		}
		else
		{
			/*	Just read part of the packet.
			 *	Update pointers to remainder of packet, and quit.
			 */

			part = max - sum;
			bytecopy( part, psrc->pread, pdest );
			psrc->size -= part;
			psrc->pread += part;
			sum += part;
		}
	}
	return( sum );
}

void fifo_done( dl_head_typ *phead )
{
	if( phead == NULL )
		return;

	fifo_flush( phead );
	dl_free( phead );
}

int fifo_flush( dl_head_typ *phead )
{
	int sum;
	fifo_item_typ *pitem;
	
	if( phead == NULL )
		return( ERROR );

	sum = 0;
	while( (pitem = (fifo_item_typ *) dl_rm_last( phead )) != NULL )
	{
		sum += pitem->size;
		FREE( pitem->pdata );
		FREE( pitem );
	}
	return( sum );
}

#ifdef TEST

/* Sample test script to run with differing buffer sizes.

sys_fifo 1 1 <sys_fifo.c >temp.out
diff sys_fifo.c temp.out
sys_fifo 1 130 <sys_fifo.c >temp.out
diff sys_fifo.c temp.out
sys_fifo 130 1 <sys_fifo.c >temp.out
diff sys_fifo.c temp.out
sys_fifo 53 53 <sys_fifo.c >temp.out
diff sys_fifo.c temp.out
sys_fifo 30 31 <sys_fifo.c >temp.out
diff sys_fifo.c temp.out
sys_fifo 31 30 <sys_fifo.c >temp.out
diff sys_fifo.c temp.out

*/

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_INPUT_SIZE		32
#define DEFAULT_OUTPUT_SIZE		55

void main( int argc, char *argv[] )
{
	char in_buff[MAX_LINE_LEN+1];
	char out_buff[MAX_LINE_LEN+1];
	int input_size, output_size;
	dl_head_typ *phead;

	input_size = DEFAULT_INPUT_SIZE;
	output_size = DEFAULT_OUTPUT_SIZE;

	if( 2 <= argc )
		input_size = atoi( argv[1] );

	if( 3 <= argc )
		output_size = atoi( argv[2] );

	if( (phead = dl_create()) == NULL )
	{
		fprintf( stderr, "dl_create()\n" );
		exit( EXIT_FAILURE );
	}

	input_sum = 0;
	while( fgets( in_buff, input_size + 1, stdin ) != NULL )
	{
		if( (n = fifo_write( phead, in_buff, strlen( in_buff ) )) == ERROR)
		{
			fprintf( stderr, "fifo_write()\n" );
			exit( EXIT_FAILURE );
		}
		else
			input_sum += n;
	}

	output_sum = 0;
	while( (n = fifo_read( phead, out_buff, output_size )) != ERROR )
	{
		out_buff[n] = END_OF_STRING;
		fputs( out_buff, stdout );
		output_sum += n;
	}

	exit( EXIT_SUCCESS );
}

#endif
