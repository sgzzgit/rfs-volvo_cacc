/**\file
 * SAE J1939 basic message types.
 * This file contains definitions for the SAE J1939 serial control
 * and communications vehicle network.
 *
 */

/** CAN extended frame format, used by J1939.
 * See SAE J1939/21, Figures 2 and 3
 * For info, not used by code -- hardware does decode.
 */

typedef unsigned char MSG_BIT;
typedef unsigned int MSG_FIELD; 
typedef unsigned char MSG_BYTE;
typedef unsigned char FRAME_BIT;
	
struct can_extended_frame {
	FRAME_BIT SOF;			/* frame bit 1, START OF FRAME */
	MSG_FIELD identifier;		/* 11 bits, bits 28:18 of identifier;
						 frame bits 2:12 */
	FRAME_BIT SRR;			/* frame bit 13; recessive */	
	FRAME_BIT IDE;			/* frame bit 14; recessive
					 * dominant for CAN standard frame
					 */
	MSG_FIELD identifier_extension; /* 18 bits; bits 17:0; frame 15:32 */
	FRAME_BIT RTR;			/* frame bit 33 */
	FRAME_BIT  r1;			/* frame bit 34 */
	FRAME_BIT  r0;			/* frame bit 35 */
	MSG_FIELD data_length_code;	/* frame bit 36:39; number of
  					 * bytes in data payload
					 */
	MSG_BYTE data_field[8];		/* 64 bits max;  */ 
	MSG_FIELD CRC;			/* 15 bits */
	FRAME_BIT CRC_delimiter;	/* begin no bit stuffing */
	MSG_FIELD ACK;			/* 2 bits; any device may set to
					 * dominant to indicate correct 
					 * CRC received
					 */
	FRAME_BIT end_of_frame;		/* 1 bit, END OF FRAME */
}; 

/** J1939 Protocol Data Unit (PDU) format.
 * See SAE J1939, 3.1.2
 */
struct j1939_pdu {
	MSG_FIELD priority;	/* bits 3:1, frame bits 2:4; ID 28:26, used for
				 * arbitration, lower priority for data not
				 * time critical; 000 highest priority
				 */
	MSG_BIT R;		/* frame bit 5;ID 25; reserved for future use */
	MSG_BIT DP;		/* frame bit 6;ID 24; 0 currently  */
	MSG_FIELD pdu_format;	/* Protocol Data Unit Format (PF); bits 8:1;
				 * frame bits 7:12, 15:16; ID 23:16 
				 */
	MSG_FIELD pdu_specific; /* PDU Specific (PS); 
				 * bits 8:1, frame bits 18:24; ID 15:8
				 * if 0<=PF<=239, destination address (DA);
				 * if 240<=PF<=255, group extension (GE);
				 */
	MSG_FIELD src_address;	/* bits 8:1; frame bits 25:32; ID 7:0 */
	MSG_BYTE data_field[8];	/* 64 bits maximum */
	int numbytes;		/* number of bytes in data_field */
};

#define J1939_MAX_FRAMES 255
struct j1939_multipacket {
	struct j1939_pdu pdu[J1939_MAX_FRAMES]; /* must be first fields */
	int numpackets;
	int in_progress;
	int pgn;	/* Parameter group number of message in progress */
	int received[J1939_MAX_FRAMES];
	timestamp_t timestamp[J1939_MAX_FRAMES];
};

typedef unsigned int PGN_TYPE;

/** J1939 Parameter Group
 * Parameter Group numbers are assigned by SAE, associated with values of
 * PDU fields R, DP, PF and PS fields, specify characteristics of message 
 * type, such as multipacket.
 * See SAE J1939, 3.1.2
 */
struct j1939_parameter_group {
	PGN_TYPE parameter_group_number;	/* 3-byte number */
	char *acronym;				/* character string */
	MSG_BIT R;				/* reserved for future use */
	MSG_BIT DP;				/* page 0 or 1  */
	MSG_FIELD pdu_format;	/* Protocol Data Unit Format (PF) (8 bits) */
	MSG_FIELD pdu_specific; /* PDU Specific (PS) (8 bits) DA or GE */ 
};

/** J1939 Name format (8 bytes)
 * See SAE J1939, 3.1.3.
 */
struct j1939_name{
	MSG_BIT arbitrary_address_capable;	/* 1 bit */
	MSG_FIELD industry_group;		/* 3 bit */
	MSG_FIELD vehicle_system_instance;	/* 4 bit */
	MSG_FIELD vehicle_system;		/* 7 bit */
	MSG_BIT reserved;			/* 1 bit */
	MSG_FIELD function;			/* 8 bit */
	MSG_FIELD function_instance;		/* 5 bit */
	MSG_FIELD ECU_instance;			/* 3 bit */
	MSG_FIELD manufacturer_code;		/* 11 bit */
	MSG_FIELD identity_number;		/* 21 bit */
};

/** Model 1939STB RS-232 to J1939 Converter format
 * See 1939STB4600 Manual from B & B Electronics
 */

