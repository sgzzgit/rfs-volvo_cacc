/*	 Header file for AVCS timing functions. 
 */

#include <time.h>
#include <sys/times.h>
#include <limits.h>

struct avcs_timing {
	time_t clock_res;
	struct timespec start_time;
	struct timespec end_time;
	clock_t start_tick;
	clock_t end_tick;
	long exec_num;
	double AET;
	double WCET;
	clock_t utime;
	clock_t stime;
	clock_t cutime;
	clock_t cstime;
};

extern void avcs_start_timing(struct avcs_timing *tm);
extern void avcs_end_timing(struct avcs_timing *tm);
extern void avcs_print_timing(FILE *stream, struct avcs_timing *tm);

