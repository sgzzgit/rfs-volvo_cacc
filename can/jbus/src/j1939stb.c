/*\file
 * Translates J1939 Protocol Data Unit into B&B Model 1939STB.
 *
 * Originally written for QNX4, ported to QNX6 in 2005, updated
 * for consistency with new CAN drivers in 2009, updated version
 * not yet tested with the actual B&B converter. (Sue Dickey, July 2009)
 */

#include "old_include/std_jbus.h"

#undef DEBUG_SEND
#undef DEBUG_GET

void
pdu_to_stb (struct j1939_pdu *pdu, struct j1939_stb_transmit *stb, int slot)
{
	int i;

	stb->check1 = 0x81;
	stb->check2 = 0x21;
	stb->control1 = CTRL1_EXTERNAL_CMD |
			 ((pdu->numbytes + 7) & CTRL1_NUMBYTES_MSK); 
	stb->control2 = slot & CTRL2_SLOT_MSK;
	stb->msg_info[0] = MI0_VALID_RESET | MI0_TXIE_RESET |
				MI0_RXIE_RESET | MI0_INTPND_RESET;
	stb->msg_info[1] = MI1_RMTPND_RESET | MI1_TXRQ_RESET |
				MI1_CPUUPD_RESET | MI1_NEWDAT_RESET;
	stb->msg_info[2] = FILL_M2(pdu);
	stb->msg_info[3] = FILL_M3(pdu);
	stb->msg_info[4] = FILL_M4(pdu);
	stb->msg_info[5] = FILL_M5(pdu);
	stb->msg_info[6] = (((pdu->numbytes & CTRL1_NUMBYTES_MSK)
				 << MI6_DLC_SHIFT) | MI6_XTD);
	for (i = 0; i < 8; i++)
		stb->data_field[i] = pdu->data_field[i];
	
}

void send_bytes_stb(int fd, struct j1939_stb_transmit *stb)
{
	int i;
	write(fd, &stb->check1, 1); 
	write(fd, &stb->check2, 1); 
	write(fd, &stb->control1, 1); 
	write(fd, &stb->control2, 1); 
	for (i = 0; i < 7; i++) 
		write(fd, &stb->msg_info[i], 1);
	for (i = 0; i < 8; i++) 
		write(fd, &stb->data_field[i], 1);
#ifdef DEBUG_SEND
	printf("check1 0x%x ", stb->check1);
	printf("check2 0x%x\n", stb->check2);
	printf("control1 0x%x ", stb->control1); 
	printf("control2 0x%x\n", stb->control2); 
	printf("\tmsg_info: " ); 
	for (i = 0; i < 7; i++) 
		printf(" %02x", stb->msg_info[i]);
	printf("\n");
	printf("\tdata_field: ");
	for (i = 0; i < 8; i++) 
		printf(" %d", stb->data_field[i]);
	printf("\n");
#endif
}

/* send an internal message to converter (change baud or vendor string) */

void
send_stb_internal(int fd, short control_code, int baud_rate) 
{
	int i;
	struct j1939_stb_transmit tmp_stb;
	struct j1939_stb_transmit *stb = &tmp_stb;

	stb->check1 = 0x81;
	stb->check2 = 0x21;
	stb->control1 = LOBYTE(control_code);
	stb->control2 = HIBYTE(control_code);
#ifdef DEBUG
	fprintf(stdout, "0x%x 0x%x \n", stb->control1, stb->control2);
#endif
	for (i = 0; i < 7; i++)
		stb->msg_info[i] = 0;
	if (control_code == J1939STB_BAUD_RATE) {
		int baud_code = 7372800/(baud_rate * 16);
		stb->msg_info[0] = 0x3; /* 1 stop bit, 8 bit words, no parity */
		stb->msg_info[1] = LOBYTE(baud_code);
		stb->msg_info[2] = HIBYTE(baud_code);
#ifdef DEBUG
		fprintf(stdout, "0x%x 0x%x 0x%x\n", stb->msg_info[0],
			stb->msg_info[1], stb->msg_info[2]);
#endif
	}
	for (i = 0; i < 8; i++)
		stb->data_field[i] = 0;
	send_bytes_stb(fd, stb);
}

/* low order byte of state_code is msg_info[0], second lowest is msg_info[1],
 * second highest is low-order nibble of msg_info[6]
 */
void
send_stb_external (int fd, struct j1939_pdu *pdu, int state_code, int slot) 
{
	struct j1939_stb_transmit tmp_stb;
	struct j1939_stb_transmit *stb = &tmp_stb;

	pdu_to_stb(pdu, stb, slot);

	stb->msg_info[0] = state_code & 0xff;
	stb->msg_info[1] = ((state_code & 0xff00) >> 8) & 0xff;
	stb->msg_info[6] |= ((state_code & 0xff0000) >> 16) & 0xf; 
	send_bytes_stb(fd, stb);
}

/*
 * sequence to first update the data in the converter, then trigger a send,
 * then turn off the send
 * Always returns 1, there is no error reportin.
 */
