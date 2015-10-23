/**\file 
 *	dmm32.c     $Id: dmm32.c 6752 2009-11-20 03:03:14Z jspring $ 
 *
 *  dmm32.c   Functions for handling a Diamond-MM-32-AT card from
 *            Diamond Systems Corporation
 *	
 *  Copyright (c)2003   Regents of the University of California
 *
 *	$Log$
 *	Revision 1.3  2005/09/02 03:13:34  dickey
 *	Update before import of Quartz timer card driver.
 *	Modified Files:
 *	 	Makefile README das_default.c das_init.c das_man.c das_man.h
 *	 	das_util.c dmm32.c dmm32.h garnet.c io_func.c null_das.c
 *	 	realtime.ini
 *
 *	Revision 1.1  2004/09/20 19:47:08  dickey
 *	
 *	Check-in, null_das tested, garnet tested, interrupts and dmm34 not tested
 *	Modified Files:
 *	 	Makefile README das_default.c das_init.c das_man.c das_man.h
 *	 	das_util.c garnet.c io_func.c null_das.c realtime.ini
 *	 Added Files:
 *	 	dmm32.c dmm32.h garnet.h
 *	
 * 	
 */

#include <sys_qnx6.h>
#include <x86/inout.h>
#include <local.h>
#include <sys_mem.h>
#include <timestamp.h>
#include <das_clt.h>
#include "das_man.h"
#include "dmm32.h"

#undef DO_TRACE

/* Register offsets */
#define START_CONV       0x00
#define READ_AD_LSB      0x00
#define AUX_DIG_OUT      0x01
#define READ_AD_MSB      0x01
#define AD_LOW_CHANNEL   0x02
#define AD_HIGH_CHANNEL  0x03
#define DAC_LSB          0x04
#define AUX_DIG_IN       0x04
#define DAC_MSB          0x05
#define UPDATE_DACS      0x05
#define FIFO_DEPTH       0x06
#define FIFO_CONTROL     0x07
#define FIFO_STATUS      0x07
#define MISC_CONTROL     0x08
#define AD_STATUS        0x08
#define INT_AD_CLOCK     0x09
#define COUNTER_DIGIO    0x0a
#define ANALOG_CONF      0x0b
#define ANALOG_READBACK  0x0b

#define DIGIO_CONTROL   0x0f

static dmm32_typ *pmain_board;
static char enable=0;               /* Set =1 when conversions desired */
static char intID;               /* Interrupt ID for attaching & detaching ints*/
static int intr_cnt = 0;
static int intr_cnt_sav = 0;

/*	Buffer for A/D conversions.
 */

static int ad_buff[MAX_DMM32_ANALOG];

/*
 *	Create software timer
 */
static void timer_sw_init(IOFUNC_ATTR_T *pattr)
{
	int status;

    	/* Attach to the timer */
#ifdef DO_TRACE
    	printf("DMM32.C:timer_sw_set: SW timer synced to system HW clock\n");
#endif
    	status = timer_create(CLOCK_REALTIME, &pattr->tmr_pulse_event, 
			&pattr->ad_timer_id);

    	if(status == -1) {
		printf("Unable to attach timer.");
    	}
}

/*
 *	Set software timer, ticks are in milliseconds
 */
static void timer_sw_set(IOFUNC_ATTR_T *pattr, unsigned int ticks)
{
    	struct itimerspec timer;
#ifdef DO_TRACE
    	printf("DMM32.C:timer_sw_set: ticks=%d (msec)\n",ticks);	
#endif
    	
        timer.it_value.tv_sec = ticks/1000;	// 1000 ms/sec
        timer.it_value.tv_nsec = 1000000*(ticks % 1000); //1000000ns/ms
        timer.it_interval.tv_sec = ticks/1000;     // 1000 ms/sec
        timer.it_interval.tv_nsec = 1000000*(ticks % 1000); //1000000ns/ms

        timer_settime(pattr->ad_timer_id, 0, &timer, NULL);
}

