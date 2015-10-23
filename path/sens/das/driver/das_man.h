/**\file
 *	das_man.h
 *
 *	Structures, definitions, and prototypes for the data
 *	acquisition manager.
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *
 *	static char rcsid[] = "$Id: das_man.h 6710 2009-11-11 01:52:27Z dickey $";
 *
 *	$Log$
 *	Revision 1.4  2005/09/02 03:13:34  dickey
 *	Update before import of Quartz timer card driver.
 *	Modified Files:
 *	 	Makefile README das_default.c das_init.c das_man.c das_man.h
 *	 	das_util.c dmm32.c dmm32.h garnet.c io_func.c null_das.c
 *	 	realtime.ini
 *
 *	Revision 1.2  2004/09/20 19:47:08  dickey
 *	
 *	Check-in, null_das tested, garnet tested, interrupts and dmm34 not tested
 *	Modified Files:
 *	 	Makefile README das_default.c das_init.c das_man.c das_man.h
 *	 	das_util.c garnet.c io_func.c null_das.c realtime.ini
 *	 Added Files:
 *	 	dmm32.c dmm32.h garnet.h
 *	
 *	
 *	Rewritten for QNX6 as a standard resource manager.
 *
 */
#ifndef DAS_MAN_H
#define DAS_MAN_H

#include <sys_qnx6.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "das_clt.h"

/* forward declarations needed so new IOFUNC_OCB_T and IOFUNC_ATTR_T defines
 * can be used in sys/iofunc.h
 */

struct das_ocb;
struct das_attr;

#define IOFUNC_OCB_T struct das_ocb
#define IOFUNC_ATTR_T struct das_attr
#include <sys/iofunc.h>
#include <sys/dispatch.h>

struct das_seq_str
{
	short int n_entries;
	char seq[NUM_SCAN_ENT];
};

/** Represents a request for data from a client.
 */
struct das_res_queue
{
	int nconv;		///	# of conversions	
	int cur_conv;		///	# of conversions left to do.
	int chnl;		///	Current channel 	
	int rcvid;		///	Used to notify client.  
	struct sigevent event;	///	Used to notify client   
	float buffer[NUM_SCAN_ENT];
};

/** Finish queue of channel holds data to be sent back to client,
 *  ready queue holds current request for conversions.
 */ 
struct das_res_chnl		///	Used for A/D and D/A		
{
	struct das_res_queue *f_hd;	///	Head of finish queue	
	struct das_res_queue *r_hd;	///	Head of ready queue	
};
				
/** Information per Open Context Block */
typedef struct das_ocb
{
	iofunc_ocb_t io_ocb;
	struct das_seq_str ad_seq;  /// A/D scan array.	
	struct das_res_chnl ad;
	struct das_res_chnl da;
} IS_PACKED das_ocb_t;

/** Function pointers will be initialized to default functions, which can
 * be replaced with functions specific to the device if additional hardware
 * functionality is required. 
 */
typedef struct {
	int (*da_term)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, int chnl);
	int (*da_sync)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
				 int chnl, float data);
	int (*digital_dir)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		 long mask);
	int (*digital_in)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		int port, long *pbits);
	int (*digital_out)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		int port, long mask, long bits,
		long *pold_bits, long *pnew_bits);
	int (*ad_enqueue)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		struct sigevent event);
	int (*ad_read)(resmgr_context_t *ctp, RESMGR_OCB_T *ocb);
	int (*ad_set_scan)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			 int n_entries);
	int (*ad_set_sample)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			 int chnl, unsigned long ticks);
	int (*ad_term)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, int chnl);
	int (*ad_pulse)(message_context_t *ctp, int code, unsigned flags,
			void *pocb);	/// must match pulse_attach template
	int (*tmr_mode)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		 unsigned timer, unsigned mode, unsigned value);
	int (*tmr_scan)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			 int num_entries);
	int (*tmr_read)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			 int count);
	int (*tmr_pulse)(message_context_t *ctp, int code, unsigned flags,
			void *pocb);	/// must match pulse_attach template
	void (*das_close)(resmgr_context_t *ctp, RESMGR_OCB_T *pocb);
} IS_PACKED das_func_t;

