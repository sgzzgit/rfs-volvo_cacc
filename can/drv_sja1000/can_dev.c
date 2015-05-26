/*\file
 * 
 * Device level code, based on device-level can4linux-3.5.4 code for the
 * Phillips SJA 1000.
 *
 * See SJA1000_3.pdf and AN9076.pdf in ../doc and can4linux directory.
 *
 */
#include "defs.h"
#include "can_mm.h"
#include "can_man.h"
#include "sja1000.h"

/** Must be set to the memory-mapped base address of the CAN registers.
 *  Used by the CANin and CANout macros to access registers.
 */
canregs_t *can_base_addr;

/** Global per channel variables from sja1000funcs.c
 *  For PATH driver, each channel has a separate driver, so MAX_CHANNELS is 1
 *  Most of these are not really used in the PATH CAN driver for QNX6.
 */

int IRQ[MAX_CHANNELS]; 
int Baud[MAX_CHANNELS]             = { 0x0 };
unsigned int AccCode[MAX_CHANNELS] = { 0x0 };
unsigned int AccMask[MAX_CHANNELS] = { 0x0 };
int Timeout[MAX_CHANNELS]          = { 0x0 };
int Outc[MAX_CHANNELS]    = { 0x0 };
int TxErr[MAX_CHANNELS]   = { 0x0 };
int RxErr[MAX_CHANNELS]   = { 0x0 };
int Overrun[MAX_CHANNELS] = { 0x0 };
int Options[MAX_CHANNELS] = { 0x0 };
atomic_t Can_isopen[MAX_CHANNELS];
int Outc[1];
unsigned int dbgMask;
canmsg_t last_Tx_object[MAX_CHANNELS];  
msg_fifo_t Rx_Buf[MAX_BUFSIZE][CAN_MAX_OPEN];
msg_fifo_t Tx_Buf[MAX_BUFSIZE];
int IRQ_requested[CAN_MAX_OPEN];
int Can_minors[CAN_MAX_OPEN];                        
int selfreception[MAX_CHANNELS][CAN_MAX_OPEN];      
int use_timestamp[MAX_CHANNELS];           
int wakeup[MAX_CHANNELS];                           
int listenmode;                        
wait_queue_head_t CanWait[MAX_CHANNELS][CAN_MAX_OPEN];
wait_queue_head_t CanOutWait[MAX_CHANNELS];
int CanWaitFlag[MAX_CHANNELS][CAN_MAX_OPEN];      

/** Global variables used in this file
 */
time_t last_time_can_sent; 	
unsigned int intr_in_handler_count = 0; 
unsigned int tx_interrupt_count = 0; 
unsigned int rx_interrupt_count = 0; 
unsigned int shadow_buffer_count = 0;	// no shadow buffer on SJA1000
					// counts unusual events
unsigned int rx_message_lost_count = 0;

/** Forward declarations for receive and transmits interrupt 
 *  handler auxilary functions.
 */
void rx_process_interrupt(IOFUNC_ATTR_T *pattr);
void tx_process_interrupt(IOFUNC_ATTR_T *pattr);

/**
 *	Clear the error counts and return the old counts.
 */
can_err_count_t can_dev_clear_errs()
{
	can_err_count_t errs;
	errs.intr_in_handler_count = intr_in_handler_count;
	errs.tx_interrupt_count = tx_interrupt_count;
	errs.rx_interrupt_count = rx_interrupt_count;
	errs.shadow_buffer_count = shadow_buffer_count;
	errs.rx_message_lost_count = rx_message_lost_count;
	intr_in_handler_count = 0;
	tx_interrupt_count = 0;
	rx_interrupt_count = 0;
	shadow_buffer_count = 0;
	rx_message_lost_count = 0;
	return (errs);
}

/**
 *	Return the current error count..
 */
can_err_count_t can_dev_get_errs()
{
	can_err_count_t errs;
	errs.intr_in_handler_count = intr_in_handler_count;
	errs.tx_interrupt_count = tx_interrupt_count;
	errs.rx_interrupt_count = rx_interrupt_count;
	errs.shadow_buffer_count = shadow_buffer_count;
	errs.rx_message_lost_count = rx_message_lost_count;
	return (errs);
}


/**
 *	can_dev_init is called by the can_man resource manager
 *	to initialize the Phillips SJA1000 chip.
 */
