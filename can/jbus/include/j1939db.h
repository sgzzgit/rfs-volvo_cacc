/**\file
 * 	Declarations and definitions for j1939db.c.  
 *
 *
 * Copyright (c) 2005   Regents of the University of California
 *
 */
#ifndef J1939DB_H
#define J1939DB_H

#define J1939_DB_OFFSET 500	/* numbers are assigned starting here */

extern int j1939db_update_err;
extern int j1939db_send_count;
extern int j1939db_receive_count;


/** Function pointer types stored in arrays corresponding to J1939
 * Parameter Group Number (initialized in this file) and in arrays
 * holding lists of messages to send.
 *
 * It is assumed that any message that arrives is converted to a single
 * database variable type, that will not change for any application, but
 * that messages to be sent can be constructed from any information in the
 * database that the function pointed to by snd_fptr chooses to access,
 * and that this information may vary for differnt applications.
 */

typedef void (*rcv_fptr) (struct j1939_pdu *pdu, void *dbv);
typedef	void (*print_fptr) (void *dbv, FILE *fp, int numeric);
typedef void (*snd_fptr) (db_clt_typ *pclt, struct j1939_pdu *pdu,
				void *pdbv_info, void *pitem_info,
				void *psend_info); 

/**
 * Type for internal index table used to find database variable from
 * PGN of J1939 variable as read from J1939 serial network.
 */
typedef struct {
	void *pdu2;	/* pointer to array for PDU2 format */
	int db_num;	/* number of database variable */
} j1939_pgn_index;

/**
 * Type for Jbus database variable information used in PGN indexed array..
 * Holds function pointers to print and conversion routines, to take
 * the J1939 Protocol Data Unit (PDU) and write it into a database variable..
 * Also holds database variable size, PGN, and an "active" field, to be set
 * if variable is being used by some application program.
 */
typedef struct {
	int pgn;
	int dbv_size;	/* size of data base variable */
	rcv_fptr pdu_to_dbv;
	snd_fptr dbv_to_pdu;
	print_fptr print_dbv;
	int active;	/* set if being used by control programs  */
} j1939_dbv_info;

/**
 *	A message will be sent out on the CAN bus if
 *	cycle number % modulus == turn
 */
typedef struct{
	int db_num;	// database number corresponding to message to be sent 
	int db_size;	// size of message to be sent
	int priority;	// priority to set in message header on J1939 bus
	int turn;	// cycle index to schedule the send
	int modulus;	// modulus for scheduling 
} j1939_send_item;

typedef struct{
	j1939_send_item *plist;
	int list_size;
	unsigned int src;	// J1939 address of the sending port
	unsigned int dst;	// only one destination per list
	int send_fd;	// only one J1939 bus device descriptor per list
	jbus_func_t driver; // function pointers to driver send and receive	
} j1939_send_info;


extern j1939_pgn_index pdu1_index[];
extern j1939_pgn_index pdu2_240[];
extern j1939_pgn_index pdu2_254[];
extern j1939_pgn_index pdu2_255[];
extern j1939_dbv_info db_ref[]; 
extern j1939_dbv_info j1587_db_ref[]; 
extern db_clt_typ * open_local_database(char **argv);

extern db_clt_typ * j1939_database_init(char **argv);

extern void close_local_database(db_clt_typ *pclt);

extern int get_pgn_dbf(int pgn);

extern j1939_dbv_info *get_jdbv_info(int db_num);

extern int update_local_database(db_clt_typ *pclt, int db_num,
			 j1939_dbv_info *info, void *buf);

extern void write_pgn_to_database(FILE *fp, int pgn, struct j1939_pdu *pdu,
			 db_clt_typ *pclt, int debug);
extern void * read_from_database(int db_num, db_clt_typ *pclt);

extern void send_to_jbus(db_clt_typ *pclt, j1939_send_info *send_list,
			int list_size);


#endif //J1939DB_H
