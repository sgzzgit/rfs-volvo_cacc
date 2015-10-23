/**\file
 *
 * Copyright (c) 1996   Regents of the University of California
 *
 *
 *	A collection of routines to read parameters from
 *	DOS/Windows style .ini files.
 *
 * Revision 1.3  1996/10/17  23:36:52  path
 * Add copyright notice
 *
 * Revision 1.2  1995/06/20  05:50:08  lchen
 * Add documentation
 *
 * Revision 1.1  1994/11/16  19:03:59  lchen
 * Initial revision
 *
 */

#include "sys_os.h"

#include "local.h"
#include "sys_tok.h"
#include "sys_ini.h"

#define ASC_EQUAL_SIGN		'='
#define ASC_LEFT_ARRAY		'['

extern int readline( FILE *pfin, char *pbuff, int maximum );
static char *get_entry( FILE *pfile, char *pentry );

static long section_offset;

/*	sys_tok.c routines don't like token with value 0.
 */

str_tok_typ bool_table[] =
{
	{	"TRUE",		1},
	{	"YES",		1},
	{	"1",		1},
	{	"FALSE",	-1},
	{	"NO",		-1},
	{	"0",		-1},
	{	NULL, 		END_OF_WORD_LIST}
};

/*
 *	SYNOPSIS
 *
 *	#include <stdio.h>
 *
 *	#include <local.h>
 *	#include <sys_ini.h>
 *
 *	bool_typ get_ini_bool( FILE *pfile, char *pentry, bool_typ bool_def );
 *	long get_ini_long( FILE *pfile, char *pentry, long long_def );
 *	unsigned get_ini_hex( FILE *pfile, char *pentry, unsigned hex_def );
 *	double get_ini_double( FILE *pfile, char *pentry, double doub_def );
 *	char *get_ini_string( FILE *pfile, char *pentry, char *ps_def );
 *
 *	pfile	-	The pointer to an input file which must have been opened using
 *				the get_ini_section() routine.
 *	pentry	-	Pointer to a string which identifies the initialization
 *				variable name.
 *	xxx_def	-	The default value which should be returned if the
 *				desired variable name is not found in the file.
 *				If necessary, a special value can be used for error detection.
 *
 *	DESCRIPTION
 *	These are a collection of routines which read initialization values
 *	from Windows style .ini files.  The section name must have been
 *	specified in a prior get_ini_section() call.
 *
 *	As indicated by the function names, the various routines obtain
 *	boolean (TRUE, YES, 1, or FALSE, NO, 0), long integer,
 *	hexadecimal, double, or string values from the file.  Typical variable
 *	initialization entries for those respective types might look like:
 *
 *	[windows]
 *	ScreenSaveActive=1 
 *	DoubleClickSpeed=452 
 *	PortAddress=9af1
 *	GoodDouble=12345.6789
 *	Programs=com exe bat pif
 *
 *	Note that:
 *	The variable name ends with the first = sign.
 *	A variable cannot begin with [, which denotes a section name.
 *	Blank lines between variables are ignored.
 *	Comments begin with a # symbol.
 *	The hexadecimal routine does not use a leading 0x.
 *	The integer input routines will ignore incorrect trailing characters, but
 *	accept the leading information.
 *	
 *	RETURN
 *	If the desired variable name is found, the value from the file is returned.
 *	If the name is not found, the default value is returned.
 *
 */

bool_typ get_ini_bool( FILE *pfile, char *pentry, bool_typ bool_def )
{
	char *presult;
	char *ps;
	int bool_value;

	if( (presult = get_ini_string( pfile, pentry, NULL )) == NULL )
		return( bool_def );
	else
	{
		for( ps = presult; *ps != END_OF_STRING;  )
			*ps++ = toupper( *ps );

		if( tok_find_code( bool_table, presult, &bool_value ) == TRUE )
		{
			if( bool_value == 1 )
				return( TRUE );
			else
				return( FALSE );
		}
		else
			return( bool_def );
	}
}

long get_ini_long( FILE *pfile, char *pentry, long long_def )
{
	extern long atol();
	char buffer[MAX_LINE_LEN+1];
	char *pbuffer, *presult;

	if( (presult = get_entry( pfile, pentry )) == NULL )
		return( long_def );
	else
	{
		for( pbuffer = buffer; isdigit( *presult ) 
			&& (pbuffer < (buffer + MAX_LINE_LEN)); )
		{
			*pbuffer++ = *presult++;
		}
	}

	*pbuffer = END_OF_STRING;
	if( strlen( buffer ) == 0 )
		return( long_def );
	else
		return( atol( buffer ) );
}

unsigned get_ini_hex( FILE *pfile, char *pentry, unsigned hex_def )
{
	char buffer[MAX_LINE_LEN+1];
	char *pbuffer, *presult;

	if( (presult = get_entry( pfile, pentry )) == NULL )
		return( hex_def );
	else
	{
		for( pbuffer = buffer; isxdigit( *presult ) 
			&& (pbuffer < (buffer + MAX_LINE_LEN)); )
		{
			*pbuffer++ = *presult++;
		}
	}

	*pbuffer = END_OF_STRING;
	if( strlen( buffer ) == 0 )
		return( hex_def );
	else
		return( atoh( buffer ) );
}