void can_dev_init(unsigned int base_address, unsigned int bit_speed,
		unsigned char extended_frame)
{
	can_base_addr = (canregs_t *) mmap_device_memory(NULL,
		SJA1000_MAP_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		0, base_address);

	printf("Using memory mapped access, 0x%08x mapped to 0x%08x\n",
		base_address, (unsigned int) can_base_addr);
	printf("speed %d, %s\n",
		bit_speed,
		extended_frame?"extended frame":"standard frame");
	fflush(stdout);

	/** Set up global variables used by sja1000funcs
	 *  to set up timing and acceptance
	*/
        Baud[MY_CHANNEL] = bit_speed;
        AccCode[MY_CHANNEL] = 0x00000000;
        AccMask[MY_CHANNEL] = 0xFFFFFFFF; /// Accept everything
	Outc[MY_CHANNEL] = CAN_OUTC_VAL;
	if (CAN_ChipReset(MY_CHANNEL) < 0) 
		printf("Error returned from SJA1000 reset\n");
	else 
		CAN_StartChip(MY_CHANNEL);
	printf("Phillips SJA1000 started, mode 0x%08x\n", CANin(0, canmode));
}

int can_dev_interrupt(IOFUNC_ATTR_T *pattr)
{
	int i=0;
	int retval = 0;	// set to number of client notifies for receive
#ifdef DO_TRACE
	struct timeb current;
	ftime(&current);
	printf("interrupt: sec %d ms %3d\n", current.time, current.millitm);
	fflush(stdout);
#endif
	unsigned char status_val;
	unsigned char ir_val = CANin(MY_CHANNEL, canirq);
	unsigned char rmc = CANin(MY_CHANNEL, reserved3);

	while (ir_val) { 
		if(i>0) {
			intr_in_handler_count++; 
#ifdef DO_TRACE
			printf("i %d:%s intval=%d\n",
				i,pattr->devname, ir_value);
#endif
		}
		++i;
#ifdef DO_TRACE
		status_val = CANin(MY_CHANNEL, canstat);
		printf("interrupt: value 0x%02x, status 0x%02x\n", 
			ir_val, status_val);
		fflush(stdout);
#endif	
		if (ir_val & CAN_OVERRUN_INT) {           
			CANout(minor, cancmd, CAN_CLEAR_OVERRUN_STATUS);
			rx_message_lost_count++;
		}

		if (ir_val & CAN_RECEIVE_INT) {
			retval++;
			rx_interrupt_count++;
#ifdef DO_TRACE_RX
			printf("RX interrupt\n");
			fflush(stdout);
#endif
   			rx_process_interrupt(pattr);

			CANout(MY_CHANNEL, cancmd, CAN_RELEASE_RECEIVE_BUFFER);       
		}                                                         


		if (ir_val & CAN_TRANSMIT_INT) {
#ifdef DO_TRACE_TX
			printf("TX interrupt\n");
			fflush(stdout);
#endif
			tx_interrupt_count++;
			tx_process_interrupt(pattr);
		}

		if (ir_val & CAN_ERROR_INT) {
			shadow_buffer_count++;
			if ((status_val & CAN_BUS_STATUS_BIT)) {
#ifdef DO_TRACE_ERR
				printf("Bus off; try to reset SJA1000\n");
				fflush(stdout);
#endif
				//Try resetting the chip
				if (CAN_ChipReset(MY_CHANNEL) < 0) 
					printf("Error on reset\n");
				else 
					CAN_StartChip(MY_CHANNEL);
				return 0; 
			}
			if((status_val & CAN_ERROR_STATUS)!=0) {
#ifdef DO_TRACE_ERR
				printf("Warning: error status.\n");
				fflush(stdout);
#endif
			}
		}

		if (ir_val & CAN_WAKEUP_INT) {
#ifdef DO_TRACE_ERR
			printf("Unexpected interrupt: %d\n", ir_val);
			fflush(stdout);
#endif
		}

		/// Return if have received all messages there at start
		/// Don't want to stay here too long and starve receiver

		if (i > rmc) return (retval);

		ir_val = CANin(MY_CHANNEL, canirq);

	}
	return (retval);
}

/**  rx_process_interrupt: reads message from chip
 *   and queues for resource manager
 */
