/**\file	
 *	sys_time.c
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *
 *	DESCRIPTION
 *
 *	Utilities for handling TMS time, SYSTEM time, NEW time, and
 *	COMPOUND time. 
 *
 */

#include <sys_os.h>
#include "local.h"
#include "sys_time.h"
#include "sys_str.h"

#define	ONE_MONTH		MAX_DAYS_PER_MONTH*TENTHS_SEC_PER_DAY

#define	YYMMDD_LENGTH	6
#define DAYS_PER_WEEK	7

/*
 *	Table of elapsed days since the 1989.
 *  Terminates with a zero.
 *  Used in    :	get_sys_time()
 */
static long elapsed_days[] = 
{
	365, 730, 1095, 1461, 1826, 2191, 0
};


/*
 *	Table of days per month for 1989 to 1995.
 *	Terminates with a zero.
 *	Used in    : 	sys2tms_time()
 */

static long calendar[] = 
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	0
};

static long sys_time_from_midnight( long now );
/*	SYNOPSIS 
 * 
 *	#include <sys_time.h>
 *
 *	long get_sys_time()
 *
 *	DESCRIPTION
 *	For BSD and OSK, gets the current time in tenths of a second 
 *	since 1989.
 *
 *	RETURN
 *		total		-	total tenths of a second if the operation  
 *						succeeds
 *		ERROR_LONG	-	if not succeed
 *
 *	Notes:
 *	VALID FROM JAN. 1, 1989 - Oct. 21, 1995.
 *	October 21, 1995 is when the Sun version of a long overflows.
 *	It overflows at 12:00 A.M. October 21, 1995. 
 *
 *	BUGS
 *	The DOS version doesn't give the current time, but
 *	does increment by tenth's of second.
 */

#ifdef DOS
long get_sys_time()
{
	return( (10L*clock()/CLK_TCK) );
}
#else
long get_sys_time()
{
	time_t timer;						/* Type definition */

	long cal_sec, cal_min, cal_hour, cal_day, cal_year, total;
	long days_since_89, tenths_since_89;
	long *pelapsed_days;
	
	struct tm *ptime;					/* Structure definition */
	
	pelapsed_days = elapsed_days;		/* Initialize pointer to */
										/* elapsed days table.   */

	if( time(&timer) == ERROR )			/* Get current local time */
		return( ERROR_LONG );

	ptime = ( struct tm *)localtime( &timer );
		
	cal_sec  = ptime->tm_sec;
	cal_min  = ptime->tm_min; 
	cal_hour = ptime->tm_hour;
	cal_day  = ptime->tm_yday;
	cal_year = ptime->tm_year;    

	cal_sec  *= TENTHS_SEC_PER_SEC;	/* Calculate total tenths since	*/
	cal_min  *= TENTHS_SEC_PER_MIN;	/* the start of the year. 		*/
	cal_hour *= TENTHS_SEC_PER_HR;
	cal_day  *= TENTHS_SEC_PER_DAY;

	total = cal_sec + cal_min + cal_hour + cal_day;

	if( cal_year <  SYS_TIME_MIN_YEAR )		/*	Valid Year check:	 */
		return( ERROR_LONG );				/*	1989 <= year <= 1995 */	
	
	else if( cal_year > SYS_TIME_MAX_YEAR )
	{
		return( ERROR_LONG );
	}
	
	if( cal_year > SYS_TIME_MIN_YEAR )			/* Calculate elapsed */
	{											/* since 1989.		 */
		while(  ( cal_year - 1 ) != SYS_TIME_MIN_YEAR )
		{
			--cal_year;
			++pelapsed_days;

			if( *pelapsed_days == 0 )
				return( ERROR_LONG );
		}
		days_since_89 	= *pelapsed_days;
		tenths_since_89 = days_since_89 * TENTHS_SEC_PER_DAY;
		total += tenths_since_89;

		/* Upper limit. */
		if( ( total >= MAX_SYS_TIME ) || ( total < 0 ) )  
			return( ERROR_LONG );		/* Oct. 21, 1995, check. */
	}
	return( total );
}
#endif

#ifdef DOS

/*	Doesn't allow interrupts, and always returns a zero.
 */

int sleep( wait )
unsigned int wait;
{
	long wait_time;
	long end_time;

	if( wait_time <= 0 )
		return;

	wait_time = wait * 10L;

	inc_sys_time( get_sys_time(), wait_time, &end_time );

	while( cmp_sys_time( end_time, get_sys_time() ) > 0)
		;
	return( 0 );
}
#endif


/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int sys_time_interval( now, parray, n_items )
 *	long now		-	current time in tenths of a second
 *	long *parray	-	pointer to an array of tenths of a 
 *						second, for n_items intervals
 *	int n_items		-	number of elements in parray
 *
 *	DESCRIPTION	
 *	Compute the current time since midnight.
 * 
 *	RETURN
 *		array_index	-	offset into the array pointed to by parray
 *		ERROR		-	if now == ERROR_LONG
 *		ERROR		-	if sys_time_from_midnight == ERROR_LONG
 *
 *	Notes:		VALID FROM JAN. 1989 -  OCT. 21, 1995
 */

