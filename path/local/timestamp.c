/**\file Timestamp utility functions.
 *
 *	Assumptions: timestamps are within a single day, or two
 *	consecutive days, but not exceeding a 24 hour difference between
 *	any two timestamps operated on by increment and decrement.
 *	Used for creating trace data and simulating test runs based on trace
 *	data, and for diagnostics.
 *
 *	Also includes utilities for changing GPS UTC format to PATH
 *	hour, min, sec, millisec timestamp, for changing a timestamp
 *	in UTC time to the computer's currently set local time, using 
 *	the ftime function to get the local time offset information,
 *	and for adjusting a struct timespec by specified number of
 *	milliseconds.
 *
 * Copyright (c) 2003 Regents of the University of California
 *
 * Author: Sue Dickey
 */
#include <sys_os.h>
#include "timestamp.h"

#undef DEBUG_TIMESTAMP

int valid_timestamp(timestamp_t *ts)
{
	int retval = ( 0 <= ts->hour && ts->hour < 24 
		&& 0 <= ts->min && ts->min < 60
		&& 0 <= ts->sec && ts->sec < 60
		&& 0 <= ts->millisec && ts->millisec < 1000);
#ifdef DEBUG_TIMESTAMP
	printf("retval %d\n", retval);
	fflush(stdout);
#endif
	return retval; 
}


/** Converts timestamp string to timestamp_t type
 * return 0 for error, 1 for success
 */
int str2timestamp(char *str, timestamp_t *ts)
{
	char scratch[4];	/// field width of 3 plus null termination 

	/* if invalid, set to 0 */
	if (! (isdigit(str[0]) && isdigit(str[1]) ///hour
		&& isdigit(str[3]) && isdigit(str[4]) ///minute
		&& isdigit(str[6]) && isdigit(str[7]) ///second
		&& isdigit(str[9]) && isdigit(str[10])
			&& isdigit(str[11]))) {
		ts->hour = 0;
		ts->min = 0;
		ts->sec = 0;
		ts->millisec = 0;
		return 0;
	}
#ifdef DEBUG_TIMESTAMP
	printf("str2timestamp valid\n");
	fflush(stdout);
#endif

	memset(scratch, 0, 4);	/// initialize to null 
	scratch[0] = str[0];
	scratch[1] = str[1];
	ts->hour = atoi(scratch);
	scratch[0] = str[3];
	scratch[1] = str[4];
	ts->min = atoi(scratch);
	scratch[0] = str[6];
	scratch[1] = str[7];
	ts->sec = atoi(scratch);
	scratch[0] = str[9];
	scratch[1] = str[10];
	scratch[2] = str[11];
	ts->millisec = atoi(scratch);
#ifdef DEBUG_TIMESTAMP
	printf("%02d:%02d:%02d.%03d\n", ts->hour, ts->min, ts->sec, ts->millisec);
	fflush(stdout);
#endif
	if (!valid_timestamp(ts)){
		ts->hour = 0;
		ts->min = 0;
		ts->sec = 0;
		ts->millisec = 0;
		return 0;
	}
#ifdef DEBUG_TIMESTAMP
	printf("str2timestamp returning 1\n");
	fflush(stdout);
#endif
	return 1;
}	

/** Converts timestamp_t type to null-terminated timestamp string
 * return 0 for error, 1 for success
 */
int timestamp2str(timestamp_t *ts, char *str)
{
	if (!valid_timestamp(ts)) {
		sprintf(str, "%s", "00:00:00:000");
		return 0;
	}
	sprintf(str, "%02d:", ts->hour); 
	sprintf(&str[3], "%02d:", ts->min); 
	sprintf(&str[6], "%02d.", ts->sec); 
	sprintf(&str[9], "%03d", ts->millisec); 
	return 1;
}

/** Print timestamp on specified file 
 * return 0 for error, 1 for success
 */
int print_timestamp(FILE *fp, timestamp_t *ts)
{
	if (fprintf(fp, "%02d:", ts->hour) < 0) return 0; 
	if (fprintf(fp, "%02d:", ts->min) < 0) return 0; 
	if (fprintf(fp, "%02d.", ts->sec) < 0) return 0;; 
	if (fprintf(fp, "%03d ", ts->millisec) < 0) return 0; 
	return 1;
}

/** Puts timestamp str in character buffer 
 *  For convenience returns number of characters put in buffer. 
 *  Can create problems if buffer is not large enough.
 *  Note that bad values in hour, min, etc can cause more than
 *  2 characters to be printed.
 */
int sprint_timestamp(char* buffer, timestamp_t *ts)
{
	int cnt = 0;
	cnt += (sprintf(buffer, "%02d:", ts->hour)); 
	cnt += (sprintf(buffer + cnt, "%02d:", ts->min)); 
	cnt += (sprintf(buffer + cnt, "%02d.", ts->sec)); 
	cnt += (sprintf(buffer + cnt, "%03d ", ts->millisec)); 
	return cnt;
}


