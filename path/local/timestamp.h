/**\file Header file for timestamp utility functions (see timestamp.c).
 *
 * Copyright (c) 2003 Regents of the University of California
 *
 * Author: Sue Dickey
 */
#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <stdio.h>
#include <sys_os.h>

typedef struct {
	char hour;	/// Timestamp of last database update
	char min;
	char sec;
	short int millisec;
} IS_PACKED timestamp_t;

/** timestamp string format hh:mm:ss.mmm */
#define TS_STR_LEN	12	

/** given timestamp pointer, convert to milliseconds since midnight (int)*/
#define TS_TO_MS(ts) \
	(((((ts)->hour * 60*60) + ((ts)->min * 60) + (ts)->sec) * 1000)\
			+ (ts)->millisec)

/** given timestamp pointer, convert to seconds since midnight (double)*/
#define TS_TO_SEC(ts) \
	(((((ts)->hour * 60*60) + ((ts)->min * 60) + (ts)->sec) )\
			+ (ts)->millisec/1000.0)

#define TS_MAX_MSEC  (24 * 60 * 60 * 1000)

/** return 0 for error, 1 for success */
extern int str2timestamp(char *str, timestamp_t *ts);
extern int timestamp2str(timestamp_t *ts, char *str);
extern void ms_to_ts(int ms, timestamp_t *ts);
extern int print_timestamp(FILE *fp, timestamp_t *ts);
extern int sprint_timestamp(char *buffer, timestamp_t *ts);
extern int get_current_timestamp(timestamp_t *ts);
extern int increment_timestamp(timestamp_t *ts, timestamp_t *delta_ts,
		timestamp_t *new_ts);
extern int decrement_timestamp(timestamp_t *ts, timestamp_t *delta_ts,
		timestamp_t *new_ts);
extern int ts2_is_later_than_ts1(timestamp_t *ts1, timestamp_t *ts2);

extern int valid_timestamp(timestamp_t *ts);
extern int is_timestamp_increase(timestamp_t *prev_sys_ts, timestamp_t *cur_sys_ts, timestamp_t *prev_check_ts, timestamp_t *cur_check_ts, int buffertime);
extern timestamp_t gpsutc2ts(float utc_time);
extern timestamp_t utc2local(timestamp_t utc_ts);
extern struct timespec adjust_tm(struct timespec tm, timestamp_t utc_ts);
extern void print_timespec(FILE *fp,struct timespec *pts);
extern float sec_past_midnight_float(void);
void get_todays_date(int *pmonth, int *pday, int *pyear);

/** Converts hours,mins,sec,millisecs of current time to double floating point
 *  seconds  in the day
 */
static inline double get_sec_clock()
{
	timestamp_t ts;
	get_current_timestamp(&ts);
	return (ts.hour * 3600.0 + ts.min * 60.0 + ts.sec + ts.millisec/1000.0);} 
/*

 * Converts a timestamp to GPS UTC format, i.e.
 * hhmmss.ssssss
 */
static inline float ts2gpsutc(timestamp_t* ts) 
{
	float utc = ((ts->hour * 10000) + (ts->min * 100) + (ts->sec) +
               (( ts->millisec) / 1000.0));
	return utc;
}

/*
 * Determines the number of milliseconds elapsed since time1 at time2,
 * possibly negative, and stores the result in int* result.
 * Assumes both times are of the same day.
 * Returns 0 if either timestamp is invalid.
 */
static inline int time_diff(timestamp_t* ts1, timestamp_t* ts2, int* result)
{
        if (!valid_timestamp(ts1) || !valid_timestamp(ts2)) {
		return 0;
        }
        *result = TS_TO_MS(ts2) - TS_TO_MS(ts1);
        return 1;
}

/**
 *	Macros that work directly on TIMEB, used in some of our demo 2003
 *	code. Used to live (inappropriately) in J1939 header files.
 */

/* compare two timeb values, return 1 iff a >= b && a.millitm > b.millitm */

#define TIMEB_COMP(a, b)        (( (a).time > (b).time) || \
         (((a).time == (b).time) && ((a).millitm > (b).millitm) ))

/* return difference in milliseconds (t2 - t1), t2, t1 pointers to timeb */
#define TIMEB_SUBTRACT(t1,t2)  (((t2)->time * 1000 + (t2)->millitm) - \
                        ((t1)->time * 1000 + (t1)->millitm))

#endif /* TIMESTAMP_H */