int send_stb(int fd, struct j1939_pdu *pdu, int slot)
{
	send_stb_external(fd, pdu, STB_TXUPD, slot);
	send_stb_external(fd, pdu, STB_TXNEW, slot);
	send_stb_external(fd, pdu, STB_TXUPD, slot);
#if 0
	send_stb_external(fd, pdu, STB_TXRMT, slot);
#endif
	return 1;
}

/* receive_stb reads a message from the J1939 STB converter serial port
 * at end of message, sync for start of next message is done
 * if message is valid, return J1939_RECEIVE_MESSAGE_VALID
 * if message has format errors, return J1939_RECEIVE_MESSAGE_ERROR
 * if file reading error occurs, return J1939_RECEIVE_FATAL_ERROR (this
 * will indicate end of file if testing from trace) 
 * 
 * NOTE: does not return until first byte of next message is received;
 * since J1939 messages are broadcast very frequently, this should have
 * little effect on message latency; if this is a problem, can use
 * logic that checks only at the start, but in this case we have no
 * way to keep from delivering a message to higher levels when bytes
 * are lost.
 */
 
int
receive_stb(int fd, struct j1939_pdu *pdu, int *external, int *slot_or_type)
{
	unsigned char nextbyte = 0;
	struct j1939_stb_receive stb;
	int message_bad = 0; 
	int n = 0;
	int i;

	if (read(fd, &nextbyte, 1) < 1)
		return J1939_RECEIVE_FATAL_ERROR;
	else
		stb.function = nextbyte;

	*slot_or_type = stb.function & STB_RCV_TYPE_MSK;

	switch ((stb.function & STB_RCV_MSG_MSK) >>  4) {
	case 0:	*external = 1;
		break;
	case 2: *external = 0;
		break;
	default:
		/* invalid STB function value */
		message_bad = 1;
		break;
	}

	if (!message_bad) {
		for (i = 0; i < 4; i++) {
			if (read(fd, &nextbyte, 1) < 1)
				return J1939_RECEIVE_FATAL_ERROR;
			else
				stb.j1939_info[i] = nextbyte;
		}		

		pdu->priority = EXTRACT_PRIORITY(&stb);
		pdu->pdu_format = EXTRACT_PF(&stb);
		pdu->pdu_specific = EXTRACT_PS(&stb);
		pdu->src_address = EXTRACT_SRC_ADDR(&stb);

		if (read(fd, &nextbyte, 1) < 1)
			return J1939_RECEIVE_FATAL_ERROR;
		else
			stb.MC = nextbyte;
				
		pdu->numbytes = stb.MC >>MI6_DLC_SHIFT;
		if (*external) {
			/* J1939 message */
			if (read(fd, &nextbyte, 1) < 1)
				return J1939_RECEIVE_FATAL_ERROR;
			do {
				pdu->data_field[n++] = nextbyte;
				if (read(fd, &nextbyte, 1) < 1)
					return J1939_RECEIVE_FATAL_ERROR;
			} while (n < 8 ); 
		} else {
			/* converter internal message */
			if (read(fd, &nextbyte, 1) < 1)
				return J1939_RECEIVE_FATAL_ERROR;
			if (nextbyte != 0xaa)
				message_bad = 1;
			else {
				do {
					pdu->data_field[n++] = nextbyte;
					if (read(fd, &nextbyte, 1) < 1)
						return J1939_RECEIVE_FATAL_ERROR;
				} while (n < 8 && nextbyte != 0xaa);
			}
		}	
	}

	/* read next byte to check for start of message */

	if (read(fd, &nextbyte, 1) < 1) {
	        return J1939_RECEIVE_FATAL_ERROR;
	}
	
	/* if nextbyte is start of message, all is well */	
        if ((nextbyte == J1939_STB_SOM)  && !message_bad)
		return J1939_RECEIVE_MESSAGE_VALID;

	/* otherwise an error has occurred, sync with start of message
	 * if possible and return error value
	 */
	if (sync_stb(fd))
		return J1939_RECEIVE_MESSAGE_ERROR;
	else
		return J1939_RECEIVE_FATAL_ERROR;

}

/* synchronizes to the start of a message from the J1939 converter
 * returns 1 if successful, 0 on fatal error */

int sync_stb(int fd)
{
	unsigned char nextbyte = 0;

	while (nextbyte != J1939_STB_SOM) {
		if (read(fd, &nextbyte, 1) < 1) {
			fprintf(stdout, "read failure\n");
			return 0;
		}
	}	
	return 1;
}

/*
 *	Open the file and call sync_stb at initializatin.
 *	This function is used to initialize jbus_func_t.
 */
int init_stb(char *filename, int flags, void *extra)
{
	int fd = open(filename, flags);
	if (fd != -1)
		sync_stb(fd);
	return (fd);
}		
	
/*
 *	pfd parameter an address for function type compatibility
 * 	with CAN drivers using handles instead of real file descriptor 
 */
int close_stb(int *pfd)
{
	// file close not desirable on serial port with QNX6?
	return (0);
}