#define J1939STB_SERIAL_DEVICE_NAME	"/dev/ser1"

struct j1939_stb_transmit{
	MSG_BYTE check1;	/* must be 0x81 */
	MSG_BYTE check2;	/* must be 0x21 */
	MSG_BYTE control1;	/* control info for converter */
	MSG_BYTE control2;	/* control info for converter */
	MSG_BYTE msg_info[7];	/* control and header info for J1939 message */
	MSG_BYTE data_field[8]; /* data payload for J1939 message */
};	

#define	CTRL1_INTERNAL_CMD	1<<5
#define CTRL1_EXTERNAL_CMD	1<<4
#define CTRL1_NUMBYTES_MSK	0xf

/** When the CTRL1_EXTERNAL_CMD bit is 1, control2 contains the slot number
 * where the message should be stored on the 1939STB. Valid slots are
 * 1 to 14, slot 15 is reserved for receiving messages. When the
 * CTRL1_INTERNAL_CMD is 1, control2 may specify the following two
 * internal commands:
 */

#define CTRL2_VENDOR_VERSION	0x1
#define CTRL2_SET_BAUD		0x2
#define CTRL2_SLOT_MSK		0xf

/** Bytes 5 and 6 of j1939_stb (msg_info[0] and msg_info[1]) are programmed
 * with two bit fields that have the following significance:
 *	
 *	Function on write	Function on read
 *
 * 01	Reset element		Element is reset
 * 10	Set element		Element is set
 */


#define MI0_VALID_SET		(2<<6)
#define MI0_VALID_RESET		(1<<6)
#define MI0_TXIE_SET		(2<<4)
#define MI0_TXIE_RESET		(1<<4)
#define MI0_RXIE_SET		(2<<2)
#define MI0_RXIE_RESET		(1<<2)
#define MI0_INTPND_SET		2
#define MI0_INTPND_RESET	1

#define MI1_RMTPND_SET		(2<<6)
#define MI1_RMTPND_RESET	(1<<6) 
#define MI1_TXRQ_SET		(2<<4)
#define MI1_TXRQ_RESET		(1<<4)
#define MI1_MSGLOST_SET		(2<<2)
#define MI1_MSGLOST_RESET	(1<<2)
#define MI1_CPUUPD_SET		(2<<2)
#define MI1_CPUUPD_RESET	(1<<2)
#define MI1_NEWDAT_SET		2
#define MI1_NEWDAT_RESET	1

/** Bytes 7, 8, 9 and 10 (msg_info[2:5]) of the Model1939STB transmit format
 * are filled with the identifier and address info of the J1939 message.
 * The following expressions give the values needed for each byte
 * as macros operating on a pointer of type struct j1939_pdu *
 */

#define FILL_M2(j) ((((j)->priority & 0x7) << 5) |\
			(((j)->R & 0x1) << 4) | \
			(((j)->DP & 0x1) << 3) | \
			(((j)->pdu_format >> 5) & 0x7))

#define FILL_M3(j) ((((j)->pdu_format & 0x1f) << 3) |\
			(((j)->pdu_specific >> 5) & 0x7)) 

#define FILL_M4(j) ((((j)->pdu_specific & 0x1f) << 3) |\
			(((j)->src_address >> 5) & 0x7)) 
	 
#define FILL_M5(j) ((((j)->src_address & 0x1f) << 3))

/** Byte 11 (msg_info[6]) has the Data Length Code, Message Direction
 * and XTD bit (always 1, since J1939 does not support standard frames)
 */

#define MI6_DLC_SHIFT	 	4	
#define MI6_DIR_TRANSMIT 	(1<<3)	
#define MI6_XTD			(1<<2)

/** state codes used to construct messages to converter have low order byte
 * with value for msg_info[0], second lowest byte with value for msg_info[1],
 * and third lowest bytes with value for msg_info[6], low nibble.
 */

#define STB_RECEIVE	0x045599	/* no transmit, set VALID, RXIE */
#define STB_TXUPD	0x0c69a5	/* transmit, VALID, TXIE, TXRQ, CPUUPD */
#define STB_TXNEW	0x0c66a5	/* transmit, VALID, TXIE, TXRQ, NEWDAT */
#define STB_TXRX	0x0c55a9	/* transmit, VALID, TXIE, RXIE */
#define STB_TXIE	0x0c55a5	/* transmit, VALID, TXIE */
#define STB_TXRMT	0x0ca5a5	/* transmit, RMTPND, TXRQ, VALID, TXIE */
  
struct j1939_stb_receive{
	MSG_BYTE id;		/* must be 0x42 */
	MSG_BYTE function;	/* controller code for message type */
	MSG_BYTE j1939_info[4];	/* priority, R, DP and PDU */
	MSG_BYTE MC;		/* same format as msg_info[6], Byte 11
				 * on transmit
				 */
	MSG_BYTE data_field[8]; /* data payload for J1939 message */
	MSG_BYTE final;		/* set to FF for J1939 messages;
				 * AA for converter messages;
				 */	
};	

