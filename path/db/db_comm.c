/**\file
 *	db_comm.c
 *
 *	This is a library of support routines for interprocess
 *	communications for use by database servers and clients.  The
 *	routines are designed to provide a consistent programming
 *	interface for IPC, bassed on a message passing model.
 *
 *	In general, application programs should not need to call
 *	these routines.  The database library uses these routines
 *	to connect to the server.
 *
 *	Code was originally implemented on QNX4, using QNX networking
 *	with send/receive/reply semantics, and used successfully for
 *	for PATH projects for a decade or more.	Although it was originally
 *	written as a networked database, in fact it was never used in that
 *	way, but only as an IPC mechanism including triggering on a single
 *	processor. The variability of network delays generally made it 
 *	undesirable for implicit messaging to be used for communication
 *	between network nodes in real-time control systems.
 * 
 *	In 2006 the code was ported to use POSIX message queues under Linux,
 *	with the intent to support IPC and triggering on a single network node.
 *
 *      In 2008 the code was ported to QNX 6 using low(er) level message
 *      passing interfaces (rather than Resource or IO managers).
 *
 *	Stubs for a TCP/IP implementation were part of the original 
 *	QNX4 code. They have been left in but have never been made to work.
 *
 */

#include "db_include.h"
#ifdef __QNXNTO__
#include <sys/dispatch.h>
#endif
#undef DO_TRACE

#define QNX_MAX_MX_PARTS		2

/**
 *	This function is new for COMM_PSX_XPORT support
 *	For COMM_QNX_XPORT it just returns whatever is passed in
 *	as "pid". For COMM_PSX_XPORT, it creates a file name by 
 *	concatenating the pservname and pid arguments, and returns
 *	the result of running ftok on that filename.
 */
long int comm_get_handle (int xport, char *pservname, int pid )
{
	char filename[MAX_LINE_LEN];
	long key_val;
	switch (xport)
	{
	case COMM_PSX_XPORT:
		sprintf( filename, "%s_%d", pservname, pid);
		key_val = ftok(filename, PATH_COMM_PROJECT_ID);
		return (key_val);
	case COMM_QNX6_XPORT:
	case COMM_QNX_XPORT:
	default:
		return pid;
	}
}

/**
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include "db_comm.h"
 *
 *	comm_clt_typ *comm_init( int xport, int handle );
 *
 *	xport	-	COMM_QNX6_XPORT for QNX 6 networking,
 *                      COMM_QNX_XPORT for QNX 4 networking,
 *			COMM_TCP_XPORT for TCP/IP networking.
 *				(TCP/IP is currently unsupported).
 *			COMM_PSX_XPORT for Posix message queues
 *
 *	handle	-	For QNX 6 networking, this is a channel id
 *                      For QNX 4 networking, this is the peer process pid or
 *			      COMM_WILD_HANDLE.
 *			For TCP/IP, this is an open file descriptor.
 *			For Posix message queues, this is the key used in 
 *			      the call to msgget.
 *
 *	DESCRIPTION
 *	
 *	This routine creates a communications channel of the specified
 *	transport.  This routine is intended for use by servers which
 *	require specific configurations on the handle.  In general,
 *	clients should use the comm_name_init() call.
 *
 *	RETURN
 *	NULL		-	If there is a memory or transport failure.
 *	non-NULL	-	If the call succeeds.
 *
 */

comm_clt_typ *comm_init( int xport, int handle )
{
	comm_clt_typ *pnew;

	if( (pnew = CALLOC( 1, sizeof( comm_clt_typ ) )) == NULL )
		return( NULL );

	switch( pnew->xport = xport )
	{
	case COMM_QNX6_XPORT:
		pnew->xport_info.chid = handle;
		return( pnew );

	case COMM_QNX_XPORT:
		pnew->xport_info.pid = handle;
		return( pnew );

	case COMM_TCP_XPORT:
		pnew->xport_info.fd = handle;
		return( pnew );

	case COMM_PSX_XPORT:
		pnew->xport_info.qid = msgget(handle, 0666 | IPC_CREAT);
		return( pnew );

	default:
		FREE( pnew );
		return( NULL );
	}
}

