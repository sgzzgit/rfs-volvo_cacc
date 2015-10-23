/**\file
 *	db_slv.c
 *
 *	USAGE AND OPTIONS
 *
 *	db_slv -[QqPSv?]
 *
 *      -q                      Sets up QNX 4 style communication
 *
 *      -Q                      Sets up QNX 6 style communication
 *
 *	-P number		Sets the priority of the task.  Increasing
 *				priority will ensure that clients will
 *				receive prompt attention by the server.
 *
 *	-v			Verbose flag.  Prints client activity
 *				to screen.  Useful for debugging, not
 *				for real-time control.
 *
 *	-S name			Specify an alternative database name.
 *				Allows possibility of multiple servers
 *				and databases on one host.
 *
 *	-\?			Prints the program usage, options, and
 *				option defaults to the screen.
 *
 *	DESCRIPTION
 *	This server program and the associated client library provide
 *	a standardized interface for communications and sharing
 *	system-wide variables.  It is designed for expansion to
 *	multiple CPUs and network use.
 *
 *	Variables in the database are referenced by numerical names,
 *	types, and formats which are defined by the application.  For
 *	efficiency, individual variables are generally C structures
 *	which consolidate information useful to several application
 *	tasks.  To support real-time access, variables are kept in
 *	memory, and have no persistance after execution is complete.
 *
 *	Basic client requests are served in a first-in, first-out
 *	order.  The database supports the concept of a "trigger".
 *	This allows a client to block until a variable of interest
 *	changes.
 *
 *	There are two major data structures in this slave server.
 *	The first is a list of current clients.
 *	The second is a list of variables.
 *	Each variable in the list contains a list of trigger clients.
 *
 *	Interprocess communications use native QNX network
 *	communications on QNX4, and POSIX message queues on Linux.
 *
 *	db_slv initializes by:
 *	1)	Processing command line options.
 *	2)	Create the database structure, and register it with the 
 *		operating system.
 *	3)	Create a structure to maintain current users.
 *	4)	Setting up signal handling.
 *
 *	The program then iteratively serves client requests to:
 *	1)	Log in and log out.
 *	2)	Create new variables.
 *	3)	Read the current value of a variable from the database.
 *	4)	Update an existing variable in the database.
 *	5)	Destroy a variable in the database.
 *	6)	Register a trigger on a variable.
 *	7)	Unregister a variable trigger.
 *		
 *	The program will run until signaled to quit via the keyboard,
 *	a parent program, the kill or slay command, or there is a 
 *	major failure in processing client requests.
 *
 *	SYSTEM VARIABLES
 *	All variables in the database are defined by the clients.
 *
 *	EXIT CONDITIONS
 *	EXIT_FAILURE
 *	If any one of the following events occurs during initialization:
 *	-	An invalid command line option or usage is requested.
 *	-	Failure to create the database or client list structures.
 *		If a significant failure occurs in processing a client request.
 *
 *	EXIT_SUCCESS
 *		If the program completes successfully, or is terminated from 
 *		the keyboard by Control-C, or the kill or slay commands.
 *
 *	SYSTEM SERVICES
 *	The SIGQUIT, SIGINT, and SIGTERM signals are caught, and
 *	cause successful program termination.  All other signals are ignored.
 *	The database is published to clients via the QNX name service.
 *
 *	HISTORY 
 *      The server was extensively used for PATH QNX4 real-time
 *      control projects. For Linux, it was decided to port this code
 *      to use Posix message queues to replace QNX4 messaging.  For
 *      QNX 6 (Neutrino), the port used native QNX messaging with the
 *      raw interfaces rather than Resource or IO managers.
 *
 */

#include "db_include.h"
#ifdef __QNXNTO__
#include <sys/dispatch.h>
#endif

static void sig_hand( int code );
static void usage( char *pargv0 );

static bool_typ slv_var_reply( comm_clt_typ *pclt, unsigned cmd, 
			db_entry_typ *pentry );
static bool_typ slv_bool_reply( comm_clt_typ *pclt, unsigned cmd, 
			bool_typ status );

