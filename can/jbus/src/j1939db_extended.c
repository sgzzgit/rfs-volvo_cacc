/**\file
 *	Routines to write J1939 data to database.  
 * Isolates details of data conversion and database access from the main 
 * receive and transmit processes for the J1939 serial data link.
 *
 * Ported to QNX6 in May 2005.
 *
 *
 * Copyright (c) 2005   Regents of the University of California
 *
 */

#undef DEBUG
#undef DO_TRACE
#undef DO_TRACE_SEND
#include "std_jbus_extended.h"

#ifndef __QNXNTO__
#define ts2_is_later_than_ts1(a,b) TIMEB_COMP((*(b)),(*(a)))
#endif

extern int fpin;	
extern jmp_buf env;

static int clt_error = 0;

// The following variables are available to calling programs
int j1939db_update_err = 0;	//incremented on failure to update DB 
int j1939db_send_count = 0;	//incremented when J1939 message is sent
int j1939db_receive_count = 0;	//incremented when J1939 message is received

/** PGN index tables entered by hand from SAE J1939 documentation.
 *  Messages are added to the table when the support code is written.
 */
j1939_pgn_index pdu1_index[256] = {
	{NULL, DB_J1939_TSC1_VAR},	/* 0 Torque Speed Control TSC1 */
	{NULL, 500},	/* 1 Transmission Control 1 */
	{NULL, 500},	/* 2 ISO11992	EBS11 */
	{NULL, 500},	/* 3 ISO11992	EBS21 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 15 */
	{NULL, DB_J1939_VOLVO_XBR_WARN_VAR},		    /* 16 */
	{NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 31 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 47 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 63 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 79 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 95 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 111 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 127 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 143 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 159 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 175 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 191 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 207 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},/* 211 Calibration Information */
	{NULL, 500},	/* 212 Data Security */
	{NULL, 500},	/* 213 Time/date Adjust */
	{NULL, 500},	/* 214 Boot Load Data */
	{NULL, 500},	/* 215 Binary Data Transfer */
	{NULL, 500},	/* 216 Memory Access Response */
	{NULL, 500},	/* 217 Memory Access Request */
	{NULL, 500},	/* 218 ISO 15765 */
	{NULL, 500},	/* 219 ISO 15765 */
	{NULL, 500},	/* 220 Anti-theft Status */
	{NULL, 500},	/* 221 Anit-theft Request */
	{NULL, 500},	/* 222 Reset */
	{NULL, 500},	/* 223 Stop/Start Broadcast */
	{NULL, 500},	/* 224 Cab Message #1 */
	{NULL, 500},	/* 225 ISO 11992 */
	{NULL, 500},	/* 226 ISO 11992 */
	{NULL, 500},	/* 227 Test */
	{NULL, 500},	/* 228 ISO 11992 */
	{NULL, 500},	/* 229 ISO 11992 */
	{NULL, 500},	/* 230 Virtual Terminal-to-Node */
	{NULL, 500},	/* 231 Node-to-Virtual Terminal */
	{NULL, 500},	/* 232 Acknowledgment */
	{NULL, 500}, 
	{NULL, 500},	/* 234 Request PG */
	{NULL, 500},	/* 235 Transfer Protocol Data Transfer */
	{NULL, 500},	/* 236 Transfer Protocol Connection Mgmt */
	{NULL, 500},	/* 237 Network Layer */
	{NULL, 500},	/* 238 Address Claimed */
	{pdu2_239, 500},	/* 239 Proprietary A */
	{pdu2_240, 500},	/* less than 100ms PDU2 PGs */
	{NULL, 500},
	{NULL, 500},
        {NULL, 500},	/* 243 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 247 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 251 */
	{NULL, 500},
	{NULL, 500},
	{pdu2_254, 500},	/* more than 100ms PDU2 PGs */
	{pdu2_255, 500},   /* Proprietary B */
};


j1939_pgn_index pdu2_239[256] = {
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, 
	{NULL, DB_J1939_VOLVO_XBR_VAR},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 15 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 31 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 47 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 63 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 79 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 95 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 111 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 127 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 143 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 159 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 175 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 191 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 207 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 223 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 239 */	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 255 */	
};

