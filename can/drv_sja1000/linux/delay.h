/** Implement Linux udelay function with nanosleep;
 *  not sure if the driver needs this to be uninterruptible?
 */
#ifndef LINUX_DELAY_H
#define LINUX_DELAY_H
#include <time.h>

/**
 *      Use on QNX to get microsecond delay available on Linux.
 */
static inline unsigned int udelay(unsigned int usec)  
{                                  
        struct timespec requested, remaining;   
        requested.tv_sec = usec/1000000;        
        requested.tv_nsec = usec*1000;          
        return ((unsigned int) nanosleep(&requested, &remaining));
}               

/**
 *	Add a definition for linux/time.h's do_gettimeofday
 *	in terms of gettimeofday on QNX6.
 */
static inline void do_gettimeofday(struct timeval *tv)
{
	gettimeofday(tv, NULL);
}

/**
 *  Also throw the linux-style atomic_t in here, taken
 * from /usr/include/asm-i386/atomic.h, from 
 * Linux version 2.6.21.5-smp (root@midas) 
 * (gcc version 4.1.2) #2 SMP Tue Jun 19 14 :58:11 CDT 2007
 */                 
typedef struct { int counter; } atomic_t;

/**
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 * 
 * Atomically reads the value of @v.
 */
#define atomic_read(v)          ((v)->counter)

/**
 * atomic_set - set atomic variable
 * @v: pointer of type atomic_t
 * @i: required value
 * 
 * Atomically sets the value of @v to @i.
 */
#define atomic_set(v,i)         (((v)->counter) = (i))

/**
 *	Other unused Linux kernel functions.
 */
#define printk(fmt,args...) 
#define wake_up_interruptible(ptr)
#define board_clear_interrupts(x)
#endif