static bool_typ slv_db_publish_name ( int xport, char *pservname, 
				      int *pname_id, void **name_struct );
static slv_db_typ *slv_db_create( void );
static bool_typ slv_db_destroy( slv_db_typ *pdb );
static void db_dump( slv_db_typ *phead );

static bool_typ db_entry_create( slv_db_typ *pdb, unsigned var, 
			unsigned type, unsigned size );
static bool_typ db_entry_rm( slv_db_typ *pdb, unsigned var, unsigned type );
static void db_entry_destroy( db_entry_typ *pentry );
static db_entry_typ *db_entry_bad( slv_db_typ *pdb );
static db_entry_typ *db_entry_find( slv_db_typ *pdb, unsigned var, 
			unsigned type );
int db_entry_cmp( db_entry_typ *p1, db_entry_typ *p2 );

static bool_typ slv_trig_set( slv_db_typ *pdb, comm_clt_typ *pclt, 
			db_data_typ *precv );
static bool_typ slv_trig_unset( slv_db_typ *pdb, comm_clt_typ *pclt, 
			db_data_typ *precv );
static bool_typ slv_update( slv_db_typ *pdb, db_data_typ *pdata, unsigned xport );

static bool_typ slv_cmd( slv_db_typ *pdb, dl_head_typ *pusr,
			comm_clt_typ *pclt, db_data_typ *precv );

static bool_typ slv_clt_add( dl_head_typ *pusr, comm_clt_typ *pclt, 
			db_login_typ *plogin );
static bool_typ slv_clt_rm( dl_head_typ *pusr, comm_clt_typ *pclt );
static void slv_clt_dump( dl_head_typ *pusr );
static int slv_clt_cmp( slv_clt_typ *p1, slv_clt_typ *p2 );
static void slv_done( slv_db_typ *pdb, dl_head_typ *pusr,
		      comm_clt_typ *psvc_clt, int name_id, 
		      char *pservname, void *name_struct );
static int trig_comm_cmp( trig_comm_typ *p1, trig_comm_typ *p2 );

static int sig_list[] =
{
	SIGTERM,
	SIGINT,
	SIGQUIT,
	ERROR
};

static bool_typ verbose_flag;
static jmp_buf exit_env;

