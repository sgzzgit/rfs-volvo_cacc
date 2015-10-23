/**\file	
 *	sys_tok.c
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *
 *	DESCRIPTION
 *	Case sensitive token recognition library utilities.
 *	This program converts:
 *	1.	A string to a word code list using the user supplied table.
 *	2.	A word code list to a string.
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
 * Revision 1.3  1996/10/17  23:59:33  path
 * Add copyright notice
 *
 * Revision 1.2  1996/03/21  02:18:35  path
 * Use strncmp instead of strcmp in tok_find_code.
 *
 * Revision 1.1  1994/01/31  22:45:16  lchen
 * Initial revision
 *
 *
 */

/*
 *	System library header files.
 */

#include <sys_os.h>
#include "local.h"
#include "sys_tok.h"


/*
 *	SYNOPSIS
 *
 *	static int tok_code2word( str_tok_typ *ptable, int code, char *pstring, int size );
 *	ptable	- Pointer to table of code words.
 *	code	- Word code.
 *	pstring - Pointer to string in which to store the converted data.
 *	size	- Length of buffer.
 *
 *	DESCRIPTION
 *	This function converts a code to a character string.
 *
 *	RETURN
 * 	int	-	number of characters in the resulting string.
 *	( Therefore, if the code is bad, the return is 0,
 *	and, if the code is for a char, the return is 1. )
 *
 */	

static int tok_code2word( ptable, code, pstring, size )
str_tok_typ *ptable;
int code;
char *pstring;
int size;
{
	*pstring = END_OF_STRING;

	/* Check if the code is a code word. */

	if( tok_find_word( ptable, code, pstring, size ) == TRUE )
	{
		return( strlen( pstring ) );
	}

	/* Check if the code is a char. */

	if( code < MAX_CHAR_CODE )
	{
		sprintf( pstring, "%c", (char) code );
		return( 1 );
	}

	/* If the code is neither a char nor a word, it is bad. */

	return( 0 );
}


/*
 *  SYNOPSIS
 *  #include <local.h>
 *  #include "sys_tok.h"
 *
 *  bool_typ tok_find_code( str_tok_typ *ptable, char *pstring, int *pcode );
 *  ptable 	-   Pointer to table of code words.
 *  pstring -   Pointer to string to be encoded.
 *  pcode   -   Word code.
 *
 *  DESCRIPTION
 *  This function converts a string to a code.
 *  It only encodes words from the table.
 *
 *  RETURN
 *  TRUE    -   If the word is in the table.
 *  FALSE   -   Otherwise.
 *
 */

bool_typ tok_find_code( ptable, pstring, pcode )
str_tok_typ *ptable;
char *pstring;
int *pcode;
{
    str_tok_typ *plist;

    for( plist = ptable; plist->token != END_OF_WORD_LIST; plist++ )
    {
        /* NOTE:  On 3/20/96 changed to use strncmp so we'll never look
         * beyond the length of the work stored in the table.  This takes
         * care of a problem with trailing ASCII blanks. */
        if( strncmp( pstring, plist->pstring, strlen(plist->pstring) ) == 0 )
        {
            *pcode = plist->token;
            return( TRUE );
        }
    }    
	*pcode = 0;
    return( FALSE );
}


/*
 *	SYNOPSIS
 *	#include <local.h>
 *	#include "sys_tok.h"
 *
 *	bool_typ tok_find_word( str_tok_typ *ptable, int code, char *pstring, int size );
 *	ptable	- 	Pointer to table of code words.
 *	code 	-	Word code.
 *	pstring	-	Pointer to string in which to store	the converted data.
 *	size	-	Size of the string to store the converted data.
 *
 *	DESCRIPTION
 *	This function converts a code to a character string.
 *	It only decodes words from the table. 
 *
 *	RETURN
 * 	TRUE	-	If the word is in the table.
 *	FALSE	-	Otherwise.
 *
 */	


bool_typ tok_find_word( ptable, code, pstring, size )
str_tok_typ *ptable;
int code;
char *pstring;
int size;
{
	int i;

	*pstring = END_OF_STRING;
	
	for( i = 0; ptable[i].token != END_OF_WORD_LIST; i++ )
	{
		if( ptable[i].token == code )
		{
			strncpy( pstring, ptable[i].pstring, size -1 );
			pstring[size-1] = END_OF_STRING;
			return( TRUE );
		}
	}
	return( FALSE );
}



/*
 *	SYNOPSIS
 *	#include <local.h>
 *	#include "sys_tok.h"
 *
 *	int tok_decode( str_tok_typ *ptable; int *ptoken, char *pstring, int size );
 *	ptable	- 	Pointer to table of code words.
 *	ptoken	-	Pointer to array of codes.
 *	pstring	-	Pointer to string in which to store the converted data.
 *	size	-	Size of the string to store the converted data.
 *
 *	DESCRIPTION
 *	This function converts a code list from ptoken to a message string.
 *
 *	RETURN
 *	Number of characters in the decoded string.  If the decoded string is
 *	longer than size, only words that fit are written in pstring,
 *	but the return value is the actual size of decoded string.
 *
 */	

