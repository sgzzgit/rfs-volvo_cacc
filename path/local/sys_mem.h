/**\file
 *	sys_mem.h
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *	
 * Revision 1.2  1996/10/17  23:47:58  path
 * Add copyright notice
 *
 * Revision 1.1  1994/01/31  22:45:16  lchen
 * Initial revision
 *
 */

//extern char *calloc();
//extern char *malloc();

#define FREE(p)					free( p )
#define CALLOC(n,size)			calloc( n, size)
#define MALLOC(size)			malloc( size )

