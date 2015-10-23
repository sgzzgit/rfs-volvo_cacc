/**\file
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 * Revision 1.2  1996/10/17  23:35:34  path
 * Add copyright notice
 *
 * Revision 1.1  1994/11/16  22:18:01  lchen
 * Initial revision
 *
 */

int fifo_write( dl_head_typ *phead, char *psrc, int size );
int fifo_read( dl_head_typ *phead, char *pdest, int max );
void fifo_done( dl_head_typ *phead );
int fifo_flush( dl_head_typ *phead );