/**
 *	SYNOPSIS
 *
 *	#include <local.h>
 *
 *	#include "db_comm.h"
 *
 *	comm_clt_typ *comm_name_init( int xport, char *phost, char *pserv );
 *
 *	xport	-	COMM_QNX6_XPORT for QNX 6 networking,
 *                      COMM_QNX_XPORT for QNX 4 networking,
 *			COMM_TCP_XPORT for TCP/IP networking.
 *				(TCP/IP is currently unsupported).
			COMM_PSX_XPORT for Posix message queues
 *
 *	phost	-	Host name of server or QNX node ID, unused on Linux.
 *
 *	pserv	-	Service name for connection. For COMM_PSX_XPORT,
 *			it is a file name that will hold the queue ID
 *			of the message queue to be connected to.  
 *
 *	DESCRIPTION
 *
 *	This routine creates a communications channel of the specified
 *	transport to the given host and well-known service name.  This
 *	routine is intended for use by the client database library,
 *	and creates the underlying communications transport.
 *
 *	RETURN
 *	NULL		-	If the host or service cannot be located, or
 *					there is a memory or transport failure.
 *	non-NULL	-	If the call succeeds.
 *
 */

comm_clt_typ *comm_name_init( int xport, char *phost, char *pserv )
{
	int node;
	int chid;
	pid_t pid;
	int handle;

	switch( xport )
	{
	case COMM_QNX6_XPORT:
		if( (chid = name_open( phost, 0 )) == ERROR )
			return( NULL );
		else
			return( comm_init( xport, chid ) );

	case COMM_QNX_XPORT:
		node = (nid_t) atol( phost );
		if( (pid = qnx_name_locate( node, pserv, 0, NULL )) == ERROR )
			return( NULL );
		else
			return( comm_init( xport, pid ) );

	case COMM_TCP_XPORT:
		return( NULL );

	case COMM_PSX_XPORT:
		if ((handle = comm_get_handle(xport,pserv,0))  == ((key_t) - 1))
		{
			perror("comm_name_init comm_get_handle");	
			return( NULL );
		} else
			return( comm_init( xport, handle) );

	default:
		return( NULL );
	}
}

/**
 *	SYNOPSIS
 *
 *	#include <local.h>
 *
 *	#include "db_comm.h"
 *
 *	comm_clt_typ *comm_clt_dup( comm_clt_typ *pclt )
 *
 *	pclt	-	A valid communications channel.
 *
 *	DESCRIPTION
 *	This returns a copy of an existing channel.  For a QNX
 *	message channel, this is just a copy of the existing information.
 *	For a TCP/IP channel, this is a duplicate descriptor,
 *	as returned by dup().
 *
 *	RETURN
 *	NULL		-	If the existing channel is invalid, or there
 *					is a memory or dup() failure.
 *	non-NULL	-	If the call succeeds.
 *
 */

