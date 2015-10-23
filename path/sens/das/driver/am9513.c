/*	FILE
 *	am9513.c
 *	static char rcsid[] = "$Id: am9513.c 6710 2009-11-11 01:52:27Z dickey $";
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 * 	Port to QNX6 (replace inp with in8, outpw with out16, etc.)
 * Copyright (c) 2005   Regentsof of the University of California
 *
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
 * Revision 1.3  1996/10/16  18:22:32  path
 * Add copyright notice
 *
 * Revision 1.2  1994/11/17  00:29:39  lchen
 * Modifications for triggers.
 *
 * Revision 1.1  1994/07/01  17:45:03  lchen
 * Initial revision
 *
 */

/*
** 2002 ISEP - J. Almeida
** add AM9513_SAVE_TO_HOLD_CMD in the am9513_get_counter
** required by the cts9513 chip
*/


#include <sys_qnx6.h>
#include <x86/inout.h>
#include <local.h>
#include <sys_mem.h>

#include "am9513.h"

#define AM9513_TIMER_1				0
#define AM9513_TIMER_2				1
#define AM9513_TIMER_3				2
#define AM9513_TIMER_4				3
#define AM9513_TIMER_5				4

/*	AM9513 counter/timer.
 */

#define AM9513_DATA_PORT			0x0

#define AM9513_STATUS8_PORT			0x1
#define AM9513_COMMAND8_PORT		0x1

#define AM9513_STATUS16_PORT		0x2
#define AM9513_COMMAND16_PORT		0x2

/*	Index to access counter values, and
 *	Mask for use with counters in commands.
 */

#define AM9513_MASK(i)				(0x0001 << (i))

/*	Command Codes.
 *	The ATMIO-16 card requires that the high bits
 *	be set when writing to the command register.
 *
 *	The master mode command is necessary to select BCD divisor scaling.
 *	Otherwise, options could be set via the command register.
 *	Select appropriate bus size.
 *	FOUT off.
 *	Disable data pointer incrementing.
 */

#define MASK_16_BIT_CMD				0xff00
#define AM9513_MASTER_16_MODE		0xf000
#define AM9513_MASTER_8_MODE		0xd000

#define AM9513_RESET_CMD			0xff
#define AM9513_16_BIT_CMD			0xef
#define AM9513_8_BIT_CMD			0xe7
#define AM9513_ARM_CMD				0x20
#define AM9513_LOAD_N_CMD			0x40
#define AM9513_DISARM_SAVE_CMD			0x80
#define AM9513_SAVE_TO_HOLD_CMD			0xa0
#define AM9513_DISARM_CMD			0xc0
#define AM9513_STEP_CMD				0xf0

/*	These commands set the data pointer register.
 */

#define AM9513_DP_MODE_CMD			0x00
#define AM9513_DP_LOAD_CMD			0x08
#define AM9513_DP_HOLD_CMD			0x10
#define AM9513_DP_MASTER_CMD		0x17

#define AM9513_INACTIVE_VALUE		0x03
#define AM9513_COUNT_UP_MODE		0x04

#define BYTE_SIZE					8
#define BYTE_MASK					0xff


static void am9513_arm( am9513_typ *ptimer, unsigned counter );
static void am9513_disarm( am9513_typ *ptimer, unsigned counter );
static void command( am9513_typ *ptimer, unsigned value );
static unsigned data_in( am9513_typ *ptimer );
static void data_out( am9513_typ *ptimer, unsigned value );

/*	AM9513 initialization routine
 */

am9513_typ *am9513_init( unsigned base, bool_typ eight_bit )
{
	am9513_typ *ptimer;

	if( (ptimer = (am9513_typ *) MALLOC( sizeof( am9513_typ ) )) == NULL )
		return( NULL );

	ptimer->base = base;
	ptimer->num_scan = 0;
	ptimer->eight_bit = eight_bit;

	am9513_reset( ptimer );
	return( ptimer );
}

bool_typ am9513_done( am9513_typ *ptimer )
{
	if( ptimer == NULL )
		return( FALSE );
	else
	{
		am9513_reset( ptimer );
		return( TRUE );
	}
}

/*	Reset the chip.
 *	Set the bus size.
 *	Point 
 */

