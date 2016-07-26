/**\file
 *	static char rcsid[] = "$Id: sys_buff.h 2919 2008-08-26 21:29:47Z jspring $";
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *
 *
 *	$Log: sys_buff.h,v $
 *	Revision 1.2  2004/09/20 21:12:00  swekuang
 *	*** empty log message ***
 *	
 *	Revision 1.1.1.1  2004/08/26 23:45:04  dickey
 *	local for IDS, CCW, CACC, etc.
 *	
 * Revision 1.3  1996/10/17  23:26:07  path
 * Add copyright notice
 *
 * Revision 1.2  1995/06/20  05:50:08  lchen
 * Add documentation.
 *
 * Revision 1.1  1995/06/01  22:19:44  lchen
 * Initial revision
 *
 */

#pragma once 

#include <stdio.h>

/** Buffer type used with dynamically malloced string saving */
typedef struct
{
	dl_head_typ *phead;		/*	Head of queue of buff_data_typ.	*/
	unsigned max_size;		/*	Zero if unlimited buffer size.	*/
							/*	Otherwise, # maximum elements.	*/
	FILE *pfout;			/*	File pointer for output.		*/
} buff_typ;


/**
 * Circular buffer type used with data item saving 
 * Data from last "data_size" samples can be saved in this
 * structure. 
 */

typedef struct {
  void *data_array;     /* circular buffer */
  int data_size;  /* number of structures malloced */
  int data_count; /* control intervals saved (<=data_size) */
  int data_start; /* index of oldest item in the circular buffer */
} cbuff_typ;


buff_typ *buff_init( FILE *pfin, unsigned max_size );
bool_typ buff_done( buff_typ *pbuff );
unsigned buff_add( buff_typ *pbuff, void *pdata, unsigned size );
void init_circular_buffer(cbuff_typ *pbuff, int max_data, int item_size);
int get_circular_index (cbuff_typ *pbuff);

