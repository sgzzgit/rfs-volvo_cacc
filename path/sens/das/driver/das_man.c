/**\file
 *
 *  das_man.c 
 *
 *	This file and the associated modules provide a data acquisition
 *	system for I/O device drivers. The QNX6 version of this DAS system
 *	has been rewritten as a standard resource manager.
 *
 *	As with the QNX4 system, this system is used to construct device 
 *	drivers for analog and digital output with a structured interface.
 *	An application which wishes to access the device should use only the
 *	API provided by sys_das.c and das_msg.c.
 *
 *	The das_man module is linked into the device driver for each
 *	separate board type. The functions in sys_das are linked into the local
 *	library; das_msg.o is compiled separately, since it reuses function
 *	names that are used by the driver and thus cannot be part of the local
 *	library which is linked to both server and client code.
 *   
 */
#include <sys_qnx6.h>
#include <sys/dispatch.h>
#include <local.h>
#include "das_clt.h"
#include "das_man.h"
#undef DO_TRACE

static resmgr_connect_funcs_t  connect_func;
static resmgr_io_funcs_t	   io_func;
static IOFUNC_ATTR_T		   attr;

int main (int argc, char **argv)
{
	dispatch_t *dpp;
	resmgr_attr_t resmgr_attr;
	dispatch_context_t *ctp;
	das_info_typ *pinfo = &attr.das_info;

	/// create the dispatch structure
	if ((dpp = dispatch_create ()) == NULL) {
		perror ("Unable to dispatch_create\n");
		exit (EXIT_FAILURE);
	}

	/// initialize the various data structures
	memset (&resmgr_attr, 0, sizeof (resmgr_attr));
	resmgr_attr.nparts_max = 2;
	resmgr_attr.msg_max_size = DAS_MSG_BUF;

	/// bind default functions into the outcall tables
	iofunc_func_init (_RESMGR_CONNECT_NFUNCS, &connect_func,
			 _RESMGR_IO_NFUNCS, &io_func);
	iofunc_attr_init (&attr.io_attr, S_IFNAM | 0666, 0, 0);
	
	/// read configuration files and bind device-specific functions
	man_init(argc, argv, &connect_func, &io_func, &attr); 

	/// establish a name in the pathname space
	if (resmgr_attach (dpp, &resmgr_attr, attr.devname, _FTYPE_ANY,
		 0, &connect_func, &io_func, &attr) == -1) {
		perror ("Unable to resmgr_attach\n");
		exit (EXIT_FAILURE);
	}
	/// attach pulses to be sent by interrupt handler and timer 
	pulse_init(dpp, &attr);

	/// alloc dispatch context
	ctp = dispatch_context_alloc (dpp);

	/// initialize device and attach interrupt if required
	das_open_dev(&attr);

	/// wait here forever, handling messages
	while (1) {
		if ((ctp = dispatch_block (ctp)) == NULL) {
			perror ("Unable to dispatch_block\n");
			exit (EXIT_FAILURE);
		}
#ifdef DO_TRACE
		printf("das_man calling dispatch handler\n");
#endif		
		dispatch_handler (ctp);
	}
}
