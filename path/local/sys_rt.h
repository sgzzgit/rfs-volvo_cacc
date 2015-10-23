/**\file	
 *	sys_rt.h timer and signal utilities
 *
 */
#ifndef SYS_RT_H
#define SYS_RT_H
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <local.h>

/** 
 * posix_timer_type
 *
 * used for compatibility with existing QNX code
 */ 
typedef struct {
	int channel_id;		/* for compatibility with QNX6, may use later */
	struct itimerval itmr;
	int timer_value;	// always 1, for now
} posix_timer_typ;

extern void print_clock(void);
extern unsigned long diff_nsec(struct timespec *ptime1, struct timespec *ptime2);

extern posix_timer_typ *timer_init(unsigned msec, int chid);
extern bool_typ timer_done(posix_timer_typ *ptimer);
extern int timer_wait(posix_timer_typ *ptimer);

/* For use by application with a single timer. Others use timer_wait
 * and check returned timer code to see which timer went off.
 */
#define TIMER_WAIT(ptimer)	(void) timer_wait(ptimer)	

extern void sig_ign(int *plist, void (*pfunc)(int));
extern double bound( double lower, double value, double upper );
extern void print_timer(posix_timer_typ *ptmr);
extern unsigned delay(unsigned int msec);

#endif
