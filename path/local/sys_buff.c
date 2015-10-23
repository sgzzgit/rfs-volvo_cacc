/**\file
 *  sys_buff.c
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *	Ported to QNX6
 *
 *	This is a simple collection of routines to create,
 *	maintain, and flush data buffers in dynamic memory.
 *
 *	If you do not want to use malloc dynamically, or if you prefer
 *	to save data items rather than strings, there are also routines
 *	to save data in a circular buffer, the last N items will be
 *	printed, where N is the size of the buffer malloced at the
 *	start of the program.
 *
 *	$Log: sys_buff.c,v $
 *	Revision 1.3  2005/06/02 04:11:36  dickey
 *	Modified Files:
 *	 	sys_buff.c  Corrected calculation of insert point with deletions.
 *	
 *	Revision 1.2  2004/09/20 21:12:00  swekuang
 *	*** empty log message ***
 *	
 *	Revision 1.1.1.1  2004/08/26 23:45:04  dickey
 *	local for IDS, CCW, CACC, etc.
 *	
 *
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/select.h>

#include "local.h"
#include "sys_lib.h"
#include "sys_mem.h"
#include "sys_list.h"
#include "sys_buff.h"

typedef struct
{
	unsigned size;
	void *pdata;
} buff_data_typ;

static unsigned buff_flush( buff_typ *pbuff );

/*
 *	SYNOPSIS
 *
 *	#include <stdio.h>
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *	#include <sys_buff.h>
 *
 *	buff_typ *buff_init( FILE *pfout, unsigned max_size );
 *
 *	pfout		-	A pointer to an output stream which has is
 *					currently open, and will remain open until
 *					buff_done() is called. stdout is an acceptable
 *					pointer.
 *
 *	max_size	-	The maximum number of elements which will be kept
 *					in the output buffer.  If zero, the buffer will
 *					grow until the system runs out of memory, or
 *					buff_done() is called.  Note that this is the
 *					represents the number of elements added via
 *					buff_add() calls, not the number of bytes of
 *					memory.
 *
 *	DESCRIPTION
 *
 *	buff_init() initializes an output buffer, storing the destination
 *	output stream, and a bound, if any, on the number of buffer elements.
 *	The maximum amount of memory used by the buffer is estimated by
 *	size of element * max_size.  Memory is not allocated until buff_add()
 *	is called.
 *
 *	RETURN
 *	NULL		-	If the output stream is NULL, or there is a
 *					memory allocation problem.
 *	non-NULL	-	A pointer to the buffer.
 *
 */

buff_typ *buff_init( FILE *pfout, unsigned max_size )
{
	buff_typ *pbuff;

	if( pfout == NULL )
		return( NULL );

	if( (pbuff = (buff_typ *) MALLOC( sizeof( buff_typ ) )) == NULL )
		return( NULL );

	if( (pbuff->phead = dl_create()) == NULL )
	{
		FREE( pbuff );
		return( NULL );
	}

	pbuff->max_size = max_size;
	pbuff->pfout = pfout;

	return( pbuff );
}

/*
 *	SYNOPSIS
 *
 *	#include <stdio.h>
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *	#include <sys_buff.h>
 *
 *	bool_typ buff_done( buff_typ *pbuff );
 *
 *	pbuff	-	A pointer to a buffer created via the buff_init() call.
 *
 *	DESCRIPTION
 *	buff_done() will write the buffered data elements out to the
 *	given FILE stream, and release the buffer.  Data is written out
 *	on a first-in/first-out basis.
 *
 *	Note that the FILE stream is not closed on completion.
 *
 *	RETURN
 *	TRUE	-	If all elements are successfully written to the FILE.
 *	FALSE	-	If there is a write error, or the buffer pointer
 *				is invalid.
 *
 *	BUGS
 *	If there is a write error, there will be a memory leak.
 *
 */

bool_typ buff_done( buff_typ *pbuff )
{
	unsigned num_element;
	bool_typ status;

	if( (pbuff == NULL) || (pbuff->phead == NULL) )
		return( FALSE );

	status = TRUE;

	if( (num_element = dl_length( pbuff->phead )) != 0 )
	{
		if( buff_flush( pbuff ) != num_element )
			status = FALSE;
	}

	dl_free( pbuff->phead );

	FREE( pbuff );
	return( status );
}

/*
 *	SYNOPSIS
 *
 *	#include <stdio.h>
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *	#include <sys_buff.h>
 *
 *	unsigned buff_add( buff_typ *pbuff, void *pdata, unsigned size )
 *
 *	pbuff	-	A pointer to a buffer created via the buff_init() call.
 *	pdata	-	A pointer to data element.
 *	size	-	The size of the data element.
 *
 *	DESCRIPTION
 *	buff_add() copies a data element of the given size to an existing
 *	output buffer queue.  If the buffer exceeds the maximum size defined
 *	in the buff_init() call, old data is deleted (not saved) from the
 *	buffer.  The data may be binary or text data.
 *
 *	RETURN
 *	non-zero	-	The number of bytes added to the buffer.
 *	zero		-	If the buffer pointer, size, data pointer is invalid.
 *					If there is a memory error while adding to the buffer.
 *
 */

