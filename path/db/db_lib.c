/*	FILE
 *	db_lib.c
 *
 *	This file contains a routines for printing database
 *	commands and variables.
 */

#include "db_include.h"

static void db_print_cmd( db_data_typ *pdata );
static void db_print_var( db_data_typ *pdata );

/*
 *	SYNOPSIS
 *
 *	#include <local.h>
 *	#include <sys_list.h>
 *
 *	#include "db_comm.h"
 *	#include "db_clt.h"
 *	#include "db_lib.h"
 *
 *	void db_print( db_data_typ *pdata );
 *
 *	pdata	-	A pointer to the packet to be printed.
 *
 *	DESCRIPTION
 *	Prints a command or variable to standard out.  For user
 *	defined variables, only the name and type are printed,
 *	since the format of the actual data is user specified.
 *
 *	RETURN
 *	none.
 *
 */

void db_print( db_data_typ *pdata )
{
	if( pdata == NULL )
	{
		printf( "NULL db_data_typ pointer\n" );
		return;
	}

	switch( pdata->cmd )
	{
	case DB_LOGIN_CMD:
	case DB_LOGOUT_CMD:
	case DB_READ_CMD:
	case DB_UPDATE_CMD:
	case DB_CREATE_CMD:
	case DB_DESTROY_CMD:
	case DB_TRIG_SET_CMD:
	case DB_TRIG_UNSET_CMD:
		db_print_cmd( pdata );
		break;

	case DB_BAD_CMD:
	default:
		db_print_var( pdata );
		break;
	}
}

static void db_print_cmd( db_data_typ *pdata )
{
	switch( pdata->cmd )
	{
	case DB_BAD_CMD:
		printf( "Bad database cmd.\n" );
		break;

	case DB_LOGIN_CMD:
		printf( "Login: %s\tuid: %3d\tgid: %3d\n", 
			pdata->value.login_data.name,
			pdata->value.login_data.uid,
			pdata->value.login_data.gid );
		break;

	case DB_LOGOUT_CMD:
		printf( "Logout by uid: %d\n", pdata->value.login_data.uid );
		break;

	case DB_CREATE_CMD:
		printf( "Entry created: %u\t type: %u\n", pdata->var, pdata->type );
		break;

	case DB_READ_CMD:
		printf( "Read on var: %d\n", pdata->var );
		break;

	case DB_TRIG_SET_CMD:
		printf( "Trigger set on var: %d\n", pdata->var );
		break;

	case DB_TRIG_UNSET_CMD:
		printf( "Trigger released on var: %d\n", pdata->var );
		break;

	case DB_UPDATE_CMD:
		printf( "Update:\n" );
		db_print_var( pdata );
		break;

	case DB_DESTROY_CMD:
		printf( "Entry destroyed: %u\n", pdata->var );
		break;

	default:
		printf( "Unknown command type\n" );
		break;
	}
}

static void db_print_var( db_data_typ *pdata )
{
	printf( "Var: %u,\tType: %u\tTime: %9.3lf\tValue: ",
		pdata->var, pdata->type, pdata->time );

	switch( pdata->type )
	{
	case DB_DOUBLE_TYPE:
		printf( "%lf\n", pdata->value.db_double );
		break;

	case DB_LONG_TYPE:
		printf( "%ld\n", pdata->value.db_long );
		break;

	case DB_BOOL_TYPE:
		printf( "%d\n", pdata->value.db_bool );
		break;

	case DB_BAD_TYPE:
		printf( "Bad var type\n" );
		break;

	default:
		printf( "User defined type\n" );
		break;
	}
}

