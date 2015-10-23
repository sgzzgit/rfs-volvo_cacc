/**\file	
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 * Revision 1.2  1996/10/17  23:37:45  path
 * Add copyright notice
 *
 * Revision 1.1  1994/11/16  19:03:59  lchen
 * Initial revision
 *
 */

bool_typ get_ini_bool( FILE *pfile, char *pentry, bool_typ bool_def );
long get_ini_long( FILE *pfile, char *pentry, long long_def );
unsigned get_ini_hex( FILE *pfile, char *pentry, unsigned hex_def );
double get_ini_double( FILE *pfile, char *pentry, double def_value );
char *get_ini_string( FILE *pfile, char *pentry, char *ps_def );
FILE *get_ini_section( char *pname, char *psection );

