/**	
 *	static char rcsid[] = "$Id";
 *
 *	Utility routines for A/D and D/A data.
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *
 *	$Log$
 *	Revision 1.2  2004/09/20 19:47:08  dickey
 *	Check-in, null_das tested, garnet tested, interrupts and dmm34 not tested
 *	Modified Files:
 *	 	Makefile README das_default.c das_init.c das_man.c das_man.h
 *	 	das_util.c garnet.c io_func.c null_das.c realtime.ini
 *	 Added Files:
 *	 	dmm32.c dmm32.h garnet.h
 *
 */

#include <sys_qnx6.h>
#include <local.h>
#include <sys_rt.h>
#include "das_clt.h"
#include "das_man.h"


/*	This only works for unipolar and bipolar inputs, not
 *	arbitrary ranges.
 */

int das_to_volt(das_info_typ *pdas, int *pfrom, int n, float *pto)
{
	int i;
   
	for(i = 0; i < n; i++, pfrom++)
	{
		if(pdas->ad_min < 0)
			*pfrom -= (pdas->ad_step - 1.0)/2.0;

		*pto++ = *pfrom * (pdas->ad_max - pdas->ad_min) / pdas->ad_step;
	}
	return(n);
}

/* Use this version if a true input bipolar number is obtained, i.e.
 * when the input numbers may be positive and negative.
 */

int bipolar_das_to_volt(das_info_typ *pdas, int *pfrom, int n, float *pto)
{
	int i;
   
	for(i = 0; i < n; i++, pfrom++)
	{

		*pto++ = *pfrom * (pdas->ad_max - pdas->ad_min) / pdas->ad_step;
	}
	return(n);
}

/*	This only works for unipolar and bipolar outputs, not
 *	arbitrary ranges.
 */

int das_to_step( das_info_typ *pdas, float *pfrom, int n, int *pto )
{
	int i;

	for( i = 0; i < n; i++, pfrom++ )
	{
		*pfrom = bound( pdas->da_min, *pfrom, pdas->da_max );
		*pto++ = (*pfrom - pdas->da_min) * pdas->da_step
					/(pdas->da_max - pdas->da_min);
	}
	return( n );
}


