/**\file	
 *	dmm32.h    Header file for Diamond-MM-32_AT Board
 *
 *
 * Copyright (c) 2005   Regents of the University of California
 *
 */

#ifndef DMM32_H
#define DMM32_H

#define MIN_DMM32_ANALOG			2
#define MAX_DMM32_ANALOG			32

// Board type used internally in dmm32.c
typedef struct
{
	unsigned base;
	unsigned num_analog;	/* Number of channels being converted. */
	unsigned ad_status;
	das_info_typ das_info;
	int error;

} dmm32_typ;

// Functions supplied by dmm32.c, referenced by DAS common code
extern int das_open_dev(IOFUNC_ATTR_T *pattr);
//extern void das_close(resmgr_context_t *ctp, RESMGR_OCB_T *pocb);
extern int das_func_init(IOFUNC_ATTR_T *pattr);
extern void das_handle_interrupt(RESMGR_OCB_T *pocb);
extern int das_ad_data(float *pdata, int n_data);

// Utility functions used within dmm32.c
extern dmm32_typ *dmm32_init(das_info_typ das_info);
extern bool_typ dmm32_scan(dmm32_typ *pboard, char *ptable, int num_analog);
extern bool_typ dmm32_done(dmm32_typ *pboard);
extern int dmm32_get_ad(dmm32_typ *pboard, float *pdest, int n_data);
extern void dmm32_get_dig(dmm32_typ *pboard, int port, long *pbits);
extern void dmm32_set_dig(dmm32_typ *pboard, int port, long bits);
extern int toggle_LED(void);
extern void print_ad(void);
extern void dmm32_set_dig_dir(dmm32_typ *pboard, long mask); 

// Functions assigned to DAS function pointers by das_func_init
extern int dmm32_tmr_pulse (message_context_t *ctp, int code, unsigned flags,
		 void *pocb);
extern int dmm32_ad_term(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, int chnl);
extern int dmm32_ad_set_scan(resmgr_context_t *ctp, RESMGR_OCB_T *pocb ,
			  int n_entries);
extern int dmm32_ad_set_sample(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			 int chnl, unsigned long ticks);
extern int dmm32_ad_enqueue(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
			struct sigevent event);
extern int dmm32_da_sync(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
			 int chnl, float data);
extern int dmm32_digital_in(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
                        int port, long *pbits);
extern int dmm32_digital_dir(resmgr_context_t *ctp, RESMGR_OCB_T *pocb, 
			long mask);
extern int dmm32_digital_out(resmgr_context_t *ctp, RESMGR_OCB_T *pocb,
                        int port, long mask, long bits, long *pold_bits,
                        long *pnew_bits);
#endif //DMM32_H
