/*	FILE
 *	sys_arg.c
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *	Routine for supporting generation of command line argument counts 
 *	and argv blocks for process control.
 *
 *	Modified Files:
 *	sys_arg.c sys_linux.h sys_os.h sys_rt.c sys_str.c CVS: sys_tok.c
 *	sys_trk.c
 *	----------------------------------------------------------------------
 *	Minor changes for compiler warning removal
 *	
 *	Revision 1.1.1.1  2006/07/21 22:54:27  dickey
 *	For code on Linux
 *	
 *	Revision 1.1.1.1  2003/03/25 22:38:26  dickey
 *	QNX$ local library sources
 *	
 * Revision 1.2  1996/10/17  23:20:11  path
 * Add copyright notice
 *
 * Revision 1.1  1994/01/31  22:45:16  lchen
 * Initial revision
 *
 *
 */

#include "sys_os.h"
#include "local.h"

#define ARG_WHITESPACE		"	 "				/*	Space and tab	*/

/*	SYNOPSIS
 *
 *	int arg_make( char *ps, char ***pargv );
 *
 *	char *ps		-	null-terminated string which contains the 
 *						command line.
 *
 *	char ***pargv	-	On return, this will point to the new argv[]
 *						array.
 *
 *	DESCRIPTION
 *
 *	Creates an argc count and argv array from the given command line.
 *	A copy of the given command line is split into null-terminated 
 *	strings and an argv pointer array is built.  The last pointer
 *	in the argv[] array is a NULL.
 *
 *	Both the pointer array and the arguments themselves are built 
 *	from malloc() memory.  
 *	Thus the caller should eventually release the copy of
 *	the command line and also the argv array.
 *	A typical calling sequence would be:
 *
 *	int newargc;
 *	char *newargv[];
 *	char buffer[MAX_LINE_LEN+1];
 *
 *	sprintf( buffer, "child -o option" );
 *	if( (newargc = arg_make( buffer, &newargv )) != ERROR )
 *	{
 *		os9exec(.....);
 *		free( newargv[0] );
 *		free( newargv );
 *	}
 *
 *	
 *	The given command line is returned intact.
 *	Arguments should be separated by spaces or tabs.
 *
 *	RETURN
 *		Argument count, or
 *		ERROR if there is a memory problem.
 *
 */

int arg_make( ps, pargv )
char *ps;
char ***pargv;
{
	int argc;
	char *ptemp, *pdata, *pindex;
	char **pargs;

	/*	Make a temporary copy for counting,
	 *	and a final copy of the argument strings
	 *	to return to the user.
	 */

	if( ((ptemp = strdup( ps )) == NULL) || ((pdata = strdup( ps )) == NULL) )
	{
		if( ptemp != NULL )
			free( ptemp );

		if( pdata != NULL )
			free( pdata );
		return( ERROR );
	}

	/*	Generate an argument count using the temporary copy.		*/

	argc = 0;
	for( pindex = strtok( ptemp, ARG_WHITESPACE ); pindex != NULL; )
	{
		pindex = strtok( NULL, ARG_WHITESPACE );
		argc++;
	}

	/*	Make enough space for a pointer to each argument, and
	 *	an extra terminating pointer.
	 */

	if( (pargs = (char **) malloc( (argc + 1) * sizeof( char * ))) != NULL )
	{
		*pargv = pargs;

		for( *pargs = strtok( pdata, ARG_WHITESPACE ); *pargs != NULL; )
			*(++pargs) = strtok( NULL, ARG_WHITESPACE );
	}
	free( ptemp );
	return( argc );
}

#ifdef ARG_TEST

#include <stdio.h>

#define NUM_TEST		100000				/* Watch for memory leakage.	*/

int main( argc, argv )
int argc;
char *argv[];
{
	int count;
	int i, newargc;
	char buffer[MAX_LINE_LEN+1];
	char **newargv;

	strcpy( buffer, argv[0] );
	strcat( buffer, " " );
	for( i = 1; i < argc; i++ )
	{
		strcat( buffer, argv[i] );
		strcat( buffer, " " );
	}

	for( count = 0; count < NUM_TEST; count++ )
	{
		newargc = arg_make( buffer, &newargv );
		for( i = 0; i < newargc; i++ )
		{
			if( strcmp( argv[i], newargv[i] ) != 0 )
				printf( "%d %s %s\n", i, argv[i], newargv[i] );
		}
		free( newargv[0] );
		free( newargv );
	}
}

#endif