int das_open_dev(IOFUNC_ATTR_T *pattr)
{
	das_info_typ *pinfo = &pattr->das_info;

	/* Stop any conversions. */
	enable = 0;

	if ((pmain_board = dmm32_init(*pinfo)) == NULL) {
		return(ERROR);
	}

	/// Create software timer and set to default of 20ms
	timer_sw_init(pattr);
	timer_sw_set(pattr, pinfo->ad_ticks);

	return(EOK);
}

void dmm32_das_close(resmgr_context_t *ctp, RESMGR_OCB_T *pocb)
{
	enable = 0;
	if(pmain_board != NULL) {
		das_ad_interrupt_detach(intID);
	}
}

/** das_func_init sets up the das_func_t function table pointed to
 *  by the device's IOFUNC_ATTR_T pointer.  
 */
int das_func_init(IOFUNC_ATTR_T *pattr)
{
	das_func_t *pfunc = &pattr->func;
#ifdef DO_TRACE
	printf("dmm32:initializing DAS function table\n");
#endif
	pfunc->da_term = das_default_da_term;
	pfunc->da_sync = das_default_da_sync;
	pfunc->tmr_mode = das_default_tmr_mode;
	pfunc->tmr_scan = das_default_tmr_scan;
	pfunc->tmr_read = das_default_tmr_read;
	pfunc->ad_pulse = das_default_ad_pulse;
	pfunc->ad_read = das_default_ad_read;
	pfunc->ad_set_scan = dmm32_ad_set_scan;
	pfunc->ad_term = dmm32_ad_term;
	pfunc->tmr_pulse = dmm32_tmr_pulse;
	pfunc->ad_set_sample = dmm32_ad_set_sample;
	pfunc->ad_enqueue = dmm32_ad_enqueue ;
	pfunc->digital_dir = dmm32_digital_dir;
	pfunc->digital_in = dmm32_digital_in;
	pfunc->digital_out = dmm32_digital_out;
	pfunc->das_close = dmm32_das_close;
	return (TRUE);
}

/**
 *  das_handle_interrupt services the pulse event sent when the interrupt
 *  associated with the device by InterruptAttachEvent in set_scan occurs
 */ 
void das_handle_interrupt(RESMGR_OCB_T *pocb)
{
        IOFUNC_ATTR_T *pattr = pocb->io_ocb.attr;
        das_info_typ *pinfo = &pattr->das_info;
	unsigned short msb;
	unsigned short lsb;
	int ad_count;
	static char unmaskcount = 0;

	++intr_cnt;

#ifdef DO_TRACE
	printf("DMM32.C:das_handle_interrupt: unmaskcount %d pinfo->irq %d intID%d\n",unmaskcount, pinfo->irq, intID);
	printf("DMM32.C:das_handle_interrupt: A/D readbacks:\n");
#endif

	for(ad_count = 0; ad_count < pmain_board->num_analog; ad_count++)
	{
	    lsb = in8(pmain_board->base + READ_AD_LSB);
	    msb = in8(pmain_board->base + READ_AD_MSB);
	    ad_buff[ad_count] = (msb << 8)+ lsb;
        /* Sign extend if necessary. */
        if ((ad_buff[ad_count] & 0x8000)!= 0)
            ad_buff[ad_count] = ad_buff[ad_count] | 0xffff0000;
#ifdef DO_TRACE
	printf(" ad_buff[%d] %d\n", ad_count, ad_buff[ad_count]);
#endif

	/* Unmask the interrupt, so we can get the next event */
	unmaskcount = InterruptUnmask (pinfo->irq, intID);

	/*  Reset interrupts by writing to INTRST bit of miscellaneous 
	**  control register. Does not affect any other bit settings
	**  in this register, such as board reset or page access */
	out8 (pmain_board->base + MISC_CONTROL, 0x08);

	}
}
char *labels[] =
{
        "A/D 7-0",
        "A/D 15-8",
        "A/D low channel",
        "A/D high channel",
        "Auxilary digital input",
        "FD9",
        "FIFO Depth",
        "FIFO Status",
        "Status",
        "Operation Status",
        "Counter/Timer",
        "Analog Configuration",
        "0",
        "1",
        "2",
        "3",
};                                                                