unsigned buff_add( buff_typ *pbuff, void *pdata, unsigned size )
{
	buff_data_typ *pitem;
	buff_data_typ node;

	if( (pdata == NULL) || (pbuff == NULL) || (size == 0) )
		return( 0 );

	if( (node.pdata = MALLOC( size )) == NULL )
		return( 0 );
	else
		bytecopy( size, pdata, node.pdata );

	node.size = size;

	if( dl_add_dup( pbuff->phead, &node,
				sizeof( buff_data_typ ), TRUE ) == FALSE )
	{
		FREE( node.pdata );
		return( 0 );
	}

	if( (pbuff->max_size != 0) && (pbuff->max_size < dl_length( pbuff->phead )))
	{
		if( (pitem = dl_rm_last( pbuff->phead )) != NULL )
		{
			FREE( pitem->pdata );
			FREE( pitem );
		}
	}
		
	return( size );
}

static unsigned buff_flush( buff_typ *pbuff )
{
	unsigned count;
	buff_data_typ *pitem;

	for( count = 0; (pitem = dl_rm_last( pbuff->phead )) != NULL; count++ )
	{
		if( fwrite( pitem->pdata, pitem->size, 1, pbuff->pfout ) != 1 )
			break;

		FREE( pitem->pdata );
		FREE( pitem );
	}
	return( count );
}

/**
 * init_circular_buffer is used to set up buffer of items of a type defined
 * by call program. sizeof(the_type_name) is used for the
 * item_size parameter. The most recent max_data items will be saved.
 */
void init_circular_buffer(cbuff_typ *pbuff, int max_data, int item_size)

{
       pbuff->data_size = max_data;
       pbuff->data_count = 0;
       pbuff->data_start = 0;
       if (max_data > 0)
               pbuff->data_array = (void *) malloc (item_size *
                                       pbuff->data_size);
       else
               pbuff->data_array = NULL;
}

/**
 * get_circular_index is used to get the index of insertion into the
 * circular buffer's data array.
 */
int get_circular_index (cbuff_typ *pbuff)
{
        int index;
        if (pbuff->data_size == pbuff->data_count) {
                /* if count at max, insert in previous start location */
                index = pbuff->data_start;
                pbuff->data_start++;
                if (pbuff->data_start == pbuff->data_size)
                        pbuff->data_start = 0;
        } else {
                /* insert at next free location, increment count */
                index = pbuff->data_start + pbuff->data_count;
		if (index >= pbuff->data_size)
			index -= pbuff->data_size;
                pbuff->data_count++;
        }
        return (index);
}

#ifdef TEST
//* An example of how to use the above dynamically malloced routines

#include <stdlib.h>

#include <getopt.h>

static void usage( char *pargv0 );

int main( int argc, char *argv[] )
{
	FILE *pfin;
	int option;
	char buffer[MAX_LINE_LEN + 1];
	buff_typ *pbuff;
	int data_size;
	int num_loop, buff_size;
	int status;
	int i;
	char *pname;

	pfin = stdin;
	buff_size = 0;
	num_loop = 1;
	pname = NULL;

	while( (option = getopt( argc, argv, "f:b:n:?" )) != EOF )
	{
		switch( option )
		{
		case 'b':
			buff_size = atoi( optarg );
			break;

		case 'n':
			num_loop = atoi( optarg );
			break;

		case 'f':
			pname = optarg;
			break;

		case '?':
		default:	
			usage( argv[0] );
			exit( EXIT_FAILURE );
		}
	}

	if( pname != NULL )
	{
		if( (pfin = fopen( pname, "r" )) == NULL )
		{
			fprintf( stderr, "Can't open %s\n", pname );
			exit( EXIT_FAILURE );
		}
	}
			
	status = EXIT_SUCCESS; 
	for( i = 0; (i < num_loop) && (status == EXIT_SUCCESS); i++ )
	{
		if( (pbuff = buff_init( stdout, buff_size )) == NULL )
		{
			fprintf( stderr, "buff_init() failed\n" );
			status = EXIT_FAILURE;
			break;
		}

		while( fgets( buffer, MAX_LINE_LEN, pfin ) != NULL )
		{
			data_size = strlen( buffer );
			if( buff_add( pbuff, buffer, data_size ) != data_size )
			{
				fprintf( stderr, "buff_add() failed\n" );
				status = EXIT_FAILURE; 
				break;
			}
		}

		buff_flush( pbuff );
		buff_done( pbuff );
		if( fseek( pfin, 0, SEEK_SET ) != 0 )
		{
			fprintf( stderr, "fseek() failed on input file. \n" );
			status = EXIT_FAILURE; 
			break;
		}
	}

	if( pname != NULL )
		fclose( pfin );

	return( status );
}

static void usage( char *pargv0 )
{
	fprintf( stderr, "Usage: %s -fbn?\n", pargv0 );
	fprintf( stderr, "\tb\tLimit on buffer elements (none).\n" );
	fprintf( stderr, "\tn\tNumber of loops (1).\n" );
	fprintf( stderr, "\tf\tInput file name (stdin).\n" );
	fprintf( stderr, "\t?\tPrints this message.\n" );
}

#endif