#define STB_RCV_MSG_MSK		0xf0
#define STB_RCV_MSG_SHFT	   4	
#define STB_RCV_TYPE_MSK	0x0f
#define EXTRACT_PRIORITY(b)	(((b)->j1939_info[0] & 0xe0) >> 5) 
#define EXTRACT_PF(b)		((((b)->j1939_info[0] & 0x7) << 5) | \
				 (((b)->j1939_info[1] & 0xf8) >> 3))
#define EXTRACT_PS(b)		((((b)->j1939_info[1] & 0x7) << 5) | \
				 (((b)->j1939_info[2] & 0xf8) >> 3))
#define EXTRACT_SRC_ADDR(b)	((((b)->j1939_info[2] & 0x7) << 5) | \
				 (((b)->j1939_info[3] & 0xf8) >> 3))

/** control codes for internal Model 1939 STB messages */

#define J1939STB_VENDOR_STRING	0x120
#define J1939STB_BAUD_RATE	0x223

/** macros for SSV CAN formatted IDs */

#define PATH_CAN_ID(j)  ((((j)->priority & 0x7) << 26) | \
			(((j)->R & 0x1) << 25) | \
			(((j)->DP & 0x1) << 24) | \
			(((j)->pdu_format & 0xff) << 16) | \
			(((j)->pdu_specific & 0xff) << 8) | \
			(((j)->src_address & 0xff))) 

#define PATH_CAN_PRIORITY(j)	 (((j) >> 26) & 0x7)
#define PATH_CAN_PF(j)		 (((j) >> 16) & 0xff)
#define PATH_CAN_PS(j)		 (((j) >> 8) & 0xff)
#define PATH_CAN_SA(j)		 ( (j) & 0xff)

#define J1939_BYTE_UNDEFINED	0xff	/* usual value to fill empty fields */
	
/** General purpose macros for extracting fields */
/** Bit number in these macros for bytes follows convention in J1939 docs */

#define BITS87(x)	(((x) & 0xc0) >> 6)
#define BITS65(x)	(((x) & 0x30) >> 4)
#define BITS43(x)	(((x) & 0x0c) >> 2)
#define BITS21(x)	 ((x) & 0x03)	
#define HINIBBLE(x)	(((x) & 0xf0) >> 4)
#define LONIBBLE(x)	 ((x) & 0x0f)

/** Macros for short (two byte) values */
#define HIBYTE(x)	(((x) & 0xff00) >> 8)
#define LOBYTE(x)	((x) & 0xff)
#define TWOBYTES(x, y)	((((x) & 0xff) << 8) | (y & 0xff))

/** Macros for int (four byte) values */
#define BYTE0(x)	(((x) & 0xff))
#define BYTE1(x)	(((x) & 0xff00) >> 8)
#define BYTE2(x)	(((x) & 0xff0000) >> 16)
#define BYTE3(x)	(((x) & 0xff000000) >> 24)
#define FOURBYTES(a3, a2, a1, a0) \
			(((a3) & 0xff) << 24) |\
			(((a2) & 0xff) << 16)|\
			(((a1) & 0xff) << 8)|\
			((a0) & 0xff) 

/** Structure used to hold send, receive and init function pointers, that
 *  may be different for different drivers.
 */
typedef struct {
	int (*send)(int fd, struct j1939_pdu *pdu, int slot); 
	int (*receive)(int fd, struct j1939_pdu *pdu, int *extra, int *slot_or_type);
	// p_other pointer is just in case other information is needed
	// for some driver initialization
	int (*init)(char *filename, int flags, void *p_other);
	int (*close)(int *pfd);
} jbus_func_t;
	
/** Return values for receive function */

#define J1939_RECEIVE_FATAL_ERROR (-1)
#define J1939_RECEIVE_MESSAGE_ERROR 0
#define J1939_RECEIVE_MESSAGE_VALID 1

/** start of message byte value for B&B J1939STB converter */
#define J1939_STB_SOM 0x42

/** Functions in j1939stb.c */
extern void
pdu_to_stb(struct j1939_pdu *pdu, struct j1939_stb_transmit *stb, int slot);

extern void
send_stb_external(int fd, struct j1939_pdu *pdu, int state_code, int slot); 

extern void
send_stb_internal(int fd, short control_code, int baud_rate); 

extern int
send_stb(int fd, struct j1939_pdu *pdu, int slot); 
 
extern int
receive_stb(int fd, struct j1939_pdu *pdu, int *external, int *slot_or_type);

extern int
sync_stb(int fd);

extern int
init_stb(char *filename, int flags, void *p_other);

extern int
close_stb(int *pfd);

/** Functions in j1939can.c */
extern int
init_can(char *filename, int flags, void *p_other);

extern int
close_can(int *pfd);
extern int
send_can(int fd, struct j1939_pdu *pdu, int slot); 

extern int
receive_can(int fd, struct j1939_pdu *pdu, int *extended, int *slot_or_type);