void am9513_reset( am9513_typ *ptimer )
{
	int i;

	command( ptimer, AM9513_RESET_CMD );

	if( ptimer->eight_bit == TRUE )
	{
		command( ptimer, AM9513_8_BIT_CMD );
		command( ptimer, AM9513_DP_MASTER_CMD );
		data_out( ptimer, AM9513_MASTER_8_MODE );
	}
	else
	{
		command( ptimer, AM9513_16_BIT_CMD );
		command( ptimer, AM9513_DP_MASTER_CMD );
		data_out( ptimer, AM9513_MASTER_16_MODE );
	}

	/*	Initialize all counters
	 *	Select command mode register
	 *	Store mode value
	 *	Select load register
	 */

	for( i = 0; i < AM9513_NUM_COUNTERS; i++ )
	{
		command( ptimer, AM9513_DP_MODE_CMD | (i + 1) );
		data_out( ptimer, AM9513_COUNT_UP_MODE );

		command( ptimer, AM9513_DP_LOAD_CMD | (i + 1) );
		data_out( ptimer, AM9513_INACTIVE_VALUE );
	}

	command( ptimer, AM9513_LOAD_N_CMD
		| AM9513_MASK( AM9513_TIMER_1 ) | AM9513_MASK( AM9513_TIMER_2 )
		| AM9513_MASK( AM9513_TIMER_3 ) | AM9513_MASK( AM9513_TIMER_4 )
		| AM9513_MASK( AM9513_TIMER_5 ) );
}

static void am9513_arm( am9513_typ *ptimer, unsigned counter )
{
	command( ptimer, AM9513_ARM_CMD | AM9513_MASK( counter) );
}

static void am9513_disarm( am9513_typ *ptimer, unsigned counter )
{
	command( ptimer, AM9513_DISARM_CMD | AM9513_MASK( counter ) );
}

bool_typ am9513_mode( am9513_typ *ptimer,
	unsigned counter, unsigned mode, unsigned value )
{
	if( (ptimer == NULL) || (counter < AM9513_TIMER_1)
		|| (AM9513_TIMER_5 < counter) )
	{
		return( FALSE );
	}

	am9513_disarm( ptimer, counter );

	command( ptimer, AM9513_DP_MODE_CMD | (counter + 1) );

	data_out( ptimer, mode );

	command( ptimer, AM9513_DP_LOAD_CMD | (counter + 1) );
	data_out( ptimer, value );
	command( ptimer, AM9513_LOAD_N_CMD | AM9513_MASK( counter ) );

	am9513_arm( ptimer, counter );

	return( TRUE );
}

unsigned am9513_get_counter( am9513_typ *ptimer, int counter )
{
//jma
	command( ptimer, AM9513_SAVE_TO_HOLD_CMD | AM9513_MASK( counter ) );
	command( ptimer, AM9513_DP_HOLD_CMD | (counter + 1) );

	return( data_in( ptimer ) ); 
}

/*	This call is safe with num_scan larger than
 *	the value in the timer structure.
 */

unsigned am9513_get_scan( am9513_typ *ptimer, unsigned *pdata, int num_scan )
{
	unsigned i;
	int max_scan;

	max_scan = min( ptimer->num_scan, num_scan );

	for( i = 0; i < max_scan; i++, pdata++ )
		*pdata = am9513_get_counter( ptimer, ptimer->scanlist[i] );

	return( i );
}

/*	Allow a zero value for num_scan.
 */

bool_typ am9513_set_scan( am9513_typ *pdevice, char const *plist, int num_scan )
{
	int i;
	int timer;

	if( AM9513_NUM_COUNTERS < num_scan )
		return( FALSE );

	for( i = 0; i < num_scan; i++, plist++ )
	{
		timer = *plist;
		if( (timer < AM9513_TIMER_1) || (AM9513_TIMER_5 < timer) )
		{
			pdevice->num_scan = 0;
			return( FALSE );
		}
		else
			pdevice->scanlist[i] = timer;
	}
	pdevice->num_scan = num_scan;
	return( TRUE );
}

static void command( am9513_typ *ptimer, unsigned value )
{
	if( ptimer->eight_bit == TRUE )
		out8( ptimer->base + AM9513_COMMAND8_PORT, (BYTE_MASK & value) );
	else
		out16( ptimer->base + AM9513_COMMAND16_PORT, MASK_16_BIT_CMD | value );
}

static unsigned data_in( am9513_typ *ptimer )
{
	unsigned lsb, msb, value;

	if( ptimer->eight_bit == TRUE )
	{
		lsb = in8( ptimer->base + AM9513_DATA_PORT );
		msb = in8( ptimer->base + AM9513_DATA_PORT );
		value = (msb << BYTE_SIZE) | lsb;
	}
	else
		value = in16( ptimer->base + AM9513_DATA_PORT );

	return( value );
}

