/**\file	
 *	db_comm.h
 *
 *	Prototypes and definitions to support the communications functions
 *	in db_comm.c.
 *
 */
#ifndef PATH_DB_COMM_H
#define PATH_DB_COMM_H

#ifdef __QNXNTO__
#include <sys/neutrino.h>
#endif /* __QNXNTO__ */
/* 8-bit number for ftok with COMM_PSX_XPORT */
#define PATH_COMM_PROJECT_ID	'P'	

#define COMM_BAD_SIZE			0
#define COMM_WILD_HANDLE		0		/*	For promiscuous receive		*/

/*	Define messaging transport.
 */

#define COMM_QNX_XPORT			1
#define COMM_TCP_XPORT			2
#define COMM_PSX_XPORT			3
#define COMM_QNX6_XPORT			4

typedef struct
{
	unsigned xport;		/*	Transport type.			*/
	union			/*	Transport information.		*/
	{
		int chid;	/*	QNX6 messaging.			*/
		pid_t pid;	/*	QNX messaging.			*/
		int fd;		/*	TCP socket.			*/
		int qid;	/*	Posix message queue id.		*/
	} xport_info;

} comm_clt_typ;

/** First two (or three on QNX 6) fields of db_data_typ, or any other 
 *  transmitted message, must match types of this
 */
typedef struct
{
#ifdef __QNXNTO__
        struct _pulse nto_pulse;/* necessary for QNX 6 messaging */
#endif
        long int msg_type;      /* required + used for Posix message queues */
        key_t key;		/* key of queue which may be used for reply */
        unsigned char data[1];	/* rest of message can be aliased here */
} comm_recv_typ;

/*	Size of complete packet, including data. 	*/
typedef unsigned comm_header_typ;

extern long int comm_get_handle(int xport, char *pservname, int pid );
extern comm_clt_typ *comm_init( int xport, int handle );
extern comm_clt_typ *comm_name_init( int xport, char *phost, char *pserv );
extern comm_clt_typ *comm_clt_dup( comm_clt_typ *pclt );
extern bool_typ comm_done( comm_clt_typ *pclt );
extern void comm_cleanup( comm_clt_typ *pclt, char *pserv, int pid );

extern bool_typ comm_sync_send( comm_clt_typ *pclt, void *pxmit, void *preply,
				unsigned xmit_size, unsigned reply_size );
extern comm_clt_typ *comm_sync_recv( comm_clt_typ *pclt, void *precv,
				unsigned recv_size );
extern bool_typ comm_sync_reply( comm_clt_typ *pclt, void *preply, 
				unsigned reply_size );
extern bool_typ comm_async_trig( comm_clt_typ *pclt, void *pxmit, unsigned xmit_size );
extern int comm_clt_cmp( comm_clt_typ *pclt1, comm_clt_typ *pclt2 );
extern void comm_print_header( comm_header_typ header );
extern void comm_print_clt( comm_clt_typ *pclt );

#define COMM_GET_XPORT(pclt)	(pclt->xport)

/* just copies to pcpy pointer, clearer than all the casting when original
 * type of pcpy is not comm_clt_typ */
static inline void comm_clt_cpy ( comm_clt_typ *porig, comm_clt_typ *pcpy)
{
	*pcpy = *porig;
}

/*  number of times to retry msgsnd or msgrcv if they fail */
#define DB_MSGQ_RETRY_LIMIT 10

#endif