int main( int argc, char *argv[] )
{
	dl_head_typ *pusr;	/*	A list of slv_clt_typ structures,
				 *	which contain client information,
				 *	and channels for communications.
				 */ 
	slv_db_typ *pdb;	/*	The variable list, which contains
				 *	data, the sizes, and a list trigger
				 *	clients for each variable.
				 */

	comm_clt_typ *psvc_clt;	/*	Client calls are received here.	*/
	comm_clt_typ *pdb_clt;	/*	This points to the current client.*/

	db_data_typ recv_buff;	/*	The buffer of information from the
				 *	client.
				 */

	char *pservname;	/*	The service name to be supported.*/

	int option;
	int priority;
	int name_id;
	void *name_struct;
	int xport = COMM_PSX_XPORT;	// transport mechanism
	int handle = COMM_WILD_HANDLE;	// for COMM_QNX_XPORT, receive all

	/*	Set to invalid values to facilitate error exits.
	 */
								
	pdb = NULL;
	psvc_clt = NULL;
	pusr = NULL;
	name_id = ERROR;
	name_struct = NULL;

	verbose_flag = FALSE;
	pservname = DEFAULT_SERVICE;

	while( (option = getopt( argc, argv, "S:P:qQv?" )) != EOF )
	{
		switch( option )
		{
		case 'v':
			verbose_flag = TRUE;
			break;

		case 'P':
			priority = atoi( optarg );
			if( setprio( 0, priority ) == ERROR )
			{
				fprintf(stderr, "Can't change priority to %d\n",
					 priority );
				exit( EXIT_FAILURE );
			}
			break;

		case 'S':
			pservname = optarg;
			break;

 		case 'q':
			xport = COMM_QNX_XPORT;
			break;

		case 'Q':
			xport = COMM_QNX6_XPORT;
			break;

		case '?':
		default:	
			usage( argv[0] );
			exit( EXIT_FAILURE );
			break;
		}
	}
	if (verbose_flag) {
		printf("Starting PATH DB data server\n");
		fflush(stdout);
	}

	if ( !slv_db_publish_name (xport, pservname, &name_id, &name_struct) )
	{ 
		fprintf( stderr, "Can't publish service %s\n", pservname);
		slv_done( pdb, pusr, psvc_clt, name_id, pservname, name_struct);
		exit( EXIT_FAILURE );
	}

	/* Beware that name_id is now the channel id in QNX 6, so
         * don't want to pass in COMM_WILD_HANDLE.  But code for QNX 4
         * assumes COMM_WILD_HANDLE, so must special case this */
        if ( COMM_QNX6_XPORT == xport ) {
	        handle = comm_get_handle( xport, pservname, name_id );
	} else {
	        handle = comm_get_handle( xport, pservname, COMM_WILD_HANDLE );
	}

	if( (pdb = slv_db_create()) == NULL )
	{
		fprintf( stderr, "%s: Can't create database\n", argv[0] );
		slv_done( pdb, pusr, psvc_clt, name_id, pservname, name_struct );
		exit( EXIT_FAILURE );
	} else if( (psvc_clt = comm_init( xport, handle ))  == NULL )
	{
		fprintf( stderr, "%s: Can't create server communications\n",
				 argv[0] );
		slv_done( pdb, pusr, psvc_clt, name_id, pservname, name_struct );
		exit( EXIT_FAILURE );
	} else if( (pusr = dl_create()) == NULL )
	{
		fprintf( stderr, "%s: Can't create usr list\n", argv[0] );
		slv_done( pdb, pusr, psvc_clt, name_id, pservname, name_struct );
		exit( EXIT_FAILURE );
	}
	else if( setjmp( exit_env ) != 0 )
	{
		slv_done( pdb, pusr, psvc_clt, name_id, pservname, name_struct );
		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );
	if (verbose_flag) 
	{
		printf("db slv: pdb 0x%x pusr 0x%x\n", 
				(unsigned int) pdb, (unsigned int) pusr);
	}

	while( (pdb_clt = comm_sync_recv( psvc_clt, &recv_buff, 
					  DB_COMM_SIZE )) != NULL )
	{
		if( verbose_flag == TRUE )
			db_print( &recv_buff );

		if( (slv_cmd( pdb, pusr, pdb_clt, &recv_buff ) == FALSE)
			&& (verbose_flag == TRUE) )
		{
			printf( "%s: processing failed.\n", argv[0] );
			db_print( &recv_buff );
		}
	}
	longjmp( exit_env, EXIT_FAILURE );
}


static void slv_done( slv_db_typ *pdb, dl_head_typ *pusr, 
		      comm_clt_typ *psvc_clt, int name_id, 
		      char *pservname, void *name_struct )
{
	int xport = psvc_clt->xport;

	if( pusr != NULL )
		dl_free( pusr );

	switch (xport)
	{
	case COMM_QNX6_XPORT:
		if ( NULL != name_struct ) {
		        name_detach( (name_attach_t *) name_struct, 0 );
		}
		break;
	case COMM_QNX_XPORT:
		if( name_id != ERROR )
			qnx_name_detach( 0, name_id );
		break;
	case COMM_PSX_XPORT:
		// terminate and clean-up after database queue
		comm_cleanup(psvc_clt, pservname, 0); 
		break;
	default:
		fprintf(stderr, " Transport %d not implemented\n", xport);
		break;	
	}
	if( psvc_clt != NULL )
		comm_done( psvc_clt );

	if( pdb != NULL )
		slv_db_destroy( pdb );

}

/**
 *	pclt for this routine is the comm for reply back to the client
 *	that this command came from.
 */
