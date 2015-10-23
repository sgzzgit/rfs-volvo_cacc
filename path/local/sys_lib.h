/**\file	
 *	sys_lib.h
 *
 * Copyright (c) 2006   Regents of the University of California
 *
 */
#ifndef PATH_SYS_LIB_H
#define PATH_SYS_LIB_H	// in case it is included twice

int	allspace( char *pc );
void bytecopy( unsigned n, char *pfrom, char *pto );
double drand48( void );
void error_exit( char *ps );
double get_val( double mean, double variance );
int	mem_cmp( char *pfrom, char *pto, int n );
char *mem_set( char *ps, int c, int n );
double rand_range( double minimum, double maximum );
int	readline( FILE *pfin, char *pbuff, int maximum );
void sys_err_hand( char *ptext, int error_num, char *pname, int linenum );
void sys_error( char *ptext, int error_num, char *pname, int linenum );
void sys_unique( char *ptag, char *punique );
void print_fd_set( fd_set *pfdset );
void fd_set_iterate( fd_set *pfdset, void (*pffunc)( int fd ) );
void print_bytes( int address, int count, byte_typ *pdata );
byte_typ byte_to_bcd( byte_typ num );
#if (defined __QNX__) && (! defined __QNXNTO__)
void (*sys_err_func)() = (void (*)()) NULL;
#endif

#endif

