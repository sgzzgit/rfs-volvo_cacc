/**\file	
 *	db_slv.h
 *
 *	Definitions for the database server only.
 *
 */

typedef dl_head_typ slv_db_typ;

/*	The actual database entry for the servers.
 */

typedef struct
{
	db_data_typ data;
	unsigned size;
	dl_head_typ *ptrig_head;	/*List of communication handles	*/
					/*	for triggers.		*/
} db_entry_typ;

/*	The login list of clients.
 */

typedef struct
{
	db_login_typ client;	/*	Information from login packet.	*/
	comm_clt_typ *pclt;
} slv_clt_typ;

typedef struct
{
        comm_clt_typ *pclt;
        struct sigevent trig_event;
} trig_comm_typ;
