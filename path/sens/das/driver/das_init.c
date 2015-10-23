/**\file	
 *
 *	static char rcsid[] = "$Id: das_init.c 6710 2009-11-11 01:52:27Z dickey $";
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *	das_init.c module contain the initialization and termination routines
 *	used in the acquisition manager.  
 *
 *	Rewritten for QNX6, since resource manager library simplifies
 *	support for basic file system functions.
 *
 *	The following is the list of functions in this module.
 *	
 *		-  man_done()	
 *		-  man_init()
 */

#include <sys_os.h>
#include <sys_rt.h>
#include <sys_ini.h>

#include "das_clt.h"
#include "das_man.h"

#undef DO_TRACE

/*	The device name is also used to specify the section for
 *	the initialization file.
 */

#define DEFAULT_CONFIG		"realtime.ini"
#define DEFAULT_DEVICE		"/dev/dmm32"

#define INI_IRQ_ENTRY		"Irq"
#define INI_PORT_ENTRY		"Port"
#define INI_CLOCK_ENTRY		"Clock"
#define INI_AD_ENTRY		"NumAnalog"
#define INI_AD_MIN_ENTRY	"MinAD"
#define INI_AD_MAX_ENTRY	"MaxAD"
#define INI_DA_MIN_ENTRY	"MinDA"
#define INI_AD_STEP_ENTRY	"ADStep"
#define INI_DA_MAX_ENTRY	"MaxDA"
#define INI_DA_STEP_ENTRY	"DAStep"
#define INI_TICKS_ENTRY		"Ticks"
#define INI_TIMER_PULSE		"TimerPulse"

#define DEFAULT_IRQ		0	///by default, no interrupt
#define DEFAULT_PORT		0x220
#define DEFAULT_CLOCK		1000000
#define DEFAULT_AD		8
#define DEFAULT_MIN		0.0
#define DEFAULT_MAX		10.0
#define DEFAULT_STEP		4096.0
#define DEFAULT_TICKS		1000
#define DEFAULT_TIMER_PULSE	0	///by default, don't use

#define DEFAULT_PRIORITY	19

static void usage(char *pargv0);

static int sig_list[] =
{
	SIGTERM,
	SIGKILL,
	ERROR
};

static jmp_buf exit_env;
static void sig_hand(int code);

/*
 *	man_init() -
 *
 *	This function is called from the main() during the initialization 
 *	of the Acquisition Manager to read the .ini file, initialize
 *	the das_info structure, and initialize the function tables.  
 *	No resource manager connect functions are currently replaced,
 *	but it may be desirable to alter the open function, e.g., sometime.
 */

