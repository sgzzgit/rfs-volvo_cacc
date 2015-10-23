/**\file	
 *	sys_str.c
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *
 *	DESCRIPTION 
 *	sys_str.c is a group of functions dedicated to string handling.
 *	These functions operate only on NULL terminated strings, 
 *	check for NULL pointer to strings, but do not check for 
 *	overflow of any receiving string.
 *	Any non-NULL pointer to a string must point to a string 
 *	terminated by a NULL terminating character or END_OF_STRING.
 *
 *	NOTES on testing flags: ( only one may be defined at a time )
 *	STR_TEST is defined if testing for string replace and delete
 *	functions is required.
 *	STR_INDEX_TEXT is defined if testing for str_index() is
 *	required.
 *
 *	$Log$
 *	Revision 1.3  2006/07/24 07:11:41  dickey
 *	---------------------------------------------------------------------
 *	Major change: corrected error in size argument to msgsnd and msgrcv
 *	Minor changes: removed old RCS tags that generate irrelevant diffs
 *	Removed some executables from version control.
 *	 Modified Files:
 *	 	clt/Makefile db/Makefile db/db_comm.c db/db_lib.h
 *	 	local/local.h local/sys_arg.c local/sys_arma.h local/sys_btr.c
 *	 	local/sys_buff.c local/sys_buff.h local/sys_fifo.c
 *	 	local/sys_fifo.h local/sys_ini.c local/sys_ini.h
 *	 	local/sys_linux.h local/sys_mem.h local/sys_os.h
 *	 	local/sys_rt_linux.c local/sys_str.c local/sys_time.c
 *	 	local/sys_tok.c
 *	 Added Files:
 *	 	clt/ipcrmall
 *	 Removed Files:
 *	 	clt/rcv_io clt/sample_read clt/sample_update clt/test_das.c
 *	 	clt/test_read clt/test_update db/clt_vars.h db/db_slv
 *	 	db/register/dbq_5341 db/register/dbq_5618
 *	 ----------------------------------------------------------------------
 *
 *	Revision 1.1.1.1  2003/03/25 22:38:26  dickey
 *	QNX$ local library sources
 *	
 * Revision 1.2  1996/10/17  23:53:42  path
 * Add copyright message
 *
 * Revision 1.1  1994/01/31  22:45:16  lchen
 * Initial revision
 *
 *
 */

#include <sys_os.h> 
#include "local.h"
#include "sys_str.h"

/*
 *	SYNOPSIS
 *	#include "sys_str.h"
 *
 *	char *str_delchr_aft( pstring, ch )
 *	char *pstring	pointer to source string	
 *	char ch			delimeter character
 *
 *	DESCRIPTION
 *	If there are any ch is within source string, delete all
 *	characters in string after the first occurrence of ch.
 *
 *	RETURNS
 *	NULL if pstring is a NULL pointer, otherwise
 *	pointer to the new string.
 *	
 *
 */
	
char *str_delchr_aft( pstring, ch )
char *pstring;
char ch;

{
	int len;
	char *proot;

	if( pstring == NULL )
	{
		return( NULL );
	}

	proot = pstring;
	len = strlen( pstring );

	while( (*pstring != ch) && (len > 0) )
	{
		pstring++;
		len--;
	}
	
	if( (len > 0) || (*pstring == ch) )
	{	
		*(++pstring) = END_OF_STRING;	
	}

	return( proot );
}

/*
 *	SYNOPSIS
 *	#include "sys_str.h"
 *
 *	char *str_delchr_bef( pstring, ch )
 *	char *pstring	pointer to source string	
 *	char ch			delimeter character
 *
 *	DESCRIPTION
 *	If there are any ch is within source string, delete all
 *	characters in string before the first occurrence of ch.
 *
 *	RETURNS
 *	NULL if pstring is a NULL pointer, otherwise
 *	pointer to the new string.
 *
 */
	
char *str_delchr_bef( pstring, ch )
char *pstring;
char ch;

