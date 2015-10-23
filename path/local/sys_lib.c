/**\file	
 *	sys_lib.c
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 *
 *	DESCRIPTION
 *	General utilities.
 *
 */

#include <sys_os.h>
#include "local.h"

#define SYS_LIB_H
#include "sys_lib.h"

#define MAX_UNIQUE_CNT			0xffff
#define PRINT_BYTES_PER_LINE	16	/* For memory dumps.	*/

#ifdef RANGETEST
main()
{
	int i;
	double test, minimum, maximum, rand_range();

	printf( "Give the minimum and maximum\n" );
	scanf( "%lf %lf", &minimum, &maximum );
	for( i = 0; i < 10000; i++ )
	{
		test = rand_range( minimum, maximum);
		printf("%8lf\n",	test );
	}
}
#endif

#ifdef RANDTEST
/*	Random number generator test */
main()
{
	int i;
	double test;

	for( i = 0; i < 10000; i++ )
	{
		test = drand48();
		printf( "%lf\n", test );
	}
}
#endif

#ifdef READLINE_TEST	/* For testing the readline function. */ 
#include    <string.h>

static char *test_buf[]=
{
	"line number 1",
	"ignored",
	"123456789012345678901234567890",
	"123456789012345678901234567890",
	"12345678901234567890",
	"#xyz",
	"",
	" #",
	"         1         2         3",
	"         4         5         6",
	"         7         8",
	"" 
};
    
static char *ans_buf[]=
{
	"line number 1",
	"ignored",
	"12345678901234567890123456789",
	"0",
	"12345678901234567890123456789",
	"0",
	"12345678901234567890",
	"         1         2         ",
	"3",
	"         4         5         ",
	"6",
	"         7         8",
	"" 
};

main()
{
	int i;
	FILE *pfile;
	char pbuff[1000];
	int	size, cursor;
	char c;

	if( (pfile = fopen( "testfile" , "w+" )) == NULL )
	{
		printf("Couldn't open file: testfile.\n" );
		exit( 1 );
	}

	for( i = 0; i < 12; ++i )
		fprintf( pfile, "%s\n", test_buf[i] );

	rewind( pfile );
	
	cursor = 0;
	do  
	{
		size = readline( pfile, pbuff, 30 );
		printf( "	pbuff  ==>%s<==\n", pbuff );
		printf( "	numofchars in string ==>%d<==\n", size );

		if( strcmp( pbuff, ans_buf[cursor] ) == 0 )
			printf( "comparison good!\n" );
		else
			printf( "comparison bad!\n" );

		cursor++;
	} while( size != ERROR );
	system( "rm testfile" );
}

#endif  /* READLINE_TEST */


/*	SYNOPSIS
 *	#include "sys_lib.h"
 *	
 *	void bytecopy( n, pfrom, pto )
 *	unsigned n	-	number of bytes to be copyed
 *	char *pfrom	-	source buffer pointer
 *	char *pto	-	destination buffer pointer
 *
 *	DESCRIPTION
 *	Use for copying n bytes from the memory location pointed to by 
 *	pfrom to the memory location pointed to by pto.
 *
 *	RETURN
 *		None
 *
 *	BUGS
 *		In general, doesn't know about overlapping copies.
 *		Size is system dependent, and limited by int representation.
 */

void bytecopy( n, pfrom, pto )
unsigned n;
char *pfrom, *pto;
{
#ifdef BSD
	bcopy( pfrom, pto, (int) n );
#endif

#if (defined __QNX__) && (! defined __QNXNTO__)
	_fmemcpy( pto, pfrom, n );
#endif

#ifdef DOS
	memcpy( pto, pfrom, n );
#endif

#ifdef OSK
	unsigned i;
	for( i = 0; i < n; i++ )
		*pto++ = *pfrom++;
#endif
	memcpy(pto, pfrom, n);
}

/*	SYNOPSIS
 *	#include <sys_lib.h>
 *
 *	void error_exit( ps )
 *	char *ps	-	a pointer to a string containing the error messages
 *
 *	DESCRIPTION 
 *	Print out error message to stderr and exit.
 *
 *	RETURN
 *		None
 */

void error_exit( ps )
char *ps;
{
	fprintf( stderr, "%d %s", errno, ps );
	exit(1);
}

