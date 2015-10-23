/**\file
 *
 *	local.h
 *
 *	Miscelaneous definitions used in legacy AVCS QNX4 code
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 */

#ifndef PATH_LOCAL_H
#define PATH_LOCAL_H	// so won't be included twice
#include <math.h> /* for fabs() */

#define	TRUE				(1)
#define	FALSE				(0)
#define ERROR				(-1)	
#define ERROR_LONG			(-1L)	
#define BAD_VALUE_FLOAT		(-1.0)
#undef NULL
#define NULL				((void *)0)

#define STDIN				0						/* Path names		*/
#define STDOUT				1
#define STDERR				2

#define TYPE_TXT			0
#define TYPE_INT			1
#define TYPE_FLOAT			2
#define TYPE_DOUBLE			3
#define TYPE_LONG			4
#define TYPE_CHAR			5
#define TYPE_STRING			6

#define END_OF_STRING		((char)'\0')
#define EMPTY_STRING		""

#define FILE_STREAM_APPEND	"a+"
#define FILE_STREAM_WRITE	"w+"
#define FILE_STREAM_READ	"r"

#define BINARY_READ     	"r"
#define	SEEK_SET			0
#define	SEEK_CUR			1
#define	SEEK_END			2
#define INDEX(ps,c)			index(ps,c)

#define BELL				((char) 0x07)
#define CR					((char) 0x0d)
#define LF					((char) 0x0a)
#define SPACE				((char) 0x20)
#define HYPHEN				((char) '-')

#define USR_NAME_SIZE		31
#define MAX_SYS_TIME_LEN	20
#define MAX_FILENAME_LEN	27
#define MAX_LINE_LEN		132
#define MAXHOSTNAMELEN		32	/* From Microware routines	*/
#define MAX_SERV_NAME_LEN	12
#define MAX_LOG_LINE_LEN	1024	// for use with sys_buff and data_log

#define INET_TEST_TCP		"test_tcp"			
#define INET_TEST_UDP		"test_udp"			

#define SOCK_SHUTDOWN_RECEIVE	0
#define SOCK_SHUTDOWN_SEND		1
#define SOCK_SHUTDOWN_ALL		2

#define SIG_CLEAR_MASK			0
#define SIG_INC_MASK			1
#define SIG_DEC_MASK			(-1)

#ifndef max
#define max(a,b)				(((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)				(((a) < (b)) ? (a) : (b))
#endif

#define SLEEP_100(t)			usleep( 10000*t )
#define GETUID()				getuid()
#define GETGID()				getgid()

#define ITIMER_NULL			((struct itimerval *) 0)

#define NANOSEC_PER_SEC			1000000000L
#define USEC_PER_SEC			1000000L
#define MSEC_PER_SEC			1000L
#define TENTHS_SEC_PER_SEC		10L
#define TENTHS_SEC_PER_MIN		( 60L*10L )
#define TENTHS_SEC_PER_HR		( 60L*60L*10L )
#define TENTHS_SEC_PER_DAY		( 24L*60L*60L*10L )

#define SECS_PER_MIN   			60L
#define MINS_PER_HOUR  			60L
#define HOURS_PER_DAY  			24L
#define MONTHS_PER_YEAR  		12
#define MAX_DAYS_PER_MONTH		31

#define PI						3.14159265358979L
#define GRAVITY_MS2				9.81
#define SMALL_NEGATIVE_FLOAT	(-0.0000001)
#define SMALL_POSITIVE_FLOAT	(0.0000001)


typedef int bool_typ;
typedef unsigned char byte_typ;
typedef unsigned char uchar_typ;

/*	Pointer to function returning void.	*/

typedef void (*pfv_typ)();

#ifndef __STDC__
#define EXIT_SUCCESS		0
#define EXIT_FAILURE		1
#endif

/*
 * Given double-precision floats a, b, returns
 *  1 if a > b
 *  0 if a = b
 * -1 if a < b
 * with error tolerance specified by float_tolerance.
 */
static inline int path_fcompare(double a, double b, double float_tolerance) {
    return (fabs(a - b) < float_tolerance) ? 0 : (a > b ? 1 : -1);
}



#endif
