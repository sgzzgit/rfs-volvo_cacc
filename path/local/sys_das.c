/**\file	
 *	static char rcsid[] = "$Id: sys_das.c 578 2005-08-19 17:28:22Z dickey $";
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *
 *	QNX6 version of the Data Acquistion System (DAS)
 *
 *	A library interface to data acquisition manager calls
 *	for typical analog to digital input conversion procedures.
 *
 *	These routines provide initial access to the device and
 *	allow application code to open, read, write, and close I/O
 *	devices. 
 *
 *	QNX6 version: includes the functionality of QNX4 das_msg.c
 *	which had to be linked separately because of name overlap
 *	with device driver functions. In the QNX6 version, all the
 *	client API can be compiled into the local library, since there
 *	is no name conflict.
 *
 *	"$Log$
 *	"Revision 1.3  2005/08/19 17:28:22  dickey
 *	"Changes made while porting Jbus code to QNX6 and adding Lane Assist Interface
 *	"
 *	"Committing in .
 *	"
 *	"Modified Files:
 *	"	Makefile sys_buff.c sys_das.c sys_das.h sys_qnx6.h sys_rt.h
 *	"	timestamp.c timestamp.h
 *	"
 *	"Revision 1.2  2004/11/01 20:43:27  dickey
 *	"Bugs found and changes made while doing CAN driver.
 *	"
 *	"Modified Files:
 *	" 	das_clt.h sys_das.c
 *	"
 *	"Revision 1.1.1.1  2004/08/26 23:45:04  dickey
 *	"local for IDS, CCW, CACC, etc.
 *	""
 */
#ifdef __QNXNTO__
#include "sys_qnx6.h"
#include "local.h"
#include "sys_mem.h"
#include "sys_das.h"
#include "das_clt.h"
#include <db_comm.h>
#include <db_clt.h>
#include <sys/iomsg.h>

#undef DO_TRACE

#define MSEC_TO_USEC			1000L

/**	
 *
 *	das_typ *das_init(char *ppath, unsigned long sample_rate,
 *		char const *pscan, int num_scan, int chid);
 *
 *	ppath			-	Pointer to the name of the device.
 *
 *	sample_rate		-	Analog to digital sampling rate [msec].
 *						This should be 0 if (unused.
 *						
 *	pscan			-	A list of channels to be scanned.
 *						This should be NULL if (unused.
 *
 *	num_scan		-	Number of channels in the scan list.
 *						This should be -1 if (unused.
 *
 *	chid			-	The QNX6 Channel ID needed to allow
 *					the manager to send a pulse; normally
 *					this will be obtained from the task
 *					structure created by Cogent for the
 *					database (pclt->task->chid) -- (this
 *					use of the word "channel" is not
 *					related to the scanned A/D channels) 
 *					
 *
 *	DESCRIPTION
 *	This call opens the given local I/O device for exclusive use.
 *	If requested, a set of periodic analog input conversions 
 *	are initiated.  It is assumed that the pscan array contains
 *	num_scan valid channel numbers.  Once the DAS_AD_ENQUEUE()
 *	call is made, all num_scan channels will be converted every
 *	sample_rate milliseconds.
 *
 *	The voltages returned by das_ad_read() will be returned in
 *	the order of the scan list.
 *
 *	It is not possible for the application code to modify the
 *	input or output ranges of the I/O device.  Instead, this
 *	can be done by reconfiguring the initialization of the
 *	appropriate device driver.
 *
 *	The returned pointer must be used for subsequent calls
 *	to das library calls.
 *
 *	RETURN
 *	non-NULL	-	If the call succeeds.
 *	NULL		-	If there is a memory error, the device
 *					does not exist, or one of parameters is
 *					incorrect.
 */