{
	int len;
	char *ps;

	if( pstring == NULL )
	{
		return( NULL );
	}

	ps = pstring;
	len = strlen( ps );

	while( (*ps != ch) && (len > 0) )
	{
		ps++;
		len--;
	}

	if( (len > 0) || (*ps == ch) )
	{	
		str_mov( pstring, ps ); 
	}

	return( pstring );
}

/*
 *	SYNOPSIS
 *	#include "sys_str.h"
 *
 *	int str_delchr( pstring, ch )
 *	char *pstring	pointer to source string	
 *	char ch			character to be deleted
 *
 *	DESCRIPTION
 *	If there is any ch within source string, delete from string
 *	the first occurrence of character ch.
 *
 *	RETURN
 *	TRUE if first occurrence of ch was deleted.
 *	FALSE otherwise.
 *
 */

int str_delchr( pstring, ch )
char *pstring;
char ch;

{
	char *proot;

	if( pstring == NULL )
	{
		return( FALSE );
	}

	proot = pstring;
	while( *pstring != END_OF_STRING )
	{
		if( *pstring == ch )
		{
			*pstring++ = END_OF_STRING;
			strcat( proot, pstring );
			return( TRUE );
		}
		else
		{
			pstring++;
		}
	}
	return( FALSE );
}

/*
 *	SYNOPSIS
 *	#include "sys_str.h"
 *
 *	int str_delchr_all( pstring, ch )
 *	char *pstring	source string	
 *	char ch			character to be deleted
 *
 *	DESCRIPTION
 *	If there is any ch within source string, delete from string
 *	all occurrences of character ch.
 *
 *	RETURN
 *	Number of characters ch that were deleted.
 *
 */

int str_delchr_all( pstring, ch )
char *pstring;
char ch;

{
	int count;

	count = 0;
	if( pstring == NULL )
	{
		return( (int) 0 );
	}
	
	do
	{
		count++;
	}while( str_delchr( pstring, ch ) == TRUE );
	
	return( count - 1 );

}


/*
 *	SYNOPSIS
 *	#include "sys_str.h"
 *
 *	int str_repchr_all( psource, new, old, len )
 *	char *psource		pointer to source string
 *	char new			character to use for replacement
 *	char old			character to be replaced
 *	int len				replace only on first len characters
 *
 *	DESCRIPTION
 *	Replace in psource all occurrences of the character old by
 *	character new, within the first "len" characters of psource.
 *
 *	RETURNS
 *	Number of characters replaced.
 *
 */


int str_repchr_all( psource, new, old, len )
char *psource;
char new;
char old;
int len;

{
	int cnt,repl;

	cnt=repl=0;

	if( psource == NULL )
	{
		return( (int) 0 );
	}

	while( (*psource != END_OF_STRING) && ( cnt < len ) )
	{
		cnt++;
		if( *psource == old )
		{
			repl++;
			*psource = new;
		}
		psource++;
	}

	return( repl );
}

/*
 *	SYNOPSIS
 *	#include "sys_str.h"
 *
 *	char *str_mov( pdest, psource )
 *	char *pdest		pointer to destination string
 *	char *psource	pointer to source string
 *
 *	DESCRIPTION
 *	Move the contents of string pointed to by psource to the 
 *	string pointed to by pdest.  If psource points to the same
 *	string as pdest, then psource must be greater than pdest.
 *
 *	RETURNS
 *	NULL if either psource or pdest are NULL pointers, otherwise
 *	pointer to destination or receiving string.
 *
 */

char *str_mov( pdest, psource )
char *pdest;
char *psource;