double get_ini_double( FILE *pfile, char *pentry, double def_value )
{
	double value;
	char *ps;

	if( (ps = get_entry( pfile, pentry )) == NULL )
		value = def_value;
	else
	{
		if( sscanf( ps, "%lf", &value ) != 1 )
			value = def_value;
	}
		
	return( value );
}

char *get_ini_string( FILE *pfile, char *pentry, char *ps_def )
{
	static char buffer[MAX_LINE_LEN+1];
	char *presult;

	if( (presult = get_entry( pfile, pentry )) == NULL )
	{
		if( ps_def == NULL )
			return( NULL );
		else
			strncpy( buffer, ps_def, MAX_LINE_LEN );
	}
	else
		strncpy( buffer, presult, MAX_LINE_LEN );
		
	buffer[MAX_LINE_LEN] = END_OF_STRING;
	return( buffer );
}

/*
 *	SYNOPSIS
 *
 *	#include <stdio.h>
 *
 *	#include <local.h>
 *	#include <sys_ini.h>
 *
 *	FILE *get_ini_section( char *pname, char *psection );
 *
 *	pname		-	The name of a configuration file.
 *	psection	-	The name of a section within a configuration file.
 *					It is not necessary to place brackets around the
 *					section name.
 *
 *	DESCRIPTION
 *	This routine opens the given configuration file, and locates the
 *	relevent section.  It is necessary to call get_ini_section() before
 *	reading initialization values.  It is necessary to call fclose() with
 *	the file pointer when done. It is also necessary to close one section
 *	before calling get_ini_section() again withing the same program.
 *
 *	A typical configuration file is text, and might contain lines like:
 *
 *	[first_section_name]
 *	MoreData=TRUE
 *	AnotherVariable=1.2345
 *
 *	[windows_section]
 *	ScreenSaveActive=1
 *	DoubleClickSpeed=452
 *
 *	[last_section]
 *	MoreData=FALSE
 *
 *	Note that:
 *	Blank lines are ignored.
 *	Comments begin with a # symbol.
 *	Section and variable names should all be on separate lines.
 *
 *	RETURN
 *	non-NULL	-	A file pointer for the given file name and section name.
 *	NULL		-	If the section or file can't be found.
 *
 *	BUGS
 *	Within a single program, only one configuration section and file can be
 *	open at once.
 */

FILE *get_ini_section( char *pname, char *psection )
{
	FILE *pfin;
	char section[MAX_LINE_LEN+1];
	char buffer[MAX_LINE_LEN+1];

	section_offset = ERROR_LONG;
	if( (pfin = fopen( pname, FILE_STREAM_READ )) == NULL )
		return( NULL );

	sprintf( section,"[%s]", psection );	/* Format the section name */

	/*	Move through file 1 line at a time until a section is matched or EOF
	 */

	do
	{
		if( readline( pfin, buffer, MAX_LINE_LEN ) == EOF )
		{
			fclose( pfin );
			return( NULL );
		}
	} while( strcmp( buffer, section ) != 0 );

	section_offset = ftell( pfin );

	return( pfin );
}

static char *get_entry( FILE *pfile, char *pentry )
{
	static char buffer[MAX_LINE_LEN+1];
	char *pstring;
	int len;

	if( fseek( pfile, section_offset, SEEK_SET ) != 0 )
		return( NULL );

	len = strlen( pentry );
	do
	{
		if( readline( pfile, buffer, MAX_LINE_LEN ) == EOF )
			return( NULL );

		for( pstring = buffer; isspace( *pstring ); pstring++ )
			;

		if( *pstring == ASC_LEFT_ARRAY )/*	Overrun section?	*/
			return( NULL );
	}
	while( strncmp( buffer, pentry, len ) != 0 );

	/*	Parse out the first equal sign
	 */

	pstring = strchr( buffer, ASC_EQUAL_SIGN );
	pstring++;
	if( strlen( pstring ) == 0 )		/*	No setting?					*/
		return( NULL );
	else
		return( pstring );
}

#ifdef TEST

#define TEST_FILE			"win.ini"
#define GOOD_SECTION_1		"windows"
#define GOOD_ENTRY_BOOL		"ScreenSaveActive"
#define GOOD_BOOL			TRUE
#define GOOD_ENTRY_LONG		"DoubleClickSpeed"
#define GOOD_ENTRY_HEX		"PortAddress"
#define GOOD_LONG			452L
#define GOOD_ENTRY_DOUBLE	"GoodDouble"
#define GOOD_ENTRY_STRING	"Programs"
#define GOOD_SECTION_2		"Extensions"
#define BAD_SECTION			"Bad section"
#define BAD_DOUBLE			(-1.0)

