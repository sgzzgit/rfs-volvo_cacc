/** \file     
 *	das_default.c      
 *
 *  Copyright (c) 2003   Regents of the University of California
 *
 *  QNX6 version, required functions for DAS manager
 *
 *  By default, these functions are used. To override, assign
 *  a new function to the function pointer in das_func_init (the
 *  only function which MUST be re-implemented for each device)
 *  This method of replacing default with device-specific functions 
 *  follows the model used by the QNX6 resource manager library for io_funcs.
 */

#include <sys_qnx6.h>
#include <local.h>
#include <sys_mem.h>

#include "das_clt.h"
#include "das_man.h"

#undef DO_TRACE

/** Placeholder for function to perform synchronous D/A
 * 	conversion on channel chnl using 'data'
 */
int das_default_da_sync(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			int chnl, float data)
{
#ifdef DO_TRACE
	printf("enter das_default_da_sync\n");
#endif
	return (ENOSYS);
}

/** 	Function to terminate all pending D/A requests
 *
 *	If device-specific actions are required, the device-specific function
 *	can call this function after it has finished the hardware work.
 *	May also be called to terminate requests when resource manager
 *	is halted by a signal -- in that case only the device-specific
 *	functions are important.
 */

int das_default_da_term(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, int chnl)
{
#ifdef DO_TRACE
	printf("enter das_default_da_term\n");
#endif
	/* Perform any required hardware functions first, then empty queues */
	if (pocb != NULL) {
		pocb->da.f_hd = NULL;
		pocb->da.r_hd = NULL;
	}
	return (EOK);
}

/** Placeholder for function to set direction of bits to input or output
 */
int das_default_digital_dir(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, long mask)
{
#ifdef DO_TRACE
	printf("enter das_default_digital_dir\n");
#endif
	return (ENOSYS);
}

/** Placeholder for digital input function.
 */ 
int das_default_digital_in(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
			int port, long *pbits)
{
#ifdef DO_TRACE
	printf("enter das_default_digital_in\n");
#endif

        *pbits = 0xdeadbeef;
	
	return (ENOSYS);
}

/** Placeholder for digital output function.
 */
int das_default_digital_out(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			int port, long mask, long bits, long *pold_bits,
			long *pnew_bits)
{
#ifdef DO_TRACE
	printf("enter das_default_digital_out\n");
#endif
	*pold_bits = 0xdeadbeef;
	*pnew_bits = 0xdeadbeef;
	return (ENOSYS);
}

static struct das_res_queue read_entry; 
static struct das_res_queue fin_entry;

/**
 * 	das_default_ad_enqueue()
 *
 *	Does general bookkeeping to enqueue a A/D request,
 *	from a client. The device-specific function can call
 *	the convenience functions:  das_ad_start_enqueue before
 *	doing the necessary hardware work, and das_ad_finish_enqueue
 *	afterwards. 
 *
 */

void das_ad_start_enqueue(int rcvid, RESMGR_OCB_T *pocb, struct sigevent event)
{
	struct das_res_queue *pentry;

	/*
	 *	Use read queue entry that is global to this file
	 */

	pentry = &read_entry;
	pentry->rcvid = rcvid; 
	pentry->event = event;
	pentry->nconv = pocb->ad_seq.n_entries;
}

void das_ad_finish_enqueue(RESMGR_OCB_T *pocb)
{
	pocb->ad.r_hd = &read_entry;
}

int das_default_ad_enqueue (resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
			struct sigevent event)
{
#ifdef DO_TRACE
	printf("enter das_default_ad_enqueue\n");
#endif
	das_ad_start_enqueue(ctp->rcvid, pocb, event);

	/* hardware setup goes here, if function is active for this driver */

	das_ad_finish_enqueue(pocb);
	return (EOK);
}

/**
 *	das_default_ad_read() -  Retrieve the stored data from the A/D
 *		 conversions previously requested by client.
 */

int das_default_ad_read(resmgr_context_t *ctp, RESMGR_OCB_T *pocb)
{
	static das_msg_t msg; // don't allocate on stack

	/* Are there any requests finished? (the only time f_hd != NULL is
	 * when it's set to &fin_entry in das_default_ad_pulse, below)
	*/
	if (pocb->ad.f_hd == NULL)
		return (EDEADLK);

	/*
	 *	Re-queue request based on the das_ad_read() call.
	 *	Assume that the conversion hasn't changed.
	 */

	read_entry.cur_conv = read_entry.nconv;
	pocb->ad.r_hd = &read_entry;

	/*
	 *	 Setup message reply header and data buffer.
	 */
	msg.n = pocb->ad.f_hd->nconv; /// tells client how many were read

	SETIOV(&ctp->iov[0], &msg, sizeof(msg));
	SETIOV(&ctp->iov[1], pocb->ad.f_hd->buffer,
			pocb->ad.f_hd->nconv * sizeof(float));

#ifdef DO_TRACE
	printf("DAS_DEFAULT.C:das_default_ad_read: f_hd 0x%x nconv %d\n", (int) pocb->ad.f_hd, pocb->ad.f_hd->nconv);
#endif
	MsgReplyv(ctp->rcvid, EOK, ctp->iov, 2);
		
	 /*	Remove from finish queue.  */

	pocb->ad.f_hd = NULL;
	return (_RESMGR_NOREPLY);                                       
}
 
