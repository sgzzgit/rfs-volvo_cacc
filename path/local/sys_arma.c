/**\file	
 *
 * Copyright (c) 2006   Regents of the University of California
 */

#include <malloc.h>
#include <signal.h>

#include "local.h"
#include "sys_mem.h"
#include "sys_rt.h"
#include "sys_arma.h"

/**
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_arma.h>
 *
 *	arma_11_typ *arma_11_init( double coef_y1, double coef_x0, double coef_x1,
 *		int bound_type, double lower_bound, double upper_bound, double init_value );
 *
 *	coef_y1		-The filter coefficients for the respective terms of
 *	coef_x0		the autoregressive/moving average filter.  See below.
 *	coef_x1
 *
 *	bound_type	-The desired value bounding, as given in sys_arma.h
 *
 *	lower_bound	-The bound values, if bounding is in effect.
 *	upper_bound
 *
 *	init_value	-The initial filter variable value. i.e. X[0] and Y[0].
 *
 *	DESCRIPTION
 *	This call sets up parameters for first order autoregressive, moving average
 *	filtering, i.e. ARMA( 1, 1 ).  On subsequent calls to arma_11_get_value(),
 *	new values of X are inputted by the user, and new values of Y are outputted.
 *
 *	Without bounding, the output value at time step k is given by:
 *
 *		Y[k] = coef_y1 * Y[k-1] + coef_x0 * X[k] + coef_x1 * X[k-1]
 *
 *	If desired, input bounding will constrain the value of X[ k ] which is used
 *	and stored.  Output bounding will contstrain the value of Y[ k ] which is
 *	returned.  Absolute bounding will contrain the allowable values of
 *	X or Y.  Relative bounding will contrain relative changes in X or Y, as
 *	compared to prior values of X or Y.
 *
 *	With real-time data, the effect of the filter, of course, depends on the
 *	time interval at which the arma_11_get_value() call is made.
 *
 *	RETURN
 *
 *	non-NULL	-	A pointer to the filter.
 *	NULL		-	If there is a memory problem, or the bound_type is bad.
 */