das_typ *das_init(char *ppath, unsigned long sample_rate,
	char *pscan, int num_scan, int chid)
{
	das_typ *pdas;
	das_info_typ info;
	bool_typ error_flag;

#ifdef DO_TRACE
	printf("SYS_DAS.C:das_init: %s\n", ppath);
#endif
	if ((pdas = MALLOC(sizeof(das_typ))) == NULL)
		return(NULL);

#ifdef DO_TRACE
	printf("SYS_DAS.C:malloc successful\n");
#endif
	pdas->num_scan = num_scan;

	/*	Set up bad error returns to allow simple exit on failure.
	 */

	error_flag = FALSE;
	pdas->fd = ERROR;
	pdas->chid = 0;

	if ((pdas->fd = das_open(ppath, O_RDWR)) == ERROR) {
#ifdef DO_TRACE
		printf("SYS_DAS.C:open error for %s\n", ppath);
#endif
		error_flag = TRUE;
	} else if (das_get_info(pdas->fd, &info) == ERROR) {
#ifdef DO_TRACE
		printf("SYS_DAS.C:get info error for %s\n", ppath);
#endif
		error_flag = TRUE;
	} else {
		pdas->chid = chid;
		pdas->num_analog = info.num_ai;
		pdas->ad_min = info.ad_min;
		pdas->ad_max = info.ad_max;
		pdas->da_min = info.da_min;
		pdas->da_max = info.da_max;
#ifdef DO_TRACE
		printf("SYS_DAS.C:initializing das pointer 0x%x\n", (int) pdas);
		printf("SYS_DAS.C:num_ai %d, ad_min %f ad_max %f\n", 
			info.num_ai, info.ad_min, info.ad_max);
#endif

		if (info.num_ai < num_scan) {
			error_flag = TRUE;
			printf("SYS_DAS.C:num_ai %d < num_scan %d\n", 
				info.num_ai, num_scan);
		} else if (num_scan <= 0) {
			pdas->ticks = 0;
			pdas->result_size = 0;
		} else {
			pdas->ticks = (unsigned long) (double)
				sample_rate*info.pace_clk/
				(MSEC_TO_USEC*num_scan);

			pdas->result_size = num_scan;
		}
	}

	/*	Allow an early exit.
	 */

	if (error_flag == TRUE) {
#ifdef DO_TRACE
		printf("SYS_DAS.C:das_init: error before scan/sample, errno %d\n", errno);
#endif
		das_done(pdas);
		return(NULL);
	}

	printf("SYS_DAS.C:num_scan %d\n", num_scan);
	if (0 <= num_scan) {
		printf("SYS_DAS.C:using ad_set_scan\n");
		if (das_ad_set_scan(pdas->fd, pscan, pdas->num_scan) == ERROR){
#ifdef DO_TRACE
			printf("SYS_DAS.C:ad_set_scan failed\n");
#endif
			error_flag = TRUE;
		}
	}

	if (error_flag == TRUE) {
		das_done(pdas);
		return(NULL);
	}
	else
		 return(pdas);
}

/**
 *
 *	bool_typ das_done(das_typ *pdas);
 *
 *	pdas	-	Pointer to the data acquisition device.
 *
 *	DESCRIPTION
 *	Closes access to an I/O device, freeing the application's
 *	resources. 
 *
 *	RETURN
 *	TRUE	-	If the call succeeds.
 *	FALSE	-	If the pointer is invalid, or there is a system error.
 */

int coid;
bool_typ das_done(das_typ *pdas)
{
	if (pdas == NULL)
		return(FALSE);

	ConnectDetach(coid);
	if (pdas->fd != ERROR)
	{
		if (pdas->pinput_request != NULL)
			FREE(pdas->pinput_request);

		das_close(pdas->fd);
	}

	FREE(pdas);
	return(TRUE);
}

/**
 *	das_open -  Send a open message to the 'das' resource manager.
 */

int das_open(const char *path, int oflag)
{
	int fd;
#ifdef DO_TRACE
	printf("das_open %s\n", path);
#endif
	fd = open(path, oflag);
	if (fd == ERROR)
		printf("file open error %d\n", errno);
	
	return(fd);
}