comm_clt_typ *comm_clt_dup( comm_clt_typ *pclt )
{
	comm_clt_typ *pnew;

	if( pclt == NULL )
		return( NULL );

	if( (pnew = MALLOC( sizeof( comm_clt_typ ) )) == NULL )
		return( NULL );

	switch( pclt->xport )
	{
	case COMM_QNX6_XPORT:
		pnew->xport = pclt->xport;
		pnew->xport_info.chid = pclt->xport_info.chid;
		break;

	case COMM_QNX_XPORT:
		pnew->xport = pclt->xport;
		pnew->xport_info.pid = pclt->xport_info.pid;
		break;

	case COMM_TCP_XPORT:
		if( (pnew->xport_info.fd = dup( pclt->xport_info.fd ))
				 == ERROR )
		{
			FREE( pnew );
			return( NULL );
		}
		break;

	case COMM_PSX_XPORT:
		pnew->xport = pclt->xport;
		pnew->xport_info.qid = pclt->xport_info.qid;
		break;

	default:
		FREE( pnew );
		return( NULL );
	}
	return( pnew );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *
 *	#include "db_comm.h"
 *
 *	bool_typ comm_done( comm_clt_typ *pclt );
 *
 *	pclt	-	A valid communications channel.
 *
 *	DESCRIPTION
 *	This closes an existing communications channel, and frees
 *	all associated system resources.
 *
 *	RETURN
 *	FALSE	-	If the channel is invalid.
 *	TRUE	-	If the call succeeds.
 */

bool_typ comm_done( comm_clt_typ *pclt )
{
	if( pclt == NULL )
		return( FALSE );

	switch( pclt->xport )
	{
	case COMM_TCP_XPORT:
		close( pclt->xport_info.fd );
		FREE( pclt );
		return( TRUE );

	case COMM_QNX6_XPORT:
	case COMM_QNX_XPORT:
	case COMM_PSX_XPORT:
	default:
		FREE( pclt );
		return( FALSE );
	}
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *
 *	#include "db_comm.h"
 *
 *	bool_typ comm_sync_send( comm_clt_typ *pclt, void *pxmit,
 *		void *preply, unsigned xmit_size, unsigned reply_size );
 *
 *	comm_clt_typ *comm_sync_recv( comm_clt_typ *pclt, void *precv,
 *				unsigned recv_size );
 *
 *	bool_typ comm_sync_reply( comm_clt_typ *pclt, void *preply, 
 *				unsigned reply_size );
 *
 *	pclt		-	A valid communications channel.
 *				For the send call, this should
 *				be a channel to a peer.
 *	pxmit, preply	-	Pointers to data for transmission
 *				in a send or reply call.
 *				For Posix message queue implementation,
 *				the preply pointer is overloaded to hold
 *				a pointer to the comm_clt_typ channel to
 *				msgrcv on for the reply.
 *	precv		-	Pointer for incoming data from a
 *				receive call.
 *	xmit_size, recv_size, -	The respective buffer sizes.
 *				For Posix message queues, the size of
 *				the initial type field, which must be long,
 *				must be subtracted before calling msgsnd
 *				and msgrcv.
 *				
 *
 *	DESCRIPTION
 *	These calls provide support for the QNX synchronous (blocking)
 *	communications messaging model.  In this model, sending processes
 *	block until the peer process receives and replys to the message.
 *	Similarly, receiving processes will block, waiting to reply to a
 *	send from another process.
 *	
 *	RETURN
 *	comm_sync_send(), comm_sync_reply():
 *		TRUE	-If the call succeeds.  For a send call, data will
 *				be in the preply buffer.
 *		FALSE	-If the channel is invalid, or the underlying
 *				transport fails.
 *
 *	comm_sync_recv():
 *		NULL	-If the channel is invalid, or the underlying
 *				transport fails.
 *		non-NULL-A pointer to a static channel structure which
 *				is the sending process.  This should be copied,
 *				or used immediately in a reply call.
 *
 */

bool_typ comm_sync_send( comm_clt_typ *pclt, void *pxmit, void *preply,
				unsigned xmit_size, unsigned reply_size )
{
	struct _mxfer_entry xmit_mx[QNX_MAX_MX_PARTS];
	struct _mxfer_entry reply_mx[QNX_MAX_MX_PARTS];
	comm_header_typ xmit_header, reply_header;
	comm_clt_typ *pread = (comm_clt_typ *) preply;
	int reply_qid; 

	if( pclt == NULL )
		return( FALSE );

	xmit_header = sizeof( comm_header_typ ) + xmit_size;
	switch( pclt->xport )
	{
	case COMM_QNX6_XPORT:
 	        if (0 > MsgSend_r( pclt->xport_info.chid, pxmit, xmit_size, 
				   preply, reply_size ))
		{
			return( FALSE );
		}
		break;
	case COMM_QNX_XPORT:
		_setmx( &xmit_mx[0], &xmit_header, sizeof( comm_header_typ ) );
		_setmx( &xmit_mx[1], pxmit, xmit_size );

		_setmx( &reply_mx[0], &reply_header, sizeof( comm_header_typ ) );
		_setmx( &reply_mx[1], preply, reply_size );

		if( Sendmx( pclt->xport_info.pid, QNX_MAX_MX_PARTS, 
			QNX_MAX_MX_PARTS, xmit_mx, reply_mx ) != 0 )
		{
			return( FALSE );
		}
		break;
	case COMM_PSX_XPORT:
		reply_qid= pread->xport_info.qid;
		/* blocks if no space, could change to IPC_NOWAIT to avoid */
#ifdef DO_TRACE
		printf(
		"Sending to msgq %d, size %d, receive reply at %d, size %d\n",
		pclt->xport_info.qid, xmit_size - sizeof(long),
			 reply_qid, reply_size - sizeof(long));
		fflush(stdout);
#endif
		{
			int repeat = 0;
			while ( (msgsnd( pclt->xport_info.qid, pxmit, 
				xmit_size - sizeof(long), 0 ) == -1) &&
				repeat++ < DB_MSGQ_RETRY_LIMIT ); 
			if (repeat >= DB_MSGQ_RETRY_LIMIT)
			{
				perror( "comm_sync_send send" );
				return( FALSE );
			}
		}
#ifdef __CYGWIN__
		/* message type exception does not appear to be supported
		 * may have trouble with triggers */
		{
			int repeat = 0;
			while (( msgrcv( reply_qid, preply,
				 reply_size - sizeof(long), 0, 0 )  == -1) &&
				repeat++ < DB_MSGQ_RETRY_LIMIT ); 
			if (repeat >= DB_MSGQ_RETRY_LIMIT)
			{	
				perror( "comm_sync_send reply");
				return( FALSE );
			}		
		}
#else
		/* ignore trigger messages; anything else will be reply */
		{
			int repeat = 0;
			while (( msgrcv( reply_qid, preply,
				 reply_size - sizeof(long),
				 DB_TRIGGER, MSG_EXCEPT )  == -1 ) &&
				repeat++ < DB_MSGQ_RETRY_LIMIT ); 
			if (repeat >= DB_MSGQ_RETRY_LIMIT)
			{	
				perror( "comm_sync_send reply");
				return( FALSE );
			}		
		}
#endif
		break;
	default:
		return( FALSE );
	}
	return( TRUE );
}

comm_clt_typ *comm_sync_recv( comm_clt_typ *pclt, void *precv,
			      unsigned recv_size )
{
	static comm_clt_typ sender;
	struct _mxfer_entry recv_mx[QNX_MAX_MX_PARTS];
	comm_header_typ header;
	comm_recv_typ *pcr = ( comm_recv_typ * ) precv;
	/* This is treating precv polymorphically -- it is a db_data_typ! */

	if( pclt == NULL )
		return( NULL );

	switch( pclt->xport )
	{
	case COMM_QNX6_XPORT: {
 	        struct _msg_info msginfo;
 	       	int recv_ret;
	
		sender.xport = COMM_QNX6_XPORT;
		
		while(1) {
		        recv_ret = MsgReceive_r( pclt->xport_info.chid, precv, 
						 recv_size, &msginfo );
			if( 0 > recv_ret ) { /* ERROR */
			        return( NULL );
			} else if ( 0 == recv_ret ) { /* PULSE */
			       /* For now, ignore pulses */
#ifdef __QNXNTO__ /* Protect reference to nto_pulse 
		     (field undefined on other OSes) */
			        if ( pcr->nto_pulse.type == _IO_CONNECT )
				{
				        MsgReply_r(recv_ret, EOK, NULL, 0);
				}
#endif
 			        continue;
			} else {
			       /* Must handle the case of a new client
				* name_open() call, which requires an
				* ack */
#ifdef __QNXNTO__ /* Protect reference to nto_pulse
		     (field undefined on other OSes) */
			        if ( pcr->nto_pulse.type == _IO_CONNECT )
				{
				        MsgReply_r(recv_ret, EOK, NULL, 0);
					continue;
				}
#endif
			        /* There is a potential hazard that the
				 * message may be broken into parts, but this
				 * is remote! -- would need a MsgRead_r() 
				 * call here.
				 * ANOTHER remote possibility -- message is
				 * bigger than expected... */
				/* while (msginfo.srcmsglen > msginfo.msglen)
				 * if (msginfo.msglen > recv_size) */
				sender.xport_info.chid = recv_ret;
				break;
			}
		}

		break; }
	case COMM_QNX_XPORT:
		_setmx( &recv_mx[0], &header, sizeof( comm_header_typ ) );
		_setmx( &recv_mx[1], precv, recv_size );
		sender.xport = COMM_QNX_XPORT;

		if( (sender.xport_info.pid = Receivemx( pclt->xport_info.pid,
			QNX_MAX_MX_PARTS, recv_mx )) == ERROR)
		{
			return( NULL );
		}
		break;
	case COMM_PSX_XPORT: {
		int recv_qid = pclt->xport_info.qid;
#ifdef DO_TRACE
		printf("receiving on msgq %d, recv_size %d\n", recv_qid,
			recv_size - sizeof(long));
		fflush(stdout);
#endif
		{
			int repeat = 0;
			while (( msgrcv( recv_qid, precv,
				 recv_size - sizeof(long), 0, 0 )  == -1 ) &&
				 repeat++ < DB_MSGQ_RETRY_LIMIT);
			if (repeat >= DB_MSGQ_RETRY_LIMIT)
			{	
				perror( "comm_sync_recv");
				return( NULL );
			}		
		}
		sender.xport = COMM_PSX_XPORT;
		sender.xport_info.qid = msgget(pcr->key, 0);
#ifdef DO_TRACE
		printf("received from msgq %d\n", sender.xport_info.qid);
		fflush(stdout);
#endif
		break; }
	default:
		return( NULL );
	}
	return( &sender );
}

/** 
 * 	The pclt parameter is determined by a previous send call.
 */
bool_typ comm_sync_reply( comm_clt_typ *pclt, void *preply, 
				unsigned reply_size )
{
	struct _mxfer_entry reply_mx[QNX_MAX_MX_PARTS];
	comm_header_typ reply_header;

	if( pclt == NULL )
		return( FALSE );

	switch( pclt->xport )
	{
	case COMM_QNX6_XPORT:

	        if( 0 > MsgReply_r( pclt->xport_info.chid, EOK, preply, 
				    reply_size ))
			return( FALSE );

		break;
	case COMM_QNX_XPORT:
		_setmx(&reply_mx[0], &reply_header, sizeof( comm_header_typ ) );
		_setmx( &reply_mx[1], preply, reply_size );
		if( Replymx( pclt->xport_info.pid, QNX_MAX_MX_PARTS, reply_mx )
				 != 0 )
			return( FALSE );

		break;
	case COMM_PSX_XPORT:
#ifdef DO_TRACE
		printf("Replying to msgq %d, size %d\n",
				pclt->xport_info.qid,
				reply_size - sizeof(long));
		fflush(stdout);
#endif
		
		{
			int repeat = 0;
			/* blocks if no space, could change to IPC_NOWAIT flag */
			while ((msgsnd( pclt->xport_info.qid, preply, 
				reply_size - sizeof(long), 0 ) == -1 ) && 
				repeat++ < DB_MSGQ_RETRY_LIMIT);
			if (repeat >= DB_MSGQ_RETRY_LIMIT)
			{
				perror( "comm_sync_reply" );
				return( FALSE );
			}
		}
		break;

	default:
		return( FALSE );
	}
	return( TRUE );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *
 *	#include "db_comm.h"
 *
 *	bool_typ comm_async_trig( comm_clt_typ *pclt, void *pxmit,
 *		unsigned xmit_size );
 *
 *	pclt		-	A valid communications channel to a peer.
 *	pxmit		-	Pointer to data for transmission
 *	xmit_size	-	The buffer size.
 *
 *	DESCRIPTION
 *	This is a nonblocking transmission to a peer.  For QNX 4, this
 *	is a Trigger() call, not a true data transmission.  Thus the
 *	data buffer is ignored, and the static data associated with
 *	the proxy is received by the peer.  The transmit buffer,
 *	was included in the call interface to support the concept of
 *	asynchronous communications with TCP/IP. For Posix message queues,
 *	the pxmit packet data buffer needs to include the same msg_type
 *	and key fields as with the comm_sync functions, with the key
 *	field set to 0 since no reply is expected. For QNX 6, this is a
 *      saved pulse (nonblocking) message encapsulated in the pxmit 
 *      structure -- it was saved when the trigger was set.
 *
 *	RETURN
 *	FALSE	-	If the channel is invalid, or the transport fails.
 *	TRUE	-	If the call succeeds.
 *
 */

bool_typ comm_async_trig( comm_clt_typ *pclt, void *pxmit, unsigned xmit_size )
{
	bool_typ status;

	status = FALSE;

	if( pclt == NULL )
		return( status );

	switch( pclt->xport )
	{
	case COMM_QNX6_XPORT:
	{       struct sigevent *event = (struct sigevent *) pxmit;
		int del_ret = MsgDeliverEvent_r( pclt->xport_info.chid, event );
		if( EOK == del_ret )
			status = TRUE;
		break;
	}
	case COMM_QNX_XPORT:
		if( Trigger( pclt->xport_info.pid ) != ERROR )
			status = TRUE;
		break;
	case COMM_PSX_XPORT:
#ifdef DO_TRACE
		printf("Trigger sent to %d, size %d\n",
			pclt->xport_info.qid, xmit_size - sizeof(long));
		fflush(stdout);
#endif
		{
			int repeat = 0;
			while (( msgsnd( pclt->xport_info.qid, pxmit,
			 xmit_size - sizeof(long), IPC_NOWAIT ) == -1 ) &&
				repeat++ < DB_MSGQ_RETRY_LIMIT); 
			if (repeat >= DB_MSGQ_RETRY_LIMIT) 
				perror( "comm_async_trig " );
			else
				status = TRUE;
		}	
		break;

	default:
		break;
	}
	return( status );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *
 *	#include "db_comm.h"
 *
 *	void comm_print_clt( comm_clt_typ *pclt );
 *
 *	pclt		-	A valid communications channel.
 *
 *	DESCRIPTION
 *	Prints information about the given communications channnel
 *	for debugging.
 *
 *	RETURN
 *	none.
 *
 */

void comm_print_clt( comm_clt_typ *pclt )
{
	if( pclt == NULL )
		printf( "NULL communications pointer.\n" );
	else
	{
		switch( pclt->xport )
		{
		case COMM_QNX6_XPORT:
			printf( "QNX 6 xport chid %d.\n", pclt->xport_info.chid );
			break;

		case COMM_QNX_XPORT:
			printf( "QNX xport pid %d.\n", pclt->xport_info.pid );
			break;

		case COMM_TCP_XPORT:
			printf( "TCP xport fd %d.\n", pclt->xport_info.fd );
			break;

		case COMM_PSX_XPORT:
			printf( "PSX xport qid %d.\n", pclt->xport_info.qid );
			break;

		default:
			printf( "Unknown communications transport %d\n",
					pclt->xport );
			break;
		}
	}
}

void comm_print_header( comm_header_typ header )
{
	printf( "Comm header: %u", header );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *
 *	#include "db_comm.h"
 *
 *	int comm_clt_cmp( comm_clt_typ *p1, comm_clt_typ *p2 ):
 *
 *	p1, p2	-	Valid communications channels.
 *
 *	DESCRIPTION
 *	Compares two communications channels, and decides if
 *	they point to the same QNX peer process or file descriptor.
 *
 *	RETURN
 *	zero		-	If they are the same.
 *	non-zero	-	If they are different.
 *
 */

int comm_clt_cmp( comm_clt_typ *p1, comm_clt_typ *p2 )
{
	if( (p1 == NULL) || (p2 == NULL) )
		return( -1 );

	if( p1->xport != p2->xport )
		return( -1 );

	if( p1 == p2 )
		return( 0 );

	switch( p1->xport )
	{
	case COMM_QNX6_XPORT:
		if( p1->xport_info.chid == p2->xport_info.chid )
			return( 0 );
		else
			return( 1 );

	case COMM_QNX_XPORT:
		if( p1->xport_info.pid == p2->xport_info.pid )
			return( 0 );
		else
			return( 1 );

	case COMM_TCP_XPORT:
		if( p1->xport_info.fd == p2->xport_info.fd )
			return( 0 );
		else
			return( 1 );

	case COMM_PSX_XPORT:
		if( p1->xport_info.qid == p2->xport_info.qid )
			return( 0 );
		else
			return( 1 );

	default:
		return( 1 );
	}
}

void comm_cleanup(comm_clt_typ *pclt, char *pserv, int pid)
{
	char filename[MAX_LINE_LEN];

	switch( pclt->xport )
	{
	case COMM_PSX_XPORT:
		msgctl(pclt->xport_info.qid, IPC_RMID, NULL);
		sprintf(filename, "%s_%d", pserv, pid);
		fprintf(stderr, "unlinking %s\n", filename);
		unlink(filename);
		break;

	case COMM_QNX6_XPORT:
	case COMM_QNX_XPORT:
	case COMM_TCP_XPORT:
	default:
		break;
	}
}
