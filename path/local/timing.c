/* Functions added to programs to get timing data.
 */

#include <sys_os.h>
#include "timing.h"

void avcs_start_timing(struct avcs_timing *tm)

{
	struct timespec res;
	struct timespec tp;
	struct tms ret_tms;
	clock_getres(CLOCK_REALTIME, &res);
	tm->clock_res =  res.tv_nsec;
	clock_gettime(CLOCK_REALTIME, &tp);
	tm->start_time = tp; 
	tm->end_time = tp;	/* to initialize */
	tm->exec_num = 1;	/* legal divisor if not calculated */
	tm->AET = 0.0;		/* Average Execution Time */
	tm->WCET = 0.0;		/* Worst Case Execution Time */
	tm->start_tick = times(&ret_tms);
	tm->utime = ret_tms.tms_utime;
	tm->stime = ret_tms.tms_stime; 
	tm->cutime = ret_tms.tms_cutime;	
	tm->cstime = ret_tms.tms_cstime;	
}

void avcs_end_timing(struct avcs_timing *tm)

{
	struct timespec tp;
	struct tms ret_tms;
	clock_gettime(CLOCK_REALTIME, &tp);
	tm->end_time = tp; 
	tm->end_tick = times(&ret_tms);
	tm->utime = ret_tms.tms_utime;
	tm->stime = ret_tms.tms_stime; 
	tm->cutime = ret_tms.tms_cutime;	
	tm->cstime = ret_tms.tms_cstime;	
	tm->AET = (double)tm->utime/(double)tm->exec_num;
}

void avcs_print_timing(FILE *stream, struct avcs_timing *tm)

{
	fprintf(stream, "Clock resolution: %ld ns\n", tm->clock_res);	
	fprintf(stream, "Real time %.6f ms\n",
		(tm->end_time.tv_sec - tm->start_time.tv_sec) * 1000.0 +
		(tm->end_time.tv_nsec - tm->start_time.tv_nsec) /1000000.0);
	fprintf(stream, "Total clock ticks: %ld\n",
		tm->end_tick - tm->start_tick);
	fprintf(stream, "User time %ld ms\n", tm->utime);
	fprintf(stream, "System time %ld ms\n", tm->stime);
	fprintf(stream, "Child user time %ld ms\n", tm->cutime);
	fprintf(stream, "Child system time %ld ms\n", tm->cstime);	
	fprintf(stream, "Number of executions %ld\n", tm->exec_num);
	fprintf(stream, "Average user time/loop execution %.6f ms\n", tm->AET);
	fprintf(stream, "Worst case execution time %.6f ms\n", tm->WCET);
}
