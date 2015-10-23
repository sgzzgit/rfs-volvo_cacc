/**\file
 * io_func.c
 *
 *	Replacement functions for the default iofunc functions
 *	in the DAS resource manager.	
 */
#include <sys_qnx6.h>
#include <local.h>
#include <das_clt.h>
#include "das_man.h"
#undef DO_TRACE

int io_devctl (resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
	int status;
	int dcmd;
	void *data;
	IOFUNC_ATTR_T *pattr = ocb->io_ocb.attr;
	das_func_t *pfunc = &pattr->func;

	/* temporary pointers for devctl data */
	struct sigevent event;
	das_sample_t *pset;
	das_da_sync_t *psync;
	das_din_t *pdin;
	das_dout_t *pdout;
	das_tmr_mode_t *ptmr_mode;
	long mask;
	int chnl;

	/// see if it's a standard POSIX-supported devctl()
	if ((status = iofunc_devctl_default (ctp, msg, (iofunc_ocb_t *) ocb)) != _RESMGR_DEFAULT) {
		return (status);
	}

	/** set up "data" to point to the data area after the devctl msg struct
         * this pointer is the same for input message and output message type
	 * save dcmd field and clear message header 
	 * when data is to be returned in the reply, ctp->iov is set
	 * up, otherwise no data is returned
	 */
	data = _DEVCTL_DATA(msg->i);  
	dcmd = msg->i.dcmd;
	memset (&msg->o, 0, sizeof (msg->o));

	///  see which command it was, and act on it
	switch (dcmd) {

	case DCMD_DAS_QUERY:
		* (das_info_typ *) data = pattr->das_info; 
		msg->o.nbytes = sizeof(das_info_typ);
	        return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + msg->o.nbytes));
		
	case DCMD_DAS_AD_ENQ:
		event =  * (struct sigevent *) data;
		return (pfunc->ad_enqueue(ctp, ocb, event));
			
	case DCMD_DAS_AD_TERM:
		chnl = * (int *) data;
		return (pfunc->ad_term(ctp, ocb, chnl));
				
	case DCMD_DAS_AD_SET_SMP:
		pset = (das_sample_t *) data;
		return (pfunc->ad_set_sample(ctp, ocb,
			 pset->chnl, pset->ticks));
				
	case DCMD_DAS_DA_SYNC:
		psync = (das_da_sync_t *) data;
		status = pfunc->da_sync(ctp, ocb, psync->chnl, psync->data);
#ifdef DO_TRACE
		printf("DCMD_DAS_AD_DA_SYNC: status %d\n", status);
#endif
		return (status);

	case DCMD_DAS_DIG_DIR:
		pdin = (das_din_t *) data;
#ifdef DO_TRACE
		printf("DCMD_DAS_DIG_DIR: mask %#x\n", (unsigned int)pdin->bits);
#endif
		return (pfunc->digital_dir(ctp, ocb, pdin->bits));
				
	case DCMD_DAS_DIG_IN:
		pdin = (das_din_t *) data;
#ifdef DO_TRACE
		printf("DCMD_DAS_DIG_IN: port %#x bits %#x\n", 
			pdin->port, pdin->bits);
#endif
		status = pfunc->digital_in(ctp, ocb, pdin->port,
				&pdin->bits);
		msg->o.nbytes = sizeof(das_din_t);
	        return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + msg->o.nbytes));
		
	case DCMD_DAS_DIG_OUT:
		pdout = (das_dout_t *) data;
		status = pfunc->digital_out(ctp, ocb, pdout->port,
				pdout->mask,pdout->bits, &pdout->old_bits,
				&pdout->new_bits);
		msg->o.nbytes = sizeof(msg) + sizeof(das_dout_t);
#ifdef DO_TRACE
		printf("&msg->o 0x%lx, pdout 0x%lx\n", (long) &msg->o, (long) pdout);
		printf("DCMD_DAS_DIG_OUT: old 0x%lx, new 0x%lx\n",
				pdout->old_bits, pdout->new_bits);
#endif
	        return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + msg->o.nbytes));
				
	case DCMD_DAS_TMR_MODE:
		ptmr_mode = (das_tmr_mode_t *) data;
		return(pfunc->tmr_mode(ctp, ocb, ptmr_mode->timer,
			ptmr_mode->mode, ptmr_mode->value));
				
	/* This dcmd is for testing only, will call pfunc->ad_pulse
	 * normally called only when hardware interrupt is received.
	 */
	case DCMD_DAS_TEST_PULSE:
#ifdef DO_TRACE
		printf("DAS_TEST_PULSE, ppocb 0x%x\n", (int) &ocb);
#endif
		status = pfunc->ad_pulse((message_context_t *) ctp, 0, 0, &ocb);
		return(status);
	default:
		return (ENOSYS);
	}
}

/** Custom messages are used instead of devctl when the send and reply
 * messages are of different sizes and at least one of them is of significant
 * size. A two-part send or reply avoids extra copying.
 *
 * This function is called by the system resource manager when a message
 * of type IO_MSG is received, the subtype is used for our application
 * specific functions. The type das_msg_t is used to cast the system
 * io_msg_t in order to get the data count that is sent in the second part of
 * the message, or is returned in the second part of the reply.
 */
int io_msg (resmgr_context_t *ctp, io_msg_t *msg, RESMGR_OCB_T *ocb)
{
	int status;
	IOFUNC_ATTR_T *pattr = ocb->io_ocb.attr;
	das_func_t *pfunc = &pattr->func;
	das_msg_t *pdas_msg = (das_msg_t *) msg;	

#ifdef DO_TRACE
	printf("io_msg, subtype %d\n", msg->i.subtype);
#endif
	switch (msg->i.subtype) {

	case IOMSG_DAS_AD_READ:
		return (pfunc->ad_read(ctp, ocb));	

	case IOMSG_DAS_AD_SET_SCAN:
		status = pfunc->ad_set_scan(ctp, ocb, pdas_msg->n);
#ifdef DO_TRACE
		printf("IOMSG_DAS_AD_SET_SCAN: status %d\n", status);
#endif
		return (status);

	case IOMSG_DAS_TMR_SCAN:
		return (pfunc->tmr_scan(ctp, ocb, pdas_msg->n));
				
	case IOMSG_DAS_TMR_READ:
		return (pfunc->tmr_read(ctp, ocb, pdas_msg->n));
	default:
		return (_RESMGR_NOREPLY);	/* ignore */
	}
	return (ENOSYS);
}
	
/** 
 *	Calls a device driver specific function to perform any
 *	necessary hardware actions before closing the device.
 *	then call the default close_ocb function.
 */
int io_close_ocb(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
	int status;
	IOFUNC_ATTR_T *pattr = ocb->io_ocb.attr;
	das_func_t *pfunc = &pattr->func;
	pfunc->das_close(ctp, ocb);
	return(iofunc_close_ocb_default(ctp, NULL, (iofunc_ocb_t *) ocb));
}
	