void man_init(int argc, char *argv[], resmgr_connect_funcs_t *pconn,
	resmgr_io_funcs_t *pio, IOFUNC_ATTR_T *pattr)
{
	int opt;
	char *pconfig;
	FILE *pfile;
	das_info_typ *pinfo = &pattr->das_info;
	
	/*
	 *	Setup default parameters.
	 */

	pconfig = DEFAULT_CONFIG;
	pattr->verbose_flag = FALSE;
	pattr->devname = DEFAULT_DEVICE;
	
	/**	Parse command line for arguments.
	 */

	while((opt = getopt(argc, argv, "p:f:v?")) != EOF) {
		switch(opt) {
		case 'p':						/*	Path name.		*/
			pattr->devname = strdup(optarg);
			break;
			
		case 'f':						/*	Configuration filename.	*/
			pconfig = optarg;
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

	/** Parse configuration file and initialize das_info_typ
	 */
	if((pfile = get_ini_section(pconfig, pattr->devname)) == NULL) {
		fprintf(stderr, "fail get_ini_section() on file %s, section %s\n",
			pconfig, pattr->devname);
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	pinfo->irq = get_ini_long(pfile, INI_IRQ_ENTRY, DEFAULT_IRQ);
	pinfo->port = get_ini_hex(pfile, INI_PORT_ENTRY, DEFAULT_PORT);
	pinfo->pace_clk = get_ini_long(pfile, INI_CLOCK_ENTRY, DEFAULT_CLOCK);
	pinfo->num_ai = get_ini_long(pfile, INI_AD_ENTRY, DEFAULT_AD);
	pinfo->ad_ticks = get_ini_long(pfile, INI_TICKS_ENTRY, DEFAULT_TICKS);
	pinfo->ad_min = get_ini_double(pfile, INI_AD_MIN_ENTRY,
						DEFAULT_MIN);
	pinfo->ad_max = get_ini_double(pfile, INI_AD_MAX_ENTRY,
						DEFAULT_MAX);
	pinfo->ad_step = get_ini_double(pfile, INI_AD_STEP_ENTRY,
						DEFAULT_STEP);
	pinfo->da_min = get_ini_double(pfile, INI_DA_MIN_ENTRY,
						DEFAULT_MIN);
	pinfo->da_max = get_ini_double(pfile, INI_DA_MAX_ENTRY,
						DEFAULT_MAX);
	pinfo->da_step = get_ini_double(pfile, INI_DA_STEP_ENTRY,
						DEFAULT_STEP);
	fclose(pfile);

	pattr->ad_ocb = NULL;	/// will be set by ad_set_scan
#ifdef DO_TRACE
	printf("DAS_INIT: Device %s Config file %s Irq %d Port %#x clock %d no of analog inputs %d\
 ticks %d AD min %f AD max %f AD step %f DA min %f DA max %f DA step %f\n", 
		pattr->devname, pconfig, pinfo->irq, pinfo->port, pinfo->pace_clk,
		pinfo->num_ai, pinfo->ad_ticks, pinfo->ad_min, pinfo->ad_max,
		pinfo->ad_step, pinfo->da_min,  pinfo->da_max, pinfo->da_step);
#endif
	/** Initialize resource manager function tables with
	 *  DAS specific functions for devctl and iomsg
	 */
	pio->devctl = io_devctl;
	pio->msg = io_msg;
	pio->close_ocb = io_close_ocb;

	/** Initialize das_func_t structure with device-specific functions 
	 */
	das_func_init(pattr);

	/**	Check tick/sample, abort if invalid
	 */

	if ((pinfo->ad_ticks < MIN_SAMPLE_TICK)
		|| (MAX_SAMPLE_TICK < pinfo->ad_ticks)) {
		fprintf(stderr, "get_ini_section()\n");
		exit(EXIT_FAILURE);
	} 

	/** Set up exit environment for kill signals
	 */

	if (setjmp(exit_env) != 0) {
		pattr->func.ad_term(NULL, NULL, 0);
		pattr->func.da_term(NULL, NULL, 0);
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);
}

void pulse_init(dispatch_t *dpp, IOFUNC_ATTR_T *pattr)
{
	das_info_typ *pinfo = &pattr->das_info;
	das_func_t *pfunc = &pattr->func;
	struct sigevent *pevent;

	/// attach pulse to be sent by interrupt handler
	if (pinfo->irq != 0) {
		pevent = &pattr->ad_pulse_event;
		if ((pevent->sigev_code = pulse_attach(dpp,
			 MSG_FLAG_ALLOC_PULSE, 0, pfunc->ad_pulse,
			 &pattr->ad_ocb)) == ERROR) {
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
#ifdef DO_TRACE
		printf("DAS_INIT.C:pulse_init: DMM32 interrupt pulse attached: sigev_code %d sigev_coid %#x\n",
			pevent->sigev_code, pevent->sigev_coid);
#endif
	}

	/// attach pulse to be sent by timer interrupt to resource manager 
	if (pfunc->tmr_pulse != NULL) {
		pevent = &pattr->tmr_pulse_event;
		if ((pevent->sigev_code = pulse_attach(dpp,
			 MSG_FLAG_ALLOC_PULSE, 0, pfunc->tmr_pulse,
			 &pattr->ad_ocb)) == ERROR) {
			fprintf(stderr, "Unable to attach timer pulse.\n");
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
#ifdef DO_TRACE
		printf("DAS_INIT.C:pulse_init: Timer interrupt pulse attached: sigev_code %d sigev_coid %#x\n",
			pevent->sigev_code, pevent->sigev_coid);
#endif
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

static void sig_hand(int code)
{
        longjmp(exit_env, code);
}