/**
 *	das_get_info - 
 */

int das_get_info(int fd, das_info_typ *pinfo)
{
	int status;
	memset(pinfo, 0, sizeof(das_info_typ));
	status = devctl(fd, DCMD_DAS_QUERY, (void *) pinfo,
			sizeof(das_info_typ), NULL);
#ifdef DO_TRACE
	printf("SYS_DAS.C:das_get_info:port %d\n", pinfo->port);
	printf("SYS_DAS.C:das_get_info:irq %d\n", pinfo->irq);
	printf("SYS_DAS.C:das_get_info:num_ai %d\n", pinfo->num_ai);
	printf("SYS_DAS.C:das_get_info:ad_min %f\n", pinfo->ad_min);
	printf("SYS_DAS.C:das_get_info:ad_max %f\n", pinfo->ad_max);
	printf("SYS_DAS.C:das_get_info:ad_step %f\n", pinfo->ad_step);
	printf("SYS_DAS.C:das_get_info:da_min %f\n", pinfo->da_min);
	printf("SYS_DAS.C:das_get_info:da_max %f\n", pinfo->da_max);
	printf("SYS_DAS.C:das_get_info:da_step %f\n", pinfo->da_step);
	printf("SYS_DAS.C:das_get_info:ad_ticks %ld\n", pinfo->ad_ticks);
	printf("SYS_DAS.C:das_get_info:pace_clk %ld\n", pinfo->pace_clk);
#endif
	return (status);
}

/**
 *	das_close - Send a close message to the driver 
 */

int das_close(int fd)
{
	return(close(fd));
}

/**
 *	das_da_sync -  Perform a synchronous D/A conversion. 
 */

int das_da_sync(int fd, int chnl, float data)
{
	das_da_sync_t da_sync;
	da_sync.chnl = chnl;
	da_sync.data = data;

#ifdef DO_TRACE
	printf("das_da_sync: fd %d, chnl %d, data %f\n", fd, chnl, data);
#endif
	return (devctl(fd, DCMD_DAS_DA_SYNC, (void *) &da_sync,
			 sizeof(das_da_sync_t), NULL));

}

/**
 *
 *	bool_typ das_set_da(das_typ *pdas, int channel, float value);
 *
 *	pdas	-	Pointer to the data acquisition device.
 *	channel	-	The desired output channel.
 *	value	-	The desired output voltage.
 *
 *	DESCRIPTION
 *
 *	Sends the given voltage to an I/O device.  The analog output is
 *	set synchronously.  Values which are out of bound for the
 *	given device may be truncated.
 *
 *	RETURN
 *	TRUE	-	If the call succeeds.
 *	FALSE	-	If the pointer or channel is invalid.
 */

bool_typ das_set_da(das_typ *pdas, int channel, float value)
{
	if (pdas == NULL)
		return(FALSE);

#ifdef DO_TRACE
	printf("das_set_da: fd %d, chnl %d, data %f\n",
			 pdas->fd, channel, value);
#endif
	if (das_da_sync(pdas->fd, channel, value) != EOK)
		return(FALSE);
	else
		return(TRUE);
}

/**	
 *
 *	int DAS_DA_TERM(das_typ *pdas);
 *
 *	pdas	-	Pointer to the data acquisition device.
 *
 *	This terminates any D/A conversions on the given data
 *	acquisition device.
 *
 *	Returns status from devctl.	
 *
 */

int das_da_term(das_typ *pdas, int chnl)
{
	return (devctl(pdas->fd, DCMD_DAS_DA_TERM, (void *) &chnl,
			 sizeof(int), NULL));
}

/**
 *  das_digital_dir - Send a message to set digital I/O direction (either
 *                    input or output) on a given port.
 *
 *  fd    -    Pointer to the acquisition device
 *  port, bits	- Same data type used as for das_digital_out, but
 *		coding may be device dependent 
 */