static bool_typ slv_cmd( slv_db_typ *pdb, dl_head_typ *pusr,
	comm_clt_typ *pclt, db_data_typ *precv )
{
	bool_typ status;
	db_entry_typ *pentry;

	switch( precv->cmd )
	{
	case DB_CREATE_CMD:
		status = db_entry_create( pdb, precv->var, precv->type, 
					(unsigned) precv->value.db_long );
		slv_bool_reply( pclt, DB_CREATE_REPLY, status );

		if( verbose_flag == TRUE )
			db_dump( pdb );
		break;

	case DB_READ_CMD:
		if( (pentry = db_entry_find( pdb, precv->var, precv->type ))
				 == NULL )
		{
			pentry = db_entry_bad( pdb );
			status = FALSE;
		}
		else
			status = TRUE;

		slv_var_reply( pclt, DB_READ_REPLY, pentry );
		break;

	case DB_UPDATE_CMD:
		status = slv_update( pdb, precv, pclt->xport );

		slv_bool_reply( pclt, DB_UPDATE_REPLY, status );
		break;

	case DB_DESTROY_CMD:
		status = db_entry_rm( pdb, precv->var, precv->type );
		slv_bool_reply( pclt, DB_DESTROY_REPLY, status );

		if( verbose_flag == TRUE )
			db_dump( pdb );
		break;

	case DB_LOGIN_CMD:
		status = slv_clt_add( pusr, pclt, &precv->value.login_data );

		slv_bool_reply( pclt, DB_LOGIN_REPLY, status );

		if( verbose_flag == TRUE )
		{
			slv_clt_dump( pusr );
			db_dump( pdb );
		}
		break;

	case DB_LOGOUT_CMD:
		status = slv_clt_rm( pusr, pclt );
		slv_bool_reply( pclt, DB_LOGOUT_REPLY, status );

		if( verbose_flag == TRUE )
			slv_clt_dump( pusr );
		break;

	case DB_TRIG_SET_CMD:
		status = slv_trig_set( pdb, pclt, precv );
		slv_bool_reply( pclt, DB_TRIG_SET_REPLY, status );
		if( verbose_flag == TRUE )
			db_dump( pdb );
		break;

	case DB_TRIG_UNSET_CMD:
		status = slv_trig_unset( pdb, pclt, precv );
		slv_bool_reply( pclt, DB_TRIG_UNSET_REPLY, status );
		if( verbose_flag == TRUE )
			db_dump( pdb );

		break;

	default:
		status = FALSE;
		break;
	}
	return( status );
}

static void sig_hand( int code )
{
	longjmp( exit_env, code );
}

static bool_typ slv_var_reply( comm_clt_typ *pclt, unsigned cmd, 
	db_entry_typ *pentry )
{
	pentry->data.cmd = cmd;

	return( comm_sync_reply( pclt, &pentry->data, DB_COMM_SIZE ));
}


static bool_typ slv_bool_reply( comm_clt_typ *pclt, unsigned cmd, 
		bool_typ status )
{
	db_data_typ reply_buff;

	reply_buff.cmd = cmd;
	reply_buff.var = DB_BAD_VAR;
	reply_buff.type = DB_BOOL_TYPE;

	reply_buff.value.db_bool = status;
	return ( comm_sync_reply( pclt, &reply_buff, DB_COMM_SIZE ) );
}