/** Information per device manager */
typedef struct das_attr
{
	iofunc_attr_t io_attr;	/// standard system information 
	char *devname;		/// device path name
	das_info_typ das_info;  /// initialization info, see das_clt.h	
	bool_typ verbose_flag;
	das_func_t func;	/// pointers to device-specific implementations
	das_ocb_t *ad_ocb;	/// address of ocb with A/D scan set 
	timer_t ad_timer_id;    /// Used to set sampling rate in software
	struct sigevent ad_pulse_event;	/// initialized in pulse_init
	struct sigevent tmr_pulse_event; /// initialized in pulse_init
} IS_PACKED das_attr_t;

/** Initialization functions in das_init.c
 */
extern void man_init(int argc, char *argv[], resmgr_connect_funcs_t *pconn,
	resmgr_io_funcs_t *pio, IOFUNC_ATTR_T *pattr);
extern void pulse_init(dispatch_t *dpp, IOFUNC_ATTR_T *pattr);

/** Replacements for resource manager io functions in io_func.c
 */
extern int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg,
			 RESMGR_OCB_T *ocb);
extern int io_msg(resmgr_context_t *ctp, io_msg_t *msg, RESMGR_OCB_T *ocb);
extern int io_close_ocb(resmgr_context_t *ctp, void *reserved,
			 RESMGR_OCB_T *ocb);

/** Default and helper functions in das_default.c
 *  These may be replaced in das_func_t structure by device-specific functions
 */
extern int das_default_da_term(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	int chnl);
extern int das_default_da_sync(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	int chnl, float data);
extern int das_default_digital_dir(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	long mask);
extern int das_default_digital_in(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
	int port, long *pbits);
extern int das_default_digital_out(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	int port, long mask, long bits, long *pold_bits, long *pnew_bits);
extern int das_default_ad_set_sample(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	int chnl, unsigned long ticks);
extern void das_ad_start_enqueue(int rcvid, RESMGR_OCB_T *pocb,
	struct sigevent event);
extern void das_ad_finish_enqueue(RESMGR_OCB_T *pocb);
extern int das_default_ad_enqueue (resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
	struct sigevent event);
extern int das_default_ad_read(resmgr_context_t *ctp, RESMGR_OCB_T *pocb);
extern int das_ad_read_scan_msg(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	int num_entries);
extern int das_ad_interrupt_attach(IOFUNC_ATTR_T *pattr);
extern int das_ad_interrupt_detach(int id);
extern int das_default_ad_set_scan(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	int num_entries);
extern int das_default_ad_term(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	int chnl);
extern int das_default_ad_pulse(message_context_t *ctp, int code, unsigned flags,
	void *pocb);
extern int das_default_tmr_mode(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
	 unsigned timer, unsigned mode, unsigned value);
extern int das_tmr_read_scan_msg(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
                        char *plist, int num_entries);
extern int das_default_tmr_scan(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
	int num_entries);
extern int das_default_tmr_read(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
	int count);

/** Utility functions in das_util.c
 */
extern int das_to_volt( das_info_typ *pdas, int *pfrom, int n, float *pto );
extern int bipolar_das_to_volt( das_info_typ *pdas, int *pfrom, int n, float *pto );
extern int das_to_step( das_info_typ *pdas, float *pfrom, int n, int *pto );

/** Functions which must be implemented in device-specific file.
 *  (default functions may also be re-implemented as required).
 */

/** das_func_init sets up the das_func_t function table pointed to
 *  by the device's IOFUNC_ATTR_T pointer.  
 */
extern int das_func_init(IOFUNC_ATTR_T *pattr);

/** das_open_dev performs device-specific initialization.
 */
extern int das_open_dev(IOFUNC_ATTR_T *pattr);

/**
 *  das_handle_interrupt services the pulse event sent when the interrupt
 *  associated with the device by InterruptAttachEvent in set_scan occurs
 */ 
extern void das_handle_interrupt(RESMGR_OCB_T *pocb);

/**
 *  das_ad_data copies analog data into a floating point array,
 *	using the correct transformation for the device.
 *  Must be supplied by each A/D driver device file, no default function.
 */
extern int das_ad_data(float *pdata, int n_data);

#endif /* DAS_MAN_H */