/*
 *	SYNOPSIS
 *	#include <sys_lib.h>
 *
 *	void sys_error( ptext, error_num, pname, linenum )
 *	char *ptext			error description
 *	int error_num		error number, usually errno.
 *	char *pname			name of the program where error occurred.
 *	int linenum			program line number.
 *
 *	DESCRIPTION
 *		Conditionally calls the function (*sys_err_func)() with
 *		the same arguement list, if the user has previously
 *		installed a handler into the sys_err_func global variable.
 *		sys_err_hand() is provided in this library for that use.
 *
 *		A typical call is:
 *			sys_error( "Error message", errno, __FILE__, __LINE__ );
 *
 *	RETURN
 *		Nothing.
 * 
 */ 

void sys_error( ptext, error_num, pname, linenum )
char *ptext;
int error_num;
char *pname;
int linenum;
{
// seems to be causing trouble on Linux and QNX 6 (Neutrino)
#if (defined __QNX__) && (! defined __QNXNTO__)
	if( sys_err_func != (void (*)()) NULL )
		(*sys_err_func)( ptext, error_num, pname, linenum );
#endif
}

/*
 *	SYNOPSIS
 *
 *	void sys_err_hand( ptext, error_num, pname, linenum )
 *	char *ptext			error description
 *	int error_num		error number, usually errno
 *	char *pname			name of the program where error occurred
 *	int linenum			program line number
 *
 *	DESCRIPTION
 *		Standard error function available for debugging errors.
 *
 *	RETURN
 *		Nothing.
 * 
 */ 

void sys_err_hand( ptext, error_num, pname, linenum )
char *ptext;
int error_num;
char *pname;
int linenum;
{
	fprintf( stderr, "ERROR: %s, No. : %d\n", ptext, error_num );
	fprintf( stderr, "FILE: %s, LINE : %d\n", pname, linenum );
}

/*	SYNOPSIS
 *	#include "sys_lib.h"
 *
 *	double drand48()
 *
 *	DESCRIPTION
 * 	Implementation of the function in the L'Ecuyer article in the
 * 	June 1988 issue of the Comm. of the ACM
 *
 *	RETURN
 *		A double random number, uniformly distributed (0.0,1.0)
 */

double drand48()
{
	/* Fixed seed values */ 

	static double seed1 = 127.0;		
	static double seed2 = 29345.0;		
	static double seed3 = 10999.0;
	int z, k;
	
	k = seed1/206;

	seed1 = 157 * (seed1 - k*206) - k*21;

	if( seed1 < 0.0)
		seed1 += 32363.0;

	k = seed2/217;

	seed2 = 146 * (seed2 - k *217) - k*45;

	if( seed2 < 0.0)
		seed2 += 31727.0;

	k = seed3/222;

	seed3 = 142 * (seed3 - k *222) - k*133;	

	if( seed3 < 0.0) 
		seed3 += 31657.0;

	z = seed1 - seed2;

	if( z > 706 ) 
		z -= 32362;

	z += seed3;

	if( z < 1 )
		z += 32362;

	return( z*3.0899e-5 );
}

/*	SYNOPSIS
 *	#include <sys_lib.h>
 *
 *	double rand_range( minimum, maximum )
 *	double minimum, maximum	- Noninclusive minimum and maximum values,
 *							  specifying the range of the random value.
 *
 *	DESCRIPTION
 *	Return a double in the noninclusive range (minimum, maximum).
 *	If maximum <= minimum, minimum is returned.
 *
 *	RETURN
 *	A random double value.
 */

double rand_range( minimum, maximum )
double minimum, maximum;
{
	double value;

	if( maximum <= minimum )
		return( minimum );
	else
	{
		value = (maximum - minimum) * drand48();
		return( minimum + value );
	}
}


/*	SYNOPSIS
 *
 *	#include <sys_lib.h>
 *
 *	double get_val( mean, range )
 *	double mean
 *	double range
 *
 *	DESCRIPTION
 *	Calculate the value of a random number given mean and range.
 *	The number is in the range (mean - range, mean + range).
 *
 *	RETURN
 *		A random double value.
 */

double get_val( mean, range )
double mean;
double range;
{
	double prob;

	prob = drand48();

	if( (drand48()) < (double)0.5 )
		prob = -prob;

	return( (double) (mean + prob*range) );
}			

