/**\file
 *	db_clt.c
 *
 *	This is a collection of routines to support client side
 *	programming for the system database.  This includes calls for
 *	logging in and out of the server, as well as creating, reading,
 *	updating, and destroying variables in the database.  In addition,
 *	a client may register to recieve a data trigger for notification
 *	that a variable of interest has changed.
 */

#include "db_include.h"

#ifdef __QNXNTO__
#include <sys/dispatch.h>
#endif

#ifdef __QNX__
/* client side special trigger creation not needed on Linux
 * triggers are delivered to same queue as used for read */
static bool_typ clt_trig_create( db_clt_typ *pclt, db_data_typ *buff);
static long clt_trig_destroy( db_clt_typ *pclt, unsigned var, unsigned type );
static clt_trig_typ *clt_trig_find( dl_head_typ *phead, unsigned var,
		unsigned type );
static dl_node_typ *clt_trig_node( dl_head_typ *phead, unsigned var,
		unsigned type );
static int clt_trig_cmp( clt_trig_typ *p1, clt_trig_typ *p2 );
#else
#define clt_channel() 0
#define clt_channel_done() 0
#endif

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include <db_comm.h>
 *	#include <db_clt.h>
 *
 *	bool_typ clt_update( db_clt_typ *pclt, unsigned variable, unsigned type,
 *			unsigned size, void *pdata );
 *
 *	pclt		-	A handle to a connected client.
 *	variable	-	The variable to be changed.
 *	type		-	The type of the variable to be changed.
 *	size		-	Memory size to which pdata points.
 *	pdata		-	This points to the data to be sent.
 *
 *	DESCRIPTION
 *
 *	Attempts to write a data update to the server.
 *
 *	RETURN
 *
 *	TRUE		If data is successfully written.
 *	FALSE		If the communications channel has failed or filled.
 *				If this is the case, the channel will be left
 *				in an indeterminate state, and should be closed.
 *				The call may also fail if the variable is unknown to 
 *				the server, or is too large.
 */

