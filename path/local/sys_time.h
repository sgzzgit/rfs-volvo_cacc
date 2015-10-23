/**\file	
 *	sys_time.h
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *	Structure definitions for sys_time.c routines.
 */
#ifndef PATH_SYS_TIME_H
#define PATH_SYS_TIME_H

#define	JULTIME_STR				"%02d%02d%02d"
#define	JULTIME_INTS			3
#define	MILTIME_STR				"%02d:%02d:%02d"
#define	MILTIME_INTS			3
#define	FIRST_MONTH				1

#define	MILTIME_LEN				8
#define	JULTIME_LEN				6

#define	SYS_TIME_MIN_YEAR		89
#define SYS_TIME_MAX_YEAR		95

#define TIME_NEXT_MINUTE		1
#define TIME_NEXT_HOUR			2
#define TIME_NEXT_DAY			3
#define TIME_NEXT_WEEK			4
#define TIME_NEXT_MONTH			5

/*	4 byte longs run out on Oct. 21, 1995  = 2485 days	*/

#define		MAX_SYS_TIME			(2485L*TENTHS_SEC_PER_DAY)

struct tms_time_str
{
	unsigned char day, hour, minute, sec;
};


struct new_time_str
{
	unsigned char year, month, day, hour, minute, sec, tenths;
};

typedef long sys_time_typ;

/*	Function prototypes for sys_time.c routines.
 */

long get_sys_time( void );
int diff_sys_time( long end_time, long start_time, long *pdiff_time);
int inc_sys_time( long time, long delta_time, long *padjusted_time );
int cmp_sys_time( long t1, long t2 );
int sys2tms_time(long time, struct tms_time_str *ptms_time );
long get_time( void *ptime, int year, int month );
int sys_time_days( int year, int month, int day );
char *filedate( void );
struct tm *yesterday( void );
sys_time_typ time_next( int when );
char *filemonth( void );
void sec2label( long total_sec, char *pbuff );
int read_yymmdd( char *pname, int *pyear, int *pmonth, int *pday );    

void time_newtocmp( struct new_time_str *pnew_time, 
		char *pjulian, char*pmil_time );
int time_cmptonew( char *pjulian, char *pmil_time, 
		struct new_time_str *pnew_time );
int time_systonew( long sys_time, struct new_time_str *pnew_time );
long time_newtosys( struct new_time_str *pnew_time );
long time_cmptosys( char *pjulian, char *pmil_time );
int time_systocmp( long sys_time, char *pjulian, char *pmil_time );
void time_newtoold( struct new_time_str *pnew_time,
		struct tms_time_str *ptms_time, int *pyear, int *pmonth );
void time_oldtonew( struct tms_time_str *ptms_time, int year, int month,
		struct new_time_str *pnew_time );
long time_oldtosys( struct tms_time_str *ptms_time, int year, int month );
int time_systoold( long sys_time, struct tms_time_str *ptms_time,
		int *pyear, int *pmonth );
int read_julian( char *pfilename, char *pjulian );
void time_prn_new( struct new_time_str *pnew );
void time_prn_old( struct tms_time_str *ptms_time, int year, int month );
void time_prn_cmp( char *pjulian, char *pmil_time );

#endif