{
	char *proot;

	if( ( psource == NULL ) || ( pdest == NULL ) )
	{
		return( NULL );
	}

	proot = pdest;
	while( *psource != END_OF_STRING )
	{
		*pdest++ = *psource++;
	}
	*pdest = END_OF_STRING;
	
	return( proot );
}
	

	
/*
 *  SYNOPSIS
 *
 *  char *str_index( pstring, psub_string )
 *  char *pstring           pointer to string to be searched
 *  char *psub_string       pointer to string to be found
 *
 *  DESCRIPTION
 *  str_index() finds a string embedded within another string.
 *  This function expects pstring to point to a string terminated
 *  by an END_OF_STRING.
 *
 *  RETURNS
 *      Pointer into pstring where the substring starts on a successful
 *              search.
 *      A NULL pointer on an unsucessful search.
 *
 *  NOTE:
 *  This function returns a failed search when presented with two
 *  NULL strings. A test function called STR_INDEX_TEST is provided
 *	for testing purposes.
 */

char *str_index( pstring, psub_string )
char *pstring;
char *psub_string;
{
    char *pchar;
    char *psub_first_char;
 
    int sub_string_len;
 
    pchar           = pstring;
    psub_first_char = psub_string;
    sub_string_len  = (int) strlen( psub_string );
 
    while( *pchar != END_OF_STRING )
    {
        if((*pchar == *psub_first_char ) &&
            (strncmp( psub_string, pchar, sub_string_len ) == 0))
        {
            return( pchar );
        }
        else
            pchar++;
    }
    return( NULL );
}
 

/*	SYNOPSIS
 *
 *	#include <sys_str.h>
 *
 *	char *str_copy( s1, s2, max_len )
 *	char *s1			pointer to destination string.
 *	char *s2			pointer to source string.
 *	int max_len			maximum string length which s1 may have.
 *
 *	DESCRIPTION
 *	Copy maxlen characters from string s2 to string s1.  The string
 *	s1 is guaranteed to be NULL terminated.
 *
 *	RETURN
 *	pointer to destination string, s1.
 */

char *str_copy( s1, s2, max_len )
char *s1;
char *s2;
int max_len;
{

	if( strlen( s2 ) < max_len )
	{
		strcpy( s1, s2 );
	}
	else
	{
		strncpy( s1, s2, max_len - 1 );
		s1[ max_len - 1] = END_OF_STRING;
	} 

	return( s1 );
}
	

#ifdef STR_TEST

main()

{
	char ptest[30];

	ptest[0] = END_OF_STRING;
	strcpy(ptest, "abcdef.p.ccmd");

	printf( "ORIGINAL %s\n", ptest);
	str_delchr_all( ptest, 'f' );
	printf( "ALL f DELETED %s\n", ptest);
	printf( "abcde.p.ccmd LEFT\n");
	str_delchr( ptest, 'c' );
	printf( "FIRST c DELETED %s\n", ptest);
	printf( "abde.p.ccmd LEFT\n");
	str_delchr( ptest, '.' );
	printf( "FIRST . DELETED %s\n", ptest);
	printf( "abdep.ccmd LEFT\n");
	str_delchr_aft( ptest, 'm' );
	printf( "DELETE CHARACTERS AFTER FIRST m %s\n", ptest);
	printf( "abdep.ccm LEFT\n");
	str_delchr_aft( ptest, 'c' );
	printf( "DELETE CHARACTERS AFTER FIRST c %s\n", ptest);
	printf( "abdep.c LEFT\n");
	str_delchr_bef( ptest, 'p' );
	printf( "DELETE CHARACTERS BEFORE FIRST p %s\n", ptest);
	printf( "p.c LEFT\n");
	str_repchr_all( ptest, 'd', 'c', strlen(ptest) );
	printf( "REPLACE ALL c BY d  %s\n", ptest); 
	printf( "p.d LEFT\n");

}

#endif


#ifdef STR_INDEX_TEST
main()
{
	char *pstring;

	if( ( pstring = str_index( "", "") ) == NULL )
        printf("str_index:\tNULL string test returned an ERROR\n");
    else
    {  
        printf("str_index   :\tNULL string test FAILED\n");
        printf("return value:\t%s\n", pstring );
    }
	exit(0);
}
#endif