int das_digital_dir(int fd, short int port, long bits)
{
	das_din_t das_dir;
	das_dir.port = port;
	das_dir.bits = bits;
	return (devctl(fd, DCMD_DAS_DIG_DIR, (void *) &das_dir,
			 sizeof(das_din_t), NULL));
}

/**
 *	das_digital_out - Send message to output data on digital port specified by 
 *					'port'.
 *
 *  fd     -    Pointer to the acquisition device
 *  port   -    Port address for output
 *  bits   -    Desired bit settings (for bits specified in the mask)
 *  mask   -    Each bit is '1' if this bit is to be changed here, '0' if not
 *
 *	Returns EOK unless devctl error.
 */

int das_digital_out(int fd, int port, long bits, long mask,
	long *pold_bits, long *pnew_bits)
{
	int err;
	das_dout_t das_dout;
	das_dout.port = port;
	das_dout.mask = mask;
	das_dout.bits = bits;
	das_dout.old_bits = 0xdeadbeef;	/// these will be changed
	das_dout.new_bits = 0xdeadbeef; /// unless devctl fails
#ifdef DO_TRACE
	printf("das_digital_out: port %d, bits 0x%lx, mask 0x%lx\n",
			 port, bits, mask);
#endif
	err = devctl(fd, DCMD_DAS_DIG_OUT, (void *) &das_dout,
			 sizeof(das_dout_t), NULL);
#ifdef DO_TRACE
	printf(" status %d, port %d, bits 0x%lx, mask 0x%lx\n",
			err, das_dout.port, das_dout.bits, das_dout.mask);
	printf(" old 0x%lx, new 0x%lx\n",
			das_dout.old_bits, das_dout.new_bits);
#endif

	if (pold_bits)
		*pold_bits = das_dout.old_bits;
	if (pnew_bits)
		*pnew_bits = das_dout.new_bits;
#ifdef DO_TRACE
	if (pold_bits && pnew_bits)
		printf("pold_bits 0x%x old_bits 0x%lx, pnew_bits 0x%x new_bits 0x%lx\n",
		 pold_bits, *pold_bits, pnew_bits, *pnew_bits);
#endif

	return(err);
}

/**	
 *
 *	int DAS_DIGITAL_IN(int fd, int port, long *pbuf);
 *
 *	fd  	-	Pointer to the data acquisition device.
 *
 *	port	-	The digital input port being read.
 *				This will vary depending on the specific
 *				device type.
 *
 *	pbuff	-	Buffer for the input value.
 *
 *	DESCRIPTION
 *
 *	This macro reads digital input values into the given buffer.
 *	For efficiency with devices such as the PCTIO-10 and the
 *	ATMIO-16, this call causes all digital input ports to be read,
 *	and the values are consolidated into the buffer, regardless
 *	of the port specified.
 *
 *	Returns EOK unless devctl error.
 */

int das_digital_in(int fd, int port, long *pbits)
{
	int err;
	das_din_t das_din;
	das_din.port = port;
	das_din.bits = 0;
#ifdef DO_TRACE
	printf("das_digital_in: port %d\n", das_din.port);
#endif
	err = devctl(fd, DCMD_DAS_DIG_IN, (void *) &das_din,
			 sizeof(das_din_t), NULL);
	*pbits = das_din.bits;  
	return(err);
}