void rx_process_interrupt(IOFUNC_ATTR_T *pattr)
{
	//printf("receiving\n");
	
	can_msg_t msg;
	unsigned char frm_info = CANin(MY_CHANNEL, frameinfo);
	int ext = frm_info & CAN_EFF;
   	int i;
	unsigned int ID[4];

#ifdef DO_TRACE_RX
	printf("RX_INTERRUPT: frm_info 0x%08x\n", frm_info);
#endif
	msg.size = frm_info & 0x0f;
	if (ext) {
		for (i=0; i < msg.size && i < 8; i++) 
			msg.data[i] = CANin(MY_CHANNEL, 
					frame.extframe.canxdata[i]);
		
		ID[0] = CANin(MY_CHANNEL, frame.extframe.canid1);
		ID[1] = CANin(MY_CHANNEL, frame.extframe.canid2);
		ID[2] = CANin(MY_CHANNEL, frame.extframe.canid3);
		ID[3] = CANin(MY_CHANNEL, frame.extframe.canid4);
#ifdef DO_TRACE_RX
		printf("ID: %08x %08x %08x %08x\n", ID[0], ID[1], ID[2], ID[3]); 
#endif
		msg.id = (ID[0] << 21) | (ID[1] << 13) |
				 (ID[2] << 5) | (ID[3] >> 3); 
#ifdef DO_TRACE_RX
		printf("msg.id: %08x\n", msg.id); 
#endif
		SET_EXTENDED_FRAME(msg);
	} else {
		for (i=0; i < msg.size && i < 8; i++) 
			msg.data[i] = CANin(MY_CHANNEL, 
					frame.stdframe.candata[i]);
		ID[0] = CANin(MY_CHANNEL, frame.stdframe.canid1) << 3;
		ID[1] = CANin(MY_CHANNEL, frame.extframe.canid2) >> 5;
		msg.id = ID[0] | ID[1];
	}	
#ifdef DO_TRACE_RX
	printf("RX_INTERRUPT: msg.id %d, msg.size %d, data[0] 0x%02x\n", 
			CAN_ID(msg), msg.size, msg.data[0]);
#endif
	
	can_new_msg(&msg, pattr);
}

/**
 *	 tx_process_interrupt: sends a new message after notification
 *	 of transmission of old one
 */
void tx_process_interrupt(IOFUNC_ATTR_T *pattr)
{

	can_cq_pop_first(&pattr->out_buff);

	if(pattr->out_buff.data_count != 0)
		can_send(pattr);
	else
		;//printf("no more messages waiting to be sent\n");
}


/** 
 *   	can_send: gets the message from the front of the transmit
 *	queue and puts it on the CAN bus.
 */ 
void can_send(IOFUNC_ATTR_T *pattr)
{
	int i;
	can_msg_t *tx = can_cq_read_first(&pattr->out_buff);
	unsigned char tx2reg;

	/*info and identifier fields */
	tx2reg = tx->size;

	if (IS_EXTENDED_FRAME(*tx)) {
		CANout(MY_CHANNEL, frameinfo, CAN_EFF + tx2reg);               
		CANout(MY_CHANNEL, frame.extframe.canid1, (u8)(tx->id >> 21)); 
		CANout(MY_CHANNEL, frame.extframe.canid2, (u8)(tx->id >> 13)); 
		CANout(MY_CHANNEL, frame.extframe.canid3, (u8)(tx->id >> 5));  
		CANout(MY_CHANNEL, frame.extframe.canid4,
						 (u8)(tx->id << 3) & 0xff);
		for( i=0; i < tx->size ; i++)                          
			CANout(MY_CHANNEL, frame.extframe.canxdata[i],
						 tx->data[i]);
	} else {
		CANout(MY_CHANNEL, frameinfo, CAN_SFF + tx2reg);               
		CANout(MY_CHANNEL, frame.stdframe.canid1, (u8)((tx->id) >> 3) );
		CANout(MY_CHANNEL, frame.stdframe.canid2,
						 (u8)(tx->id << 5 ) & 0xe0);
		for( i=0; i < tx->size ; i++)                          
			CANout( MY_CHANNEL, frame.stdframe.candata[ i],
						 tx->data[i]);
	}

	CANout(MY_CHANNEL, cancmd, CAN_TRANSMISSION_REQUEST);           
	last_time_can_sent = time((time_t *)NULL);	
}

/*
 *
 * 	Digital I/O is supported on some CAN cards, not on the Janus-MM.
 *
 */

int digital_dir( int port, long bits )
{
	printf(" Digital I/O is not supported\n");
        return (EOK);
}

int digital_in( int port, long *pbits )
{
	printf(" Digital I/O is not supported\n");
        return (EOK);
}

int digital_out(int port, long mask, long bits,
                                 long *pold_bits, long *pnew_bits )
{
	printf(" Digital I/O is not supported\n");
        return (EOK);
}


