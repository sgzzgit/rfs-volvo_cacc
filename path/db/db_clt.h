/**\file	
 *	db_clt.h
 *
 *	Definitions and prototypes for the client database library.
 *
 */

/*	This needs to be well known, but is local to a host.
 *	For COMM_PSX_XPORT, it is the name of a file where the
 *	db_slv pid will be written.
 */
#ifndef PATH_DB_CLT_H
#define PATH_DB_CLT_H

#include "sys_list.h"
#include "db_comm.h"

#ifdef __QNX__
#define DEFAULT_SERVICE			"db_slv"
#else
#define DEFAULT_SERVICE			"/home/path/db/register/dbq"
#endif

/*	Must be larger than the largest built in or user
 *	defined type.
 */

#define MAX_DATA_SIZE			128

/*	Size of communications packets, which are currently
 *	fixed size.
 */

#define DB_COMM_SIZE			sizeof( db_data_typ )

#define DB_TRIG_VAR(pmsg)		((pmsg)->var)
#define DB_TRIG_TYPE(pmsg)		((pmsg)->type)

/*	Reserved variable types.  These are used only for
 *	simple type checking during operations on variables.
 */

#define DB_BAD_VAR			0
#define DB_LOGIN_VAR			1
#define DB_LOGOUT_VAR			2
#define MAX_DB_BUILTIN_VAR		2

#define DB_BAD_TYPE			0
#define DB_LOGIN_TYPE			1
#define DB_LOGOUT_TYPE			2
#define DB_DOUBLE_TYPE			3
#define DB_LONG_TYPE			4
#define DB_BOOL_TYPE			5

/*	These should not be changed without changes to the servers.
 *	For the Posix message queue implementation they serve as the
 *	type field at the beginning of the message.
 */

#define DB_BAD_CMD			0
#define DB_LOGIN_CMD			1
#define DB_LOGIN_REPLY			2
#define DB_LOGOUT_CMD			3
#define DB_LOGOUT_REPLY			4
#define DB_CREATE_CMD			5
#define DB_CREATE_REPLY			6
#define DB_READ_CMD			7
#define DB_READ_REPLY			8
#define DB_UPDATE_CMD			9
#define DB_UPDATE_REPLY			10
#define DB_DESTROY_CMD			11
#define DB_DESTROY_REPLY		12
#define DB_TRIG_SET_CMD			13
#define DB_TRIG_SET_REPLY		14
#define DB_TRIG_UNSET_CMD		15
#define DB_TRIG_UNSET_REPLY		16
#define DB_TRIGGER			17	//database trigger
#define DB_TIMER			18	// artificial, for timer

#define DB_DAS_TRIGGER			100	// individual DAS drivers will
						// offset to this to get
						// unique type values

/*	Error codes.
 */

#define DB_ERR_VAR_NUM			1
#define DB_ERR_VAR_SIZE			2

#define DB_ERR_NUM(pclt)		(pclt->error)

typedef struct
{
	int uid;		/*	ID of the user and group */
	int gid;
	int key;		/*	Key used for per-process reply queue */
	double login_time;	/*	Client sends local time,
				 *	but the server will overwrite it.
				 */
	char name[USR_NAME_SIZE+1];	/*	Process or user name.	*/
} db_login_typ;

typedef struct
{
	int uid;
} db_logout_typ;

#ifdef __QNX__
typedef unsigned db_cmd_t;  /* for compatibility with prior implementation */
#else
typedef long int db_cmd_t;  /* for compatibility with Posix message queues */
#endif

typedef struct
{
#ifdef __QNXNTO__
        struct _pulse nto_pulse;/* necessary for QNX 6 messaging */
#endif
	db_cmd_t cmd;	/* Type of database operation */ 
	key_t key;	/* Required to find reply queue for message queues */ 
	unsigned var;	/* Variable and type identifier.*/
	unsigned type;	/* for database operations. */
	double time;
	union
	{
		/*	Built in types.
		 */

		double db_double;
		long db_long;
		bool_typ db_bool;
		db_login_typ login_data;
		db_logout_typ logout_data;

	        struct sigevent trig_event; /* required for QNX6 so server
					     * can reply to triggers */
		/*
		 *	User defined types end up here.
		 */

		unsigned char user[MAX_DATA_SIZE];
	} value;
} db_data_typ;

typedef struct
{
        long int msg_type;    /* required and used for Posix message queues */
        key_t key;	      /* for consistency with Posix message queues */
	unsigned var;
	unsigned type;
} trig_info_typ;

typedef struct
{
	trig_info_typ trig_info;
	comm_clt_typ *pcomm;	/* Communications handle for the trigger.*/
} clt_trig_typ;

/** On QNX4, information was returned from the database slave process using
 *  the send/reply mechanism. On Linux, each process creates a Posix message
 *  queue and sends the ID in requests to the database; this queue is used to
 *  receive replies, including synchronous read data and asynchronous triggers.
 *  The pread field is not used on QNX4.
 */
typedef struct
{
	int error;		/* The most recent error number.	*/
	comm_clt_typ *pcomm;	/* The current send channel to database. */
	comm_clt_typ *pread;	/* Reply and trigger channel from database.*/
	dl_head_typ *ptrig_head;/*	A list of clt_trig_typ.		*/
	key_t key;		/* Global key for reply and trigger channel */
	char *pservice;		/* Service name, for debug and clean-up */ 
} db_clt_typ;

/* Structure required to track trigger channel information for QNX 6 */
typedef struct
{
    int chid;  /* channel ID on QNX 6 returned by ChannelCreate */
    int coid;  /* connection ID on QNX 6 returned by ConnectAttach */
} clt_chan_typ;

extern db_clt_typ *clt_login(char *pname, char *phost, char *pserv, int xport); 
extern bool_typ clt_logout( db_clt_typ *pclt );
extern bool_typ clt_create( db_clt_typ *pclt, unsigned var, 
			unsigned type, unsigned size );
extern bool_typ clt_read( db_clt_typ *pclt, unsigned var, 
			unsigned type, db_data_typ *pbuff );
extern bool_typ clt_update( db_clt_typ *pclt, unsigned var, 
			unsigned type, unsigned size, void *pvalue );
extern bool_typ clt_destroy( db_clt_typ *pclt, unsigned var, unsigned type ); 
extern bool_typ clt_trig_set( db_clt_typ *pclt, unsigned var, unsigned type );
extern bool_typ clt_trig_unset( db_clt_typ *pclt, unsigned var, unsigned type );
extern int clt_ipc_receive( db_clt_typ *pclt, void *ptrig, int trigsize );

#define TRIGGER_PULSE_CODE _PULSE_CODE_MINAVAIL + 1
#define TIMER_PULSE_CODE _PULSE_CODE_MINAVAIL + 2

extern clt_chan_typ *clt_channel( );  /* Use for both the trigger and timer channels */
extern void clt_channel_done( );
#endif
