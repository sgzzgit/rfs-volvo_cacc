/**\file 
**	garnet.c     $Id: garnet.c 6710 2009-11-11 01:52:27Z dickey $ 
**
**  Copyright (c) 2003   Regents of the University of California
**
**	$Log$
**	Revision 1.4  2005/09/02 03:13:34  dickey
**	Update before import of Quartz timer card driver.
**	Modified Files:
**	 	Makefile README das_default.c das_init.c das_man.c das_man.h
**	 	das_util.c dmm32.c dmm32.h garnet.c io_func.c null_das.c
**	 	realtime.ini
**
**	Revision 1.2  2004/09/20 19:47:08  dickey
**	
**	Check-in, null_das tested, garnet tested, interrupts and dmm34 not tested
**	Modified Files:
**	 	Makefile README das_default.c das_init.c das_man.c das_man.h
**	 	das_util.c garnet.c io_func.c null_das.c realtime.ini
**	 Added Files:
**	 	dmm32.c dmm32.h garnet.h
**	
** 	
 */

#include <sys_qnx6.h>
#include <x86/inout.h>
#include <local.h>
#include <sys_mem.h>
#include <das_clt.h>
#include "das_man.h"
#include "garnet.h"


/* Number of Digital ports */
#define DIG_N_PORTS					0x6


/*	Register offsets.  */

	/* DIG IO registers */
	/* Read & Write*/
#define DIG_0_ADDR                  0x0
#define DIG_CTRL1_ADDR              0x3
#define DIG_CTRL2_ADDR              0x7

#define DIG_PA1_ADDR                0x0
#define DIG_PB1_ADDR                0x1
#define DIG_PC1_AADDR               0x2
#define DIG_PA2_ADDR                0x4
#define DIG_PB2_ADDR                0x5
#define DIG_PC2_AADDR               0x6


/*	BIT definitions.  */

#define DIG_0_IN                    0x010
#define DIG_1_IN                    0x002
#define DIG_2_IN                    0x009


#define DIG_DIR_IN                  1
#define DIG_DIR_OUT                 0

/*	Function Macros  */

/*	Digital IO interface  */
#define DIG_IN(pboard, port)    in8( (pboard)->base + DIG_0_ADDR + (port) +(port)/3 )

#define DIG_OUT(pboard, port, value)\
                      out8( (pboard)->base + DIG_0_ADDR + (port) + (port)/3, (value) )
	
static void garnet_reset( garnet_typ *pboard );

static garnet_typ *pmain_board;

int das_open_dev( IOFUNC_ATTR_T *pattr )
{
	das_info_typ *pinfo = &pattr->das_info;

	if( (pmain_board = garnet_init( pinfo->port )) == NULL )
	{
		return( ERROR );
	}
      
	return( EOK );
}

void das_close( resmgr_context_t *ctp, RESMGR_OCB_T *pocb)
{
	if ( pmain_board != NULL )
		garnet_done( pmain_board );

}


/* +++++++++++++++++++++++ */

garnet_typ *garnet_init( unsigned base )
{
	garnet_typ *pboard;

	if( (pboard = (garnet_typ *) MALLOC( sizeof( garnet_typ ))) == NULL )
		return( NULL );

	pboard->error = 0;
	pboard->dig_old_bits[0] = 0;
	pboard->dig_old_bits[1] = 0;
	pboard->dig_old_bits[2] = 0;
	pboard->dig_old_bits[3] = 0;
	pboard->dig_old_bits[4] = 0;
	pboard->dig_old_bits[5] = 0;

	ThreadCtl(_NTO_TCTL_IO, NULL); /// required to access I/O ports
	pboard->base = mmap_device_io(8, base);

	garnet_reset( pboard );

	return( pboard );
}


bool_typ garnet_done( garnet_typ *pboard )
{
	if( pboard == NULL )
		return( FALSE );

	garnet_reset( pboard );
	FREE( pboard );
	return( TRUE );
}

static void garnet_reset( garnet_typ *pboard )
{

	/* Digital I/O config - ALL ports as input */
	out8( pboard->base + DIG_CTRL1_ADDR, (0x80 | DIG_0_IN | DIG_1_IN | DIG_2_IN ));
	out8( pboard->base + DIG_CTRL2_ADDR, (0x80 | DIG_0_IN | DIG_1_IN | DIG_2_IN ));

}


