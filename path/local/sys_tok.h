/**\file	
 *	sys_tok.h
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *
 *	Header file for sys_tok.c.
 *
 */
#ifndef PATH_SYS_TOK_H
#define PATH_SYS_TOK_H

typedef struct
{
	char *pstring;
	int token;
} str_tok_typ;


/* 	Largest (int) representation of a char. */

#define MAX_CHAR_CODE	256

/*	Code for the last entry in the table of codes and code words. */

#define END_OF_WORD_LIST	0


int tok_encode( str_tok_typ *ptable, char *pstring, int *ptoken, int size );
int tok_decode( str_tok_typ *ptable, int *ptoken, char *pstring, int size );
bool_typ tok_find_word( str_tok_typ *ptable, int token, char *pstring, 
			int size );
bool_typ tok_find_code( str_tok_typ *ptable, char *pstring, int *pcode );
void tok_strings( str_tok_typ *ptable );

#endif
