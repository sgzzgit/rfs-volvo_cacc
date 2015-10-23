/**\file      
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *      The routines in this file provide the capability for time-dependent
 *      functions, such for longitudinal velocity profiles, or open-loop
 *      lateral control.
 *
 */

#include <sys_os.h>

#include "local.h"
#include "sys_tok.h"
#include "sys_list.h"
#include "sys_lib.h"
#include "sys_mem.h"
#include "sys_rt.h"
#include "sys_trk.h"
#include "timestamp.h"

static str_tok_typ key_table[] =
{
	{"sin",			SINUSOID_TYPE},
	{"SIN",			SINUSOID_TYPE},
	{"cosine",		COSINE_TYPE},
	{"COSINE",		COSINE_TYPE},
	{"linear",		LINEAR_TYPE},
	{"LINEAR",		LINEAR_TYPE},
	{"quadratic",		QUADRATIC_TYPE},
	{"QUADRATIC",		QUADRATIC_TYPE},
	{"ramp",		RAMP_TYPE},
	{"RAMP",		RAMP_TYPE},
	{"manual",		MANUAL_TYPE},
	{"MANUAL",		MANUAL_TYPE},
	{"smooth_sin",		SMOOTH_SIN_TYPE},
	{"SMOOTH_SIN",		SMOOTH_SIN_TYPE},
	{"smooth_ramp",		SMOOTH_RAMP_TYPE},
	{"SMOOTH_RAMP",		SMOOTH_RAMP_TYPE},
	{"",			END_OF_WORD_LIST}
};

static dl_head_typ *trk_file( char *pfilename);
static bool_typ trk_param( dl_head_typ *phead, char *pbuff );
static float trk_value( double duration, profile_typ *pprofile);

/**	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *	#include <sys_trk.h>
 *
 *	trk_profile_typ *trk_init( char *pname, float init_value );
 *
 *	pname		-The name of the file containing a profile
 *					definition.
 *
 *	init_value	-An initial value for the profile.  Used
 *			for ramps and the first derivative calculation.
 *
 *	DESCRIPTION
 *	trk_init() parses a text file which defines a list of
 *	time-dependent functions.  This sequence of functions
 *	can be used to define things such as desired velocity
 *	profiles, or open-loop control activity.
 *
 *	The following documentation describes the contents of
 *	the file with respect to defining longitudinal velocity
 *	profiles.
 *
 *	Note that there is no checking for continuity
 *	across the piece-wise functions, and minimal checking for
 *	proper definition of functions and parameters.
 *
 *	A '#' character in the input file indicates that the remainder
 *	of the line is a comment.  Examples of function formats are in
 *	the sample track.dat.
 *
 *	The function definition syntax follows the form:
 *
 *	function_type	function_duration	parameters ...
 *
 *	Function types are described below.
 *	In all cases, t is the time duration within an individual
 *	function, and 0 <= t < function_duration.
 *
 *	ramp		-Ramp from the current velocity to the given
 *			end velocity. Useful for getting up to speed, 
 *			shutting down, and connecting discontinous profiles.
 *
 *			V(t) =  Vbegin + (Vend - Vbegin)*t/function_duration
 *
 *		A example entry, which goes from the current velocity to 8 m/sec
 *		over 5 seconds:
 *
 *		ramp	5.0		8.0
 *
 *	linear		-Linear profile with given offset and slope.
 *
 *			V(t) =  a0 + a1*t
 *
 *		The parameters are a0 and a1, so a sample entry,
 *		which goes from 6 m/sec to 8.5 ms/sec over 5 seconds is:
 *
 *		linear	5.0		6.0		0.5
 *
 *	quadratic	-Second order profile with given coefficients.
 *			The parameters are a0, a1, and a2.
 *
 *			V(t) =  a0 + a1*t + a2 * t^2
 *
 *	sin and cosine	-The given function, with specified offset,
 *			amplitude, period [sec], and phase [rad].
 *
 *			V(t) =  offset + amplitude*sin( 2*PI*t/period + phase )
 *
 *	smooth_sin	-Smooth sinusoidal profile, with specified
 *				end velocity, period [sec], and phase [rad].
 *				The offset is given by the current velocity.
 *
 *			V(t) = Vbegin + (Vend - Vbegin)*t/function_duration
 *			- (Vend - Vbegin)*period/( 2*PI*function_duration )
 *						* sin( 2*PI*t/period + phase )
 * 
 *              A example entry, which goes from the current velocity to 8 m/sec
 *              over 5 seconds:
 *
 *              smooth_sin    5.0	8.0	5.0	0.0 
 *
 *      smooth_ramp      - Smooth end of ramp profile, with specified
 *                          end velocity, period [sec], and phase [rad].
 *                          The offset is given by the current velocity.
 *
 *                         V(t) = Vbegin + (Vend - Vbegin)*t/function_duration
 *                          + (Vend - Vbegin)*period/( 2*PI*function_duration )
 *                          * sin( 2*PI*t/period + phase )
 *
 *              A example entry, which goes from the current velocity to 8 m/sec
 *              over 5 seconds:
 *
 *              smooth_ramp    5.0	8.0	10.0	0.0
 * 
 *	manual		-The vehicle is assumed to be driven manually.
 *				V(t) =  0.0
 *
 *	A sample file, which provides a constant 8 m/sec velocity profile,
 *	followed by two cycles of acceleration to 9.4 m/sec follows:
 *
 *	# 20 mph with cosine
 *	linear		5.0	8.0	0.0
 *	cosine		20.0	8.7	-0.7	10.0	0.0
 *
 *	RETURN
 *	NULL		-If there is insufficient memory, or a parameter
 *				in the configuration file is incorrect.
 *	non-NULL	-If the initialization succeeds.
 *
 */