static bool_typ slv_update( slv_db_typ *pdb, db_data_typ *pdata, unsigned xport )
{
	db_entry_typ *pentry;
	dl_node_typ *pnode;
	trig_info_typ trig_pack;
	comm_clt_typ *pclt;

	if( (pentry = db_entry_find( pdb, pdata->var, pdata->type )) != NULL )
	{
		bytecopy( pentry->size, (char *) pdata->value.user,
				  (char *) pentry->data.value.user );
		pentry->data.time = get_sec_clock();
		trig_pack.msg_type = DB_TRIGGER; 
		trig_pack.key = 0;
		trig_pack.var =  pdata->var;
		trig_pack.type =  pdata->type;

		pnode = pentry->ptrig_head->pfirst;

		while( pnode != NULL )
		{
			switch (xport)
			{
			case COMM_QNX6_XPORT:
			{
			        trig_comm_typ *ptrigcomm = 
				  (trig_comm_typ *) pnode->pitem;
			        if( comm_async_trig( ptrigcomm->pclt, 
				      (void *) &(ptrigcomm->trig_event), 
				      sizeof( struct sigevent ) ) == FALSE )
				{
				        ptrigcomm = dl_rm_node( pentry->ptrig_head, &pnode );
					comm_done( ptrigcomm->pclt );
					FREE(ptrigcomm);
					if( pnode == NULL )
						pnode = pentry->ptrig_head->pfirst;
					else
						pnode = pnode->pnext;
				}
				else
				        pnode = pnode->pnext;
			        break;
                        }
		        case COMM_QNX_XPORT:
 		        default:
			{
			        pclt = (comm_clt_typ *) pnode->pitem;
			        if(  comm_async_trig( pclt, &trig_pack,
					sizeof( trig_info_typ ) ) == FALSE )
				{
					pclt = dl_rm_node( pentry->ptrig_head, &pnode );
					comm_done( pclt );
					if( pnode == NULL )
						pnode = pentry->ptrig_head->pfirst;
					else
						pnode = pnode->pnext;
				}
				else
				        pnode = pnode->pnext;
			}
			}
		}
		return( TRUE );
	}
	else
		return( FALSE );
}

static void usage( pargv0 )
char *pargv0;
{
	fprintf( stderr, "Usage: %s -[qQSPv?]\n", pargv0 );
	fprintf( stderr, "\tQ\tEnable QNX 6 messaging\n");
	fprintf( stderr, "\tq\tEnable QNX 4 messaging\n");
	fprintf( stderr, "\tS\tService name (%s)\n", DEFAULT_SERVICE );
	fprintf( stderr, "\tP\tPriority of task\n" );
	fprintf( stderr, "\tv\tVerbose mode\n" );
}

static void db_dump( slv_db_typ *pdb )
{
	dl_head_typ *phead;
	dl_node_typ *pnode;
	db_entry_typ *pentry;

	phead = pdb;
	for( pnode = phead->pfirst; pnode != NULL; pnode = pnode->pnext )
	{
		pentry = (db_entry_typ *)(pnode->pitem);
		db_print( &pentry->data );
		printf( "%d triggers\n", pentry->ptrig_head->length );
	}
}

/**
 *	For COMM_QNX6_XPORT, name_struct is needed later to detach name
 *                           and name_id is used for other interface calls.
 *	For COMM_QNX_XPORT, name_id is needed later to detach name.
 *	For COMM_PSX_XPORT, pservname will be needed to unlink file.
 */
static bool_typ slv_db_publish_name ( int xport, char *pservname, 
			 int *pname_id, void **pname_struct )
{
	int name_id = *pname_id;
	void *name_struct = *pname_struct;
	timestamp_t ts;
	FILE *fp;
	char filename[MAX_LINE_LEN];

	switch (xport) 
	{ 
	case COMM_QNX6_XPORT: {
	        name_attach_t *name_rtn = name_attach( NULL, pservname, 0 );
		if( name_rtn == NULL )
		{
			fprintf( stderr, " Can't attach to name %s\n",
						 pservname );
			return FALSE;
		}
		name_struct = (void *) name_rtn;
                name_id = name_rtn->chid;
		break; }
	case COMM_QNX_XPORT:
		if( (name_id = qnx_name_attach( 0, pservname )) == ERROR )
		{
			fprintf( stderr, " Can't attach to name %s\n",
						 pservname );
			return FALSE;
		}
		break;
	case COMM_PSX_XPORT: 	// create file 
		sprintf( filename, "%s_%d", pservname, 0); 
		if ( (fp = fopen( filename, "w" )) == NULL )
		{
			fprintf( stderr, "Can't open file %s\n", pservname);
			return FALSE;
		}
		get_current_timestamp(&ts);
		print_timestamp(fp, &ts);
		fprintf(fp, "\n");
		fclose(fp);
		break;
	default:
		break;
	} 
	*pname_id = name_id;
	return TRUE;
}

/*	The last entry in the database is always a
 *	bad variable, to be used as a reply for bogus
 *	requests.
 */

