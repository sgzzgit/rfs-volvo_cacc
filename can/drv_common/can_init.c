/**\file	
 *
 *
 *	can_init.c module contain the initialization  routines
 *	used in the CAN driver
 *
 *	The following is the list of functions in this module.
 *	
 *		- can_init
 *		- can_handle_interrupt
 *		- pulse_init
 *
 * Copyright (c) 2004   Regents of the University of California
 */

#include <sys_os.h>
#include <sys_rt.h>
#include <sys_ini.h>

#include "can_defs.h"
#include "can_man.h"

#undef DO_TRACE

/*	The DAS-style initialization file may be used for the
 *	digital I/O on the SSV CAN card.
 */

#define DEFAULT_CONFIG		"realtime.ini"
#define DEFAULT_DEVICE		"/dev/can1"

#define INI_IRQ_ENTRY		"Irq"
#define INI_PORT_ENTRY		"Port"
#define INI_EXT_ENTRY		"Ext"

#define DEFAULT_IRQ		0	///by default, no interrupt
#define DEFAULT_PORT		0x210

#define DEFAULT_PRIORITY	19
#define DEFAULT_QSIZE		150

static void usage(char *pargv0);

static int sig_list[] =
{
	SIGTERM,
	SIGKILL,
	ERROR
};

jmp_buf exit_env;
void sig_hand(int code);

int can_notify_client_err = 0;
int mask_count_non_zero = 0;

/*
 *	can_init() -
 *
 *	This function is called from the main() during the initialization 
 *	of the CAN driver to read the .ini file and initialize
 *	the can_info structure, and initialize the function tables.  
 */

void can_init(int argc, char *argv[], resmgr_connect_funcs_t *pconn,
	resmgr_io_funcs_t *pio, IOFUNC_ATTR_T *pattr)
{
	int opt;
	// These are set to 1 if specified by command line arguments
	int arg_speed = 0;
	int arg_port = 0;
	int arg_irq = 0;
	int arg_ext = 0;

	char *pconfig;
	FILE *pfile;
	can_info_t *pinfo = &pattr->can_info;
	
	/*
	 *	Setup default parameters.
	 */

	pconfig = DEFAULT_CONFIG;
	pattr->verbose_flag = FALSE;
	pattr->devname = DEFAULT_DEVICE;
	pattr->notify_pocb = NULL;
	pinfo->use_extended_frame = 1;	// by default, use extended frame
	pinfo->irq = DEFAULT_IRQ; 
	pinfo->port = DEFAULT_PORT; 
	pinfo->filter.id = 0;		// set up to accept all messages from all
	pinfo->filter.mask = 0xff;
	pinfo->bit_speed = 250;
	
	/** If arguments are specified, they override config file
	 */
	while((opt = getopt(argc, argv, "e:f:i:n:p:s:v?")) != EOF) {
		switch(opt) {
		case 'e':
			pinfo->use_extended_frame = atoi(optarg);
			arg_ext = 1;
			break;
		case 'f':
			pconfig = strdup(optarg);
			break;
		case 'i':
			pinfo->irq = atoi(optarg);
			arg_irq = 1;
			break;
		case 'n':
			pattr->devname = strdup(optarg);
			break;
		case 'p':
			pinfo->port = strtol(optarg,(char **)NULL,0); 
			printf("I/O port for base address 0x%x\n", pinfo->port);
			arg_port = 1;
			break;
		case 's':
			pinfo->bit_speed = atoi(optarg);
			printf("Bit speed set to %d\n", pinfo->bit_speed);
			arg_speed = 1;
			break;
		case 'v':
			pattr->verbose_flag = TRUE;
			break;

		default:
		case '?':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		}
	}

	/** Initialize from config file, if found.
	 */
	if((pfile = get_ini_section(pconfig, pattr->devname)) == NULL) {
		printf("No section %s in %s file found, using args, defaults\n",
			pattr->devname, pconfig);
		fflush(stdout);
	} else {

		if (!arg_irq)
			pinfo->irq = get_ini_long(pfile, INI_IRQ_ENTRY,
								 DEFAULT_IRQ);
		if (!arg_port)
			pinfo->port = get_ini_hex(pfile, INI_PORT_ENTRY,
						 DEFAULT_PORT);
		fclose(pfile);
	}
	/**
	 * Initialize circular buffers for CAN read and write.
	 */
	init_circular_buffer(&pattr->in_buff, CAN_IN_BUFFER_SIZE, 
							sizeof(can_msg_t));
	init_circular_buffer(&pattr->out_buff, CAN_OUT_BUFFER_SIZE,
							 sizeof(can_msg_t));
	printf("Circular buffers initialized\n");
	fflush(stdout);

	/** Initialize resource manager function tables with
	 *  CAN specific function for devctl 
	 */
	pio->devctl = io_devctl;

	/** Initialize resource manager function tables with
	 *  CAN specific function for open 
	 */
	pconn->open = io_open;

	/** Set up exit environment for kill signals
	 */

	if (setjmp(exit_env) != 0) {
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);
	printf("Leaving caninit\n");
	fflush(stdout);

}

