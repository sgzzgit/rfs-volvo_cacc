/**\file	
 *	sys_str.h
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 */

char *str_delchr_aft( char *pstring, char ch );
char *str_delchr_bef( char *pstring, char ch );
int str_delchr( char *pstring, char ch );
int str_delchr_all( char *pstring, char ch );
int  str_repchr_all( char* psource, char new, char old , int len );
char *str_mov( char *pdest, char *psource );
char *str_index( char *pstring, char *psub_string );
char *str_copy( char *s1, char *s2, int max_len );