/**	Macros define in sys_das.h are implemented with the lower-case
 *	named functions
 *
 *	DAS_AD_ENQUEUE(das_typ *pdas);
 *	DAS_AD_PULSE(das_typ *pdas, IP_Msg *pmsg, IP_MsgInfo *pmsg_info);
 *
 *	pdas	-	Pointer to the data acquisition device structure.
 *
 *	DESCRIPTION
 *
 *	The DAS_AD_ENQUEUE() macro initiates a previously defined
 *	analog to digital input conversions, as specified in the
 *	das_init() call.  The completion of a conversion will
 *	send a pulse to the application process.  The application
 *	process receives this pulse via a IP_Receive(), MsgReceive
 *	or MsgReceivePulse.  Messages can be tested using
 *	the DAS_AD_PULSE() macro to identify a particular pulse value.
 *
 *	RETURN for DAS_AD_ENQUEUE():
 *
 *	EOK		-	If the call succeeds.
 *	ERROR	-	If there is a device or system failure.
 *
 *	RETURN for DAS_AD_PULSE():
 *	TRUE if message is a pulse and pulse code matches	
 *	FALSE otherwise	
 *
 */

int chid;
int das_ad_enqueue(das_typ *pdas)
{
	struct sigevent event;

	/* we need a channel to receive the pulse notification on */
  	chid = ChannelCreate( 0 ); 

	/* we need a connection to that channel for the pulse to be
	     delivered on */
	if( (coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0)) == -1) {
		perror("das_ad_enqueue") ;
		return(-1);
		}

	printf("SYS_DAS.C:das_ad_enqueue: coid for enqueue: %#x chid %#x\n", coid, chid);

	/** fill in the event structure for a pulse; use the file 
	 *  descriptor of the DAS device so that this pulse can be
	 *  distinguished from other DAS devices that may be sending
	 *  pulses to this client
	 */
	SIGEV_PULSE_INIT(&event, coid, SIGEV_PULSE_PRIO_INHERIT, 
                    pdas->fd, 0);
	printf("SYS_DAS.C:das_ad_enqueue: sigevent.sigev_notify %#x sigevent.sigev_signo %#x sigevent.sigev_coid %#x sigevent.sigev_id %#x\n",
		event.sigev_notify, event.sigev_signo, 
		event.sigev_coid, event.sigev_id); 

	return (devctl(pdas->fd, DCMD_DAS_AD_ENQ, (void *) &event,
			 sizeof(struct sigevent), NULL));
}

/**
 *	das_ad_pulse(das_typ *pdas)
 *
*/

int das_ad_pulse(das_typ *pdas)
{
	struct _pulse pulse;
	static int ctr;
	time_t mytime;
	int status;
	int rcvid;

#ifdef DO_TRACE
	printf("SYS_DAS.C:das_ad_pulse called; waiting for pulse from MsgDeliverEvent in das_default.c:das_default_ad_pulse....\n");
#endif

	if( (rcvid = MsgReceivePulse( chid, &pulse, sizeof( pulse ), NULL )) 
		== ERROR) {
		perror("das_ad_pulse:MsgReceivePulse:");
		printf("SYS_DAS.C:das_ad_pulse:MsgReceivePulse:coid %#x pulse.type %#x pulse.subtype %#x pulse.code %#x pulse.scoid %#x\n", 
			pdas->fd, pulse.type, pulse.subtype, 
			pulse.code, pulse.scoid);
		return(FALSE);
		}

	if( pulse.code == pdas->fd) 
		return (das_ad_read(pdas, pdas->volts, pdas->result_size));

		printf("SYS_DAS.C:das_ad_pulse: I'm being called, but pulse.code %#x != pdas->fd %d pulse.type %d pulse.subtype %d\n",
			pulse.code, pdas->fd, pulse.type, pulse.subtype);
	return FALSE;
}

/**
 *	int DAS_AD_READ(das_typ *pdas);
 *
 *	pdas	-	Pointer to the data acquisition device.
 *
 *	The DAS_AD_READ() macro obtains a completed set of A/D
 *	conversions from the given device manager.
 *
 *	Returns	 ERROR if there is a system failure.
 *	otherwise number of channels read.
 *
 */