int tok_decode( ptable, ptoken, pstring, size )
str_tok_typ *ptable;
int *ptoken;
char *pstring;
int size;
{
	char word_buff[ MAX_LINE_LEN + 1 ];
	int *pnext;
	int num_char;

	*pstring = END_OF_STRING;
	num_char = 0;

	if( ptoken == NULL )
		return( 0 );

	for( pnext = ptoken; *pnext != END_OF_WORD_LIST; pnext++ )
	{
		num_char += tok_code2word( ptable, *pnext, word_buff, MAX_LINE_LEN );
		if( num_char < (size - 1) )
			strcat( pstring, word_buff );
	}

	return( num_char );
}


/*
 *	SYNOPSIS
 *	#include <local.h>
 *	#include "sys_tok.h"
 *
 *	int tok_encode( str_tok_typ *ptable, char *pstring, int *ptoken, int size );
 *	ptable	- 	Pointer to table of code words.
 *	pstring -	Pointer to string to be encoded.
 *	ptoken	-	Pointer to array where to put codes.
 *	size	-	Size of the array to store codes.
 *
 *	DESCRIPTION
 *	This function converts a string which is stored in pstring 
 *	to a code list, and stores the converted data in the array of integers
 *	ptoken.  The resulting codes may contain both words
 *	from the table of code words and ASCII representation of characters.
 *
 *	RETURN
 *	int	-	number of codes that the string can be converted to.  If it
 *			is larger than size, only size codes are written in the
 *			resulting code string.
 */	

int tok_encode( ptable, pstring, ptoken, size )
str_tok_typ *ptable;
char *pstring;
int *ptoken;
int size;
{
	int list_index;		/*	Index for parameter ptoken.	*/
	int word_index;		/*	Index for local variable word.	*/
	str_tok_typ *plist; 
	char *pmessage;
	char *pposition;
	int save_code; 		/* 	Code of the last matched word. */
	bool_typ match_flag;	/* Partial or full match. */

	list_index = 0;
	word_index = 0;
	save_code  = ERROR;
	pmessage   = pstring;

	do
	{
		match_flag = FALSE;

   		for( plist = ptable; plist->token != END_OF_WORD_LIST; plist++ ) 
   		{ 
			/* Check to see if the word from the message matches
			 *	the word from the table.
			 */

			if( strncmp( pmessage, plist->pstring, word_index + 1 ) == 0 )
			{
				if( strlen( plist->pstring ) == word_index + 1 )
				{
					/* If the substring matches the "full" word from
				 	 * 	the table of strings, save its code and position.
				 	 */
					save_code = plist->token;
					pposition = pmessage + word_index + 1;
				}
				match_flag = TRUE;
			}

		}

		/* 	After looking through the table, if the string of length
		 *	(word_index+1) does not match even the beginning of some 
		 *	string in the table, encode a substring of the pstring.
		 */

		if( (match_flag != TRUE) || (pmessage[word_index] == END_OF_STRING) )	
		{	

			if( save_code != ERROR )
			{
				/*	If the word gave a full match previously, 
			 	 *	encode that substring that gave a full match
			 	 *	and backup to the point where it did.
				 */

				if( list_index < (size - 1) )
					ptoken[list_index] = save_code;
				list_index++;
				save_code = ERROR;
				pmessage = pposition;
			}
			else	
			{
				/*	If no full match has occured during previous encoding
			 	 *	encode the first character and backup to that point.
			 	 */

				if( list_index < (size - 1) )
					ptoken[list_index] = (int) *pmessage;
				list_index++;
				pmessage++;
			}
			word_index = 0;
		}
		else
		{
			word_index++;
		}

	}
	while( *pmessage != END_OF_STRING );

	/*	Terminate the list of tokens with END_OF_WORD_LIST.
  	 *	If the string can be encoded into less then "size" tokens,
	 *	terminate the list.  Otherwise, put END_OF_WORD_LIST in
 	 *	the last element of the "ptoken" array.
 	 */

	if( list_index < (size - 1) )
	{
		ptoken[list_index] = END_OF_WORD_LIST;
	}
	else
		ptoken[size-1] = END_OF_WORD_LIST;

	return( list_index );
}

void tok_strings( str_tok_typ *ptable )
{
	for( ; ptable->token != END_OF_WORD_LIST; ptable++ ) 
		puts( ptable->pstring );
}

#ifdef TEST_SYS_TOK_ENCODE

#include <cms_msg.h>

main()
{
	char message_buff[ MAX_LINE_LEN+1];
	char output_buff[ MAX_LINE_LEN+1 ];
	int array[MAX_LINE_LEN+1];
	char temp[MAX_LINE_LEN+1];
	int i;
	int num_int, num_char;

	printf( "Number of tokens: " );
	scanf( "%d%d\n", &num_int, &num_char );

	printf( "=> ");
	gets( message_buff );

	tok_encode( cms_word_list, message_buff, array, num_int );

	printf( "Array: " );
	for( i = 0; array[i] != 0; i++ )
		printf( "%d  ", array[i] );
	printf( "\n" );

	printf( "Number of char: %d\n", 
		tok_decode( cms_word_list, array, output_buff, num_char ) );
	printf( "Decoded message: \"%s\"\n", output_buff );

}

#endif