int sys_time_interval( now, parray, n_items)
long now;
long *parray;
int n_items;
{

	long current_time;
	int array_index;

	if( ( now == ERROR_LONG ) || ( now < 0L ) ) 	/*Error check */
		return( ERROR );				

	if( ( current_time = sys_time_from_midnight(now) ) == ERROR_LONG )
		return( ERROR );				

	for( array_index = 0; array_index < n_items ; ++array_index, parray++ )
	{
		if( cmp_sys_time( current_time, *parray ) <= 0 )
			break;
	}
	
	if( array_index ==  n_items )		/* Array error check. */
		return( ERROR );

	return( array_index );				/* Return index into parray. */
}



/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	static long sys_time_from_midnight( now )
 *	long now		-	current time in tenths of a second
 *
 *	DESCRIPTION
 *	Calculate tenths of a second since midnight
 *
 *	RETURN
 *		current_time	-	tenths of a second since midnight
 *		ERROR_LONG		-	if diff_sys_time() < 0
 *		ERROR_LONG		-	if now == ERROR_LONG 
 *
 *	Notes:
 *		VALID FROM JAN. 1989 - OCT. 21, 1995
 *		This function should be rewritten in sys_midnight().
 */

static long sys_time_from_midnight( long now )
{
	long tenths_since_midnight;
	long last_total, running_total;
	long this_months_tenths;
	long *pmonths;

	tenths_since_midnight=	0L;
	last_total			=	0L;
	running_total		=	0L;
	this_months_tenths	=	0L;

	pmonths	= &calendar[0];

	if( ( now == ERROR_LONG ) || ( now < 0L ) )
		return( ERROR_LONG );

	if( ONE_MONTH <= now )				/* Calculate elapsed months. */
	{							     
		/* Increment running total by a month until it is >= now,*/
		/* or the month array runs out.							 */

		while( running_total <= now )
		{
			last_total = running_total;

			this_months_tenths = ( *pmonths )*TENTHS_SEC_PER_DAY;

			if(inc_sys_time( running_total,
					this_months_tenths, &running_total ) == ERROR)
				break;

			pmonths++;
			if( *pmonths == 0L)				/* Month error check. 	 */
				return( ERROR_LONG );		/* Should never be true. */
		}
		running_total = last_total;			/* Previous calculation  */
	}										/* is correct.           */ 

	if( TENTHS_SEC_PER_DAY <= now )
	{
		/* Increment running total by a day until it is >= now.  */

		while( running_total <= now )	/* Calculate elapsed days. */
		{
			last_total = running_total;
			if(inc_sys_time( running_total, 
				TENTHS_SEC_PER_DAY, &running_total) == ERROR )
				break;
		}
		running_total = last_total;
	}

	if( diff_sys_time( now, running_total, &tenths_since_midnight ) 
		== ERROR )
	{
		return( ERROR_LONG );	/* Calculate tenths since midnight.*/
	}

	return( tenths_since_midnight );
}	

/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int diff_sys_time( end_time, start_time, pdelta_time )
 *	long end_time		-	as the end time defined by the user.
 *	long start_time		-	as a start time defined by the user.
 *	long *pdelta_time	-	pointer to store difference in time.
 *
 *	DESCRIPTION
 *	Calculates the difference between two times.
 *
 *	RETURN
 *		TRUE		-	valid data
 *		ERROR		-	data negative (out of range)
 *
 */

int diff_sys_time( end_time, start_time, pdelta_time )
long end_time, start_time;
long *pdelta_time;
{
	if( (end_time < 0L) || (start_time < 0L) )	/* Valid time check. */
		return( ERROR );

	*pdelta_time = end_time - start_time;	/* Calculate delta time */
	
	return( TRUE );
}


/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int cmp_sys_time( t1, t2 )
 *	long t1, t2		-	Two system times to be compared.
 *
 *	DESCRIPTION
 *	Compares two system times, which are assumed to be valid.
 *
 *	RETURN
 *		0		-	If the two times are the same.
 *		1		-	If t2 < t1
 *		-1		-	If t1 < t2
 *
 */

int cmp_sys_time( t1, t2 )
long t1, t2;
{
	if( t1 == t2 )
		return( 0 );
	else if( t2 < t1 )
		return( 1 );
	else
		return( -1 );
}


/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int inc_sys_time( time, delta_time, padjusted_time )
 *	long time				-	a user defined time in tenths of a second.
 *	long delta_time			-	a user defined delta time in tenths of a
 *								second.
 *	long *padjusted_time	-	pointer to store the adjusted time.
 *
 *	DESCRIPTION
 *	Accepts a change in time and adds it to time.									
 *
 *	RETURN 
 *		TRUE	-	if time and adjusted_time are valid.
 *		ERROR	-	if time is invalid (negative).
 *		ERROR	-	if adjusted_time is out of range.
 *
 *	Notes:			
 *		THIS PROGRAM IS VALID FROM JAN. 1, 1989 - OCT. 21, 1995.
 */

int inc_sys_time( time, delta_time, padjusted_time )
long time, delta_time;
long *padjusted_time;
{
	if( (time < 0L) || (time == ERROR_LONG) )		/* Valid time check. */
		return( ERROR );
	
	*padjusted_time  = time + delta_time;		/* Increment time */
	
	/* Overflow check */
	if( ( *padjusted_time < 0L ) || ( *padjusted_time > MAX_SYS_TIME ) )
		return( ERROR );

	return( TRUE );
}