int dmm32_tmr_pulse (message_context_t *ctp, int code, unsigned flags, 
		void *ptr)
{	
	static int ctr = 0;
	static timestamp_t old_ts;
	static int millisec_elapsed = 0;
	unsigned char i;
	unsigned char page;

	if (ctr == 0) 
		get_current_timestamp(&old_ts);

	// Assuming tmr_pulse interval has been initialized to 20 milliseconds
	// LED should be low for 2 seconds, then high for two seconds
	// when there is an active client issuing the pulse.
	if( (intr_cnt == 10) || (intr_cnt_sav == intr_cnt) && (intr_cnt != 0) ) {
		timestamp_t ts;
#ifdef DO_TRACE
		printf("DMM32.C:dmm32_tmr_pulse: ad_timer_msg: start scan? (enable = %d)\n", enable);
#endif
		get_current_timestamp(&ts);
		time_diff(&old_ts, &ts, &millisec_elapsed);
		printf("dmm32: interrupt counter not changing at %d\n", intr_cnt);
		for (i = 0; i < 12; i++)
        	        printf("%2d: 0x%02x %s\n", i,in8(pmain_board->base + i), labels[i]);

		page = in8(pmain_board->base + 7) & 0x03;

		for (i = 12; i < 16; i++)
			printf("%2d: 0x%02x page %d.%s\n",  i, in8(pmain_board->base + i),
                                page, labels[i]);

		fflush(stdout);
		if( intr_cnt == intr_cnt_sav )
			exit(EXIT_FAILURE);
		}
	if (enable == 1){

		/* Start a scan by requesting an A/D conversion. */
		out8 (pmain_board->base + START_CONV, 0);

		if(!ctr){
//			toggle_LED();
#ifdef DO_TRACE
			/* Wait until scan is finished before looking at data */
			while((in8(pmain_board->base + AD_STATUS) & 0x80)>> 7);
			print_ad();

			/* Interrupt should have been sent after A/D conversion 			is finished */
			printf("DMM32.C:dmm32_tmr_pulse: Interrupt status register %#hhx (A/D interrupt is msb)\n",
        			in8(pmain_board->base + INT_AD_CLOCK) );           
#endif
		}
	}
	intr_cnt_sav = intr_cnt;
	return 0;
}

int toggle_LED(void) {

	static unsigned char aux_dig_out = 0;

	if(aux_dig_out) {
		aux_dig_out = 0x00;
		}
	else {
		aux_dig_out = 0x08;
		}
	out8 (pmain_board->base + AUX_DIG_OUT, aux_dig_out);
	return 0;
}

void print_ad(void) {

        int ad_count;

printf("DMM32.C:print_ad: A/D readbacks:\n");
        for(ad_count = 0; ad_count < pmain_board->num_analog; ad_count++)
        {
	    printf(" ad_buff[%d] %d\n", ad_count, ad_buff[ad_count]);
        }

}

dmm32_typ *dmm32_init(das_info_typ das_info)
{
        dmm32_typ *pboard;

	if((pboard = (dmm32_typ *)MALLOC(sizeof(dmm32_typ)))== NULL)
		return(NULL);

	pboard->num_analog = das_info.num_ai;
	pboard->error = 0;
	pboard->ad_status = 0;
	pboard->das_info = das_info;

	ThreadCtl(_NTO_TCTL_IO, NULL); /// required to access I/O ports
	pboard->base = mmap_device_io(8, das_info.port);

	/* Perform a full reset of the board. */
	out8(pboard->base + MISC_CONTROL, 0x20);

#ifdef DO_TRACE
	printf("dmm32_init: board reset port %#x\n", das_info.port);
#endif
	return(pboard);
}

/** Diamond-MM-32-A specific functions to set scan
 */