/**
 * Returns 1 if timestamp is different from last call to check_timestamp,
 * and returns 0 if timestamp hasn't changed.,
 */
int
check_timestamp(timestamp_t *pts)
{
        static timestamp_t last_ts;	// will be initialized on first call
        if (last_ts.hour == pts->hour &&
		last_ts.min == pts->min &&
		last_ts.sec == pts->sec &&
		last_ts.millisec == pts->millisec) {
		last_ts = *pts;
                return 0;
	} else {
		last_ts = *pts;
                return 1;
	}
}

/** Gets current time in timestamp format. 
 * return 0 for error, 1 for success
 */
int get_current_timestamp(timestamp_t *ts)
{
	struct timeb timeptr_raw;
	struct tm time_converted;

	if (ftime (&timeptr_raw) == -1)	
		return 0; 

	/* on QNX4, localtime_r should be defined to _localtime */
	localtime_r ( &timeptr_raw.time, &time_converted );

        ts->hour = time_converted.tm_hour;
        ts->min = time_converted.tm_min;
        ts->sec = time_converted.tm_sec;
        ts->millisec = timeptr_raw.millitm;
	return 1;
}

float sec_past_midnight_float(void) {
	struct timeb timeptr_raw;
	struct tm time_converted;
	float sec_past_midnight;

	if (ftime (&timeptr_raw) == -1)	
		return 0; 

	localtime_r ( &timeptr_raw.time, &time_converted );

        sec_past_midnight = (time_converted.tm_hour * 3600) + (time_converted.tm_min * 60) + time_converted.tm_sec + (timeptr_raw.millitm / 1000.0);
	return sec_past_midnight;
}
	
void ms_to_ts(int ms, timestamp_t *ts)
{
	int t;
	t = ms;		//time in milliseconds
	ts->millisec = t % 1000;
	t -= ts->millisec;
	t /= 1000;		//time in seconds
	ts->sec = t % 60;
	t -= ts->sec;
	t /= 60;		//time in minutes
	ts->min = t % 60;
	t -= ts->min;
	t /= 60;		//time in hours
	ts->hour = t;
}

/** Increments timestamp, wrapping around midnight boundary.
 *  Returns 0 if input timestamps are not valid.
 */
int increment_timestamp(timestamp_t *ts, timestamp_t *delta_ts,
                timestamp_t *new_ts)
{
	int ts_ms;
	int delta_ts_ms;
	int new_ts_ms;

	if (!valid_timestamp(ts) || !valid_timestamp(delta_ts))
		return 0;

	ts_ms = TS_TO_MS(ts);
	delta_ts_ms = TS_TO_MS(delta_ts);

	new_ts_ms = (ts_ms + delta_ts_ms) % TS_MAX_MSEC;

	ms_to_ts(new_ts_ms, new_ts);
	return 1;
}

/** Decrements timestamp
 *  returns 0 if either input timestamp is out of range
 *  if delta_ts is greater than ts, adds TS_MAX_MSEC to ts
 *  before decrementing (assumes that midnight boundary has been crossed)
 */
int decrement_timestamp(timestamp_t *ts, timestamp_t *delta_ts,
                timestamp_t *new_ts)
{
	int ts_ms;
	int delta_ts_ms;
	int new_ts_ms;

	if (!valid_timestamp(ts) || !valid_timestamp(delta_ts))
		return 0;
	
	ts_ms = TS_TO_MS(ts);
	delta_ts_ms = TS_TO_MS(delta_ts);

	if (delta_ts_ms > ts_ms)
		ts_ms += TS_MAX_MSEC;

	new_ts_ms = ts_ms - delta_ts_ms;

	ms_to_ts(new_ts_ms, new_ts);

	return 1;
}

/** ts2_is_later_than_ts1
 *  Assumes distance between timestamps is less than 12 hours. 
 *  If greater than 12 hours, assume seconds count has wrapped
 *  around the midnight boundary, so that smaller number is
 *  the most recent timestamp.
 *  Returns 0 if either timesstamp is invalid.
 *  Returns 1 if ts2 is more recent than ts1
 */
int ts2_is_later_than_ts1(timestamp_t *ts1, timestamp_t *ts2)
{
	int ts1_ms;
	int ts2_ms;
	int diff;

	if (!valid_timestamp(ts1) || !valid_timestamp(ts2))
		return 0;
	
	ts1_ms = TS_TO_MS(ts1);
	ts2_ms = TS_TO_MS(ts2);

	diff = ts2_ms - ts1_ms;
	if (diff > 0)	// ts2_ms greater
		if (diff <= TS_MAX_MSEC/2) // less than 12 hours greater
			return 1;	// ts2 later
	diff = -diff;	// ts1_ms greater
	if (diff > TS_MAX_MSEC/2)	// more than 12 hours greater
			return 1;	// ts2 later
	return 0; // ts1 later
}