arma_11_typ *arma_11_init( double coef_y1, double coef_x0, double coef_x1,
	int bound_type, double lower_bound, double upper_bound, double init_value )
{
	arma_11_typ *pfilt;

	if( (pfilt = MALLOC( sizeof( arma_11_typ ) )) == NULL )
		return( NULL );

	pfilt->coef_y1 = coef_y1;
	pfilt->coef_x0 = coef_x0;
	pfilt->coef_x1 = coef_x1;
	pfilt->lower_bound = lower_bound;
	pfilt->upper_bound = upper_bound;
	arma_11_set_value( pfilt, init_value );

	switch( bound_type )
	{
	case ARMA_BOUND_NONE:
	case ARMA_BOUND_INPUT_ABS:
	case ARMA_BOUND_INPUT_REL:
	case ARMA_BOUND_OUTPUT_ABS:
	case ARMA_BOUND_OUTPUT_REL:
		pfilt->bound_type = bound_type;
		break;

	default:
		FREE( pfilt );
		return( NULL );
	}
	return( pfilt );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_arma.h>
 *
 *	void arma_11_set_value( arma_11_typ *pfilt, double x_value );
 *
 *	pfilt	-	A filter created by the arma_11_init() call.
 *	x_value	-	The desired state of the filter.
 *
 *	DESCRIPTION
 *	Given an initialized filter, this call sets the existing state of
 *	the filter, i.e. X[0], and Y[0].  It is safe to call this function
 *	multiple times.  Each call replaces the existing state and no
 *	filtering takes place.
 *
 *	RETURN
 *	none.
 *
 *	BUGS
 *	There is no diagnostic if the filter is bad (i.e. NULL).
 */

void arma_11_set_value( arma_11_typ *pfilt, double x_value )
{
	if( pfilt != NULL )
	{
		pfilt->x1 = x_value;
		pfilt->y1 = x_value;
	}
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_arma.h>
 *
 *	double arma_11_get_value( arma_11_typ *pfilt, double x_value );
 *
 *	pfilt	-	A filter created by the arma_11_init() call.
 *	x_value	-	A new input value to the filter.
 *
 *	DESCRIPTION
 *	Given an initialized filter, this call takes a new input value X from
 *	the user, and returns an output value Y from the filter.
 *	For the filter equation see the documentation for arma_11_init().
 *	Each call to arma_11_get_value() represents a new time step.
 *
 *	If input bounding (relative or absolute) is in effect, the change in
 *	x_value or range of x_value might be constrained.  Similarly, output
 *	bounding constrains the calculated value returned by the filter.
 *
 *	RETURN
 *	The output value of the filter, after appropriate bounding.
 *
 *	BUGS
 *	If the filter is bad (i.e. NULL), the input value is returned.
 */

double arma_11_get_value( arma_11_typ *pfilt, double x_value )
{
	double y_value;

	if( pfilt == NULL )
		return( x_value );
	else
	{
		switch( pfilt->bound_type )
		{
		case ARMA_BOUND_INPUT_ABS:
			x_value = bound( pfilt->lower_bound, x_value,
							pfilt->upper_bound );
			break;

		case ARMA_BOUND_INPUT_REL:
			x_value = bound( pfilt->x1 + pfilt->lower_bound,
							x_value, pfilt->x1 + pfilt->upper_bound );
			break;

		case ARMA_BOUND_OUTPUT_ABS:
		case ARMA_BOUND_OUTPUT_REL:
		case ARMA_BOUND_NONE:
		default:
			break;
		}

		y_value =  pfilt->coef_y1 * pfilt->y1 + pfilt->coef_x0 * x_value
							+ pfilt->coef_x1 * pfilt->x1;

		switch( pfilt->bound_type )
		{
		case ARMA_BOUND_OUTPUT_ABS:
			y_value = bound( pfilt->lower_bound, y_value,
							pfilt->upper_bound );
			break;

		case ARMA_BOUND_OUTPUT_REL:
			y_value = bound( pfilt->x1 + pfilt->lower_bound,
							y_value, pfilt->x1 + pfilt->upper_bound );
			break;

		case ARMA_BOUND_INPUT_ABS:
		case ARMA_BOUND_INPUT_REL:
		case ARMA_BOUND_NONE:
		default:
			break;
		}

		pfilt->y1 = y_value;
		pfilt->x1 = x_value;
		return( y_value );
	}
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_arma.h>
 *
 *	bool_typ arma_11_done( arma_11_typ *pfilt );
 *
 *	pfilt	-	A filter created by the arma_11_init() call.
 *
 *	DESCRIPTION
 *	This releases the memory used by the filter.
 *
 *	RETURN
 *	TRUE	-	If the memory is released.
 *	FALSE	-	If the pointer is invalid (NULL).
 *
 */

bool_typ arma_11_done( arma_11_typ *pfilt )
{
	if( pfilt == NULL )
		return( FALSE );
	else
	{
		FREE( pfilt );
		return( TRUE );
	}
}

#ifdef TEST_1

#include <stdio.h>
#include <stdlib.h>

#define COEFF			0.9
#define LOWER_BOUND		(-10.0)
#define UPPER_BOUND		10.0
#define INIT_VALUE		0.0

main()
{
	double value;
	arma_11_typ *pfilt;

	if( (pfilt = arma_11_init( 0.0, COEFF, 1.0 - COEFF, ARMA_BOUND_INPUT_REL,
		LOWER_BOUND, UPPER_BOUND, INIT_VALUE )) == NULL )
	{
		perror( "arma_11_init()" );
		exit( EXIT_FAILURE );
	}

	while( scanf( "%lf", &value ) == 1 )
		printf( "%lf\n", arma_11_get_value( pfilt, value ) );

	arma_11_done( pfilt );
	exit( EXIT_SUCCESS );
}
#endif

#ifdef TEST_2

#include <stdio.h>
#include <stdlib.h>

#define MANIFOLD_FILTER			0.10
#define CONTROL_STARTUP_TIME	0.040
#define INTERVAL				0.020

static void old_filter( double time_control, double value )
{
	static double wr_old = 0.0;
	static double preceed_vel_old, preceed_vel_oldp;
	static double ma_old, mades_old, pm_flt;
	static double wr_filt;

	double wr, wr_raw;
	double preceed_vel;
	double pm, ma, mades;

	wr = value;

	if( 50.0 < wr )
		wr = 0.0;

	if( time_control < CONTROL_STARTUP_TIME )
	{
		wr_old = wr;
		wr_filt = wr;
	}
	wr_raw = bound( wr_old - 0.1, wr, wr_old + 0.1 );
	wr_old = wr_raw;
	wr_filt = 0.1 * wr_raw + 0.9 * wr_filt;
	printf( "%6.3lf ", wr_filt );

	if( time_control < CONTROL_STARTUP_TIME )
	{
		preceed_vel_old = value;
		preceed_vel_oldp = value;
	}

	preceed_vel = bound( preceed_vel_oldp - 0.1, value,
					preceed_vel_oldp + 0.1 );

	preceed_vel_oldp = preceed_vel;
	preceed_vel = 0.1 * preceed_vel + 0.9 * preceed_vel_old;
	preceed_vel_old = preceed_vel;
	printf( "%6.3lf ", preceed_vel );

	mades = value;
	ma = value;
	pm = value;

	/* filtering ma and mades */

	if( time_control < CONTROL_STARTUP_TIME )
	{
		ma_old = ma;
		mades_old = mades;
		pm_flt = pm;
	}

	ma = MANIFOLD_FILTER * ma + (1.0 - MANIFOLD_FILTER) * ma_old;
	mades = MANIFOLD_FILTER * mades
			+ (1.0 - MANIFOLD_FILTER) * mades_old ;
	ma_old = ma;
	mades_old = mades;
	pm_flt = MANIFOLD_FILTER * pm + (1.0 - MANIFOLD_FILTER) * pm_flt;
	printf( "%6.3lf %6.3lf %6.3lf\n", ma, mades, pm_flt );
}

main()
{
	double value, time;;
	arma_11_typ *pwr_filt;
	arma_11_typ *ppv_filt;
	arma_11_typ *pma_filt;
	arma_11_typ *pmades_filt;
	arma_11_typ *ppm_filt;

	if( ((pwr_filt = arma_11_init( 0.9, 0.1, 0.0,
			ARMA_BOUND_INPUT_REL, -0.1, 0.1, 0.0 )) == NULL)
		|| ((ppv_filt = arma_11_init( 0.9, 0.1, 0.0,
			ARMA_BOUND_INPUT_REL, -0.1, 0.1, 0.0 )) == NULL)
		|| ((pma_filt = arma_11_init( 0.9, 0.1, 0.0,
			 ARMA_BOUND_NONE, -0.1, 0.1, 0.0 )) == NULL)
		|| ((pmades_filt = arma_11_init( 0.9, 0.1, 0.0,
			 ARMA_BOUND_NONE, -0.1, 0.1, 0.0 )) == NULL)
		|| ((ppm_filt = arma_11_init( 0.9, 0.1, 0.0,
			 ARMA_BOUND_NONE, -0.1, 0.1, 0.0 )) == NULL) )
	{
		perror( "arma_11_init()" );
		exit( EXIT_FAILURE );
	}

	time = 0.0;
	while( scanf( "%lf", &value ) == 1 )
	{
		if( time < CONTROL_STARTUP_TIME )
		{
			arma_11_set_value( pwr_filt, value );
			arma_11_set_value( ppv_filt, value );
			arma_11_set_value( pma_filt, value );
			arma_11_set_value( pmades_filt, value );
			arma_11_set_value( ppm_filt, value );
		}
		printf( "%6.3lf ", arma_11_get_value( pwr_filt, value ) );
		printf( "%6.3lf ", arma_11_get_value( ppv_filt, value ) );
		printf( "%6.3lf ", arma_11_get_value( pma_filt, value ) );
		printf( "%6.3lf ", arma_11_get_value( pmades_filt, value ) );
		printf( "%6.3lf ", arma_11_get_value( ppm_filt, value ) );
		old_filter( time, value );
		time += INTERVAL;
	}

	arma_11_done( pwr_filt );
	arma_11_done( ppv_filt );
	arma_11_done( pma_filt );
	arma_11_done( pmades_filt );
	arma_11_done( ppm_filt );
	exit( EXIT_SUCCESS );
}

#endif