void main( int argc, char *argv[] )
{
	FILE *pfin;
	long sect_offset;
	bool_typ bool_value;
	long long_value;
	double double_value;
	char *pstring_value;
	unsigned hex_value;

	if( (pfin = get_ini_section( TEST_FILE, GOOD_SECTION_1 )) == NULL )
	{
		printf( "Can't find section %s\n", GOOD_SECTION_1 );
		exit( EXIT_FAILURE );
	}
	sect_offset = ftell( pfin );

	if( (bool_value = get_ini_bool( pfin, GOOD_ENTRY_BOOL, FALSE )) == FALSE )
		printf( "Boolean failed on %s\n", GOOD_ENTRY_BOOL );
	else
		printf( "Boolean got value %d\n", bool_value );

	if( (long_value = get_ini_long( pfin, GOOD_ENTRY_LONG, 0L )) == 0L )
		printf( "Long failed on %s\n", GOOD_ENTRY_LONG );
	else
		printf( "Long got value %ld\n", long_value );

	if( (hex_value = get_ini_hex( pfin, GOOD_ENTRY_HEX, 0L )) == 0L )
		printf( "Hex failed on %s\n", GOOD_ENTRY_HEX );
	else
		printf( "Hex got value %x\n", hex_value );

	pstring_value = get_ini_string( pfin, GOOD_ENTRY_STRING, "FAIL" );
		printf( "String result was %s\n", pstring_value );

	if( (double_value = get_ini_double( pfin, GOOD_ENTRY_DOUBLE,
		BAD_DOUBLE )) == BAD_DOUBLE )
	{
		printf( "Double failed, entry %s\n", GOOD_ENTRY_DOUBLE );
	}
	else
		printf( "Double got value %lf\n", double_value );

	fclose( pfin );

	if( (pfin = get_ini_section( TEST_FILE, BAD_SECTION )) != NULL )
	{
		printf( "Shouldn't have found section %s\n", BAD_SECTION );
		fclose( pfin );
		exit( EXIT_FAILURE );
	}
	if( (pfin = get_ini_section( TEST_FILE, GOOD_SECTION_2 )) == NULL )
	{
		printf( "Can't find section %s\n", GOOD_SECTION_2 );
		exit( EXIT_FAILURE );
	}
	exit( EXIT_SUCCESS );
}
#endif

#if FALSE

/***************************************************************************
 * Function:    write_private_profile_string()
 * Arguments:   <char *> section - the name of the section to search for
 *              <char *> entry - the name of the entry to find the value of
 *              <char *> buffer - pointer to the buffer that holds the string
 *              <char *> file_name - the name of the .ini file to read from
 * Returns:     TRUE if successful, otherwise FALSE
 ***************************************************************************/
int write_private_profile_string(char *section, 
    char *entry, char *buffer, char *file_name)

{   FILE *rfp, *wfp;
    char tmp_name[15];
    char buff[MAX_LINE_LENGTH];
    char t_section[MAX_LINE_LENGTH];
    int len = strlen(entry);
    tmpnam(tmp_name); /* Get a temporary file name to copy to */
    sprintf(t_section,"[%s]",section);/* Format the section name */
    if( !(rfp = fopen(file_name,"r")) )  /* If the .ini file doesn't exist */ 
    {   if( !(wfp = fopen(file_name,"w")) ) /*  then make one */
        {   return(0);   }
        fprintf(wfp,"%s\n",t_section);
        fprintf(wfp,"%s=%s\n",entry,buffer);
        fclose(wfp);
        return(1);
    }
    if( !(wfp = fopen(tmp_name,"w")) )
    {   fclose(rfp);
        return(0);
    }
    /* Move through the file one line at a time until a section is 
     * matched or until EOF. Copy to temp file as it is read. */
    do
    {   if( !read_line(rfp,buff) )
        {   /* Failed to find section, so add one to the end */
            fprintf(wfp,"\n%s\n",t_section);
            fprintf(wfp,"%s=%s\n",entry,buffer);
            /* Clean up and rename */
            fclose(rfp);
            fclose(wfp);
            unlink(file_name);
            rename(tmp_name,file_name);
            return(1);
        }
        fprintf(wfp,"%s\n",buff);
    } while( strcmp(buff,t_section) );
    /* Now that the section has been found, find the entry. Stop searching
     * upon leaving the section's area. Copy the file as it is read
     * and create an entry if one is not found.  */
    while( 1 )
    {   if( !read_line(rfp,buff) )
        {   /* EOF without an entry so make one */
            fprintf(wfp,"%s=%s\n",entry,buffer);
            /* Clean up and rename */
            fclose(rfp);
            fclose(wfp);
            unlink(file_name);
            rename(tmp_name,file_name);
            return(1);

        }
        if( !strncmp(buff,entry,len) || buff[0] == '\0' )
            break;
        fprintf(wfp,"%s\n",buff);
    }
    if( buff[0] == '\0' )
    {   fprintf(wfp,"%s=%s\n",entry,buffer);
        do
        {
            fprintf(wfp,"%s\n",buff);
        } while( read_line(rfp,buff) );
    }
    else
    {   fprintf(wfp,"%s=%s\n",entry,buffer);
        while( read_line(rfp,buff) )
        {
             fprintf(wfp,"%s\n",buff);
        }
    }
    /* Clean up and rename */
    fclose(wfp);
    fclose(rfp);
    unlink(file_name);
    rename(tmp_name,file_name);
    return(1);
}
#endif
