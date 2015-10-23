/*      FILE
**      qmm10xt.h
**
**      José Miguel S. Almeida, 2002
**      ISEP, Porto,    Portugal
**
**      Initial revision
**
 */
#ifndef QMM10XT_H
#define QMM10XT_H

#include "am9513.h"

/*	Digital ports, one for input and one for output. 
*	It is not possible to configure the port directions.
 */

#define QMM10XT_DIG_IN_PORT			0
#define QMM10XT_DIG_OUT_PORT			1

/*	Counter/timer definitions.
 *	There are five timers on each of the two AM9513 chips.
 *	User programs can specify timers numbered from 0 to 9
 *	in the list of timers passed to the driver. 0-4 will be
 *	on the first chip (am9513id=0), 5-9 will be on the second
 *	chip (am9513id=1)
 */
#define QMM10XT_N_9513				2
#define QMM10XT_N_TIMERS			10

#define QMM10XT_TIMER_1				0
#define QMM10XT_TIMER_2				1
#define QMM10XT_TIMER_3				2
#define QMM10XT_TIMER_4				3
#define QMM10XT_TIMER_5				4

typedef struct
{
 // do not change the declaration order (must match the div_t in stdlib.h)
 int am9513_id;
 int timer_id;
} tmr_scan_list_t;

#define GET_TIMER_ID( timerinfo) ((timerinfo).rem)

#define GET_9513_ID( timerinfo) ((timerinfo).quot)

typedef struct
{
	unsigned base;
	int error;
	long dig_old_bits;
	am9513_typ *ptimer[QMM10XT_N_9513];
	div_t tmr_scan_list[QMM10XT_N_TIMERS];
	int num_scan;
} qmm10xt_typ;


qmm10xt_typ *qmm10xt_init( unsigned base );

bool_typ qmm10xt_done( qmm10xt_typ *pboard );

#endif