/**     can_handle_interrupt
 *
 *      When pulse is received by the resource manager
 *      as a result of the InterruptAttachEvent in pulse_init
 *      the device-specific routine can_dev_interrupt is called to
 *      reset any interrupt registers, etc. Furthermore,
 *      any event registered by the client with can_arm
 *      is delivered to the client, who will then
 *      do a read to get the data that has been copied into the
 *      message buffer..
 */

int can_handle_interrupt (message_context_t *ctp, int code, unsigned flags,
				 void *ptr)
{
        int status;
	int mask_count;
	int num_recv = 0;	// set to number of CAN messages received 
	IOFUNC_ATTR_T *pattr = (IOFUNC_ATTR_T *) ptr;	
	can_info_t *pinfo = &pattr->can_info;
	RESMGR_OCB_T *pocb = pattr->notify_pocb;

#ifdef DO_TRACE
        printf("enter can_handle_interrupt: pocb 0x%x\n", (unsigned int) pocb);
	fflush(stdout);
#endif
        
        num_recv = can_dev_interrupt(pattr);
	
#ifdef DO_TRACE
	printf("can_handle_interrupt: num_recv %d, pocb 0x%x\n", 
		num_recv, (unsigned int) pocb);
	fflush(stdout);
#endif
	while (num_recv > 0) {
		if (pocb != NULL) {
			status = MsgDeliverEvent(pocb->rcvid, &pocb->clt_event);
#ifdef DO_TRACE
			printf("MsgDeliverEvent %d \n", pocb->rcvid);
			fflush(stdout);
#endif
			if (status == ERROR) {
				can_notify_client_err++;
#ifdef DO_TRACE
				printf("Failed to deliver CAN client notify event\n");
#endif
			}
		}
		num_recv--;
        }

	if ((mask_count = InterruptUnmask(pinfo->irq, pinfo->intr_id)) != 0) {
		mask_count_non_zero++;
	}
#ifdef DO_TRACE
	printf("mask_count %d\n", mask_count);
	fflush(stdout);
#endif

        return(EOK);
}

/** Attach pulse to be sent by interrupt handler to event that
 *  will connected to the interrupt by InterruptAttachEvent in can_dev_arm.
 */
void pulse_init(dispatch_t *dpp, IOFUNC_ATTR_T *pattr)
{
	can_info_t *pinfo = &pattr->can_info;
	struct sigevent *pevent;
	int mask_count;

#ifdef DO_TRACE
	printf("pulse_init: irq %d\n", pinfo->irq);
	fflush(stdout);
#endif
	if (pinfo->irq != 0) {
		pevent = &pattr->hw_event;
		if ((pevent->sigev_code = pulse_attach(dpp,
			 MSG_FLAG_ALLOC_PULSE, 0, can_handle_interrupt,
			 pattr)) == ERROR) {
			fprintf(stderr, "Unable to attach irq_handler pulse.\n");
			exit(EXIT_FAILURE);
		}
	        if ((pevent->sigev_coid = message_connect(dpp,
			 MSG_FLAG_SIDE_CHANNEL)) == ERROR) {
			fprintf(stderr, "Unable to attach pulse to channel.\n");
			exit(EXIT_FAILURE);
		}
		pevent->sigev_notify = SIGEV_PULSE;
		pevent->sigev_priority = -1;
		pevent->sigev_value.sival_int = 0;	
		if ((pinfo->intr_id = InterruptAttachEvent(pinfo->irq,
				 pevent, _NTO_INTR_FLAGS_TRK_MSK)) == -1) {
			perror("InterruptAttach");
		} else {
			printf("IRQ %d attached: ID %d\n", pinfo->irq,
				pinfo->intr_id);
		}
		fflush(stdout);
		if ((mask_count = InterruptUnmask(pinfo->irq, pinfo->intr_id)) != 0) {
			printf("mask count: %d\n", mask_count);
		}
	}

}

static void usage(char *pargv0)
{
	fprintf(stderr, "%s:\t-[pfv]\n", pargv0);
	fprintf(stderr, "\t\tp\tPath name (%s).\n", DEFAULT_DEVICE);
	fprintf(stderr, "\t\tf\tConfiguration file (%s).\n", DEFAULT_CONFIG);
	fprintf(stderr, "\t\tv\tVerbose mode.\n");
	fprintf(stderr, "\t\t?\tPrints this message.\n");
}

void sig_hand(int code)
{
        longjmp(exit_env, code);
}