bool_typ clt_update( db_clt_typ *pclt, unsigned var, unsigned type,
			unsigned size, void *pvalue )
{
	db_data_typ reply_buff;
	db_data_typ xmit_buff;

	if( (pclt == NULL) || (MAX_DATA_SIZE < size) )
		return( FALSE );

	comm_clt_cpy( pclt->pread, (comm_clt_typ *) &reply_buff); 

	xmit_buff.cmd = DB_UPDATE_CMD;
	xmit_buff.key = pclt->key;
	xmit_buff.var = var;
	xmit_buff.type = type;

	xmit_buff.time = get_sec_clock();
	bytecopy( size, pvalue, (char *) xmit_buff.value.user );

	if( comm_sync_send( pclt->pcomm, &xmit_buff, &reply_buff, 
		DB_COMM_SIZE, DB_COMM_SIZE ) == TRUE )
	{
		return( reply_buff.value.db_bool );
	}
	else
		return( FALSE );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include <db_comm.h>
 *	#include <db_clt.h>
 *
 *	bool_typ clt_read( db_clt_typ *pclt, unsigned var, unsigned type, 
 *					db_data_typ *presult );
 *
 *	pclt		-	A handle to a connected client.
 *	var			-	The variable to be retrieved.
 *	type		-	The type of the variable to be retrieved.
 *	presult		-	The result will be placed in this structure.
 *					This is not a native C type.
 *	
 *	DESCRIPTION
 *
 *	Retrieves the given variable from the server, and places the
 *	result into the given structure.
 *
 *	RETURN
 *
 *	TRUE		If the data is successfully retrieved.
 *	FALSE		If the call fails, or 
 *				the variable is not known to the server.
 */

bool_typ clt_read( db_clt_typ *pclt, unsigned var, unsigned type, 
	db_data_typ *pdata )
{
	db_data_typ xmit_buff;
	comm_clt_cpy( pclt->pread, (comm_clt_typ *) pdata); 

	xmit_buff.cmd = DB_READ_CMD;
	xmit_buff.key = pclt->key;
	xmit_buff.type = type;
	xmit_buff.var = var;

	if( comm_sync_send( pclt->pcomm, &xmit_buff, pdata, DB_COMM_SIZE,
		DB_COMM_SIZE ) != TRUE )
	{
		return( FALSE );
	}

	if( pdata->var == DB_BAD_VAR )
		return( FALSE );
	else
		return( TRUE );
}

/**
 *	On QNX4 no reply queue is necessary, because send creates
 *	a reply buffer/handle. For the Posix message queue
 *	implementation on Linux we will use the pid to create 
 *	create a uniquely named file and use ftok to get the id.
 *	(If the ftok function is not good, we may be better off
 *	just using pids instead.)
 */
long int clt_get_reply_queue_key(int xport, char *pserv, int pid)
{
	FILE *fp;
	timestamp_t ts;
	char filename[MAX_LINE_LEN];

	// file operation only really needed for COMM_PSX_XPORT
	// may have some debugging value on QNX4

	sprintf( filename, "%s_%d", pserv, pid);
	if ( ( fp = fopen( filename, "w" )) == NULL )
	{
		fprintf( stderr, "Can't open file %s\n", filename);
		return FALSE;
	}
	get_current_timestamp(&ts);
	print_timestamp(fp, &ts);
	fprintf(fp, "\n");
	fclose(fp);

	return ( comm_get_handle ( xport, pserv, pid ) );
}
		 

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include <db_comm.h>
 *	#include <db_clt.h>
 *
 *	db_clt_typ *clt_login( char *phost, char *pserv, char *pname,
 *						int xport );
 *
 *	phost		-	Hostname of server.
 *					NULL if host is local.
 *
 *	pserv		-	Database (service) name.
 *
 *	pname		-	Name of current process, or login name.
 *				The login name, uid, and gid are not required
 *				to be unique for a given server and database,
 *				but are available for debugging.
 *
 *	xport		-	The desired communications transport, as defined
 *					in db_comm.h.
 *
 *	DESCRIPTION
 *
 *	Creates a communications channel of the specified type and logs
 *	into the server.  The given hostname and specified
 *	database name are used.  This also initializes the variable
 *	trigger list. On Linux, it also creates a reply channel.
 *
 *	RETURN
 *
 *	non-NULL	A handle to a client.
 *	NULL		If the client cannot be created or
 *				the server or database is not available.
 */

db_clt_typ *clt_login( char *pname, char *phost, char *pserv, int xport )
{
	db_clt_typ *pclt;
	comm_clt_typ *pcomm;
	db_data_typ login;
	db_data_typ reply;
	dl_head_typ *ptrig_head;
//	static int badinits = 0;

	pclt = NULL;
	pcomm = NULL;
	ptrig_head = NULL;

	if ((pclt = MALLOC( sizeof( db_clt_typ ) )) == NULL)
	{
//		printf("MALLOC failed\n");
		return NULL;
	}
	if ((pcomm = comm_name_init( xport, phost, pserv)) == NULL)
	{
//		printf("comm_name_init failed %d times\n", badinits);
		FREE( pclt );
		return NULL;
	}
		
	if  ((ptrig_head = dl_create()) == NULL) 
	{
//		printf("Trigger list create failed\n");
		FREE( pclt );
		return( NULL );
	}

	pclt->pcomm = pcomm;
	pclt->ptrig_head = ptrig_head;
	pclt->pservice = pserv;

	if ( COMM_PSX_XPORT == xport) {
	    /*  needed for Posix message queue implementation */
	    login.key = clt_get_reply_queue_key(xport, pserv, getpid());
	    pclt->key = login.key;
    } else {
        login.key = 0;
        pclt->key = 0;
    }
	pclt->pread = comm_init( xport, login.key );
	comm_clt_cpy( pclt->pread, (comm_clt_typ *) &reply ); 

	login.value.login_data.uid = GETUID();
	login.value.login_data.gid = GETGID();
	login.value.login_data.key = login.key; 


	strncpy( login.value.login_data.name, pname, USR_NAME_SIZE );
	login.value.login_data.name[USR_NAME_SIZE] = END_OF_STRING;

	login.cmd = DB_LOGIN_CMD;
	login.type = DB_LOGIN_TYPE;
	login.var = DB_LOGIN_VAR;

	login.value.login_data.login_time = get_sec_clock();

	/*	Assume that these are evaluated completely, and in order.
	 */

	if( (comm_sync_send( pcomm,  &login, &reply,
			DB_COMM_SIZE, DB_COMM_SIZE ) == FALSE )
		|| (reply.value.db_bool != TRUE) )
	{
		comm_done( pcomm );
		FREE( pclt );
		dl_free( ptrig_head );
		return( NULL );
	}
	else
		return( pclt );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include <db_comm.h>
 *	#include <db_clt.h>
 *
 *	bool_typ clt_logout( db_clt_typ *pclt );
 *
 *	pclt	-	Handle of a currently open client.
 *	pserv   -	Name of service, needed for clean-up on Linux
 *
 *	DESCRIPTION
 *
 *	This call will unset all requested data triggers,
 *	log out from the database server, close the communications
 *	channel, and release other system resources.
 *	used by the server and client.
 *
 *	RETURN
 *
 *	TRUE		If the connection is successfully closed.
 *	FALSE		If the client handle is invalid, or a
 *				system call fails.  If a system call
 *				fails, the client handle may be left
 *				in an indeterminant state, but should not
 *				be used.
 */

bool_typ clt_logout( db_clt_typ *pclt )
{
	db_data_typ logout;
	db_data_typ reply;
	dl_node_typ *pnode;
	clt_trig_typ *ptrig;
	char *pserv = pclt->pservice;


	if( pclt == NULL )
		return( FALSE );

	comm_clt_cpy( pclt->pread, (comm_clt_typ *) &reply ); 

	/*	This is not a simple loop, because lower level routines
	 *	use the trigger list.  If the clt_trig_unset() fails, just
	 *	abort because things are in an indeterminate state between
	 *	the client and server structures.  Note that the server
	 *	will attempt to clean up stale data triggers.
	 */

	while( pclt->ptrig_head->length != 0 )
	{
		pnode = pclt->ptrig_head->pfirst;
		ptrig = (clt_trig_typ *) pnode->pitem;
		if( clt_trig_unset( pclt, ptrig->trig_info.var,
			ptrig->trig_info.type ) == FALSE )
		{
			break;
		}
	}

	dl_free( pclt->ptrig_head );

	logout.cmd = DB_LOGOUT_CMD;
	logout.key = pclt->key;
	logout.value.logout_data.uid = GETUID();
	logout.type = DB_LOGOUT_TYPE;
	logout.var = DB_LOGOUT_VAR;

	comm_sync_send( pclt->pcomm, &logout, &reply,
	DB_COMM_SIZE, DB_COMM_SIZE );
	
	clt_channel_done();

	comm_done( pclt->pcomm );

	// terminate and clean-up after reply queue
	comm_cleanup( pclt->pread, pserv, getpid());

	comm_done( pclt->pread );
		
	FREE( pclt );
	return( reply.value.db_bool );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include <db_comm.h>
 *	#include <db_clt.h>
 *
 *
 *	bool_typ clt_create( db_clt_typ *pclt, unsigned var,
 *			unsigned type, unsigned size );
 *	
 *	pclt		-	A handle to a connected client.
 *	var			-	The variable to be created.
 *	type		-	The type of the variable to be created.
 *	size		-	Size of the variable.
 *
 *
 *	DESCRIPTION
 *	Create a variable for public access in the database,
 *	with the given inital value and type.  The variable
 *	value will be filled with binary zeroes.
 *	
 *	RETURN
 *	TRUE	-	If the creation succeeds.
 *	FALSE	-	If it fails.  This might be due
 *				to a communications failure, a
 *				clash with a previously created variable,
 *				a memory failure or a bad data size.
 *
 */

bool_typ clt_create( db_clt_typ *pclt, unsigned var,
		unsigned type, unsigned size )
{
	db_data_typ reply_buff;
	db_data_typ xmit_buff;

	if( var <= MAX_DB_BUILTIN_VAR )
	{
		pclt->error = DB_ERR_VAR_NUM;
		return( FALSE );
	}

	if( MAX_DATA_SIZE < size )
	{
		pclt->error = DB_ERR_VAR_SIZE;
		return( FALSE );
	}

	comm_clt_cpy( pclt->pread, (comm_clt_typ *) &reply_buff ); 

	xmit_buff.cmd = DB_CREATE_CMD;
	xmit_buff.key = pclt->key;
	xmit_buff.var = var;
	xmit_buff.type = type;
	xmit_buff.value.db_long = size;

	if( comm_sync_send( pclt->pcomm, &xmit_buff, &reply_buff, 
		DB_COMM_SIZE, DB_COMM_SIZE) == TRUE )
	{
		return( reply_buff.value.db_bool );
	}
	else
		return( FALSE );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include <db_comm.h>
 *	#include <db_clt.h>
 *
 *
 *	bool_typ clt_destroy( db_clt_typ *pclt, unsigned var, unsigned type );
 *	
 *	pclt		-	A handle to a connected client.
 *	var			-	The variable to be destroyed.
 *
 *	DESCRIPTION
 *	Destroy the given variable in the database.
 *	
 *	RETURN
 *	TRUE	-	If the call succeeds.
 *	FALSE	-	If it fails.  This might occur 
 *				if there is a communications failure or if
 *				the variable is unknown to the server.
 *
 */

bool_typ clt_destroy( db_clt_typ *pclt, unsigned var, unsigned type )
{
	db_data_typ reply_buff;
	db_data_typ xmit_buff;

	if( pclt == NULL )
		return( FALSE );

	comm_clt_cpy( pclt->pread, (comm_clt_typ *) &reply_buff ); 

	xmit_buff.cmd = DB_DESTROY_CMD;
	xmit_buff.key = pclt->key;
	xmit_buff.var = var;
	xmit_buff.type = type;

	if( comm_sync_send( pclt->pcomm, &xmit_buff, &reply_buff, 
		DB_COMM_SIZE, DB_COMM_SIZE ) == TRUE )
	{
		return( reply_buff.value.db_bool );
	}
	else
		return( FALSE );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include <db_comm.h>
 *	#include <db_clt.h>
 *
 *	bool_typ clt_trig_set( db_clt_typ *pclt, unsigned var, unsigned type );
 *
 *	pclt		-	A handle to a connected client.
 *	var		-	The name of the variable to be monitored.
 *	type		-	The type of the variable to be monitored.
 *
 *	DESCRIPTION
 *	Request trigger service for change on a given variable from the server.
 *	A trigger from the server is a message which gives the variable
 *	name and type.  The message can be tested with the DB_TRIG_VAR() macro.
 *
 *	RETURN
 *	TRUE			-	If the callback registration succeeds.
 *	FALSE			-	If the communications to the server fails,
 *						or the trigger creating fails.
 *
 */

bool_typ clt_trig_set( db_clt_typ *pclt, unsigned var, unsigned type )
{
	db_data_typ reply_buff;
	db_data_typ xmit_buff;

	xmit_buff.cmd = DB_TRIG_SET_CMD;
	xmit_buff.key = pclt->key;
	xmit_buff.var = var;
	xmit_buff.type = type;
	
#ifdef __QNX__ /* Protect the calls to trig_create/destroy, which are
		* only defined on QNX systems */
	if( FALSE == clt_trig_create( pclt, &xmit_buff) )
	{
	        return( FALSE );
	}
#endif
	comm_clt_cpy( pclt->pread, (comm_clt_typ *) &reply_buff ); 

	if( FALSE == comm_sync_send( pclt->pcomm, &xmit_buff, &reply_buff, 
				     DB_COMM_SIZE, DB_COMM_SIZE ) )
	{
#ifdef __QNX__
	        clt_trig_destroy( pclt, var, type );
#endif
		return( FALSE );
	}

	return( reply_buff.value.db_bool );
}

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include <db_comm.h>
 *	#include <db_clt.h>
 *
 *
 *	bool_typ clt_trig_set( db_clt_typ *pclt, unsigned var, unsigned type );
 *
 *	pclt		-	A handle to a connected client.
 *	var			-	The name of the variable.
 *	type		-	The type of the variable.
 *
 *	DESCRIPTION
 *	Release trigger service for a given variable.
 *
 *	RETURN
 *	TRUE			-	If the call succeeds.
 *	FALSE			-	If the trigger was not registered,
 *						or the communications fails.
 *
 */

bool_typ clt_trig_unset( db_clt_typ *pclt, unsigned var, unsigned type )
{
	db_data_typ reply_buff;
	db_data_typ xmit_buff;

	xmit_buff.cmd = DB_TRIG_UNSET_CMD;
	xmit_buff.key = pclt->key;
	xmit_buff.var = var;
	xmit_buff.type = type;
	reply_buff.value.db_bool = FALSE;

	switch ( pclt->pcomm->xport )
	{
	case COMM_QNX_XPORT:
#ifdef __QNX__ /* Protect the calls to trig_create/destroy, which are
		* only defined on QNX systems */
	        if( (xmit_buff.value.db_long
			= clt_trig_destroy( pclt, var, type )) == ERROR )
		{
			return( FALSE );
		}
#endif
	        break;
	case COMM_QNX6_XPORT:
#ifdef __QNX__
	        clt_trig_destroy( pclt, var, type );
#endif
	        break;
	default:
	        break;
	}
	comm_clt_cpy( pclt->pread, (comm_clt_typ *) &reply_buff ); 

	comm_sync_send( pclt->pcomm, &xmit_buff, &reply_buff, 
			DB_COMM_SIZE, DB_COMM_SIZE );

	return( reply_buff.value.db_bool );
}

/**
 *
 *	clt_ipc_receive
 *	
 *		Wrapper for Receive on QNX4, for msgrcv on Linux.
 *		Return values allow disambiguation of triggers from
 *		the database or DAS (data acquisition system) or
 *		expiration of timer.
 */
int clt_ipc_receive( db_clt_typ *pclt, void *ptrig, int trigsize )
{
	int retval;
	trig_info_typ *pt = (trig_info_typ *) ptrig;
	switch ( pclt->pcomm->xport )
	{
	case COMM_QNX6_XPORT:
	{       /* Need to manage both timer and trigger pulses.  

		   For triggers, don't get back a trig_info_typ --
		   just the var, but this is OK
		 */
	        struct _pulse ret_pulse;
		clt_chan_typ *clt_chan = clt_channel( );
		int chid = 0;
		int recv_ret = 0;
		if ( NULL != clt_chan ) {
		    chid = clt_chan->chid;
		} else {
		    return DB_BAD_CMD;
		}

		recv_ret = MsgReceivePulse_r( chid, &ret_pulse, 
						  sizeof(ret_pulse), NULL );

		if( 0 > recv_ret ) { /* ERROR */
			if (EINTR == -recv_ret) {
				return( DB_TIMER );
			} else if (ETIMEDOUT == -recv_ret) {
				return( DB_TIMER );
			} else {
		        return( DB_BAD_CMD );
			}
		} 
#ifdef __QNXNTO__ /* Protect structure definition of _pulse fields */
		else if ( 0 == recv_ret ) { /* PULSE */
			if ( TRIGGER_PULSE_CODE == ret_pulse.code ) {
			      pt->var = (unsigned) ret_pulse.value.sival_int;
			      pt->type = (unsigned) ret_pulse.value.sival_int;
			      return( DB_TRIGGER );
			}
			else if ( TIMER_PULSE_CODE == ret_pulse.code ) 
			{
			        return( DB_TIMER );
			}
			else
			{
			        return( DB_BAD_CMD );
			}
		}
#endif
		return ( recv_ret );
	}
	case COMM_QNX_XPORT:
		return ( Receive( 0, ptrig, trigsize) );
	case COMM_PSX_XPORT:
		memset( ptrig, 0, trigsize);
		retval = msgrcv( pclt->pread->xport_info.qid,
				ptrig, trigsize, 0, 0 );
		if ( retval == -1 )
		{
			if ( errno == EINTR )
				return ( DB_TIMER );
			else
				return ( DB_BAD_CMD );;
		} else
			return ( pt->msg_type );
	case COMM_TCP_XPORT:	// not implemented
	default:
		return DB_BAD_CMD; 
	}
}

#ifdef __QNX__

static dl_node_typ *clt_trig_node( dl_head_typ *phead, unsigned var,
				   unsigned type )
{
	clt_trig_typ find_key;

	find_key.trig_info.var = var;
	find_key.trig_info.type = type;

	return( dl_first( phead, &find_key, clt_trig_cmp ) );
}

static clt_trig_typ *clt_trig_find( dl_head_typ *phead, unsigned var,
				    unsigned type )
{
	dl_node_typ *pnode;

	if( (pnode = clt_trig_node( phead, var, type )) == NULL )
		return( FALSE );
	else
		return( (clt_trig_typ *) pnode->pitem );
}

/*	SYNOPSIS
 *
 *	static int clt_trig_cmp( clt_trig_typ *p1, clt_trig_typ *p2 );
 *	clt_trig_typ *p1, *p2 -	Two trigger entries to be compared.
 *
 *	DESCRIPTION
 *	Compares two trigger entries to see if they
 *	refer to the same variable and type.
 *
 *	RETURN
 *		0			-	If the variables are the same.
 *		non-zero	-	Otherwise.
 *
 */

static int clt_trig_cmp( clt_trig_typ *p1, clt_trig_typ *p2 )
{
	if( (p1->trig_info.var == p2->trig_info.var) &&
	    (p1->trig_info.type == p2->trig_info.type) )
	{
		return( 0 );
	}
	else if( p2->trig_info.var < p1->trig_info.var )
		return( 1 );
	else
		return( -1 );
}

/*	These have QNX specific code in the proxy creation/destruction.
 */

/* NOTE: This channel can be used by both the trigger and timer code, though
 *           timers on QNX 6 are currently handled with signals.
 */

clt_chan_typ *client_channel = NULL;

clt_chan_typ *clt_channel( )
{
    if ( NULL == client_channel ) {
    
        int chid, coid;
        chid = ChannelCreate_r( 0 );

        coid = ConnectAttach_r( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );
        if ( coid < 0 ) {
            fprintf( stderr, "Can't set up client pulse channel\n");
			return NULL;
		}
		client_channel = CALLOC( 1, sizeof(clt_chan_typ) );
		client_channel->chid = chid;
		client_channel->coid = coid;
	}
	return client_channel;
}

void clt_channel_done( )
{
    if ( NULL != client_channel ) {
	        ConnectDetach_r( client_channel->coid );
	        ChannelDestroy_r( client_channel->chid );
	        FREE( client_channel );
	        client_channel = NULL;
	}
}

static bool_typ clt_trig_create( db_clt_typ *pclt, db_data_typ *xmit_buf )
{
	trig_info_typ trig_pack;
	clt_trig_typ entry;
	long handle;

	if( clt_trig_find( pclt->ptrig_head, xmit_buf->var, xmit_buf->type ) != NULL )
		return( FALSE );

	trig_pack.type = xmit_buf->type;
	trig_pack.var = xmit_buf->var;

	entry.trig_info.type = xmit_buf->type;
	entry.trig_info.var = xmit_buf->var;

	switch ( pclt->pcomm->xport )
	{
	case COMM_QNX6_XPORT:
		{
	        int coid = 0;
	        clt_chan_typ *clt_chan = clt_channel( );
		struct sigevent *trig_event = &(xmit_buf->value.trig_event);
            if ( NULL != clt_chan ) {
                coid = clt_chan->coid;
            } else {
                return FALSE;
            }

		/* Note that saving/sending just the var field (and
		   not the type field) is OK -- no clients require
		   both, and most override the type field with a copy
		   of the var field.
		 */
		SIGEV_PULSE_INIT( trig_event, coid, SIGEV_PULSE_PRIO_INHERIT, 
				  TRIGGER_PULSE_CODE, xmit_buf-> var );


		/* The pcomm field is unnecessary for QNX6, and
		   there's no point in recording the sigevent --
		   trigger is determined by the var and type (or,
		   rather, just the var)
		 */
		entry.pcomm = NULL;

		if( TRUE != dl_add_dup( pclt->ptrig_head, &entry,
					sizeof( clt_trig_typ ), FALSE ) )
		{
			return( FALSE );
		}

		break;
		}
	case COMM_QNX_XPORT:

  	        if( ((handle = qnx_proxy_attach( 0, &trig_pack,
			        sizeof( trig_pack ), -1 )) == ERROR )
			|| ((entry.pcomm = comm_init( pclt->pcomm->xport,
						      handle )) == NULL) )
		{
			return( FALSE );
		}

		if( TRUE != dl_add_dup( pclt->ptrig_head, &entry,
					sizeof( clt_trig_typ ), FALSE ) )
		{
			comm_done( entry.pcomm );
			return( FALSE );
		}
		xmit_buf->value.db_long = handle;
	      break;
	default:
		break;
	}

	return( TRUE );
}

static long clt_trig_destroy( db_clt_typ *pclt, unsigned var, unsigned type )
{
	dl_node_typ *pnode;
	clt_trig_typ *pentry;
	long handle = 0L;

	if( (pnode = clt_trig_node( pclt->ptrig_head, var, type )) == NULL )
		return( ERROR );
	else
	{
		pentry = dl_rm_node( pclt->ptrig_head, &pnode );
		if (COMM_QNX_XPORT == pclt->pcomm->xport )
		{
		        handle = pentry->pcomm->xport_info.pid;
			qnx_proxy_detach( handle );
			comm_done( pentry->pcomm );
		}
		FREE( pentry );
	}
	return( handle );
}
	
#endif /* __QNX__ */