bool_typ dmm32_scan(dmm32_typ *pboard, char *ptable, int num_analog)
{
	char *pentry;

#ifdef DO_TRACE
	printf("dmm32_scan called, base address 0x%x, num entries %d\n",
		 pmain_board->base, num_analog);
#endif
	if((num_analog < MIN_DMM32_ANALOG)|| (MAX_DMM32_ANALOG < num_analog))
		return(FALSE);

	pboard->num_analog = num_analog;

	/* Reset the FIFO. */
	out8 (pboard->base + FIFO_CONTROL, 0x02);

	/* Set low channel and high channel for scans. */
	pentry = ptable;
	out8 (pboard->base + AD_LOW_CHANNEL, *pentry);
	pentry = pentry + (num_analog - 1);
	out8 (pboard->base + AD_HIGH_CHANNEL, *pentry);

	/* Set FIFO control register with FIFO enable = 1 (0x08)and
	 * Scan enable = 1 (0x04). */
	out8 (pboard->base + FIFO_CONTROL, 0x0c);

	/* Set FIFO depth register so an interrupt occurs at end of scan. */
	/* (divided by 2 because that's what the FIFO depth register wants) */
	out8 (pboard->base + FIFO_DEPTH, num_analog/2);

	/* Set Interrupt and A/D clock control register with A/D interrupt
	 * enable = 1 (0x80), clock enable = 0 (for conversions occur with
	 * software clock only), clock select = 0 (enable software triggers). */
	out8 (pboard->base + INT_AD_CLOCK, 0x80);

	/* Set analog configuration register for 10 usec between conversions,
	 * range of +/- 10 volts. */
	out8 (pboard->base + ANALOG_CONF, 0x28);
	while((in8(pboard->base + ANALOG_READBACK)& 0x80)>> 7);

	

#ifdef DO_TRACE
	printf("Low = %d set: %d\n",in8(pboard->base + AD_LOW_CHANNEL), *ptable);
	printf("High = %d set: %d\n",in8(pboard->base + AD_HIGH_CHANNEL), *pentry);
	printf("FIFO status register = %#x set: 0x0c\n",in8(pboard->base + FIFO_CONTROL));
	printf("FIFO depth = %d set: %d\n",in8(pboard->base + FIFO_DEPTH), num_analog/2);
	printf("Interrupt and A/D clock status = %#x set: 0x80\n",in8(pboard->base + INT_AD_CLOCK));
	printf("Analog configuration register = %#x set: 0x28\n",in8(pboard->base + ANALOG_CONF));

	printf("dmm32_scan complete, initialized.\n");

	/* Blink LED
	out8 (pboard->base + AUX_DIG_OUT, 0x00);
	sleep(1);
	out8 (pboard->base + AUX_DIG_OUT, 0x08);
	sleep(1);
	out8 (pboard->base + AUX_DIG_OUT, 0x00);
	sleep(1);
	out8 (pboard->base + AUX_DIG_OUT, 0x08);
	*/
#endif
	return(TRUE);
}

int dmm32_ad_set_scan(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
                        int num_entries)
{
        IOFUNC_ATTR_T *pattr = pocb->io_ocb.attr;
        das_info_typ *pinfo = &pattr->das_info;
        int status = EOK;


#ifdef DO_TRACE
        printf("enter dmm32_ad_set_scan, num_entries %d, irq %d\n",
                num_entries, pinfo->irq);
#endif
        if ((status=das_ad_read_scan_msg(ctp, pocb, num_entries)) == ERROR)
                return (ENOSYS);

	// May want to attach interrupt before calling dmm32_scan??

	(void) dmm32_scan(pmain_board, pocb->ad_seq.seq, num_entries); 

	if (pinfo->irq != 0)
		intID = das_ad_interrupt_attach(pattr);

	MsgReply(ctp->rcvid, EOK, NULL, 0);
	return (_RESMGR_NOREPLY);
}

bool_typ dmm32_done(dmm32_typ *pboard)
{
	/* Clear enable so no more conversions started on timer ticks. */
	enable = 0;

	if(pboard == NULL)
		return(FALSE);

	/* Write a RESETA (0x20)and INTRST (0x08)to the miscellaneous
	 * control register to reset everything. */
	out8(pboard->base + MISC_CONTROL, 0x28);

	FREE(pboard);
	printf("DMM32.C:dmm32_done:Exiting....\n");
	return(TRUE);
}

int das_ad_data(float *pdata, int n_data)
{
	return(dmm32_get_ad(pmain_board, pdata, n_data));
}