static slv_db_typ *slv_db_create( void )
{
	slv_db_typ *pdb;

	if( (pdb = dl_create()) == NULL)
		return( NULL );

	if( db_entry_create( pdb, DB_BAD_VAR, DB_BAD_TYPE, MAX_DATA_SIZE )
				 == FALSE )
	{
		slv_db_destroy( pdb );
		return( NULL );
	}
	else
		return( pdb );
}

static bool_typ slv_db_destroy( slv_db_typ *pdb )
{
	dl_head_typ *phead;
	dl_node_typ *pnode;

	if( pdb == NULL )
		return( FALSE );

	phead = pdb;
	for( pnode = phead->pfirst; pnode != NULL; pnode = pnode->pnext )
		db_entry_destroy( (db_entry_typ *) (pnode->pitem) );

	dl_free( phead );
	return( TRUE );
}

/*	The initial value of the db_entry_typ is set to 0.
 */

static bool_typ db_entry_create( slv_db_typ *pdb, unsigned var, 
		unsigned type,  unsigned size )
{
	db_entry_typ *pentry;
	dl_head_typ *ptrig_head;

	if( sizeof( db_entry_typ ) < size )
		return( FALSE );

	if( db_entry_find( pdb, var, type ) != NULL )
		return( FALSE );

	if( (pentry = CALLOC( 1, sizeof( db_entry_typ ) )) == NULL )
		return( FALSE );

	if( (ptrig_head = dl_create()) == NULL )
	{
		FREE( pentry );
		return( FALSE );
	}

	pentry->ptrig_head = ptrig_head;
	pentry->data.cmd = DB_BAD_CMD;
	pentry->data.var = var;
	pentry->data.type = type;
	pentry->data.time = get_sec_clock();
	pentry->size = size;

	if( dl_insert( pentry, pdb ) == FALSE )
	{
		db_entry_destroy( pentry );
		FREE( pentry );
		return( FALSE );
	}
	return( TRUE );
}

static bool_typ db_entry_rm( slv_db_typ *pdb, unsigned var, unsigned type )
{
	db_entry_typ *pentry;
	dl_node_typ *pnode;
	db_entry_typ find_key;

	find_key.data.var = var;
	find_key.data.type = type;

	if( (pnode = dl_first( pdb, &find_key, db_entry_cmp )) == NULL )
		return( FALSE );

	pentry = dl_rm_node( pdb, &pnode );
	db_entry_destroy( pentry );

	return( TRUE );
}

/*	SYNOPSIS
 *
 *	int db_entry_cmp( db_entry_typ *p1, db_entry_typ *p2 );
 *	p1, p2 			-	Two variable entries to be compared.
 *
 *	DESCRIPTION
 *	Compares two database entries to see if they
 *	refer to the same variable and type.
 *
 *	RETURN
 *		0			-	If the variables are the same.
 *		non-zero	-	Otherwise.
 *
 */

int db_entry_cmp( db_entry_typ *p1, db_entry_typ *p2 )
{
	if( (p1->data.var == p2->data.var) && (p1->data.type == p2->data.type) )
		return( 0 );
	else if( p2->data.var < p1->data.var )
		return( 1 );
	else
		return( -1 );
}

static db_entry_typ *db_entry_bad( slv_db_typ *pdb )
{
	dl_node_typ *pnode;
	dl_head_typ *phead;

	phead = pdb;
	pnode = phead->plast;

	return( (db_entry_typ *) pnode->pitem );
}

static db_entry_typ *db_entry_find( slv_db_typ *pdb, unsigned var, 
		unsigned type )
{
	dl_node_typ *pnode;
	db_entry_typ find_key;

	find_key.data.var = var;
	find_key.data.type = type;

	if( (pnode = dl_first( pdb, &find_key, db_entry_cmp )) == NULL )
		return( NULL );
	else
		return( (db_entry_typ *) pnode->pitem );
}