/**
 *	das_default_ad_set_scan() - Set block scan entries for A/D conversions.
 *
 *	The actual AD set scan routine will be hardware specific, but
 *	if the device needs this function, the helper routines can be
 *	called by the device specific routine to read the message,
 *	before any hardware specific processing, and to attach the
 *	interrupt, if required. This routine will also set the ad_ocb
 *	pointer in the das_attr_t structure, since it will be accessed.
 *	by the pulse handler when the interrupt handler sends a pulse.
 */

int das_ad_read_scan_msg(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
			int num_entries)

{
	int status;
	IOFUNC_ATTR_T *pattr = pocb->io_ocb.attr;
	pattr->ad_ocb = pocb; 
	status = MsgRead(ctp->rcvid, pocb->ad_seq.seq, num_entries, 
			sizeof(das_msg_t));
	pocb->ad_seq.n_entries = num_entries;
#ifdef DO_TRACE
	printf("das_ad_read_scan_msg: status %d, num_entries %d\n", 
			status, num_entries);
	if (status != -1) {
		int i;
		for (i = 0; i < num_entries; i++) {
			if (i % 8 == 0)
				printf("\n %d-%d: ", i, i+7);
			printf("%d ", pocb->ad_seq.seq[i]);
		}
		printf("\n");
	}
#endif
	return (status);
}

int das_ad_interrupt_attach(IOFUNC_ATTR_T *pattr)
{
	das_info_typ *pinfo = &pattr->das_info;
	int intID;

	printf("DAS_DEFAULT.C:das_ad_interrupt_attach: Attaching pattr->pinfo->irq %d to &pattr->ad_pulse_event \n", pinfo->irq);

	/* Must request I/O privilege */
	ThreadCtl( _NTO_TCTL_IO, 0 );

	intID = (InterruptAttachEvent(pinfo->irq, 
		&pattr->ad_pulse_event, _NTO_INTR_FLAGS_TRK_MSK));
	printf("DAS_DEFAULT.C:das_ad_interrupt_attach: intID %d\n", intID);
	return intID;
}

 int das_ad_interrupt_detach(int id)
{
	return (InterruptDetach(id));
}

int das_default_ad_set_scan(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			int num_entries)
{
	IOFUNC_ATTR_T *pattr = pocb->io_ocb.attr;
	das_info_typ *pinfo = &pattr->das_info;
	int status = EOK;

#ifdef DO_TRACE
	printf("enter das_default_ad_set_scan, num_entries %d, irq %d\n",
		num_entries, pinfo->irq);
#endif
	if ((status=das_ad_read_scan_msg(ctp, pocb, num_entries)) == ERROR)
		return (ENOSYS);

	/** perform any hardware operations necessary before attaching interrupt 
	 */

	if (pinfo->irq != 0)
		status = das_ad_interrupt_attach(pattr);
#ifdef DO_TRACE
        printf("DAS_DEFAULT.C:das_default_ad_set_scan intID %d\n",status);
#endif

	/** perform any hardware operations necessary after attaching interrupt
	 */

	
	MsgReply(ctp->rcvid, EOK, NULL, 0);
	return (_RESMGR_NOREPLY);
}

/**
 *	Placeholder for function to set the sampling rate for A/D
 *	operations.  The scan list should be set first, since
 *	the A/D clock rate depends on the number of conversions.
 */
int das_default_ad_set_sample(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			int chnl, unsigned long ticks)
{
#ifdef DO_TRACE
	printf("enter das_default_ad_set_sample\n");
#endif
	return (ENOSYS);
}

/**
 *	das_default_ad_term() -  Terminate all current, pending and
 *		 finished A/D requests and empty queues.
 *
 *	If device-specific actions are required, the device-specific function
 *	can call this function after it has finished the hardware work.
 *	May also be called to terminate requests when resource manager
 *	is halted by a signal -- in that case only the device-specific
 *	functions are important.
 */

int das_default_ad_term(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, int chnl)
{
#ifdef DO_TRACE
	printf("enter das_default_ad_term\n");
#endif
	/* Do any device specific actions to terminate requests 
	 * then empty queues
	 */
	if (pocb) {	/// initiated by client call to file descriptor
		pocb->ad.r_hd = NULL;
		pocb->ad.f_hd = NULL;
	}

	return (EOK);
}

