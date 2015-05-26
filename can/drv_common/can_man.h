/**\file
 *	can_man.h
 *
 *	Structures, definitions, and prototypes for the SSV CAN driver
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 */
#ifndef CAN_MAN_H
#define CAN_MAN_H

#include <sys_os.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "sys_list.h"
#include "sys_buff.h"
#include "das_clt.h"
#include "can_defs.h"

/**
 *	Defined in can_init.c
 */
extern int can_notify_client_err;
extern int mask_count_non_zero;

/**
 *	Defined in can_dev.c
 */

extern unsigned int shadow_buffer_count;
extern unsigned int intr_in_handler_count;
extern unsigned int rx_interrupt_count;
extern unsigned int rx_message_lost_count;
extern unsigned int tx_interrupt_count;

/* forward declarations needed so new IOFUNC_OCB_T and IOFUNC_ATTR_T defines
 * can be used in sys/iofunc.h
 */

struct can_ocb;
struct can_attr;

#define IOFUNC_OCB_T struct can_ocb
#define IOFUNC_ATTR_T struct can_attr
#include <sys/iofunc.h>
#include <sys/dispatch.h>

/** Information per Open Context Block; may be one for CAN and one for
 *  digital I/O per instance of driver; digital I/O does not send notify now
 */
typedef struct can_ocb
{
	iofunc_ocb_t io_ocb;
        int rcvid;              /// Used to notify client.
        struct sigevent clt_event;  /// Used to notify client, from client
} can_ocb_t;

/** Information per device manager */
typedef struct can_attr
{
	iofunc_attr_t io_attr;	/// standard system information 
	char *devname;		/// device path name
	can_info_t can_info;  /// initialization info	
	cbuff_typ in_buff;	/// Holds CAN messages until client reads
	cbuff_typ out_buff;	/// Holds CAN messages until written to bus
	can_ocb_t *notify_pocb;   /// OCB of client to be notified 
	bool_typ verbose_flag;
	struct sigevent hw_event;	/// initialized in pulse_init
} can_attr_t;

#define CAN_IN_BUFFER_SIZE	150
#define CAN_OUT_BUFFER_SIZE	150

/** Initialization functions in can_init.c
 */
extern void can_init(int argc, char *argv[], resmgr_connect_funcs_t *pconn,
	resmgr_io_funcs_t *pio, IOFUNC_ATTR_T *pattr);
extern int can_handle_interrupt(message_context_t *ctp, int code,
			unsigned flags, void *ptr);
extern void pulse_init(dispatch_t *dpp, IOFUNC_ATTR_T *pattr);

/** Replacements for resource manager io functions in io_func.c
 */
extern int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg,
			 RESMGR_OCB_T *ocb);
extern int io_open(resmgr_context_t *ctp, io_open_t *msg, 
	RESMGR_HANDLE_T *handle, void *extra);

/** Interface functions in can_if.c
 */
extern void can_cq_add(cbuff_typ *pbuff, can_msg_t *new_msg);
extern can_msg_t *can_cq_read_first(cbuff_typ *pbuff);
extern can_msg_t *can_cq_pop_first(cbuff_typ *pbuff);
extern void can_new_msg(can_msg_t *pmsg, IOFUNC_ATTR_T *pattr);

/** 
 *	Functions referenced in io_func.c, defined in can_dev.c
 */
extern can_msg_t can_dev_read(IOFUNC_ATTR_T *pattr);
extern int can_dev_write(RESMGR_OCB_T *pocb, can_msg_t *pmsg);
extern int can_dev_empty_q(IOFUNC_ATTR_T *pattr);
extern int can_dev_arm(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
                                        struct sigevent event);
int can_dev_add_filter(IOFUNC_ATTR_T *pattr, can_filter_t filter);
extern can_err_count_t can_dev_clear_errs();
extern can_err_count_t can_dev_get_errs();

/** Device-level functions in can_dev.c 
 */
extern void can_dev_init(unsigned int base_address, unsigned int bit_speed,
			unsigned char extended_frame);
extern int can_dev_interrupt(IOFUNC_ATTR_T *pattr);
extern void can_send(IOFUNC_ATTR_T *pattr);

extern int digital_dir(int port, long bits);
extern int digital_in(int port, long *pbits);
extern int digital_out(int port, long bits, long mask,
	long *pold_bits, long *pnew_bits);

#endif /* CAN_MAN_H */