/*
 *	SYNOPSIS
 *	#include <sys_lib.h>
 *
 *	int readline( FILE *pfin, char *pbuff, int size );
 *	*pfin 		-	pointer to file you're reading from.
 *	*pbuff		-	character array into which character are being read.
 *	 size		-	The size of the buffer.
 *					(counted from 1, ie. 1 .. size).
 *
 *	DESCRIPTION
 *
 *  This function attempts to read in a block of data of length "size" 
 *  form the file pointed to by "pfile".  The read operation is done 
 *  from the current position of the file pointer (in the file).  If 
 *  there are fewer characters than "size" left on the line (a line is 
 *  a string of characters terminated by a "/n" or an EOF character)
 *  then only those characters will be read in. 
 * 
 *	The integer value returned by the function will be equal to the 
 *  number of characters read in unless readline is called when the 
 *  file pointer is pointing directly at "EOF" then the function 
 *  will return a -1 (the system constant "ERROR" ).
 *
 *  The string of characters returned in pbuff will always be 
 *  terminated by a NULL character, "\O".  The first NULL character 
 *  in pbuff indicates the end of the string that was read in. 
 *  Be forewarned there may be more than one NULL character in pbuff, 
 *  but it is only the first one that is significant.  
 *  Any characters returned in the buffer after the first NULL should 
 *  be ignored.
 *
 *  If no characters are read in then the first character in pbuff 
 *  will be the NULL character and the function will return a value 
 *  of -1 (the system constant, "ERROR" ). 
 *
 *	RETURN
 *		ERROR	size <= 0	
 *		ERROR	EOF
 *		#char	String length.	
 *			
 *	BUGS
 *  If the value of size is selected such that it is less than the
 *  number of characters on the line pointed to by pfile then the next 
 *  time readline is called it will continue by grabbing characters 
 *  from the same line. 
 *  So it is possible that many "readline"s could be called on the same 
 *  line before all the characters on the line are exhausted.
 */

int readline( pfile, pbuff, size )
FILE *pfile;
char *pbuff;
int size;
{
	char *pcursor;
	int counter;  

	if( size <= 0 ) 
		return( ERROR );

	do						/*	Until got a valid line. */
	{						/*	fgets() will only read size - 1 characters.	*/
		if( fgets( pbuff, size, pfile ) == NULL )
			return( ERROR );

		pcursor = pbuff;
		for( counter = 0; counter < size; counter++, pcursor++ ) 
		{
			if( (*pcursor == '\n') || (*pcursor == '#') )
				break;
		}
		*pcursor = END_OF_STRING; 

	} while( allspace( pbuff ) == TRUE );

	return( counter );
}

#ifdef NAMETEST
int main( argc, argv )
int argc;
char *argv[];
{
	int save_old();

	if( argc != 2 )
	{
		printf( "%s: usage %s filename\n", argv[0], argv[0] );
		exit( 0 );
	}
	save_old( argv[1] );
}
#endif


int allspace( pc )
char *pc;
{
	while( *pc != END_OF_STRING )
	{
		if( isspace( *pc++ ) == FALSE )
			return( FALSE );
	}
	return( TRUE );
}

/*	SYNOPSIS
 *
 *	#include <sys_lib.h>
 *
 *	void sys_unique( ptag, punique )
 *	char *ptag			An identifier to build from, i.e. argv[0].
 *	char *punique		The returned string, which should allow a
 *						MAX_FILENAME_LEN result.
 *
 *	DESCRIPTION
 *	Build a unique string in the format:
 *
 *		TTTXXXXYYYY
 *
 *	where:	TTT 	is the user supplied tag.
 *			XXXX  	is the process id in hexadecimal.
 *			YYYY	is an internal counter which starts from 0,
 *					and may wrap around after 0xffff. 
 *
 *	Note that OS-9 event names are limited by the system 
 *	to 11 characters.
 */

void sys_unique( ptag, punique )
char *ptag, *punique;
{
	static long unique_cnt = 1;
	int pid;
	char buffer[MAX_FILENAME_LEN+1];

	pid = getpid();
	sprintf( buffer, "%s%04x%04lx", ptag, pid, unique_cnt++ );
	strncpy( punique, buffer, MAX_FILENAME_LEN );
	if( (unique_cnt < 1) || (MAX_UNIQUE_CNT < unique_cnt) )
		unique_cnt = 1;
}