trk_profile_typ *trk_init( char *pname, float init_value )
{
	trk_profile_typ *ptrack;
	profile_typ *pprofile;

	if( (ptrack = MALLOC( sizeof( trk_profile_typ )) ) == NULL )
		return( NULL );

	ptrack->phead = NULL;
	ptrack->prev_value = init_value;

	if( (ptrack->phead = trk_file( pname )) == NULL )
		return( NULL );

	ptrack->time_0 = get_sec_clock();
	ptrack->prev_time = ptrack->time_0;
	ptrack->pcurrent = ptrack->phead->pfirst;
	pprofile = (profile_typ *) ptrack->pcurrent->pitem;

	if( pprofile->type == RAMP_TYPE ) 
		pprofile->param.ramp.start = ptrack->prev_value;
	else if( (pprofile->type == SMOOTH_SIN_TYPE ) || (pprofile->type ==
			SMOOTH_RAMP_TYPE) )
		pprofile->param.smooth.start = ptrack->prev_value;

	return( ptrack );
}

/**	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *	#include <sys_trk.h>
 *
 *	void trk_done( trk_profile_typ *ptrack );
 *
 *	ptrack	-Pointer to the tracking profile.
 *
 *	DESCRIPTION
 *	This release all system resources associated with the
 *	given tracking profile.
 *
 *	RETURN
 *	none.
 */

void trk_done( trk_profile_typ *ptrack )
{
	if( ptrack == NULL )
		return;

	if( ptrack->phead )
		dl_free( ptrack->phead );

	FREE( ptrack );
}

/**	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *	#include <sys_trk.h>
 *
 *	bool_typ trk_run( trk_profile_typ *ptrack,
 *			float *pvalue, float *pderiv );
 *
 *	ptrack	-	Pointer to the tracking profile.
 *	pvalue	-	Output buffer for the current value.
 *	pderiv	-	Output buffer for the current derivative.
 *
 *	DESCRIPTION
 *	This call obtains the value and derivative for the
 *	current time, of the user defined profile.
 *
 *	RETURN
 *	TRUE	-	If current values are returned to the user.
 *	FALSE	-	If the user defined profile has been completed.
 */

bool_typ trk_run( trk_profile_typ *ptrack, float *pvalue, float *pderiv )
{
	profile_typ *pprofile;
	double duration;
	double now;
	float value;

	pprofile = (profile_typ *) ptrack->pcurrent->pitem;

	now = get_sec_clock();
	duration = now - ptrack->time_0;

	if( pprofile->duration < duration )
	{
		if( (ptrack->pcurrent = ptrack->pcurrent->pnext) == NULL )
			return( FALSE );

		pprofile = (profile_typ *) ptrack->pcurrent->pitem;
		ptrack->time_0 = get_sec_clock();
		duration = 0.0;

		if( pprofile->type == RAMP_TYPE )
			pprofile->param.ramp.start = ptrack->prev_value;
		else if( (pprofile->type == SMOOTH_SIN_TYPE ) || (pprofile->type
			== SMOOTH_RAMP_TYPE) )
			pprofile->param.smooth.start = ptrack->prev_value;
	}

	value = trk_value( duration, pprofile );
	*pderiv = (value - ptrack->prev_value)/(now - ptrack->prev_time);
	*pvalue = value;
	ptrack->prev_value = value;
	ptrack->prev_time = now;
	return( TRUE );
}

static dl_head_typ *trk_file( char *pfilename )
{
	char buffer[MAX_LINE_LEN + 1];
	dl_head_typ *phead;
	FILE *pfile;
	int linecount;

	if( (pfile = fopen( pfilename, "r" )) == NULL )
		return( NULL );

	if( (phead = dl_create()) == NULL )
	{
		fclose( pfile );
		return( NULL );
	}

	for( linecount = 0; readline( pfile, buffer, MAX_LINE_LEN + 1 ) != ERROR;
		linecount++ )
	{
		if( trk_param( phead, buffer ) == FALSE )
		{
			linecount = 0;
			break;
		}
	}

	fclose( pfile );
	if( linecount == 0 )
	{
		dl_free( phead );
		return( NULL );
	}
	else
		return( phead );
}