j1939_pgn_index pdu2_240[256] = {
	{NULL, DB_J1939_ERC1_VAR}, 
	{NULL, DB_J1939_EBC1_VAR}, /* 1 Electronic Brake Controller EBC1 */
	{NULL, DB_J1939_ETC1_VAR}, /* 2 Electr. Transmission Controller 1 */
	{NULL, DB_J1939_EEC2_VAR},
	{NULL, DB_J1939_EEC1_VAR},
	{NULL, DB_J1939_ETC2_VAR}, /* 5 Electr Transmission Controller 2 */
	{NULL, 500}, 			/* 6 Electronic Axle Controller 1 */
	{NULL, 500}, 			/* 7 */
	{NULL, 500}, 
	{NULL, DB_J1939_VDC2_VAR},		/* 9 */ 
	{NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 15 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 31 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 47 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 63 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 79 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 95 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 111 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 127 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 143 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 159 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 175 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 191 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 207 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 223 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 239 */	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 255 */	
};

j1939_pgn_index pdu2_253[256] = {
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 15 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 31 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 47 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 63 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 79 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 95 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 111 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 127 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 143 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 159 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 175 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 191 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, DB_J1939_EBC5_VAR},			    /* 196 */ 
	{NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 207 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 223 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, DB_J1939_MVS_X_E_VAR}, 
	{NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 239 */	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},	
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 255 */	
};

j1939_pgn_index pdu2_254[256] = {
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, 
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 15 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 31 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 47 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, 
	{NULL, DB_J1939_VOLVO_TARGET_VAR}, {NULL, DB_J1939_VOLVO_EGO_VAR}, //FE33 & FE34
	{NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 63 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 79 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 95 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 111 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 127 */
	{NULL, 500}, {NULL, DB_J1939_GFI2_VAR}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 143 */
	{NULL, 500}, {NULL, 500}, {NULL, DB_J1939_EI_VAR}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 159 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 175 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, DB_J1939_FD_VAR}, {NULL, 500},
	{NULL, DB_J1939_EBC2_VAR}, 			/* 191 */
	{NULL, 500}, {NULL, DB_J1939_HRVD_VAR},
//	{NULL, DB_J1939_ERC2_VAR},			/* 194 */ 
	{NULL, 500},					/* Actual 194 (remove if DB_J1939_ERC2_VAR is re-established later*/
	{NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 207 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, DB_J1939_TURBO_VAR},	/* 221 */
	{NULL, 500},
	{NULL, DB_J1939_EEC3_VAR},	/* 223 */
	{NULL, DB_J1939_VD_VAR},	/* 224 */
	{NULL, DB_J1939_RCFG_VAR},	/* 225 */
	{NULL, 500},	/* 226 Transmission Configuration */	
	{NULL, DB_J1939_ECFG_VAR},	/* 227 */
	{NULL, 500},	/* 228 Shutdown */	
	{NULL, 500},
	{NULL, 500},
	{NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, DB_J1939_ETEMP_VAR},
	{NULL, 500}, /* 239 */
	{NULL, DB_J1939_PTO_VAR},
	{NULL, DB_J1939_CCVS_VAR},	/* 241 */
	{NULL, DB_J1939_LFE_VAR},	/* 242 */
	{NULL, DB_J1939_VP_X_VAR},		/* 243 */ 
	{NULL, 500},
	{NULL, DB_J1939_AMBC_VAR},	/* 245 */
	{NULL, DB_J1939_IEC_VAR},	/* 246 */
	{NULL, DB_J1939_VEP_VAR},	/* 247 */
	{NULL, DB_J1939_TF_VAR},	/* 248 */
	{NULL, 500}, 
	{NULL, DB_J1939_VOLVO_BRK_VAR},	/* 250 */
	{NULL, DB_J1939_RF_VAR},	/* 251 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500} /* 255 */
};

