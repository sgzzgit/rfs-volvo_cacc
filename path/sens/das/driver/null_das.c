/** \file     
 *	null_das.c      
 *
 *  Copyright (c) 2003   Regents of the University of California
 *
 *  Sample device driver using DAS I/O manager.
 *
 *  Shows required das_func_init function, and a couple of replacement
 *  functions used for testing, as an example of how to replace
 *  functions if desired.
 */

#include <sys_qnx6.h>
#include <local.h>
#include <sys_mem.h>

#include "das_clt.h"
#include "das_man.h"

#define DO_TRACE

int null_tmr_pulse(void);

/** simulates data stored in hardware */
static long digital_out_val = 0;

/** Replacement function, to illustrate how to initialize it in das_func_init 
 */
int null_das_digital_out(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
                        int port, long mask, long bits, long *pold_bits,
                        long *pnew_bits)
{
#ifdef DO_TRACE
        printf("enter null_das_digital_out: ");
	printf("mask 0x%lx, bits 0x%lx\n", mask, bits);
#endif
        *pold_bits = digital_out_val;
	digital_out_val = (digital_out_val & ~mask) | (bits & mask);
        *pnew_bits = digital_out_val;

        return (EOK);
}
static void timer_sw_init(IOFUNC_ATTR_T *pattr)
{
        int status;

        /* Attach to the timer */
#ifdef DO_TRACE
        printf("NULL.C:timer_sw_set: SW timer synced to system HW clock\n");
#endif
        status = timer_create(CLOCK_REALTIME, &pattr->tmr_pulse_event,
                        &pattr->ad_timer_id);

        if(status == -1) {
                printf("Unable to attach timer.");
        }
}

/*
 *      Set software timer, ticks are in milliseconds
 */
static void timer_sw_set(IOFUNC_ATTR_T *pattr, unsigned int ticks)
{
        struct itimerspec timer;
#ifdef DO_TRACE
        printf("NULL.C:timer_sw_set: ticks=%d (msec)\n",ticks);
#endif

        timer.it_value.tv_sec = ticks/1000;     // 1000 ms/sec
        timer.it_value.tv_nsec = 1000000*(ticks % 1000); //1000000ns/ms
        timer.it_interval.tv_sec = ticks/1000;     // 1000 ms/sec
        timer.it_interval.tv_nsec = 1000000*(ticks % 1000); //1000000ns/ms
        timer_settime(pattr->ad_timer_id, 0, &timer, NULL);
}


/** das_func_init sets up the das_func_t function table pointed to
 *  by the device's IOFUNC_ATTR_T pointer.  
 */
int das_func_init(IOFUNC_ATTR_T *pattr)
{
	das_func_t *pfunc = &pattr->func;
#ifdef DO_TRACE
	printf("initializing DAS function table\n");
#endif

	pfunc->digital_dir = das_default_digital_dir;
	pfunc->digital_in = das_default_digital_in;
	pfunc->digital_out = null_das_digital_out;
	pfunc->da_term = das_default_da_term;
	pfunc->da_sync = das_default_da_sync;
	pfunc->tmr_mode = das_default_tmr_mode;
	pfunc->tmr_scan = das_default_tmr_scan;
	pfunc->tmr_read = das_default_tmr_read;
	pfunc->ad_set_sample = das_default_ad_set_sample;
	pfunc->ad_enqueue = das_default_ad_enqueue ;
	pfunc->ad_pulse = das_default_ad_pulse;
	pfunc->ad_read = das_default_ad_read;
	pfunc->ad_set_scan = das_default_ad_set_scan;
	pfunc->ad_term = das_default_ad_term;
	pfunc->tmr_pulse = null_tmr_pulse;
	return (TRUE);
}

/** das_open_dev performs device-specific initialization.
 */
int das_open_dev(IOFUNC_ATTR_T *pattr)
{
#ifdef DO_TRACE
	printf("enter das_open_dev\n");
#endif
        /// Create software timer and set to default of 20ms
        timer_sw_init(pattr);
        timer_sw_set(pattr, 20);

	return (TRUE);
}

/**
 *  das_handle_interrupt services the pulse event sent when the interrupt
 *  associated with the device by InterruptAttachEvent in set_scan occurs
 */ 
void das_handle_interrupt(RESMGR_OCB_T *pocb)
{
#ifdef DO_TRACE
	printf("enter das_handle_interrupt\n");
#endif
} 

/**
 *  das_ad_data copies analog data into a floating point array,
 *	using the correct transformation for the device.
 *	returns the number of conversions
 */
int das_ad_data(float *pdata, int n_data)
{
#ifdef DO_TRACE
	printf("enter das_ad_data\n");
#endif
	return (n_data);
}

                                                                  
int null_tmr_pulse (void )
{
        static int ctr;
#ifdef DO_TRACE
        if( (ctr++ %50) == 0)
        printf("NULL.C:null_tmr_pulse: ad_timer_msg: start scan?\n");
#endif
        return 0;                                                 
}                                                                 
                                                                  