bool_typ trk_param( dl_head_typ *phead, char *pbuff )
{
	char keyword[MAX_LINE_LEN+1];
	profile_typ buffer;

	if( sscanf( pbuff, "%s%f", keyword, &buffer.duration ) != 2 )
		return( FALSE );

	if( tok_find_code( key_table, keyword, (int *) &buffer.type ) == FALSE )
		return( FALSE );

	if( buffer.duration <= 0.0 )
		return( FALSE );

	/*	The assignment of keyword and buffer.duration could
	 *	be suppressed, but seems to cause a NULL assignment with
	 *	the 4.21 compiler.
	 */

	switch( buffer.type )
	{
	case MANUAL_TYPE:
		if( sscanf( pbuff, "%s%f",
			keyword,
			&buffer.duration ) != 2 )
		{
			return( FALSE );
		}
		break;

	case SINUSOID_TYPE:
	case COSINE_TYPE:
		if( sscanf( pbuff, "%s%f%f%f%f%f",
			keyword,
			&buffer.duration,
			&buffer.param.sinusoid.offset,
			&buffer.param.sinusoid.amplitude,
			&buffer.param.sinusoid.period,
			&buffer.param.sinusoid.phase ) != 6 )
		{
			return( FALSE );
		}
		break;

	case LINEAR_TYPE:
		if( sscanf( pbuff, "%s%f%f%f",
			keyword,
			&buffer.duration,
			&buffer.param.linear.a0,
			&buffer.param.linear.a1 ) != 4 )
		{
			return( FALSE );
		}
		break;

	case QUADRATIC_TYPE:
		if( sscanf( pbuff, "%s%f%f%f%f",
			keyword,
			&buffer.duration,
			&buffer.param.quadratic.a0,
			&buffer.param.quadratic.a1,
			&buffer.param.quadratic.a2 ) != 5 )
		{
			return( FALSE );
		}
		break;

	case RAMP_TYPE:
		if( sscanf( pbuff, "%s%f%f",
			keyword,
			&buffer.duration,
			&buffer.param.ramp.target ) != 3 )
		{
			return( FALSE );
		}
		break;

	case SMOOTH_SIN_TYPE:
	case SMOOTH_RAMP_TYPE:
		if( sscanf( pbuff, "%s%f%f%f%f",
			keyword,
			&buffer.duration,
			&buffer.param.smooth.target,
			&buffer.param.smooth.period,
			&buffer.param.smooth.phase ) != 5 )
		{
			return( FALSE );
		}
		break;

	default:
		return( FALSE );
	}

	if( dl_add_dup( phead, &buffer, sizeof( profile_typ ), FALSE ) == FALSE )
		return( FALSE );
	else
		return( TRUE );
}

float trk_value( double duration, profile_typ *pprofile )
{
	float velocity;

	switch( pprofile->type )
	{
	case MANUAL_TYPE:
		velocity = MANUAL_VELOCITY;
		break;

	case SINUSOID_TYPE:
		velocity = pprofile->param.sinusoid.offset 
		+ pprofile->param.sinusoid.amplitude
		* sin( 2.0 * PI * duration / pprofile->param.sinusoid.period
			+ pprofile->param.sinusoid.phase );
		break;

	case COSINE_TYPE:
		velocity = pprofile->param.sinusoid.offset 
		+ pprofile->param.sinusoid.amplitude
		* cos( 2.0 * PI * duration / pprofile->param.sinusoid.period
				+ pprofile->param.sinusoid.phase );
		break;

	case LINEAR_TYPE:
		velocity = pprofile->param.linear.a0 
				+ duration * pprofile->param.linear.a1;
		break;

	case QUADRATIC_TYPE:
		velocity = pprofile->param.quadratic.a0 
		+ duration * pprofile->param.quadratic.a1
		+ duration * duration * pprofile->param.quadratic.a2;
		break;

	case RAMP_TYPE:
		velocity = pprofile->param.ramp.start
		+ (pprofile->param.ramp.target - pprofile->param.ramp.start)
		* duration/pprofile->duration;
		break;

	case SMOOTH_SIN_TYPE:
		velocity = pprofile->param.smooth.start
		+ (pprofile->param.smooth.target - pprofile->param.smooth.start)
		* duration/pprofile->duration
		- (pprofile->param.smooth.target - pprofile->param.smooth.start)
		* (pprofile->param.smooth.period/pprofile->duration) / (2.0 *PI)
		* sin( 2.0 * PI * duration / pprofile->param.smooth.period
		+ pprofile->param.smooth.phase ); 

		break;

	case SMOOTH_RAMP_TYPE:
       		velocity = pprofile->param.smooth.start
                + (pprofile->param.smooth.target - pprofile->param.smooth.start)
                * duration/pprofile->duration
                + (pprofile->param.smooth.target - pprofile->param.smooth.start)
                * (pprofile->param.smooth.period/pprofile->duration) / (2.0 *PI)
                * sin( 2.0 * PI * duration / pprofile->param.smooth.period
                + pprofile->param.smooth.phase );
        	break;
	
	default:
		velocity = 0.0;
		return( FALSE );
	}
	return( velocity );
}