/** Check timestamp 
 * returns 0 if timestamp is not increased or invalid
 * return 1 otherwise
 * prev_sys_ts is the old local system time
 * cur_sys_ts is the current local system time
 * prev_check_ts is the old check timestamp
 * cur_check_ts is the current check timestamp
 */ 

int is_timestamp_increase(timestamp_t *prev_sys_ts, timestamp_t *cur_sys_ts, 
		timestamp_t *prev_check_ts, timestamp_t *cur_check_ts,
		int buffertime)
{
 	if (!valid_timestamp(cur_sys_ts) ||
			!valid_timestamp(cur_check_ts) ||
			!valid_timestamp(prev_sys_ts) ||
			!valid_timestamp(prev_check_ts))
 		return 0;
 	
 	if (TS_TO_MS(cur_sys_ts)-TS_TO_MS(prev_sys_ts) >buffertime &&
		 TS_TO_MS(cur_check_ts)-TS_TO_MS(prev_check_ts) > 0 )
			return 0;
 	else 
 		return 1; 	
}

/** Converts float represention UTC time strings as received from
 *  GPS into an hour/min/sec/millisec timestamp_t
 */ 

timestamp_t gpsutc2ts(float utc_time)
{
        timestamp_t ts;
        float remainder;
        ts.hour = utc_time/10000;
        remainder = utc_time - ts.hour * 10000;
        ts.min = remainder /100;
        remainder = utc_time - ts.hour * 10000 - ts.min * 100;
        ts.sec = (int) remainder;
        remainder = utc_time -ts.hour * 10000 - ts.min * 100 - ts.sec;
        ts.millisec = 1000 * remainder;
        return (ts);
}

/** Calls ftime to determine the offset for the local timezone, subtracts
 *  this from the UTC timestamp to get the local time.
 */
timestamp_t utc2local(timestamp_t utc_ts)
{
	timestamp_t local_ts;
	timestamp_t adjust_ts;
	struct timeb timeptr_raw;
	short adjust_min;

	ftime(&timeptr_raw);
	
	adjust_min = timeptr_raw.timezone;
	adjust_ts.hour = adjust_min/60;
	adjust_ts.min = 0;
	adjust_ts.sec = 0;
	adjust_ts.millisec = 0;
	decrement_timestamp(&utc_ts, &adjust_ts, &local_ts);

	return (local_ts);
}
	
/** Input is a struct timespec from clock_gettime and a timestamp_t 
 *  representing the current UTC hour/min/sec/millisec obtained from GPS.
 *  Returns a struct timespec that can be used by clock_settime to
 *  set the clock to match the GPS UTC time.
 */ 

struct timespec adjust_tm(struct timespec tm, timestamp_t utc_ts)
{
	struct timespec ret;
	int tm_sec = tm.tv_sec;
	int tm_day = tm_sec/(24*3600); 
	int new_sec = TS_TO_MS(&utc_ts)/1000 + tm_day * 86400;
	
	int sec_12hour = 12*3600;

	// assumes original time was within twelve hours of correct
	// in order to get day right 


	if ((tm_sec - new_sec) > sec_12hour) 
		new_sec -= 24*3600;
	else if ((tm_sec - new_sec) < -sec_12hour)
		new_sec += 24*3600;

	ret.tv_sec = new_sec;
	ret.tv_nsec = utc_ts.millisec * 1000 * 1000;
	
	return (ret);
}

static char *mnames[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

/**
 * Utility for printing timespecs in year month day hour:min:sec.nanosec
 * format
 */

void print_timespec(FILE *fp,struct timespec *pts)
{
	struct tm tm_val;
	tm_val = *localtime(&pts->tv_sec);
	fprintf(fp, "%4d %s %2d %02d:%02d:%02d.%09ld ",
		tm_val.tm_year + 1900,
		mnames[tm_val.tm_mon],
		tm_val.tm_mday,
		tm_val.tm_hour,
		tm_val.tm_min,
		tm_val.tm_sec,
		pts->tv_nsec);
}

/**
 *	Function to return the number of current month, day and year
 */ 
void get_todays_date(int *pmonth, int *pday, int *pyear)
{
	struct timespec now;
	struct tm tm_val;
	clock_gettime(CLOCK_REALTIME, &now);
	tm_val = *localtime(&now.tv_sec);
	*pmonth = tm_val.tm_mon + 1;
	*pday = tm_val.tm_mday;
	*pyear = tm_val.tm_year + 1900;
}


