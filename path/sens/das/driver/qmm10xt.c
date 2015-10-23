/**\file
**	Port to QNX6 of driver for Diamond Systems Quartz counter/timer board
**      of driver originally written for QNX4 by José Miguel S. Almeida, 2002
**      ISEP, Porto,  Portugal
**
 */

#include <sys_qnx6.h>
#include <x86/inout.h>
#include <local.h>
#include <sys_mem.h>
#include "das_clt.h"
#include "das_man.h"
#include "am9513.h"
#include "qmm10xt.h"

#undef DO_TRACE

/*	Register offsets.  */
#define CTS9513_1					0x0
#define DIG_IN						0x2
#define DIG_OUT						0x2
#define CTS9513_2					0x4
#define INT_RESET					0x6
#define INT_ENABLE					0x6



/*	BIT definitions.  */

/*	Interrupt Enable Register
 *	(Note: this driver currently disables interrupts
 */
#define INTON					0x01
#define INTOFF					0x00

#define QMM10XT_INT_RESET(pboard)		inp( pboard->base +INT_RESET  )

#define QMM10XT_INT_ENABLE(pboard)\
                                    out16( pboard->base + INT_ENABLE, INTON )

#define QMM10XT_INT_DISABLE(pboard)\
                                    out16( pboard->base + INT_ENABLE, INTOFF )

#define QMM10XT_DIG_IN(pboard)		in16( pboard->base + DIG_IN )

#define QMM10XT_DIG_OUT(pboard,value)\
                                    out16( pboard->base + DIG_OUT, value )

static void qmm10xt_reset( qmm10xt_typ *pboard );

static qmm10xt_typ *pmain_board;

static int int_id;

int das_open_dev( IOFUNC_ATTR_T *pattr )
{
	das_info_typ *pinfo = &pattr->das_info;
	if( (pmain_board = qmm10xt_init( pinfo->port)) == NULL )
	{
		return( ERROR );
		}
#ifdef DO_TRACE
		printf("das_open_dev: board 0x%x, base 0x%x\n",
				pmain_board, pmain_board->base);
#endif

	return( EOK );
}

void das_close( resmgr_context_t *ctp, RESMGR_OCB_T *pocb )
{
	if( pmain_board != NULL )
	{
		qmm10xt_done( pmain_board );
	}
}


qmm10xt_typ *qmm10xt_init( unsigned base)
{
	qmm10xt_typ *pboard;

	if( (pboard = (qmm10xt_typ *) MALLOC( sizeof( qmm10xt_typ ))) == NULL )
		return( NULL );

	printf("Preparing to map QMM board at %03x\n", base);
	fflush(stdout);

        ThreadCtl(_NTO_TCTL_IO, NULL); /// required to access I/O ports
        pboard->base = mmap_device_io(8, base); //maps 8 bytes of device space
	
	printf("QMM board mapped, base 0x%x\n", base);
	fflush(stdout);

	pboard->error = 0;

	pboard->dig_old_bits= 0;

	QMM10XT_INT_DISABLE(pboard);

	if( (pboard->ptimer[0] = am9513_init( base + CTS9513_1, TRUE )) == NULL )
	{
		FREE( pboard );
		return( NULL );
	}

	if( (pboard->ptimer[1] = am9513_init( base + CTS9513_2, TRUE )) == NULL )
	{
		am9513_done( pboard->ptimer[0] );
		FREE( pboard );
		return( NULL );
	}
	qmm10xt_reset( pboard );

	return( pboard );
}


bool_typ qmm10xt_done( qmm10xt_typ *pboard )
{
	if( pboard == NULL )
		return( FALSE );

	qmm10xt_reset( pboard );
	am9513_done( pboard->ptimer[0] );
	am9513_done( pboard->ptimer[1] );
	FREE( pboard );
	return( TRUE );
}

static void qmm10xt_reset( qmm10xt_typ *pboard )
{

}


/*	Enable the data acquisition operation in scan mode */
bool_typ qmm10xt_enable( qmm10xt_typ *pboard )
{
	if( pboard == NULL )
		return( FALSE );

	return( TRUE );
}


int das_dig_in_msg( resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			int port, long *pbits )
{

/* 	reading the output port in the qmm10xxt board, return always 0xff,
* 	using the data last outputed by the driver
	*/

	if(port==QMM10XT_DIG_IN_PORT)
	 *pbits = QMM10XT_DIG_IN( pmain_board );
	else
	 *pbits = pmain_board->dig_old_bits;  
	 
#ifdef DO_TRACE
	printf("qmm10xt: DIG_IN %d\n", *pbits);
#endif
	return( EOK );
}

int das_dig_out_msg( resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
		int port, long mask, long bits,
		long *pold_bits, long *pnew_bits )
{
	long old_bits;
	long new_bits;
	
	// Write to an input port => Error
	if(port==QMM10XT_DIG_IN_PORT)
	{
	 return( ENOSYS );
	}

	old_bits=pmain_board->dig_old_bits;   
	new_bits = ( old_bits & ~mask ) |
			( bits & mask );
	pmain_board->dig_old_bits=new_bits;
	
	*pold_bits = old_bits;
	*pnew_bits = new_bits;
	QMM10XT_DIG_OUT( pmain_board, new_bits );
	
	return( EOK );
}