int dmm32_get_ad(dmm32_typ *pboard, float *pdest, int n_data)
{
	int n;

	n = bipolar_das_to_volt(&pboard->das_info, ad_buff,
			min(n_data, pboard->num_analog), pdest);

	return(n);
}

int dmm32_ad_term(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, int chnl)
{
	/* Clear enable so no more conversions performed on timer ticks. */	
	enable = 0;
	return (das_default_ad_term(ctp,pocb,chnl));
}

int dmm32_ad_set_sample(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			 int chnl, unsigned long ticks)
{
	IOFUNC_ATTR_T *pattr = pocb->io_ocb.attr;

#ifdef DO_TRACE
	printf("dmm32: das_ad_set_sample, ticks %ld\n", ticks);
#endif

	timer_sw_set(pattr, 20);
	return (EOK);

}

//int das_da_sync(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, int chnl, float data)
//{
/*********************
At the present time, the D/A calls have not been implemented for the
DMM32 series card.
The code below is QNX4 code for the ATMIOE board.
*********************/
//	return(ENOSYS);

//	int step;

//	das_to_step(&das_info, &data, 1, &step);

//	switch(chnl)
//	{
//	case ATMIOE_ANALOG_0_PORT:
//	    Board_Write(AO_DAC_0_Data_Register, step);
//	    break;

//	case ATMIOE_ANALOG_1_PORT:
//	    Board_Write(AO_DAC_1_Data_Register, step);
//	    break;
		
//	default:
//		errno = EINVAL;
//		return(ERROR);
//	}

//	return(EOK);
//}



/*
 *	Function das_ad_enqueue starts an AD operation (multiple conversions),
 *	which should already have been set up.
 */

int dmm32_ad_enqueue(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
		struct sigevent event)
{
	das_ad_start_enqueue(ctp->rcvid, pocb, event);
	printf("DMM32.C:dmm32_ad_enqueue:conversion requested; set enable=1\n");
/* Set enable=1 so conversions will occur on the next timer tick. */
	enable = 1;

	das_ad_finish_enqueue(pocb);

	return(EOK);
}
int dmm32_digital_in(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
                        int port, long *pbits)
{
	dmm32_get_dig( pmain_board, port, pbits);
#ifdef DO_TRACE
        printf("dmm32_digital_in %#x\n", (unsigned int)*pbits);
#endif
        return (EOK);
}

void dmm32_get_dig(dmm32_typ *pboard, int port, long *pbits) {
	int ctrlreg;

        /* Set page to 1 */
        out8(pboard->base + MISC_CONTROL, 0x01);
        /* Read port */
        *pbits = in8(pboard->base + port);
        ctrlreg = in8(pboard->base + DIGIO_CONTROL);
#ifdef DO_TRACE
        printf("dmm32_get_dig:port %#x bits %#x ctrlreg %#x\n", port, (unsigned int)*pbits, ctrlreg);
#endif
}

int dmm32_digital_dir(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, long mask)
{
#ifdef DO_TRACE
        printf("enter dmm32_digital_dir: mask %#x\n", (unsigned int)mask & 0xff);
#endif
	dmm32_set_dig_dir(pmain_board, (unsigned int)mask & 0xff); 
        return (EOK);
}

void dmm32_set_dig_dir(dmm32_typ *pboard, long mask) {

        /* Set page to 1 */
        out8(pboard->base + MISC_CONTROL, 0x01);
        /* Set digital direction of 3 ports */
        out8(pboard->base + DIGIO_CONTROL, mask & 0xff);
#ifdef DO_TRACE
        printf("dmm32_digital_set_dir %#x\n", (unsigned int)mask & 0xff);
#endif
}

int dmm32_digital_out(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
                        int port, long mask, long bits, long *pold_bits,
                        long *pnew_bits)
{
#ifdef DO_TRACE
        printf("enter dmm32_digital_out\n");
#endif
        dmm32_set_dig( pmain_board, port, bits);
        return (EOK);
}

void dmm32_set_dig(dmm32_typ *pboard, int port, long bits) {

        /* Set page to 1 */
        out8(pboard->base + MISC_CONTROL, 0x01);
        /* Set digital port */
        out8(pboard->base + port, bits & 0xff);
}

