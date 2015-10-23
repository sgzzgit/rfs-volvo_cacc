/*	FILE
 *	static char rcsid[] = "$Id: am9513.h 6710 2009-11-11 01:52:27Z dickey $";
 *
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *
 *	$Log$
 *	Revision 1.1  2006/01/20 01:11:46  dickey
 *	World Congress 2005 changes
 *	Modified Files:
 *	 	Makefile das_default.c das_man.c io_func.c realtime.ini
 *	Added Files:
 *	 	am9513.c am9513.h qmm10xt.c qmm10xt.h rdiop.c setuidroot.sh
 *	 	wriop.c
 *
 * Revision 1.3  1996/10/16  18:22:52  path
 * Add copyright notice
 *
 * Revision 1.2  1994/11/17  00:29:39  lchen
 * Modifications for triggers.
 *
 * Revision 1.1  1994/07/01  17:45:03  lchen
 * Initial revision
 *
 */

#ifndef AM9513_H
#define AM9513_H

/*	Note that on the National ATMIO-16, only 1, 2, and 5 are available.
 */

#define AM9513_NUM_COUNTERS			5

/*	Besides the frequency, these modes also
 *	set to count up, repetitively.
 */

#define AM9513_1KHZ_MODE			0x8e25
#define AM9513_10KHZ_MODE			0x8d25
#define AM9513_100KHZ_MODE			0x8c25
#define AM9513_1MHZ_MODE			0x8b25

#define AM9513_COUNT_MODE			0x1025

typedef struct
{
	unsigned base;
	bool_typ eight_bit;
	int num_scan;
	int scanlist[AM9513_NUM_COUNTERS];
} am9513_typ;

am9513_typ *am9513_init( unsigned base, bool_typ eight_bit );
bool_typ am9513_done( am9513_typ *ptimer );
void am9513_reset( am9513_typ *ptimer );
unsigned am9513_get_counter( am9513_typ *ptimer, int counter );
bool_typ am9513_mode( am9513_typ *ptimer, unsigned counter,
			unsigned mode, unsigned value );
bool_typ am9513_set_scan( am9513_typ *ptimer, char const *plist, int num_scan );
unsigned am9513_get_scan( am9513_typ *ptimer, unsigned *pdata, int num_scan );

#endif
