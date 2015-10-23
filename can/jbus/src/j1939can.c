/**\file
 * Uses CAN driver read and write to send and receive J1939 Protocol Data Units
 *
 * Copyright (c) 2005-9   Regents of the University of California
 *
 * Ported to QNX6 May 2005
 * Updated to remove all Cogent references and to use multiple 
 * drivers July 2009.
 * QNX4 version removed.
 *
 */
#include "std_jbus_extended.h"
#include "can_client.h"

#define DEBUG

/** 
 *	Calls can_open. If flags are RDONLY, can_open will arm the
 *	pulse. If access to channel and connection IDs used by
 *	the CAN driver are actually required outside of the CAN driver,
 * 	and its client calls, we must call operations on the "fd"
 *	returned by can_open (actually a pointer) to retrieve them. 
 */
int init_can(char *filename, int flags, void *p_other)
{
	return (can_open(filename, flags));
}

int close_can(int *pfd)
{
	return (can_close(pfd));
}

/** PATH CAN driver does not yet use state code or slot arguments, provided
 * for compatibility with B&B STB converter send routine.
 * Returns 0 on error, 1 on success.
 */
int send_can (int fd, struct j1939_pdu *pdu, int slot) 
{
	unsigned long id = PATH_CAN_ID(pdu); 
#ifdef DEBUG
	int i;
	printf("send_can: PATH_CAN_ID 0x%x, numbytes %d src_address %#x ", id, pdu->numbytes, pdu->src_address);
	for (i = 0; i < 8; i++)
		printf("%d ", pdu->data_field[i]);
	printf("\n");
#endif
	pdu->src_address = (0x2A & 0xFF);
	if (can_write(fd, id, 1, &pdu->data_field,
		 (unsigned char) (pdu->numbytes & 0xff)) == -1) { 
		fprintf(stderr, "send_can: can_write failed\n");
		return 0;
	} else
		return 1;
}

/** Returns J1939_CAN_MESSAGE_ERROR (0) on can_read failure.
 * Otherwise returns the (positive) number of bytes in the message,
 * no fatal error conditions are recognized.
 */
int
receive_can(int fd, struct j1939_pdu *pdu, int *extended, int *slot)
{
	unsigned long id;
	int retval;
	char extbyte = 0;
	if ((retval=can_read(fd, &id, &extbyte,
		 &pdu->data_field, 8)) == -1) {
///		fprintf(stderr, "receive_can: can_read failed\n");
		return(J1939_RECEIVE_MESSAGE_ERROR);
	} else {
		*extended = (int) extbyte;
		pdu->priority = PATH_CAN_PRIORITY(id);
		pdu->pdu_format = PATH_CAN_PF(id);
		pdu->pdu_specific = PATH_CAN_PS(id);
		pdu->src_address = PATH_CAN_SA(id);
		pdu->numbytes = retval;
		return (retval);
	} 
}