/**	das_default_ad_pulse
 *
 *	When pulse is received by the resource manager 
 *	as a result of the InterruptAttachEvent in ad_set_scan, 
 *	a device-specific routine must be called to
 *	reset any interrupt registers, read data registers, etc. Furthermore,
 *	the event registered by the client with das_ad_enqueue 
 *	must be delivered to the client, who will then
 *	do a read to get the data that has been copied into the
 *	fin_entry.buffer.
 */  

int das_default_ad_pulse (message_context_t *ctp, int code,
                        unsigned flags, void *ptr)
{
	int status;
	das_ocb_t *pocb = * (das_ocb_t **) ptr;

        /*
         *      Device is not open, spurious interrupt, or cancelled read.
         */

        if ((pocb == NULL) || (pocb->ad.r_hd == NULL))
		return(_RESMGR_NOREPLY);
#ifdef DO_TRACE
	printf("DAS_DEFAULT.C:das_default_ad_pulse: received ad_pulse_event from interrupt; executing das_handle_interrupt\n");
#endif
	
	das_handle_interrupt(pocb);

        fin_entry.rcvid = pocb->ad.r_hd->rcvid;
        fin_entry.event = pocb->ad.r_hd->event;
        fin_entry.nconv = das_ad_data(fin_entry.buffer,
				 pocb->ad.r_hd->nconv);

        pocb->ad.f_hd = &fin_entry;
        pocb->ad.r_hd = NULL;

        if (fin_entry.rcvid != ERROR) {
#ifdef DO_TRACE
//		if( (++ctr % 50) == 0)
		printf("DAS_DEFAULT.C:das_default_ad_pulse: trying to MsgDeliverEvent: rcvid %d event.sigev_code %d nconv %d\n",
			fin_entry.rcvid, fin_entry.event.sigev_code, fin_entry.nconv);
#endif
		status = MsgDeliverEvent(fin_entry.rcvid, &fin_entry.event); 
		if (status == ERROR)
			printf("Failed to deliver ad pulse event\n");
        }
	return(EOK);
}

/** Placeholder for function to set mode and initial value for timer/counter.
 */
int das_default_tmr_mode(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		 unsigned timer, unsigned mode, unsigned value)
{
#ifdef DO_TRACE
	printf("enter das_default_tmr_mode\n");
#endif
	return (ENOSYS);
}
 
/** Placeholder for function to set timer scan structure for driver.
 *	The actual timer scan routine will be hardware specific, but
 *	if the device supports this function, this routine can be
 *	called by the device specific routine to read the message,
 *	before any hardware specific processing.
 *
 *	Pointer to static das_seq_str is passed from device-independent
 *	code in io_func.c. A different pointer to a device-dependent
 *	structure may be used when calling this from a device-dependent
 *	routine; the idea is to read the data directly into the desired
 *	buffer.
 *
 *	if the device needs this function, the helper routines can be
 *	called by the device specific routine to read the message,
 *	before any hardware specific processing. 
 *
 */

int das_tmr_read_scan_msg(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
			char *plist, int num_entries)

{
	IOFUNC_ATTR_T *pattr = pocb->io_ocb.attr;
	return (MsgRead(ctp->rcvid, plist, num_entries, 0));
}

int das_default_tmr_scan(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
				int num_entries)
{
	unsigned char scanlist[10];	// use device dependent buffer to read
#ifdef DO_TRACE
	printf("enter das_default_tmr_scan\n");
#endif
	return (das_tmr_read_scan_msg(ctp, pocb, scanlist, num_entries));
}

/**
 *	Placeholder function for read of timer values.
 *
 *	Device-specific function can call this function, after
 *	placing correct values in pdata array and count argument,
 *	to format reply. 
 *
 *	Pointer to buffer of static das_seq_str is passed from 
 *	device-independent code in io_func.c. A different pointer to a
 *	device-dependent structure may be used when calling this from a
 *	device-dependent routine; the idea is to reply with data directly
 *	from the desired buffer.
 */
int das_default_tmr_read(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
		int count)
{
	static das_msg_t msg; 		/// don't allocate on stack
	static unsigned int pdata[10];	/// use location where device has stored data

#ifdef DO_TRACE
	printf("enter das_default_tmr_read\n");
#endif
	/*
	 *	 Setup message reply header and data buffer.
	 */
	msg.n = count;		/// tells client how many were read

	SETIOV(&ctp->iov[0], &msg, sizeof(msg));
	SETIOV(&ctp->iov[1], pdata, count * sizeof(unsigned));

	MsgReplyv(ctp->rcvid, EOK, ctp->iov, 2);
		
	return (_RESMGR_NOREPLY);                                       
}