int das_ad_read(das_typ *pdas, float *pbuf, int nchan)
{
	das_msg_t tx_msg;	/* send message header */
	das_msg_t rx_msg;	/* reply message header */
	iov_t rx_iov[2];	/* two-part reply */
	
	memset(&tx_msg, 0, sizeof(das_msg_t));
	tx_msg.io.i.type = _IO_MSG;
	tx_msg.io.i.mgrid = _IOMGR_DAS; 
	tx_msg.io.i.subtype = IOMSG_DAS_AD_READ; 
	tx_msg.n = nchan;

/* Definition of SETIOV macro (from /usr/include/unistd.h) 
   Does not allocate memory

#define SETIOV(_iov, _addr, _len)  ((_iov)->iov_base = (void *)(_addr), (_iov)->iov_len = (_len))

*/
	/* two-part reply avoids extra copy of float A/D data */
	SETIOV(rx_iov + 0, &rx_msg, sizeof(rx_msg));
	SETIOV(rx_iov + 1, pbuf, nchan * sizeof(float) );

#ifdef DO_TRACE
	printf("SYS_DAS.C:das_ad_read: fd %d nchan %d\n", pdas->fd, nchan);
#endif

	if (MsgSendsv(pdas->fd, &tx_msg, sizeof(tx_msg), rx_iov, 2) == -1) {
		perror("MsgSendsv");
		return (ERROR);
		}
	else
		return (rx_msg.n);
}

/**	
 *
 *	int DAS_AD_TERM(das_typ *pdas);
 *
 *	pdas	-	Pointer to the data acquisition device.
 *
 *	This terminates any A/D conversions on the given data
 *	acquisition device.
 *
 *	Returns status from devctl.	
 *
 */

int das_ad_term(das_typ *pdas, int chnl)
{
	return (devctl(pdas->fd, DCMD_DAS_AD_TERM, (void *) &chnl,
			 sizeof(int), NULL));
}

/**
 *	das_ad_set_scan - Send message to set a block scan entry for A/D operations.
 *		This should be done before setting the sample rate,
 *		since the sample rate depends on the number of conversions.
 *	
 *	Returns status from message send.
 */

int das_ad_set_scan(int fd, char *plist, int n_entries)
{
	das_msg_t tx_msg;	/* send message header */
	das_msg_t rx_msg;	/* reply message header */
	iov_t tx_iov[2];	/* two-part transmit */
	int status;
	
	memset(&tx_msg, 0, sizeof(das_msg_t));
	tx_msg.io.i.type = _IO_MSG;
	tx_msg.io.i.mgrid = _IOMGR_DAS; 
	tx_msg.io.i.subtype = IOMSG_DAS_AD_SET_SCAN; 
	tx_msg.n = n_entries;

	/* two-part send avoids extra copy of list */
	SETIOV(tx_iov + 0, &tx_msg, sizeof(tx_msg));
	SETIOV(tx_iov + 1, plist, n_entries);
				
	status = MsgSendvs(fd, tx_iov, 2, &rx_msg, sizeof(rx_msg));
#ifdef DO_TRACE
	printf("SYS_DAS.C:das_ad_set_scan: send returns %d\n", status);
#endif
	return (status);
}

/**
 *	das_ad_set_sample -  Send message to set the sampling rate for A/D 
 *		operations.  The scan list should be set first, since
 *		the A/D clock rate depends on the number of conversions.
 */

int das_ad_set_sample(int fd, int chnl, unsigned long ticks)
{
	static das_sample_t set_sample;
	set_sample.ticks = ticks;
	set_sample.chnl	= chnl;
	return (devctl(fd, DCMD_DAS_AD_SET_SMP, (void *) &set_sample,
			 sizeof(set_sample), NULL));
}

/**	
 *
 *	int DAS_TMR_MODE(das_typ *pdas, unsigned timer,
 *		unsigned mode, unsigned value);
 *
 *	pdas	-	Pointer to the data acquisition device.
 *	timer	-	Timer/counter to be changed.
 *	mode	-	Mode setting for the timer/counter.
 *	value	-	Initialization value for the timer/counter.
 *
 *	DESCRIPTION
 *	This macro sets the mode and initial value for timer/counter.
 *	For the PCTIO-10 and ATMIO-16, this is the mode sent to the
 *	AMD 9513 chip.  See the National and AMD documentation for more
 *	information.
 *
 *	Returns status from devctl.
 *
 */

