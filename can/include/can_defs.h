/*
 * can_defs.h
 *
 * This file contains definitions common to client programs and driver
 *
 */

#ifndef INCLUDE_CAN_DEFS_H
#define INCLUDE_CAN_DEFS_H

//most significant bit sets frame as extended 
#define IS_EXTENDED_FRAME(MSG)		(((MSG).id) & 0x80000000)
#define SET_EXTENDED_FRAME(MSG)		(MSG).id |= 0x80000000
#define CAN_ID(MSG)			((MSG).id & ~0x80000000)

#define CAN_HDR_SIZE 5

/** Type used in devctl for CAN I/O
 */
typedef struct {
	unsigned long id; 	// CAN device id on bus 
	unsigned char size;	//0-8	
	char data[8];		//
	int error;		// set to non-zero if error on read or write
} can_msg_t;

/** Used to set filtering of CAN messages 
 */

typedef struct {
	unsigned long id;	// 0 any message id
	unsigned long mask;	// 0xff all messages
} can_filter_t;

/** State information about CAN device manager
 */
typedef struct
{
        int             port;           /// Base address of adapter
        int             irq;            /// Interrupt request line.
	int		use_extended_frame;	///1 yes, 0, no
	int		bit_speed;	/// Kb/s 
	int		intr_id;;	/// ID returned by Interrupt Attach Event
	can_filter_t	filter;
} can_info_t;

/** 
 *      struct returned to client by can_get_errs and can_clear_errs devctl
 */
typedef struct {
        unsigned int shadow_buffer_count;
        unsigned int intr_in_handler_count;
        unsigned int rx_interrupt_count;
        unsigned int rx_message_lost_count;
        unsigned int tx_interrupt_count;
} can_err_count_t;



#endif