/*	SYNOPSIS
 *
 *	#include <sys_lib.h>
 *
 *	int mem_cmp( pfrom, pto, n)
 *	char *pfrom			-	pointer to bytes for comparison
 *	char *pto			-	source bytes for comparison
 *	int n				-	number of bytes to be compared
 *
 *	DESCRIPTION
 *	This function compares two bytes arrays of size n .
 *	A FALSE is returned if the comparison fails.
 *
 *	RETURN
 *		TRUE		byte arrays are identical
 *		FALSE		byte arrays are different
 *
 */	

int mem_cmp( pfrom, pto, n )
char *pfrom, *pto;
int n;
{
	while( 0 < --n )
	{
		if( *pfrom++ != *pto++ )
			return( FALSE );
	}    
	return( TRUE );
}

char *mem_set( ps, c, n )
char *ps;
int c, n;
{
#if (defined BSD) 
	extern char *memset();
	return( memset( ps, c, n ) );
#endif

#if (defined __QNXNTO__)
	return( memset( ps, c, n ) );
#endif

#if (defined __QNX__) && (! defined __QNXNTO__)
	return( (char *) _fmemset( ps, c, n ) );
#endif

#ifdef DOS
	return( (char *) memset( ps, c, n ) );
#endif

#ifdef OSK
	while( 0 < --n )
		*ps++ = (char) c;

	return( ps );
#endif
}

/*	SYNOPSIS
 *
 *	#include <sys_lib.h>
 *
 *	void fd_set_iterate( fd_set *pfdset, void (*pffunc)( int fd ) );
 *	pfdset	-	File descriptor set mask over which to be iterated.
 *	pffunc	-	Function to be called for each mask which is set.
 *
 *	DESCRIPTION
 *	This function iterates over a file descriptor set.
 *	For each mask which is set, the given
 *	function is called with the appropriate file
 *	descriptor.
 *
 *	RETURN
 *		none
 *
 */	

void fd_set_iterate( pfdset, pffunc )
fd_set *pfdset;
void (*pffunc)();
{
	register int fd;

	if( pffunc == NULL )
		return;

	for( fd = 0; fd < FD_SETSIZE; fd++ )
	{
		if( FD_ISSET( fd, pfdset ) )
			(*pffunc)( fd );
	}
}

/*	SYNOPSIS
 *
 *	void print_fd_set( fd_set *pfdset );
 *	fd_set *pfdset		-	File descriptor set to be printed.
 *
 *	DESCRIPTION
 *	Print a simple representation of a file descriptor set.
 *
 *	RETURN
 *	none.
 *
 */

void print_fd_set( pfdset )
fd_set *pfdset;
{
	int i;
	int mask_size;

#ifdef BSD
	for( i = NOFILE - 1; i >= 0; i-- )
#endif
#ifdef OSK
	for( i = _NFILE - 1; i >= 0; i-- )
#endif
#ifdef __QNX__
	for( i = _NFILES - 1; i >= 0; i-- )
#endif
	{
		if( FD_ISSET( i, pfdset ) )
			printf( "1" );
		else
			printf( "0" );
	}
}

void print_bytes( address, count, pdata )
int address, count;
byte_typ *pdata;
{
	int i, j;

	printf( "         " );
	for( i = 0; i < PRINT_BYTES_PER_LINE; i++ )
	printf( "%02x  ", i );
	printf( "\n" );

	for( i = 0; i < count; i += PRINT_BYTES_PER_LINE )
	{
		printf( "0x%04x:  ", address + i );
		for( j = 0; j < PRINT_BYTES_PER_LINE; j++ )
		{
			if( count <= (i + j) )
				break;

			printf( "%02x  ", (int) *pdata++ );
		}
		printf( "\n" );
	}
}


/*	SYNOPSIS
 *
 *	#include <sys_lib.h>
 *
 *	byte_typ byte_to_bcd( byte_typ num );
 *
 *	num	-	Number to be converted.
 *
 *	DESCRIPTION
 *	Converts the input binary number into it's binary coded
 *	decimal representation.
 *
 *	RETURN
 *	A byte, which represents the input value in BCD format.
 *
 *	BUGS
 *	Does not check the input value for out of range values.
 */

byte_typ byte_to_bcd( num )
byte_typ num;
{
	return( ((num / 10) << 4) | (num % 10) );
}