int das_tmr_mode(int fd, unsigned timer, unsigned mode, unsigned value)
{
	static das_tmr_mode_t tmr_mode;
	tmr_mode.timer	= timer;
	tmr_mode.mode	= mode;
	tmr_mode.value	= value;
	return (devctl(fd, DCMD_DAS_TMR_MODE, (void *) &tmr_mode,
			 sizeof(tmr_mode), NULL));
}

/**	
 *
 *	int DAS_TMR_SCAN(das_typ *pdas, char *ptimers, int num_entries);
 *
 *	pdas		-	Pointer to the data acquisition device.
 *	ptimers		-	List of timer channels to be read.
 *	num_entries	-	Number of timer channels to be read.
 *
 *	DESCRIPTION
 *	This macro sets up the list of timers which will be read
 *	during a subsequent call to DAS_TMR_READ().  It is only
 *	necessary to call DAS_TMR_SCAN() once.  The valid
 *	channels and number of entries is limited by the specific
 *	I/O device.  The mode for a particular timer should be set
 *	up using the DAS_TMR_MODE() call.
 *
 *	Returns status from message send.	
 *
 */

int das_tmr_scan(int fd, char *ptimers, int num_entries)
{
	das_msg_t tx_msg;	/* send message header */
	das_msg_t rx_msg;	/* reply message header */
	iov_t tx_iov[2];	/* two-part transmit */
	
	memset(&tx_msg, 0, sizeof(das_msg_t));
	tx_msg.io.i.type = _IO_MSG;
	tx_msg.io.i.mgrid = _IOMGR_DAS; 
	tx_msg.io.i.subtype = IOMSG_DAS_TMR_SCAN; 
	tx_msg.n = num_entries;

	/* two-part send avoids extra copy of list */
	SETIOV(tx_iov + 0, &tx_msg, sizeof(tx_msg));
	SETIOV(tx_iov + 1, ptimers, num_entries);
				
	return (MsgSendvs(fd, tx_iov, 2, &rx_msg, sizeof(rx_msg)));
}

/**	
 *
 *	int DAS_TMR_READ(das_typ *pdas, unsigned *pdata, int count);
 *
 *	pdas	-	Pointer to the data acquisition device.
 *	pdata	-	Pointer to buffer for result of read call.
 *	count	-	Number of channels to be read/size of buffer.
 *
 *	DESCRIPTION
 *	This macro reads a set of timer/counters, and returns the result
 *	into the given buffer.  The set of timers to be read should
 *	have been set up via the DAS_TMR_SCAN() call.
 *
 *	Returns ERROR if there is a system failure.
 *	number of channels read otherwise
 *
 */

int das_tmr_read(int fd, unsigned *pdata, int count)
{
	das_msg_t tx_msg;	/* send message header */
	das_msg_t rx_msg;	/* reply message header */
	iov_t rx_iov[2];	/* two-part reply */
	
	memset(&tx_msg, 0, sizeof(das_msg_t));
	tx_msg.io.i.type = _IO_MSG;
	tx_msg.io.i.mgrid = _IOMGR_DAS; 
	tx_msg.io.i.subtype = IOMSG_DAS_TMR_READ; 
	tx_msg.n = count;

	/* two-part reply avoids extra copy of unsigned timer data */
	SETIOV(rx_iov + 0, &rx_msg, sizeof(rx_msg));
	SETIOV(rx_iov + 1, pdata, count * sizeof(unsigned));
				
	if (MsgSendsv(fd, &tx_msg, sizeof(tx_msg), rx_iov, 2) == -1)
		return (ERROR);
	else
		return (rx_msg.n);
}

#endif