/*	SYNOPSIS	
 *
 *	#include <sys_time.h>
 *
 *	int sys2tms_time( time, ptms_time )
 *	long time						-	time in tenths of a second
 *	struct tms_time_str *ptms_time	-	a pointer to the structure:
 *										tms_time
 *
 *	DESCRIPTION
 *	Convert system time into tms_time time.
 *
 *	RETURN
 *		TRUE	-	good data
 *		ERROR	-	data out of range
 *
 *	Notes:			VALID FROM JAN. 1, 1989 THRU OCT. 21, 1995 ONLY.
 */

int sys2tms_time( time, ptms_time )
long time;
struct tms_time_str *ptms_time;
{
	long calc_day;
	long *pcalendar;

	pcalendar = calendar;

	if( (time < 0L) || (time == ERROR_LONG) )	  /* Valid time check. */
		return( ERROR );

	calc_day = time/TENTHS_SEC_PER_DAY;	    /* Calculate time elements */

	time %= TENTHS_SEC_PER_DAY;
 	ptms_time->hour = ( unsigned char ) ( time / TENTHS_SEC_PER_HR );   

	time %= TENTHS_SEC_PER_HR;
	ptms_time->minute = ( unsigned char ) ( time / TENTHS_SEC_PER_MIN );  
	
	time %= TENTHS_SEC_PER_MIN;
	ptms_time->sec = ( unsigned char ) ( time / TENTHS_SEC_PER_SEC );  

	calc_day += 1L;							/* add one for today */
	while( calc_day > *pcalendar )			/* Day of month calculation */
	{
		if( *pcalendar == 0L )
			return( ERROR );
		else
		{
			calc_day  -=  *pcalendar;
			++pcalendar;
		}
	}

	ptms_time->day	=  ( unsigned char ) calc_day;
	return( TRUE );
}


/*	SYNOPSIS 
 *
 *	#include <sys_time.h>
 *
 *	long get_time( ptime, year, month )
 *	void *ptime	-	a pointer to the structure: tms_time
 *	int year	-	current year
 *	int month	-	current month
 *
 *	DESCRIPTION 
 *	Assume that the first component of the structure is a tms_time_str. 
 *	Goes on to calculate the "system time" i.e tenths of seconds elapsed 
 *	since the beginning of 1989. 
 *
 *	RETURN
 *		total_tenth_secs	-	total tenths of a second since the 
 *								beginning of 1989.
 *		ERROR_LONG			-	if the operation not succeed
 *
 *	Notes:
 *		This function should be rewritten, or go away. 
 */	

long get_time( ptime, year, month )
void *ptime;
int year;
int month;
{
	int day, hour, minute, sec;
	int sys_days = 0;
	long total_tenth_secs = 0L; 
	
	if( (year < SYS_TIME_MIN_YEAR) || (year > SYS_TIME_MAX_YEAR) 
		|| ( month < 1 ) || (month > MONTHS_PER_YEAR) )
	{
		return( ERROR_LONG );
	}

	day	 = (int) ( ((struct tms_time_str *) ptime)->day);
	hour = (int) ( ((struct tms_time_str *) ptime)->hour);
	minute	 = (int) ( ((struct tms_time_str *) ptime)->minute);
	sec	 = (int) ( ((struct tms_time_str *) ptime)->sec);	
	
	sys_days = sys_time_days( year, month, day );
	total_tenth_secs = ( ((((long)sys_days*HOURS_PER_DAY+hour)*
	MINS_PER_HOUR+minute)*SECS_PER_MIN+sec)*TENTHS_SEC_PER_SEC );

	if( total_tenth_secs < 0L )		/* overflow check */
		return( ERROR_LONG );
	else
		return( total_tenth_secs );
}





/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int sys_time_days( year, month, day )
 *	int year
 *	int month
 *	int day
 *
 *	DESCRIPTION
 *	Function to calculate the number of days elapsed since the beginning 
 *	of 1989.
 *
 *	RETURN
 *		Number of days elapsed since 1989.
 *
 *	Notes:
 *		This function should be go away or rewritten.
 *		VALID FROM JAN.1, 1989 - OCT. 21, 1995
 */

int sys_time_days( year, month, day )
int year;
int month;
int day;
{
	int i, temp;
	int m_days = 0;
	int y_days = 0;										/*	Assume 1989	*/
	
	if( year > SYS_TIME_MIN_YEAR )
		y_days = (int) elapsed_days[year-SYS_TIME_MIN_YEAR-1];

	 /*	sum the days in the prior months */
	for( i = 0; i < (month-1); i++ )
	{
		temp = (int) calendar[i];

        /* take care of leap year  */
		if( (i == 1) && ( (year % 4) == 0 ) )
			temp = 29;
		m_days += temp;
	}
	return( y_days + m_days + day -1 );
}


#ifdef BSD
/*	SYNOPSIS
 *
 *	#include <sys/time.h>
 *	#include <sys_time.h>
 *
 *	struct tm *yesterday()
 *
 *	DESCRIPTION
 *	Get time from yesterday.
 *	Only for BSD now, since OSK doesn't have gettimeofday().
 *
 *	RETURN
 *		A pointer to the time from a day ago.
 *		NULL pointer if problem.
 */

struct tm *yesterday()
{
	struct tm *py_tm;
	struct timeval time;
	struct timezone zone;
	long yesterday;

	if( gettimeofday( &time, &zone ) == ERROR )
		return( NULL );

	yesterday = time.tv_sec - (SECS_PER_MIN*MINS_PER_HOUR*HOURS_PER_DAY);
	py_tm = localtime( &yesterday );
	return( py_tm );
}
#endif


