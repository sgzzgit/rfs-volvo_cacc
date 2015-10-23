/**\file
**	garnet.h
**
**  Copyright (c) 2003    Regents of the University of California
**
** 	Initial revision
 */


#define GARNET_N_DIG_PORTS		6

typedef struct
{
	unsigned base;
	unsigned dig_dir_cfg;
	unsigned num_analog;		/*	Number of channels being converted.	*/
	int error;
	long dig_old_bits[GARNET_N_DIG_PORTS];
} garnet_typ;

garnet_typ *garnet_init( unsigned base );
bool_typ garnet_done( garnet_typ *pboard );

