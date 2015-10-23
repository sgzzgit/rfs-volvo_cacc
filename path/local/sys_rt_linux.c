/**\file 
 *	Utility functions for Linux based on PATH/AVCS QNX4 sys_rt.c 
 *	Additional functions added for QNX6-specific networking
 */
#include <sys_os.h>
#include "local.h"
#include "sys_rt.h"

#define MAX_TIMERS_PER_APP	1

/** 
 *	Only one timer supported on Linux for ITIMER_REAL 
 */
static posix_timer_typ sys_rt_timer;
	
/**
 *     nsec2timespec and timespec2nsec are available on QNX6
 *     implement them here for compatibility
 */

void nsec2timespec(struct timespec *ptm, unsigned long long ns)
{
	ptm->tv_sec = ns /  NANOSEC_PER_SEC;
	ptm->tv_nsec = ns % NANOSEC_PER_SEC;
}

unsigned long long timespec2nsec(struct timespec *ptm)
{
	unsigned long long ns;
	ns = ptm->tv_sec * (unsigned long long) NANOSEC_PER_SEC +
		ptm->tv_nsec;
	return ns;
}

/**
 *	print_clock
 *
 *	Calls clock_gettime and prints secs and nanoseconds Unix time
 */
void print_clock(void)
{
	struct timespec time;
	long tenth_msecs;	

	clock_gettime(CLOCK_REALTIME, &time);
	/* clock resolution on QNX6 is no better than 1/2 a millisec */
	tenth_msecs = (time.tv_nsec + 50000)/100000;  
	printf("Time: %09lu.%04lu\n", (unsigned long) time.tv_sec, tenth_msecs);
}

/**
 *
 *	diff_nsec	
 *
 *	Nanosecond difference between two timespec structures
 */	
unsigned long diff_nsec(struct timespec *ptime1, struct timespec *ptime2)
{
	unsigned long nsec;

	nsec = NANOSEC_PER_SEC*(ptime1->tv_sec - ptime2->tv_sec)
				+ (ptime1->tv_nsec - ptime2->tv_nsec);

	return(nsec);
}

void timer_alarm_hand(int code)
{
		return;
}

/*	
 *	timer_init(msec);
 *
 *	msec	-	Interval in milliseconds for the timer.
 *
 *	Creates a periodic timer event at the given interval.
 *	When the event occurs, the system will send a signal to
 *	the process. Returns a pointer to the timer, or
 *	NULL if there is not enough memory, not enough timers,
 *	or other system failure.
 *
 */

posix_timer_typ *timer_init(unsigned msec, int chid) 
{
	posix_timer_typ *ptimer = &sys_rt_timer;

	signal(SIGALRM, timer_alarm_hand);

        ptimer->itmr.it_interval.tv_sec = msec / 1000;
        ptimer->itmr.it_interval.tv_usec = (msec % 1000) * 1000;
        ptimer->itmr.it_value.tv_sec = msec / 1000;
        ptimer->itmr.it_value.tv_usec = (msec % 1000) * 1000;

	ptimer->channel_id = chid;	//unused

        if (setitimer(ITIMER_REAL, &ptimer->itmr, NULL) == -1)
		return (NULL);
	else
		return (ptimer);
}

/**	
 *	bool_typ timer_done(posix_timer_typ *ptimer);
 *
 *	ptimer	-	Pointer to the timer.
 *
 *	Turns off the interval timer, and releases the
 *	associated resources.  If a signal is pending, it
 *	may still arrive, but should probably be ignored.
 *
 *	Returns FALSE if the timer pointer is invalid.
 *	TRUE otherwise.
 */

bool_typ timer_done(posix_timer_typ *ptimer)
{
	if (ptimer == NULL)
		return(FALSE);

	
        ptimer->itmr.it_interval.tv_sec = 0; 
        ptimer->itmr.it_interval.tv_usec =  0;
        ptimer->itmr.it_value.tv_sec =  0;
        ptimer->itmr.it_value.tv_usec = 0; 

        (void) setitimer(ITIMER_REAL, &ptimer->itmr, NULL);

	// Nothing to free in this implementation
	return(TRUE);
}

/**	
 *	int timer_wait(posix_timer_typ *ptimer);
 *
 *	ptimer	-	Pointer to the timer.
 *
 *	Waits for pulse to be received from any 	
 *	timers. 
 *
 *	Returns value returned from pause.
 *	Should check to see if timer has really expired,
 *	or if SIGALRM was set for some other reason.
 */

int timer_wait(posix_timer_typ *ptimer)
{
	return pause();
}

/** For debugging convenience
 */
void print_timer(posix_timer_typ *ptmr)
{
	getitimer(ITIMER_REAL, &ptmr->itmr);
        printf("timer interval %d.%06d\n", ptmr->itmr.it_interval.tv_sec,
		ptmr->itmr.it_interval.tv_usec);
        printf("timer it value %d.%06d\n", ptmr->itmr.it_value.tv_sec,
		ptmr->itmr.it_value.tv_usec);
        printf("timer value %d\n", ptmr->timer_value);
}

/**	
 *	void sig_ign(int *plist, void (*pfunc)(int));
 *
 *	plist	-	A list of signals to be handled by the
 *				given function.  The list should be
 *				terminated by an invalid signal
 *				number, e.g. -1.
 *
 *	pfunc	-	A signal handler with the prototype:
 *				void function(int sig);
 *
 *	This function installs the given function as a signal
 *	handler for the specified signals.  All other signals
 *	are configured to be ignored by the application code.
 *
 */

void sig_ign(int *sig_list, void sig_hand(int sig))
{
        int i = 0;
        while (sig_list[i] != ERROR) {
                signal(sig_list[i], sig_hand);
                i++;
        }
}

/**	
 *	double bound( double lower, double value, double upper );
 *
 *	value	-	A value to be tested.
 *	lower	-	A set of bounds.
 *	upper
 *
 *	This function tests the given value against the lower and
 *	upper bounds.  If the value is within the bounds, the
 *	value is returned.  Otherwise, the constraining lower or
 *	upper value is returned.
 *
 *	double	-	A value constrained to the lower and upper bounds.
 */

double bound( double lower, double value, double upper )
{
	if( value <= lower )
		return( lower );
	else if( upper <= value )
		return( upper );
	else
		return( value );
}

/* implement QNX delay function with nanosleep */
unsigned delay(unsigned int msec)
{
	struct timespec requested, remaining;
	requested.tv_sec = msec/1000;
	requested.tv_nsec = msec*1000000;
	return ((unsigned int) nanosleep(&requested, &remaining));
}

/** nsec2timespec and timespec2nsec are QNX6 functions
 */