/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	sys_time_typ time_next( when )
 *
 *	DESCRIPTION
 *	Depending on the argument, calculate the system time at the next:
 *		minute, hour, day, week, or month.
 *
 *	Acceptable arguments are defined in the header file as:
 *		TIME_NEXT_MINUTE, TIME_NEXT_DAY, TIME_NEXT_HOUR, TIME_NEXT_WEEK,
 *		or TIME_NEXT_MONTH.
 *
 *	The beginning of the day is at midnight.
 *	The beginning of the week is on Sunday morning.
 *
 *	RETURN 
 *		The system time for the desired time.
 *		ERROR_LONG	-	if the argument is incorrect, or 
 *						the time cannot be optained.
 */

sys_time_typ time_next( when )
int when;
{
	long get_time();
	time_t timer;
	struct tm *ptime;
	struct tms_time_str tms_time;

	if( time( &timer ) == ERROR )
		return( ERROR_LONG );

	ptime = localtime( &timer );

	switch( when )
	{
	case TIME_NEXT_MINUTE:
		ptime->tm_sec = 0;
		ptime->tm_min += 1;
		break;

	case TIME_NEXT_HOUR:
		ptime->tm_sec = 0;
		ptime->tm_min = 0;
		ptime->tm_hour += 1;
		break;

	case TIME_NEXT_DAY:
		ptime->tm_sec = 0;
		ptime->tm_min = 0;
		ptime->tm_hour = 0;
		ptime->tm_mday += 1;
		break;

	case TIME_NEXT_WEEK:
		ptime->tm_sec = 0;
		ptime->tm_min = 0;
		ptime->tm_hour = 0;

		/*	"Broken down time" uses 0 as the code for Sunday.  */

		ptime->tm_mday += DAYS_PER_WEEK - ptime->tm_wday;
		break;

	case TIME_NEXT_MONTH:
		ptime->tm_sec = 0;
		ptime->tm_min = 0;
		ptime->tm_hour = 0;
		ptime->tm_mday = 1;
		ptime->tm_mon += 1;
		break;

	default:
		return( ERROR_LONG );
	}

	/*	Normalize the time	*/

#ifdef BSD
	if( timelocal( ptime ) == ERROR )
#endif
#ifdef __QNX__
	if( mktime( ptime ) == ERROR )
#endif
#ifdef OSK
	if( mktime( ptime ) == ERROR )
#endif
	{
		return( ERROR_LONG );
	}

	tms_time.sec = ptime->tm_sec;
	tms_time.minute = ptime->tm_min;
	tms_time.hour = ptime->tm_hour;
	tms_time.day = ptime->tm_mday;

	/*	Generate a system time.		*/

	return( get_time(&tms_time, ptime->tm_year, ptime->tm_mon+1) );
}

/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	char *filedate()
 *
 *	DESCRIPTION
 *	Copy a date string which has the usual format of YYMMDD into a buffer.
 *
 *	RETURN
 *	A pointer to a NULL terminated temporary string which has the usual
 *	format of YYMMDD.  You have to copy it if you want to use
 *	or append to it.
 *	NULL pointer if problem.
 */

char *filedate()
{
	time_t timer;
	struct tm *ptime;
	static char date[10];			/* Will hold date in form 901130 		*/
									/* for Nov. 30, 1990					*/

	if( time( &timer ) == ERROR )
		return( NULL );

	ptime = localtime( &timer );
	sprintf( date, "%02d%02d%02d", ptime->tm_year,
		(ptime->tm_mon) + 1, ptime->tm_mday );

	return( date );
}

/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	char *filemonth()
 *
 *	DESCRIPTION
 *	Copy a date string which has the usual format of YYMM01( First
 *	day of current month ) into a buffer.
 *
 *	RETURN
 *		A pointer to a NULL terminated temporary string which has the usual
 *		format of YYMM01( First day of current month ).  You have to copy 
 *		it if you want to use or append to it.
 *		NULL pointer if problem.
 */

char *filemonth()
{
	time_t timer;
	struct tm *ptime;
	static char date[10];			/* Will hold date in form 891101 		*/
									/* for Nov. 30, 1987					*/

	if( time( &timer ) == ERROR )
		return( NULL );

	ptime = localtime( &timer );
	sprintf( date, "%02d%02d01", ptime->tm_year,
		(ptime->tm_mon) + 1 );

	return( date );
}



/*	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	void sec2label( total_sec, pbuff )
 *	long total_sec	-	total seconds to be converted
 *	char *pbuff		-	pointer to buffer to stored the converted string
 *
 *	DESCRIPTION
 *	Convert total second into HH:MM:SS string.
 *
 *	RETURN 
 *		None
 */

void  sec2label( total_sec, pbuff ) 
long total_sec;
char *pbuff;
{
	int hour;
	int minute;
	int second;

	minute = total_sec / SECS_PER_MIN;
	second = total_sec % SECS_PER_MIN;

	hour = minute / MINS_PER_HOUR;
	minute = minute % MINS_PER_HOUR;
	sprintf(pbuff, "%2d:%02d:%02d", hour, minute, second );
}

