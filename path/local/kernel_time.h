/*- 
 * Copyright 2006 - R. Tyler Ballance <tyler@FreeBSD.org> 
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 * 
 * $P4: //depot/user/tyler/openbsm/compat/kernel_time.h#3 $ 
 */  
  
/*- 
 * This file originally sourced from 
 * http://perforce.freebsd.org/fileViewer.cgi?FSPC=//depot/user/tyler/openbsm/compat/kernel%5ftime.h&REV=3 
 * and modified to include err.h below. 
 */  
 
#ifndef _COMPAT_KERNEL_TIME_H_  
#define _COMPAT_KERNEL_TIME_H_  
 
#include <stdlib.h>  
#include <sys/time.h>  
#include <err.h>  
 
#define CLOCK_REALTIME NULL  
  
static inline   
int clock_gettime(void *dummyarg, struct timespec *tsp) {  
	struct timeval tval;
  
	if ((gettimeofday(&tval, 0)) != 0)  
	{
		err(-1, "gettimeofday");  
	}
  
	/* convert from microseconds to nanoseconds 
	 * for timeval -> timespec conversion  
	 */  
	tsp->tv_sec = tval.tv_sec;  
	tsp->tv_nsec = ((tval.tv_usec)/1000);  
  
	return 0;  
}


static inline
int clock_settime(void *dummy, const struct timespec *tp)
{
	fprintf(stderr, "ERROR: clock_settime is not supported\n");
	return -1;
}

static inline
int clock_getres(void *dummy, struct timespec *tsp)
{
	if (tsp)
	{
		tsp->tv_sec = 0;
		tsp->tv_nsec = 0; /* 0 might be safer than an arbitrary value (Zu) */
	}
	return 0;
}

#endif