j1939_pgn_index pdu2_255[256] = {
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, 
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 15 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 31 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 47 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 63 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 79 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 95 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 111 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 127 */
	{NULL, DB_J1939_EBC_ACC_VAR}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 143 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 159 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 175 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 191 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 207 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 223 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500}, /* 239 */
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500},
	{NULL, 500}, {NULL, 500}, {NULL, 500}, {NULL, 500} /* 255 */
};

/**
 * Subtract J1939_DB_OFFSET from database variable number to index into 
 * this array. Utility get_jdbv_info returns a pointer to an element of
 * this array.
 *
 * Fifth field "active" initialized to 0, currently set by long_database_init,
 * used only by * long_read_vehicle_state to decide which variables to read.
 * Later may also be used to decide which variables to write to the database.
 */
j1939_dbv_info db_ref[] = {
	{PDU, sizeof(j1939_pdu_typ), pdu_to_pdu, NULL, print_pdu, 0},
	{ERC1, sizeof(j1939_erc1_typ), pdu_to_erc1, NULL, print_erc1, 0},
	{EBC1, sizeof(j1939_ebc1_typ), pdu_to_ebc1, NULL, print_ebc1, 0},
	{ETC1, sizeof(j1939_etc1_typ), pdu_to_etc1, NULL, print_etc1, 0},
	{EEC1, sizeof(j1939_eec1_typ), pdu_to_eec1, NULL, print_eec1, 0},
	{EEC2, sizeof(j1939_eec2_typ), pdu_to_eec2, NULL, print_eec2, 0},
	{ETC2, sizeof(j1939_etc2_typ), pdu_to_etc2, NULL, print_etc2, 0},
	{TURBO, sizeof(j1939_turbo_typ), pdu_to_turbo, NULL, print_turbo, 0},
	{EEC3, sizeof(j1939_eec3_typ), pdu_to_eec3, NULL, print_eec3, 0},
	{VD, sizeof(j1939_vd_typ), pdu_to_vd, NULL, print_vd, 0},
	{ETEMP, sizeof(j1939_etemp_typ), pdu_to_etemp, NULL, print_etemp, 0},
	{PTO, sizeof(j1939_pto_typ), pdu_to_pto, NULL, print_pto, 0},
	{CCVS, sizeof(j1939_ccvs_typ), pdu_to_ccvs, NULL, print_ccvs, 0},
	{LFE, sizeof(j1939_lfe_typ), pdu_to_lfe, NULL, print_lfe, 0},
	{AMBC, sizeof(j1939_ambc_typ), pdu_to_ambc, NULL, print_ambc, 0},
	{IEC, sizeof(j1939_iec_typ), pdu_to_iec, NULL, print_iec, 0},
	{VEP, sizeof(j1939_vep_typ), pdu_to_vep, NULL, print_vep, 0},
	{TF, sizeof(j1939_tf_typ), pdu_to_tf, NULL, print_tf, 0},
	{RF, sizeof(j1939_rf_typ), pdu_to_rf, NULL, print_rf, 0},
	{HRVD, sizeof(j1939_hrvd_typ), pdu_to_hrvd, NULL, print_hrvd, 0},
	{EBC2, sizeof(j1939_ebc2_typ), pdu_to_ebc2, NULL, print_ebc2, 0},
	{RCFG, sizeof(j1939_rcfg_typ), pdu_to_rcfg, NULL, print_rcfg, 0},
	{ECFG, sizeof(j1939_ecfg_typ), pdu_to_ecfg, NULL, print_ecfg, 0},
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/* the remaining duplicate entries were added "by hand" for
	   multiple source messages - MUST BE IN SAME ORDER as definitions
	   in jbus_vars.h */
	{ERC1, sizeof(j1939_erc1_typ), pdu_to_erc1, NULL, print_erc1, 0},
	{RCFG, sizeof(j1939_rcfg_typ), pdu_to_rcfg, NULL, print_rcfg, 0},
	{TSC1,  sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	{FD, sizeof(j1939_fd_typ), pdu_to_fd, NULL, print_fd, 0},
	/* late adds: for brake control */
	{EXAC, sizeof(j1939_exac_typ), pdu_to_exac, NULL, print_exac, 0},
	{EBC_ACC, sizeof(j1939_ebc_acc_typ), pdu_to_ebc_acc, NULL, print_ebc_acc},
	/* late adds: CNG bus messages */
	{GFI2, sizeof(j1939_gfi2_typ), pdu_to_gfi2, NULL, print_gfi2, 0},
	{EI, sizeof(j1939_ei_typ), pdu_to_ei, NULL, print_ei, 0},
	/* late add: TSC1 from EBS */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/* late add: TSC1 to transmission retarder */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/*The following 8 entries are for the Volvo project 3/2015*/ 
	{VP_X, sizeof(j1939_vp_x_typ), pdu_to_vp_x, NULL, print_vp_x, 0},
	{VDC2, sizeof(j1939_vdc2_typ), pdu_to_vdc2, NULL, print_vdc2, 0},
	{MVS_X_E, sizeof(j1939_mvs_x_e_typ), pdu_to_mvs_x_e, NULL, print_mvs_x_e, 0},
	{EBC5, sizeof(j1939_ebc5_typ), pdu_to_ebc5, NULL, print_ebc5, 0},
	/* late add: TSC1 ACC to engine dbnum=538 */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/* late add: TSC1 ACC to transmission retarder  dbnum=539 */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/* late add: TSC1 brake to engine dbnum=540 */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/* late add: TSC1 VCU (or CC) to engine dbnum=541 */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/* late add: TSC1 brake to engine retarder dbnum=542 */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/* late add: TSC1 transmission to engine retarder dbnum=543 */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	/* late add: TSC1 VEC (or CC) to engine retarder dbnum=544 */
	{TSC1, sizeof(j1939_tsc1_typ), pdu_to_tsc1, NULL, print_tsc1, 0},
	{VOLVO_TARGET, sizeof(j1939_volvo_target_typ), pdu_to_volvo_target, NULL, print_volvo_target, 0},
	{VOLVO_EGO, sizeof(j1939_volvo_ego_typ), pdu_to_volvo_ego, NULL, print_volvo_ego, 0},
	{VOLVO_XBR_WARN, sizeof(j1939_volvo_xbr_warn_typ), pdu_to_volvo_xbr_warn, NULL, print_volvo_xbr_warn, 0},
	{VOLVO_XBR, sizeof(j1939_volvo_xbr_typ), pdu_to_volvo_xbr, NULL, print_volvo_xbr, 0},
	{ETC2_E, sizeof(j1939_etc2_e_typ), pdu_to_etc2_e, NULL, print_etc2_e, 0},
};

/**
 * 	Convenience routine for opening database on the current host.
 *	argv is passed to get the progran name (argv[0]) to register
 *	with the databse
 */

/**
 *      Convenience routine for opening database on the current hose
 *      argv is passed to get the progran name (argv[0]) to register
 *      with the database. Currently ignores "domain" parameter in
 *	favor of hard-coded DEFAULT_SERVICE 
 */
db_clt_typ *
open_local_database(char **argv)
{
        db_clt_typ *pclt;
        char hostname[MAXHOSTNAMELEN+1];

        /* Log in to the database (shared global memory).  Default to the
         * the current host. */
        gethostname(hostname, MAXHOSTNAMELEN+1);
        if (( pclt = clt_login( argv[0], hostname, DEFAULT_SERVICE,
                COMM_OS_XPORT)) == NULL ) {
            fprintf(stderr, "Database open error\n");
            return NULL;
        }
        return pclt;
}

/**
 *	Logs in to the database and creates database variables 
 *	for all J1939 and LAI variables. Can be called even if variables already
 *	exist, will not fail on create error.
 *
 *	If it is desired to create fewer variables, this routine could
 *	parse argv with getopt.
 *
 *	Jbus variable definitions in ../include/jbus_vars.h must be
 *	consecutive, new variables will require changing the upper bounds
 *	of the "for" loops below.
 */
db_clt_typ *
j1939_database_init(char **argv)
{
	int i;		// current DB VAR number
	int n = 0;		// number of variables created by this routine
        j1939_dbv_info *info;
	db_clt_typ *pclt;
	if ((pclt = open_local_database(argv)) == NULL)
		return NULL;
	for ( i = DB_J1939_PDU_VAR; i < DB_J1939_VOLVO_XBR_VAR; i++) {
		info = get_jdbv_info(i);
		n += clt_create(pclt, i, i, info->dbv_size);
	}
	fprintf(stderr, "%s: %d jbus variables created\n", argv[0], n);
	return( pclt );
}

/** Convenience routine to close database. */
void close_local_database(db_clt_typ *pclt)
{
	/* Log out from the database. */
	if (clt_error > 0)
		printf("clt_error %d\n", clt_error);
	if (pclt != NULL)
	        clt_logout( pclt );
}

/**
 * Returns database variable number corresponding to J1939 parameter group
 * and source or destination address of PDU, and fills in the info structure.
 */
int
get_pgn_dbv(int pgn, struct j1939_pdu *pdu, j1939_dbv_info **pinfo)
{
	int dbv;
	j1939_pgn_index *index_pdu1 = &pdu1_index[HIBYTE(pgn)];

	/* Find database variable corresponding to PGN in indices */
	if (index_pdu1->pdu2 == NULL)
		dbv = index_pdu1->db_num;
	else {
		j1939_pgn_index *index_pdu2 = index_pdu1->pdu2;
		dbv = index_pdu2[LOBYTE(pgn)].db_num;
	}

	/* Modify using source or destination address for database
 	 * variables where the message may correspond to different ECUs
	 */
	switch(dbv){
	case DB_J1939_TSC1_VAR: 
                //Destination: engine
                if (pdu->pdu_specific == J1939_ADDR_ENGINE &&
                        pdu->src_address == J1939_ADDR_ACC)
                        dbv = DB_J1939_TSC1_E_ACC_VAR;
                else
                if (pdu->pdu_specific == J1939_ADDR_ENGINE &&
                        pdu->src_address == J1939_ADDR_BRAKE)
                        dbv = DB_J1939_TSC1_E_A_VAR;
                else
                if (pdu->pdu_specific == J1939_ADDR_ENGINE &&
                        pdu->src_address == J1939_ADDR_TRANS)
                        dbv = DB_J1939_TSC1_E_T_VAR;
                else
                if (pdu->pdu_specific == J1939_ADDR_ENGINE &&
                        pdu->src_address == J1939_ADDR_CC)
                        dbv = DB_J1939_TSC1_E_V_VAR;

                //Destination: engine retarder
                if (pdu->pdu_specific == J1939_ADDR_ENG_RTDR &&
                        pdu->src_address == J1939_ADDR_ACC)
                        dbv = DB_J1939_TSC1_ER_ACC_VAR;
                else
                if (pdu->pdu_specific == J1939_ADDR_ENG_RTDR &&
                        pdu->src_address == J1939_ADDR_TRANS)
                        dbv = DB_J1939_TSC1_ER_T_VAR;
                else
                if (pdu->pdu_specific == J1939_ADDR_ENG_RTDR &&
                        pdu->src_address == J1939_ADDR_BRAKE)
                        dbv = DB_J1939_TSC1_ER_A_VAR;
                else
                if (pdu->pdu_specific == J1939_ADDR_ENG_RTDR &&
                        pdu->src_address == J1939_ADDR_CC)
                        dbv = DB_J1939_TSC1_ER_V_VAR;

	case DB_J1939_ERC1_VAR:
		if (pdu->src_address == J1939_ADDR_TR_RTDR)
			dbv = DB_J1939_ERC1_TRANS_VAR;
		break;	
	case DB_J1939_ETC2_VAR:
		if (pdu->src_address == J1939_ADDR_TRANS) {
			dbv = DB_J1939_ETC2_VAR;
		}
		else {
			dbv = DB_J1939_ETC2_E_VAR;
		}
		break;	
	case DB_J1939_RCFG_VAR:
		if (pdu->src_address == J1939_ADDR_TR_RTDR)
			dbv = DB_J1939_RCFG_TRANS_VAR;
		break;	
	default:
		break;
	}
	if (MIN_J1939_DBNUM <= dbv && dbv <= MAX_J1939_DBNUM)
		*pinfo = &db_ref[dbv - MIN_J1939_DBNUM];
	else {
		fprintf(stderr, "BAD DBNUM, pgn %d\n", pgn);
		fflush(stderr);
		*pinfo = NULL;
	}
	return dbv;
}

/**
 * Uses j1939_dbv_info structure to update database variable number db_num
 * with the data in the "buf" parameter.
 */
int
update_local_database(db_clt_typ *pclt, int db_num,
			 j1939_dbv_info *info, void *buf)

{
	if (pclt == NULL)
		return 0;
	if (clt_update(pclt, db_num, db_num, info->dbv_size,
		 (void *) buf) == FALSE )
		return 0;
	return 1;	/* successful update */
}
	
int generic = 0;	/* set to 1 by read program to store all
			 * messages as byte streams.
			 */
/**
 * Used when reading multipacket messages to keep track of which packets
 * have been received.
 */  
void set_multipacket_receive_status(int pgn, struct j1939_pdu *pdu, char *buf)

{
	int i;
	j1939_ecfg_typ *ecfg;
	j1939_rcfg_typ *rcfg;
	struct j1939_multipacket *multi = (struct j1939_multipacket *) pdu;
	switch (pgn){
	case ECFG:
		ecfg = (j1939_ecfg_typ *) &buf[0];
		ecfg->receive_status = 0;
		/* use the timestamp of the last frame received */
		ecfg->timestamp = multi->timestamp[0];
		for (i = 0; i < 4; i++){
			ecfg->receive_status <<= 1;
			ecfg->receive_status |= multi->received[i];
			if (multi->received[i] &&
			    ts2_is_later_than_ts1(&ecfg->timestamp, &multi->timestamp[i]))
			ecfg->timestamp = multi->timestamp[i];
		}
		break;
	case RCFG:
		rcfg = (j1939_rcfg_typ *) &buf[0];
		rcfg->receive_status = 0;
		for (i = 0; i < 3; i++){
			rcfg->receive_status <<= 1;
			rcfg->receive_status |= multi->received[i];
			if (multi->received[i] &&
			    ts2_is_later_than_ts1(&rcfg->timestamp, &multi->timestamp[i]))
			rcfg->timestamp = multi->timestamp[i];
		}
		break;
	default:
		break;
	}
}

/**
 * Function to access jbus database variable info from outside this file.
 * j1939_dbv_info returns a pointer to structure with information needed to
 * make database calls with database variable db_num, and with pointers to
 * utility functions for the variable. 
 *
 * Default to have active true for non-JBUS variables, allows this
 * to be used, e.g., in truckcontrol project, where all non-JBUS variables
 * will always be read.
 */
j1939_dbv_info *get_jdbv_info(int db_num)
{
	j1939_dbv_info *pinfo;
	static j1939_dbv_info dummy;

	if (db_num >= MIN_J1939_DBNUM && db_num <= MAX_J1939_DBNUM)
		pinfo = &db_ref[db_num - MIN_J1939_DBNUM];
	else {	
		pinfo = &dummy;	
		pinfo->active = 1;
	}
	return (pinfo);
}
		
/**
 * Routine to write Jbus dtabase variables to database.
 * J1939 Parameter Group Numbers each have associated data conversion/scaling
 * routines, print routines (active during debug) and database variables.
 * fp - file pointer for PGN print routines, used on Solaris and for debug.
 * pgn - parameter group number
 * pdu - pointer to single or multiple frames, depending on pgn
 * pclt - pointer to database
 * received - pointer to array of int representing whether a frame in a multipckt
 *		message sequence was actually received (NULL for single frames)
 */

void
write_pgn_to_database(FILE *fp, int pgn, struct j1939_pdu *pdu,
					 db_clt_typ *pclt, int debug)
{
	unsigned char buf[sizeof(j1939_ecfg_typ)]; /* largest type so far */
	j1939_dbv_info *info;
	int db_num = get_pgn_dbv(pgn, pdu, &info);

	info = get_jdbv_info(db_num);

	if (generic){
		info = &db_ref[0];	
		db_num = J1939_DB_OFFSET;
#ifdef __QNXNTO__
		get_current_timestamp((timestamp_t *) buf); /* first field is timestamp */
#else
		ftime((struct timeb *) buf);
#endif
	} else {
		switch (pgn) {
		case ECFG:
		case RCFG:
			set_multipacket_receive_status(pgn, pdu, (char *) buf);
			break;
		default: 
#ifdef __QNXNTO__
			get_current_timestamp((timestamp_t *) buf);
#else
		ftime((struct timeb *) buf);
#endif
			break;
		}
	}
	j1939db_receive_count++;
		
	info->pdu_to_dbv(pdu, (void *) buf);

	/** increment counter if database update not successful
	 *  If trace is on, print error message
	 */
	if (!update_local_database(pclt, db_num, info, buf)) {
		j1939db_update_err++;
#ifdef DO_TRACE
		printf("Failure to update: ");
#endif
	}
	if (debug)
	// print variable information
		info->print_dbv((void *) buf, fp, 1);
}			


/**
 * Routine to read Jbus database variables from the database.
 * Read a Jbus database variable and return the user value
 * pclt - pointer to database
 * returns 0 on error, 1 if read occurs
 * Assume database variable number and type number are the same
 */

void *
read_from_database(int db_num, db_clt_typ *pclt)
{
	static db_data_typ db_data;

	if (db_num < J1939_DB_OFFSET || db_num > J1939_DB_OFFSET + 99){
		return NULL;
	}

	if (pclt == NULL){
		return NULL;
	}

	if (clt_read(pclt, db_num, db_num, &db_data ) == FALSE ){
		return NULL;
	}

	return ((void *) &db_data.value.user);
}			

/**
 * Routine to go through a list of messages to send to the J1939 bus 
 * and send the messages that are scheduled to be sent, according
 * to the "turn" and "modulus" fields for the message in the sending list.
 *
 * This function should be called every time step, where time step
 * is the interval that is relevant for scheduling sends. 
 *
 * If a computer sends to more than one J1939 network, this function must
 * be called with the appropriate send list for each network. The 
 * cycle_number associated with the send list must be initialized to 0 before
 * the first call. 
 */

void format_pdus_and_send(db_clt_typ *pclt, j1939_send_info *sinfo,
				int *pcycle_number)
{
	int i;
	j1939_send_item *sitems = sinfo->plist;
	
	for (i = 0; i < sinfo->list_size; i++) {
		j1939_send_item *pitem = &sitems[i];
#ifdef DO_TRACE_SCHEDULE
	printf("send_list[%d]: db_num %d cycle %d modulus %d turn %d\n",
		i, pitem->db_num,*pcycle_number, pitem->modulus, pitem->turn);	
#endif
		if ((*pcycle_number % pitem->modulus) == pitem->turn) { 
			int db_num = pitem->db_num;
			j1939_dbv_info *pinfo = get_jdbv_info(db_num);
			struct j1939_pdu data_unit;
			pinfo->dbv_to_pdu(pclt, &data_unit, 
					pinfo, pitem, sinfo);
			// alway send from slot 1, ignored by some drivers
			sinfo->driver.send(sinfo->send_fd, &data_unit, 1); 
			j1939db_send_count++;
#ifdef DO_TRACE_SEND
	{
		unsigned char buf[256];
#ifdef __QNXNTO__
		get_current_timestamp((timestamp_t *) buf);
#else
		ftime((struct timeb *) buf);
#endif
		pinfo->pdu_to_dbv(&data_unit, (void *) buf);
		pinfo->print_dbv(buf, stdout, 1);
	}
#endif
		}	
	}
	// cycle number is associated with send list
	*pcycle_number = *pcycle_number + 1;
}