int read_yymmdd( ps, pyear, pmonth, pday )
char *ps;
int *pyear, *pmonth, *pday;
{
	char *pdate;

	if( (pdate = INDEX( ps, '.' )) == NULL )
		return( ERROR );

	pdate -= YYMMDD_LENGTH;

	if( pdate < ps )
		return( ERROR );

	*pyear = 0;
	*pmonth = 0;
	*pday = 0;

	sscanf( pdate, "%2d%2d%2d", pyear, pmonth, pday );

	if( *pyear < SYS_TIME_MIN_YEAR ||
		*pmonth < 1 || MONTHS_PER_YEAR < *pmonth ||
		*pday < 1 || MAX_DAYS_PER_MONTH < *pday )
	{
		return( ERROR );
	}
	else
		return( TRUE );
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	void time_newtocmp( pnew_time, pjulian, pmil_time )
 *	struct new_time_str *pnew_time
 *	char *pjulian
 *	char *pmil_time
 *
 *	DESCRIPTION
 *	Convert new time to compound time.
 *	mil_time		: HH:MM:SS
 *	julian			: YYMMDD
 *	compound time	: julian, mil_time
 *	new_time		: year, month, day, hour, minute, sec
 *
 *	RETURNS
 *		Nothing
 */

void time_newtocmp( pnew_time, pjulian, pmil_time )
struct new_time_str *pnew_time;
char *pjulian;
char *pmil_time;
{
	sprintf( pjulian, JULTIME_STR, (int) pnew_time->year, 
		(int) pnew_time->month, (int) pnew_time->day );
	
	sprintf( pmil_time, MILTIME_STR, (int) pnew_time->hour,
		(int) pnew_time->minute, (int) pnew_time->sec );
}


/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int time_cmptonew( pjulian, pmil_time, pnew_time )
 *	char *pjulian
 *	char *pmil_time
 *	struct new_time_str *pnew_time
 *
 *	DESCRIPTION
 *	Convert new time to compound time.
 *	mil_time		: HH:MM:SS
 *	julian			: YYMMDD
 *	compound time	: julian, mil_time
 *	new_time		: year, month, day, hour, minute, sec
 *
 *	RETURNS
 *		TRUE		Successful
 *		ERROR		scanf() error
 */

int time_cmptonew( pjulian, pmil_time, pnew_time )
char *pjulian;
char *pmil_time;
struct new_time_str *pnew_time;
{
	int year, month, day;
	int hour, minute, sec;
	int scanned;

	if( (scanned = sscanf( pjulian, JULTIME_STR, &year, 
		&month, &day )) != JULTIME_INTS )
	{
		return( ERROR );
	}
	pnew_time->year		= ( unsigned char ) year;
	pnew_time->month	= ( unsigned char ) month;
	pnew_time->day		= ( unsigned char ) day;

	if( (scanned = sscanf( pmil_time, MILTIME_STR, &hour, 
		&minute, &sec )) != MILTIME_INTS )
	{
		return( ERROR );
	}
	pnew_time->hour		= ( unsigned char ) hour;
	pnew_time->minute	= ( unsigned char ) minute;
	pnew_time->sec		= ( unsigned char ) sec; 
	pnew_time->tenths	= ( unsigned char  ) 0; 
	return( TRUE );
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int time_systonew( sys_time, pnew_time )
 *	long sys_time
 *	struct new_time_str *pnew_time
 *
 *	DESCRIPTION
 *	Convert system time to new time:
 *	new_time : year, month, day, hour, minute, sec, tenths
 *	sys_time : tenths of a second since Jan. 1, 1989 
 *
 *	RETURNS
 *		ERROR	time error
 *		TRUE	if successful
 */

int time_systonew( sys_time, pnew_time )
long sys_time;
struct new_time_str *pnew_time;
{
	
	int month, year;
	long calc_day;
	long *pcalendar;

	pcalendar = calendar;

	/* Valid time check. */
	if( (sys_time < 0L) || (sys_time == ERROR_LONG ) )	
	{
		return( ERROR );
	}

	/* Calculate time elements */
	calc_day = sys_time/TENTHS_SEC_PER_DAY;	

	sys_time %= TENTHS_SEC_PER_DAY;
 	pnew_time->hour = ( unsigned char )
		( sys_time / TENTHS_SEC_PER_HR ); 

	sys_time %= TENTHS_SEC_PER_HR;
	pnew_time->minute = ( unsigned char )
		( sys_time / TENTHS_SEC_PER_MIN );  
	
	sys_time %= TENTHS_SEC_PER_MIN;
	pnew_time->sec = ( unsigned char )
		( sys_time / TENTHS_SEC_PER_SEC );

	sys_time %= TENTHS_SEC_PER_SEC;
	pnew_time->tenths = ( unsigned char )
		( sys_time / TENTHS_SEC_PER_SEC );


	/* add one for today */
	calc_day += 1L;					
	month = FIRST_MONTH;
	year = SYS_TIME_MIN_YEAR;

	/* Day of month calculation */
	while( calc_day > *pcalendar )	
	{
		if( *pcalendar == 0L )
		{
			return( ERROR );
		}
		else
		{
			calc_day  -=  *pcalendar;
			pcalendar++;
			if( ( month % MONTHS_PER_YEAR ) == 0 )
			{
				year++;
				month -= MONTHS_PER_YEAR;
			}
			month++;	
		}
	}

	pnew_time->day	 =  ( unsigned char ) calc_day;
	pnew_time->month =  ( unsigned char ) month;
	pnew_time->year  =  ( unsigned char ) year;
	
	return( TRUE );
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	long time_newtosys( pnew_time )
 *	struct new_time_str *pnew_time
 *
 *	DESCRIPTION
 *	Convert new time to system time.
 *	new_time : year,month,day,hour,minute,sec,tenths
 *	sys_time : tenths of a second since 89,01,01,00,00,00
 *
 *	RETURNS
 *	returns ERROR_LONG on error, else total tenths of a second since
 *	890101, 00:00:00 
 */

long time_newtosys( pnew_time )
struct new_time_str *pnew_time;
{
	int year, month, day, hour, minute, sec, tenths;
	int sys_days;
	long total_tenth_secs; 

	sys_days = 0;
	total_tenth_secs = 0L; 
	
	year  = (int) pnew_time->year;
	month = (int) pnew_time->month;
	day	  = (int) pnew_time->day;
	hour  = (int) pnew_time->hour;
	minute	  = (int) pnew_time->minute;
	sec	  = (int) pnew_time->sec;	
	tenths= (int) pnew_time->tenths;	

	if( (year < SYS_TIME_MIN_YEAR) || (year > SYS_TIME_MAX_YEAR) 
		|| ( month < FIRST_MONTH ) || (month > MONTHS_PER_YEAR) )
	{
		return( ERROR_LONG );
	}
	
	sys_days = sys_time_days( year, month, day );
	total_tenth_secs = ( ((((long)sys_days*HOURS_PER_DAY+hour)*
	MINS_PER_HOUR+minute)*SECS_PER_MIN+sec)*TENTHS_SEC_PER_SEC+tenths );

	/* overflow check */
	if( total_tenth_secs < 0L )
	{
		return( ERROR_LONG );
	}
	return( total_tenth_secs );
}


/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	long time_cmptosys( pjulian, pmil_time )
 *	char *pjulian
 *	char *pmil_time
 *
 *	DESCRIPTION
 *	Convert compound time to system time.
 *	mil_time		: HH:MM:SS
 *	julian			: YYMMDD
 *	compound time	: julian, mil_time
 *	sys_time		: tenths of a second since 890101 000000. 
 *						( Jan. 1, 1989 )
 *
 *	RETURNS
 *	returns ERROR_LONG on error, else total tenths of a second since
 *	890101, 00:00:00 
 */

long time_cmptosys( pjulian, pmil_time )
char *pjulian;
char *pmil_time;
{
	struct new_time_str new_time;
	long sys_time;

	if( time_cmptonew( pjulian, pmil_time, &new_time ) == ERROR )
	{
		return( ERROR_LONG );
	}
	if( ( sys_time = time_newtosys( &new_time )) == ERROR_LONG )
	{
		return( ERROR_LONG );
	}
	return( sys_time );
}


/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int time_systocmp( sys_time, pjulian, pmil_time )
 *	long sys_time
 *	char *pjulian
 *	char *pmil_time
 *
 *	DESCRIPTION
 *	Convert system time to compound time.
 *	mil_time		: HH:MM:SS
 *	julian			: YYMMDD
 *	compound time	: julian, mil_time
 *	sys_time		: tenths of a second since 890101 000000. 
 *						( Jan. 1, 1989 )
 *	RETURNS
 *	error if systonew() returns error, else TRUE. 
 */

int time_systocmp( sys_time, pjulian, pmil_time )
long sys_time;
char *pjulian;
char *pmil_time;
{
	struct new_time_str new_time;
	if( time_systonew( sys_time, &new_time ) == ERROR )
	{
		return( ERROR );
	}
	time_newtocmp( &new_time, pjulian, pmil_time );
	return( TRUE );

}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	void time_newtoold( pnew_time, ptms_time, pyear, pmonth )
 *	struct new_time_str *pnew_time
 *	struct tms_time_str *ptms_time
 *	int *pyear
 *	int *pmonth
 *
 *	DESCRIPTION
 *	Convert new time to old time.
 *	new_time : year,month,day,hour,minute,sec
 *	tms_time : day,hour,minute,sec
 *	old time : year,month,tms_time
 */

void time_newtoold( pnew_time, ptms_time, pyear, pmonth )
struct new_time_str *pnew_time;
struct tms_time_str *ptms_time;
int *pyear;
int *pmonth;
{
	*pyear = (int) pnew_time->year;
	*pmonth = (int) pnew_time->month;
	ptms_time->day = pnew_time->day;
	ptms_time->hour = pnew_time->hour;
	ptms_time->minute = pnew_time->minute;
	ptms_time->sec = pnew_time->sec;
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	void time_oldtonew( ptms_time, year, month, pnew_time )
 *	struct tms_time_str *ptms_time
 *	int year
 *	int month
 *	struct new_time_str *pnew_time
 *
 *	DESCRIPTION
 *	Convert old time to new time.
 *	new_time : year,month,day,hour,minute,sec
 *	tms_time : day,hour,minute,sec
 *	old time : year,month,tms_time
 */

void time_oldtonew( ptms_time, year, month, pnew_time )
struct tms_time_str *ptms_time;
int year;
int month;
struct new_time_str *pnew_time;
{
	pnew_time->year = (unsigned char) year;
	pnew_time->month = (unsigned char) month;
	pnew_time->day = ptms_time->day;
	pnew_time->hour = ptms_time->hour;
	pnew_time->minute = ptms_time->minute;
	pnew_time->sec = ptms_time->sec;
	pnew_time->tenths = ( unsigned char ) 0; 
	
}


/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	long time_oldtosys( ptms_time, year, month )
 *	struct tms_time_str *ptms_time
 *	int year
 *	int month
 *
 *	DESCRIPTION
 *	Convert old time to system time.
 *	sys_time : tenths of a second since 890101 000000. 
 *	tms_time : day,hour,minute,sec
 *	old time : year,month,tms_time
 *
 *	RETURNS
 *	returns ERROR_LONG on error, else total tenths of a second since
 *	890101, 00:00:00 
 */

long time_oldtosys( ptms_time, year, month )
struct tms_time_str *ptms_time;
int year;
int month;
{
	return( get_time( (void*) ptms_time, year, month ));
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	int time_systoold( sys_time, ptms_time, pyear, pmonth )
 *	long sys_time
 *	struct tms_time_str *ptms_time
 *	int *pyear
 *	int *pmonth
 *
 *	DESCRIPTION
 *	Convert system time to old time.
 *	sys_time : tenths of a second since 890101 000000. 
 *	tms_time : day,hour,minute,sec
 *	old time : year,month,tms_time
 *
 *	RETURNS
 *	returns ERROR if time_systonew() returns error, else TRUE.
 */

int time_systoold( sys_time, ptms_time, pyear, pmonth )
long sys_time;
struct tms_time_str *ptms_time;
int *pyear;
int *pmonth;
{
	struct new_time_str new_time;

	if( time_systonew( sys_time, &new_time ) == ERROR )
	{
		return( ERROR );
	}
	time_newtoold( &new_time, ptms_time, pyear, pmonth );
	return( TRUE );
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	void read_julian( pfilename, pjulian )
 *	char *pfilename
 *	char *pjulian
 *
 *	DESCRIPTION
 *	Convert data file name with date to julian date.
 *	file name	: [ccccccccccccc]YYMMDD.ext 
 *	julian		: YYMMDD
 *
 *	RETURNS
 *	if the julian date does not have the proper length, returns ERROR;
 *	otherwise TRUE.
 */

int read_julian( pfilename, pjulian )
char *pfilename;
char *pjulian;
{
	char *proot;
	char fname[MAX_FILENAME_LEN+1];

	fname[0] = END_OF_STRING;
	strcpy( fname, pfilename );

	proot = fname;
	while( (*proot < (char) '0') || (*proot > (char) '9') )
	{
		proot++;
	}	

	str_delchr_bef( fname, *proot );
	str_delchr_aft( fname, '.' );
	str_delchr( fname, '.' );
	if( strlen( fname ) != YYMMDD_LENGTH )
	{	
		return( ERROR );
	}
	strcpy( pjulian, fname );
	return( TRUE );
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	void time_prn_new( pnew )
 *	struct new_time_str *pnew
 *
 *	DESCRIPTION
 *	Display new time.
 *	new_time : year,month,day,hour,minute,sec
 */

void time_prn_new( pnew )
struct new_time_str *pnew;

{
	printf( " %02d/%02d/%02d, %02d:%02d:%02d:%02d\n",
	(int) pnew->year, (int) pnew->month, (int) pnew->day,
	(int) pnew->hour, (int) pnew->minute, (int) pnew->sec , (int) pnew->tenths);
	
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	void time_prn_old( ptms_time, year, month )
 *	struct tms_time_str *ptms_time
 *	int year
 * 	int month
 *
 *	DESCRIPTION
 *	Display old time.
 *	tms_time : day,hour,minute,sec
 *	old time : year,month,tms_time
 */

void time_prn_old( ptms_time, year, month )
struct tms_time_str *ptms_time;
int year;
int month;

{
	printf( " %02d/%02d/%02d, %02d:%02d:%02d\n",
	year, month, (int) ptms_time->day,
	(int) ptms_time->hour, (int) ptms_time->minute, (int) ptms_time->sec );
	
}

/*
 *	SYNOPSIS
 *
 *	#include <sys_time.h>
 *
 *	void time_prn_cmp( pjulian, pmil_time )
 *	char *pjulian
 *	char *pmil_time
 *
 *	DESCRIPTION
 *	Display compound time.
 *	mil_time		: HH:MM:SS
 *	julian			: YYMMDD
 *	compound time	: julian, mil_time
 *
 *	RETURN
 *		Nothing
 */

void time_prn_cmp( pjulian, pmil_time )
char *pjulian;
char *pmil_time;

{
	printf( " %s, %s\n",
		pjulian, pmil_time );
	
}



#ifdef NEW_TIME_TEST

/* Define NEW_TIME_TEST for testing new time functions */ 

#include "getopt.h"
#include "sys_lib.h"

void usage()
{
	printf(" ERROR \n");
	exit( ERROR );
}

void list_arguments( pfilename, pextension, pend_time, pstart_time )
char *pfilename; 
char *pextension; 
char *pend_time, *pstart_time;

{
	printf(" FILE NAME = %s\n  ", pfilename   );
	printf(" EXTENSION = %s\n  ", pextension   );
	printf(" END_TIME  = %s\n  ", pend_time    );
	printf(" ST_TIME   = %s\n  ", pstart_time  );
}


void list_conversion( pjulian, pstart_time, pend_time )
char *pjulian;
char *pstart_time;
char *pend_time;
{
	struct tms_time_str tms_time;
	struct new_time_str new_time;
	long stime, etime, sys_time;
	int year, month;
	
	if( ( stime = time_cmptosys( pjulian, pstart_time )) == ERROR_LONG )
	{
		sys_error("list_conversion", 0, __FILE__, __LINE__ );
		exit( ERROR );
	}

	if( ( etime = time_cmptosys( pjulian, pend_time )) == ERROR_LONG )
	{
		sys_error("list_conversion", 0, __FILE__, __LINE__ );
		exit( ERROR );
	}

	printf(" START SYSTEM TIME : %ld\n", stime );
	printf(" END   SYSTEM TIME : %ld\n", etime );

	pjulian[0] = pstart_time[0] = pend_time[0] = END_OF_STRING;


	if( time_systocmp( stime, pjulian, pstart_time ) == ERROR )
	{
		sys_error("list_conversion", 0, __FILE__, __LINE__ );
		exit( ERROR );
	}

	printf(" \nSTART TIME CMP : ");
	time_prn_cmp( pjulian, pstart_time );

	if( time_systocmp( etime, pjulian, pend_time ) == ERROR )
	{
		sys_error("list_conversion", 0, __FILE__, __LINE__ );
		exit( ERROR );
	}

	printf(" \nEND TIME CMP : ");
	time_prn_cmp( pjulian, pend_time );

	if( time_systoold( etime, &tms_time, &year, &month ) == ERROR )
	{
		sys_error("list_conversion", 0, __FILE__, __LINE__ );
		exit( ERROR );
	}
 
	printf(" \nEND TIME OLD : ");
	time_prn_old( &tms_time, year, month );

	time_oldtonew( &tms_time, year, month, &new_time );
	printf(" \nEND TIME NEW : ");
	time_prn_new( &new_time );	

	if( (sys_time = time_oldtosys( &tms_time, year, month )) == ERROR )  
	{
		sys_error("list_conversion", 0, __FILE__, __LINE__ );
		exit( ERROR );
	}

	printf(" END   SYSTEM TIME : %ld\n", sys_time );

	sys_time++;

	if( time_systonew( sys_time, &new_time ) == ERROR )
	{
		sys_error("list_conversion", 0, __FILE__, __LINE__ );
		exit( ERROR );
	} 

	printf("\n END TIME NEW ");
	time_prn_new( &new_time );

}
	
int main( argc, argv )
int argc;
char *argv[];
{ 

	char filename[MAX_FILENAME_LEN+1]; 
	char extension[MAX_FILENAME_LEN+1]; 
	char julian[JULTIME_LEN+1];
	char end_time[MILTIME_LEN+1], start_time[MILTIME_LEN+1];
	int option;


	filename[0] = END_OF_STRING;
	extension[0] = END_OF_STRING;
	end_time[0] = END_OF_STRING;
	start_time[0] = END_OF_STRING;
	julian[0] = END_OF_STRING;

	while( ( option = getopt( argc, argv, "b:e:" )) != EOF )
	{
		switch( option )
		{
			case 'b':
				strcpy( start_time, optarg );
				break;

			case 'e':
				strcpy( end_time, optarg );
				break;

			default:
				usage();
		}
	}	

	if( argv[optind] == NULL )
	{
		usage();
	}

	strcpy( extension, argv [optind] ); 
	str_delchr_bef( extension, '.' );
	
	if((argv [optind] != NULL) && (extension[0] == '.'))
	{
		strcpy( filename, argv [optind] );
		list_arguments( filename, extension, end_time, start_time );
	}
	else
	{
		usage();
	}	

	read_julian( filename, julian );
	printf(" FILE DATE = %s \n", julian );
	list_conversion( julian, start_time, end_time );

	exit( FALSE );
}

#endif
	

/*********************************************************************/

/* Define TMS_TIME_TEST for testing tms time functions */ 

#ifdef TMS_TIME_TEST

#include "getopt.h"
#include "sys_lib.h"

long time_since_midnight[] = 
{
	0,	
	36000,	72000,	108000,	144000,
	180000,	216000, 252000,	288000,
	324000,	360000, 396000,	432000,
	468000,	504000, 540000,	576000,
	612000,	648000, 684000,	720000,
	756000,	792000, 828000,	864000
};

main()
{
	long time, diff_time, adjust_time;
    int  month, year, index, total_sec;
	char *pdate;
	char pbuff[20];
	struct tms_time_str tms_time;
	struct tm *pyesterday;

	month = 6;
	year = 90;     /* 1990 */

	time = get_sys_time();
	printf( "time: get_sys_time() %ld ", time );

	sys2tms_time( time, &tms_time );
	printf( "[%2d  %2d:%2d:%2d] \n", tms_time.day,
			tms_time.hour, tms_time.minute, tms_time.sec );

    index = sys_time_interval( time, time_since_midnight );
    printf( "time_since_midnight = %ld\n", time_since_midnight[index - 1] );

	diff_sys_time( time+200, time, &diff_time );
    printf( "diff_time = %ld\n", diff_time );

	inc_sys_time( time, 200, &adjust_time );
	printf( "adjust_time = %ld\n", adjust_time );

	if( ( time = get_time( &tms_time, year, month )) == ERROR_LONG )
		exit(1); 
	printf( "time : get_time() : %ld\n ", time );

	pyesterday = yesterday();
	printf("yesterday time:  ");
	printf("%d/%d/%d  ", pyesterday->tm_mon+1, pyesterday->tm_mday, 
			pyesterday->tm_year );
	printf("%d:%d:%d\n", pyesterday->tm_hour, pyesterday->tm_min, 
			pyesterday->tm_sec );

	pdate = filedate();
	printf("date = %s\n", pdate );

	pdate = filemonth();
	printf("date = %s\n", pdate );

	total_sec = 432000/10;
	sec2label( total_sec, pbuff );
	printf("pbuff = %s\n", pbuff );
}
#endif