int das_dig_dir_msg( resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		long mask )
{
	unsigned char cfg_mask;

	//Save digital dir configuration
	pmain_board->dig_dir_cfg=mask ;

	 
	cfg_mask = ( 0x80 | 
	     ( (mask & 0x01 ) == 0 ? DIG_0_IN : 0) |
	     ( (mask & 0x02 ) == 0 ? DIG_1_IN : 0) |
	     ( (mask & 0x04 ) == 0 ? DIG_2_IN : 0)  );
	out8( pmain_board->base + DIG_CTRL1_ADDR, cfg_mask );
	cfg_mask = ( 0x80 | 
	     ( (mask & 0x08 ) == 0 ? DIG_0_IN : 0) |
	     ( (mask & 0x10 ) == 0 ? DIG_1_IN : 0) |
	     ( (mask & 0x20 ) == 0 ? DIG_2_IN : 0)  );
	out8( pmain_board->base + DIG_CTRL2_ADDR, cfg_mask );

	return( EOK );
}


int das_dig_in_msg( resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		int port, long *pbits )
{
	//TO DO: verify port argument, and if it's configured for input
printf("garnet dig_in, port %d\n", port);
	if (port >= DIG_N_PORTS) 
		  {
	           return( ENOSYS );
	          }
	*pbits = DIG_IN( pmain_board, port );
	return( EOK );
}

int das_dig_out_msg( resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		int port, long mask, long bits,
		long *pold_bits, long *pnew_bits )
{
	long old_bits;
	long new_bits;

	//TO DO: verify port argument, and if it's configured for output

	if ( (port >= DIG_N_PORTS) ||
		    ( (pmain_board->dig_dir_cfg & (0x01<< port) )== 0) )
		  {
	           return( ENOSYS );
	          }

	old_bits=pmain_board->dig_old_bits[port];

	new_bits = ( old_bits & ~mask ) |
	           ( bits & mask );
	pmain_board->dig_old_bits[port]=new_bits;

	DIG_OUT( pmain_board, port, new_bits );
	
	*pold_bits = old_bits;
	*pnew_bits = new_bits;

	return( EOK );
}

#define DEBUG

/** das_func_init sets up the das_func_t function table pointed to
 *  by the device's IOFUNC_ATTR_T pointer.  
 */
int das_func_init(IOFUNC_ATTR_T *pattr)
{
	das_func_t *pfunc = &pattr->func;
#ifdef DEBUG
	printf("initializing DAS function table\n");
#endif

	pfunc->digital_dir = das_dig_dir_msg;;
	pfunc->digital_in = das_dig_in_msg;;
	pfunc->digital_out = das_dig_out_msg;
	pfunc->da_term = das_default_da_term;
	pfunc->da_sync = das_default_da_sync;
	pfunc->tmr_mode = das_default_tmr_mode;
	pfunc->tmr_scan = das_default_tmr_scan;
	pfunc->tmr_read = das_default_tmr_read;
	pfunc->tmr_pulse = NULL;
	pfunc->ad_set_sample = das_default_ad_set_sample;
	pfunc->ad_enqueue = das_default_ad_enqueue ;
	pfunc->ad_pulse = das_default_ad_pulse;
	pfunc->ad_read = das_default_ad_read;
	pfunc->ad_set_scan = das_default_ad_set_scan;
	pfunc->ad_term = das_default_ad_term;
	pfunc->das_close = das_close;
	return (TRUE);
}

/**
 *  das_handle_interrupt services the pulse event sent when the interrupt
 *  associated with the device by InterruptAttachEvent in set_scan occurs
 */ 
void das_handle_interrupt(RESMGR_OCB_T *pocb)
{
#ifdef DEBUG
	printf("No garnet interrupt!\n");
#endif
} 

/**
 *  das_ad_data copies analog data into a floating point array,
 *	using the correct transformation for the device.
 */
int das_ad_data(float *pdata, int n_data)
{
#ifdef DEBUG
	printf("No ad data\n");
#endif
	return (0);	/// garnet doesn't implement this 
}