static void data_out( am9513_typ *ptimer, unsigned value )
{
	unsigned lsb, msb;

	if( ptimer->eight_bit == TRUE )
	{
		lsb = value & BYTE_MASK;
		msb = (value >> BYTE_SIZE) & BYTE_MASK;

		out8( ptimer->base + AM9513_DATA_PORT, lsb );
		out8( ptimer->base + AM9513_DATA_PORT, msb );
	}
	else
		out16( ptimer->base + AM9513_DATA_PORT, value );
}

#ifdef TEST

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ATMIO16_TIMER_BASE		0x238
#define NUM_ITERATIONS			20

static char scan_list[] =
{
	AM9513_TIMER_1,
	AM9513_TIMER_2,
	AM9513_TIMER_5
};

void main( int argc, char *argv[] )
{
	int i, j;
	am9513_typ *ptimer;
	unsigned buffer[AM9513_NUM_COUNTERS];

	if( (ptimer = am9513_init( ATMIO16_TIMER_BASE, FALSE )) == NULL )
	{
		fprintf( stderr, "Can't create timer.\n" );
		exit( EXIT_FAILURE );
	}

	if( am9513_set_scan( ptimer, scan_list, sizeof( scan_list ) ) == FALSE )
	{
		fprintf( stderr, "Can't set timer.\n" );
		am9513_done( ptimer );
		exit( EXIT_FAILURE );
	}

	/*
	 *	Test with input on Gate #n.
	 */

	if( (am9513_mode( ptimer, AM9513_TIMER_1, 0xcca8, 0 ) != TRUE)
		|| (am9513_mode( ptimer, AM9513_TIMER_2, 0xbca8, 0 ) != TRUE)
		|| (am9513_mode( ptimer, AM9513_TIMER_5, 0xcba8, 0 ) != TRUE) )
	{
		fprintf( stderr, "Can't set timer modes.\n" );
		am9513_done( ptimer );
		exit( EXIT_FAILURE );
	}

	for( i = 0; i < NUM_ITERATIONS; i++ )
	{
		sleep( 1 );
		if( am9513_get_scan( ptimer, buffer,
			sizeof( scan_list ) ) != sizeof( scan_list ) )
		{
			break;
		}

		for( j = 0; j < sizeof( scan_list ); j++ )
			printf( "%6u\t", buffer[j] );

		printf( "\n" );
	}

	am9513_done( ptimer );
	exit( EXIT_SUCCESS );
}

#endif

#if FALSE
	/*	Program the sample interval counter, in MHZ.
	 */

	out16( pboard->base + COUNT_COMMAND, AM9513_MODE_CMD | (COUNTER_3 + 1) );
	out16( pboard->base + COUNT_DATA, AM9513_1MHZ_MODE );


	/*	This loads the value 2 into counter 3,
	 *	and steps it down to 1.
	 */

	out16( pboard->base + COUNT_COMMAND, 
		AM9513_MODE_CMD | AM9513_LOAD_CMD | (COUNTER_3 + 1) );
	out16( pboard->base + COUNT_DATA, 2 );

	out16( pboard->base + COUNT_COMMAND, 
		AM9513_MODE_CMD | AM9513_LOAD_CMD | MASK( COUNTER_3 ) );

	out16( pboard->base + COUNT_COMMAND,
		AM9513_MODE_CMD | AM9513_STEP_CMD | (COUNTER_3 + 1) );

	/*	Put the sample interval into the load register,
	 *	and arm the counter.
	 */

	out16( pboard->base + COUNT_DATA, ticks );

	am9513_arm( pboard, COUNTER_3 );

	/*	Program for sample counting on counter #4.
	 *	Point at the load register,
	 *	Load counter 4, decrement it, and arm it.
	 */

	out16( pboard->base + COUNT_COMMAND, AM9513_MODE_CMD | (COUNTER_4 + 1) );
	out16( pboard->base + COUNT_DATA, AM9513_COUNT_MODE );

	out16( pboard->base + COUNT_COMMAND, 
		AM9513_MODE_CMD | AM9513_LOAD_CMD | (COUNTER_4 + 1) );

	out16( pboard->base + COUNT_DATA, pboard->num_analog );

	out16( pboard->base + COUNT_COMMAND, 
		AM9513_MODE_CMD | AM9513_LOAD_CMD | MASK( COUNTER_4 ) );

	out16( pboard->base + COUNT_COMMAND,
		AM9513_MODE_CMD | AM9513_STEP_CMD | (COUNTER_4 + 1) );

	am9513_arm( pboard, COUNTER_4 );
#endif