static bool_typ slv_trig_set( slv_db_typ *pdb, comm_clt_typ *pclt, 
			db_data_typ *precv )
{
	db_entry_typ *pentry;
	comm_clt_typ *pnew_clt, find_key, *pfind;

	if( (pentry = db_entry_find( pdb, precv->var, precv->type )) == NULL )
		return( FALSE );

	switch ( COMM_GET_XPORT(pclt)) {
	case COMM_QNX6_XPORT:
	{	/* Uses trig_comm_typ to store the information,
		   currently only for QNX6.  This stores both
		   pnew_clt as returned by comm_init() and a copy of
		   the struct sigevent. Needs to be memory managed
		   (see trig_unset code).
		 */

	        trig_comm_typ trigfind_key, *ptrigfind, *pnew_trig_comm;
	        struct sigevent *pnew_event, *ptrig_event;
     
		find_key.xport = COMM_QNX6_XPORT;
		find_key.xport_info.chid = pclt->xport_info.chid;

		trigfind_key.pclt = &find_key;

		ptrigfind = &trigfind_key;

		if( dl_first( pentry->ptrig_head, ptrigfind, trig_comm_cmp ) != NULL )
		        return( FALSE );

		if( (pnew_clt = comm_init( COMM_QNX6_XPORT,
			find_key.xport_info.chid )) == NULL )
		{
			return( FALSE );
		}

	        pnew_trig_comm = CALLOC( 1, sizeof( trig_comm_typ ) );

		if ( NULL == pnew_trig_comm )
		        return( FALSE );

		pnew_trig_comm->pclt = pnew_clt;

		pnew_event = &(pnew_trig_comm->trig_event);
		ptrig_event = &(precv->value.trig_event); 
		
		SIGEV_PULSE_INIT( pnew_event, ptrig_event->sigev_coid,
				SIGEV_PULSE_PRIO_INHERIT,
				TRIGGER_PULSE_CODE,
				 ptrig_event->sigev_value.sival_int);

		if( dl_append( pnew_trig_comm, pentry->ptrig_head ) != TRUE )
		{
		        comm_done( pnew_clt );
			FREE( pnew_trig_comm);
			return( FALSE );
		}

		break;
	}
	case COMM_QNX_XPORT:
		find_key.xport = COMM_QNX_XPORT;
		find_key.xport_info.pid = precv->value.db_long;
		pfind = &find_key;
		if( dl_first( pentry->ptrig_head, pfind, comm_clt_cmp ) != NULL )
		        return( FALSE );
	
		if( (pnew_clt = comm_init( COMM_QNX_XPORT,
			find_key.xport_info.pid )) == NULL )
		{
			return( FALSE );
		}

		if( dl_append( pnew_clt, pentry->ptrig_head ) != TRUE )
		{
		        comm_done( pnew_clt );
			return( FALSE );
		}
		break;

	case COMM_PSX_XPORT:
		pfind = pclt;
		if( dl_first( pentry->ptrig_head, pfind, comm_clt_cmp ) != NULL )
		        return( FALSE );
	
		if( (pnew_clt = comm_clt_dup( pclt )) == NULL )
			return( FALSE );

		if( dl_append( pnew_clt, pentry->ptrig_head ) != TRUE )
		{
		        comm_done( pnew_clt );
			return( FALSE );
		}
		break;

	default:
	        break;
	}

	return( TRUE );
}