// Timer/counters functions
int das_tmr_mode_msg( resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			unsigned timer, unsigned mode, unsigned value)
{
	unsigned int am9513_number=0;
 
	if ( timer > QMM10XT_TIMER_5 )
	 {
		timer=timer-(QMM10XT_TIMER_5+1);
		am9513_number=1;
	 }
	 
	if( am9513_mode( pmain_board->ptimer[am9513_number],
			       	timer, mode, value ) == TRUE )
	{
		return( EOK );
	}
	else
		return( EINVAL );
}

 
bool_typ qmm10xt_tmr_set_scan( qmm10xt_typ *pboard, char const *plist, int num_scan )
{
	int i;
	unsigned int timer, am9513_id;

        if( QMM10XT_N_TIMERS < num_scan )
		 return( FALSE );  

        for( i = 0; i < num_scan; i++, plist++ )
         {
		pboard->tmr_scan_list[i]=div(*plist, QMM10XT_TIMER_5+1);

                am9513_id = GET_9513_ID(pboard->tmr_scan_list[i]);
                timer = GET_TIMER_ID(pboard->tmr_scan_list[i]);
		
                if ( (QMM10XT_N_9513 -1)< am9513_id )
                  {
                         pboard->num_scan = 0;
                        return( FALSE );
                  }
         }
        pboard->num_scan = num_scan;
        return( TRUE );
}


unsigned qmm10xt_tmr_get_scan( qmm10xt_typ *pboard, unsigned *pdata, int num_scan )
{
        unsigned i;
        int max_scan;
	unsigned int timer, am9513_id;
			 
        max_scan = min( pboard->num_scan, num_scan );
				 
        for( i = 0; i < max_scan; i++, pdata++ )
	  {
                am9513_id = GET_9513_ID(pboard->tmr_scan_list[i]);
                timer = GET_TIMER_ID(pboard->tmr_scan_list[i]);
                *pdata = am9513_get_counter( pboard->ptimer[am9513_id], timer );
	  }
        return( i );
}


int das_tmr_scan_msg( resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			int num_entries )
{
	int num_read;
	char plist[QMM10XT_N_TIMERS];	// hold scan list read from second part

	
	num_read = MsgRead(ctp->rcvid, plist, num_entries, sizeof(das_msg_t));
	
	if( qmm10xt_tmr_set_scan( pmain_board, plist, num_entries ) == TRUE )
		return ( EOK );
	else
		return ( EINVAL );
}


int das_tmr_read_msg( resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			int num_entries)
{
	static das_msg_t msg;           /// don't allocate on stack
	static int pdata[QMM10XT_N_TIMERS];
	unsigned int count;

	if ( ( count = qmm10xt_tmr_get_scan( pmain_board, pdata, num_entries ) )
			 != num_entries ) 
	{
		fprintf(stderr, "TMR_READ: count %d driver count %d\n",
				num_entries, count);
	}

        /*
         *       Setup message reply header and data buffer.
         */
        msg.n = count;          /// tells client how many were read
#undef DO_TEST
#ifdef DO_TEST
	{
		int i;
		for (i = 0; i < num_entries; i++)
			pdata[i] = i * 1000;
	}
#endif
        SETIOV(&ctp->iov[0], &msg, sizeof(msg));
        SETIOV(&ctp->iov[1], pdata, count * sizeof(unsigned));

        MsgReplyv(ctp->rcvid, EOK, ctp->iov, 2);

        return (_RESMGR_NOREPLY);
}


/** das_func_init sets up the das_func_t function table pointed to
 *  by the device's IOFUNC_ATTR_T pointer. Unimplemented functions
 *  use das_default version.  
 */
int das_func_init(IOFUNC_ATTR_T *pattr)
{
	das_func_t *pfunc = &pattr->func;
#ifdef DO_TRACE
	printf("initializing DAS function table\n");
#endif

	pfunc->digital_dir = das_default_digital_dir; //QMM10XT has fixed ports
	pfunc->digital_in = das_dig_in_msg;
	pfunc->digital_out = das_dig_out_msg;
	pfunc->da_term = das_default_da_term;
	pfunc->da_sync = das_default_da_sync;
	pfunc->tmr_mode = das_tmr_mode_msg;
	pfunc->tmr_scan = das_tmr_scan_msg;
	pfunc->tmr_read = das_tmr_read_msg;
	pfunc->ad_set_sample = das_default_ad_set_sample;
	pfunc->ad_enqueue = das_default_ad_enqueue ;
	pfunc->ad_pulse = das_default_ad_pulse;
	pfunc->ad_read = das_default_ad_read;
	pfunc->ad_set_scan = das_default_ad_set_scan;
	pfunc->ad_term = das_default_ad_term;
	return (TRUE);
}

/**
 *  das_handle_interrupt services the pulse event sent when the interrupt
 *  associated with the device by InterruptAttachEvent in set_scan occurs
 *  Interrupts not implemented for this device, but need interface.
 */ 
void das_handle_interrupt(RESMGR_OCB_T *pocb)
{
#ifdef DO_TRACE
	printf("enter das_handle_interrupt\n");
#endif
} 

/**
 *	Not needed for this device.
 *  das_ad_data copies analog data into a floating point array,
 *	using the correct transformation for the device.
 *	returns the number of conversions
 */
int das_ad_data(float *pdata, int n_data)
{
#ifdef DO_TRACE
	printf("enter das_ad_data\n");
#endif
	return (n_data);
}