static bool_typ slv_trig_unset( slv_db_typ *pdb, comm_clt_typ *pclt, 
			db_data_typ *precv )
{
        bool_typ retval = FALSE;

	db_entry_typ *pentry;
	dl_node_typ *pnode;
	comm_clt_typ *ptrig_clt, find_key, *pfind;

	if( (pentry = db_entry_find( pdb, precv->var, precv->type )) == NULL )
		return( FALSE );

	switch( COMM_GET_XPORT(pclt) ) 
	{
	case COMM_QNX6_XPORT:
	{       trig_comm_typ *ptrigcomm, trigfind_key, *ptrigfind;

		find_key.xport = COMM_QNX6_XPORT;
		find_key.xport_info.chid = pclt->xport_info.chid;

		trigfind_key.pclt = &find_key;

		ptrigfind = &trigfind_key;

		if( (pnode = dl_first( pentry->ptrig_head, ptrigfind,
				       trig_comm_cmp )) == NULL )
			return( FALSE );

		if( (ptrigcomm = dl_rm_node( pentry->ptrig_head, &pnode )) == NULL )
			return( FALSE );
		ptrig_clt = ptrigcomm->pclt;

		if ( FALSE == ( retval = comm_done( ptrig_clt )) )
		        return(FALSE);
		
		FREE( ptrigcomm);

		retval = TRUE;
	        break;
	}
	case COMM_QNX_XPORT:
	{
		find_key.xport = COMM_QNX_XPORT;
		find_key.xport_info.pid = precv->value.db_long;
		pfind = &find_key;
		if( (pnode = dl_first( pentry->ptrig_head, pfind, comm_clt_cmp )) == NULL )
			return( FALSE );

		if( (ptrig_clt = dl_rm_node( pentry->ptrig_head, &pnode )) == NULL )
			return( FALSE );

		retval = comm_done( ptrig_clt );
		break;
	}
	case COMM_PSX_XPORT:
	default:
	        pfind = pclt;
		if( (pnode = dl_first( pentry->ptrig_head, pfind, comm_clt_cmp )) == NULL )
			return( FALSE );

		if( (ptrig_clt = dl_rm_node( pentry->ptrig_head, &pnode )) == NULL )
			return( FALSE );
		retval = comm_done( ptrig_clt );
	}
	return( retval );
}

static void db_entry_destroy( db_entry_typ *pentry )
{
	dl_node_typ *pnode;

	for( pnode = pentry->ptrig_head->pfirst; pnode != NULL; pnode = pnode->pnext)
		comm_done( (comm_clt_typ *) (pnode->pitem) );

	dl_free( pentry->ptrig_head );
}

/*	Record the login data, but track the local time.
 */

static bool_typ slv_clt_add( dl_head_typ *pusr, comm_clt_typ *pclt, 
			db_login_typ *plogin )
{
	slv_clt_typ new_usr;

	if( (new_usr.pclt = comm_clt_dup( pclt )) == NULL )
		return( FALSE );
		
	bytecopy( sizeof( db_login_typ ), (void *) plogin, 
			(void *) &new_usr.client );

	new_usr.client.login_time = get_sec_clock();
	if( dl_add_dup( pusr, &new_usr, sizeof( slv_clt_typ ), FALSE )
			 == FALSE )
	{
		comm_done( new_usr.pclt );
		return( FALSE );
	}
	else
		return( TRUE );
}

/*	This doesn't remove the triggers associated with the client.
 */

static bool_typ slv_clt_rm( dl_head_typ *pusr, comm_clt_typ *pclt )
{
	dl_node_typ *pnode;
	slv_clt_typ find_key;
	slv_clt_typ *pold_clt;

	find_key.pclt = pclt;

	if( (pnode = dl_first( pusr, &find_key, slv_clt_cmp )) == NULL )
		return( FALSE );

	pold_clt = dl_rm_node( pusr, &pnode );

	if( comm_done( pold_clt->pclt ) != TRUE )
		return( FALSE );

	FREE( pold_clt );
	return( TRUE );
}

static void slv_clt_dump( dl_head_typ *pusr )
{
	dl_node_typ *pnode;
	slv_clt_typ *pentry;
	
	for( pnode = pusr->pfirst; pnode != NULL; pnode = pnode->pnext )
	{
		if (verbose_flag)
			printf("pnode: 0x%x\n", (unsigned int) pnode);
		pentry = (slv_clt_typ *) pnode->pitem;
		printf( "User: %s\tuid: %3d\n", pentry->client.name,
			pentry->client.uid );
		comm_print_clt( pentry->pclt );
	}
}

static int slv_clt_cmp( slv_clt_typ *p1, slv_clt_typ *p2 )
{
	return( comm_clt_cmp( p1->pclt, p2->pclt ) );
}


static int trig_comm_cmp( trig_comm_typ *p1, trig_comm_typ *p2 )
{
        return( comm_clt_cmp( p1->pclt, p2->pclt ) );
}
