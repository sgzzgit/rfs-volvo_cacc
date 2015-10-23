/**\file	
 *	static char rcsid[] = "$Id: sys_das.h 578 2005-08-19 17:28:22Z dickey $";
 *
 * Copyright (c) 2003   Regents of the University of California
 *
 *
 *	Definitions for interface to DAS I/O manager.
 *	Modified from QNX4 version for QNX6
 *
 *	$Log$
 *	Revision 1.3  2005/08/19 17:28:22  dickey
 *	Changes made while porting Jbus code to QNX6 and adding Lane Assist Interface
 *
 *	Committing in .
 *
 *	Modified Files:
 *		Makefile sys_buff.c sys_das.c sys_das.h sys_qnx6.h sys_rt.h
 *		timestamp.c timestamp.h
 *
 *	Revision 1.2  2004/11/01 20:47:27  dickey
 *	Change in declaration of das_digital_dir, for multiple ports.
 *	 Modified Files:
 *	 	sys_das.h
 *	
 *	Revision 1.1.1.1  2004/08/26 23:45:04  dickey
 *	local for IDS, CCW, CACC, etc.
 *	
 *
 */
#ifndef SYS_DAS_H
#define SYS_DAS_H

#include <db_comm.h>
#include "das_clt.h"

#define DAS_MAX_CHANNEL      		64
#define DAS_DEFAULT_PATH		"/dev/das"

/** Structures and macros for scaling voltage
 */
typedef struct
{
        double offset;                /* This is in volts. */
        double scale;                 /* This is in value/volts. */
} das_io_scale_typ;

#define SCALE_TO_VALUE(volts,pscale)    (((volts)-((pscale)->offset))\
                                            *((pscale)->scale))
#define SCALE_TO_VOLT(value,pscale)     ((value)/((pscale)->scale)\
                                            +((pscale)->offset))

typedef struct
{
	int fd;		/* from open of device name, used as pulse code */
	unsigned long ticks;
	int num_scan;
	int result_size;
	int num_analog;         /* Maximum analog input channels. */
	float ad_min, ad_max;
	float da_min, da_max;
	char *pinput_request;
	int chid;      	/* used to get connection id for device to send pulse */
	float volts[DAS_MAX_CHANNEL];
} das_typ;

/**	Client functions for DAS library.
 */

extern int das_open(const char *path, int oflag);
extern int das_get_info(int fd, das_info_typ *pinfo);
extern int das_close(int fd);

/** Digital-to-analog
 */
extern int das_da_sync(int fd, int chnl, float data);
extern bool_typ das_set_da(das_typ *pdas, int channel, float value);
extern int das_da_term(das_typ *pdas, int chnl);

/** Digital I/O
 */
extern int das_digital_dir(int fd, short int port, long bits);
extern int das_digital_in(int fd, int port, long *pbits);
extern int das_digital_out(int fd, int port, long bits, long mask,
		long *pold_bits, long *pnew_bits);

/** Analog-to-digital
 */
extern int das_ad_enqueue(das_typ *pdas);
extern int das_ad_read(das_typ *pdas, float *pbuf, int nchan);
extern int das_ad_set_scan(int fd, char *pscanlist, int n_entries);
extern int das_ad_set_sample(int fd, int chnl, unsigned long ticks);
extern int das_ad_term(das_typ *pdas, int chnl);
extern int das_ad_pulse(das_typ *pdas);

/** Timer/counter
 */
extern int das_tmr_mode(int fd, unsigned timer, unsigned mode, unsigned value);
extern int das_tmr_scan(int fd, char *ptimers, int num_entries);
extern int das_tmr_read(int fd, unsigned *pdata, int count);

/** Macros used to access client functions.
 */
#define DAS_AD_ENQUEUE(pdas)    das_ad_enqueue(pdas)

#define DAS_AD_PULSE(pdas, pmsg, pmsginfo )     das_ad_pulse(pdas, pmsg, pmsginfo) 

#define DAS_AD_READ(pdas)       das_ad_read(pdas, \
					 pdas->volts, pdas->result_size)

#define DAS_AD_TERM(pdas)       das_ad_term(pdas->fd, 0)
#define DAS_DA_TERM(pdas,n)     das_da_term(pdas->fd, n)
#define	DAS_TMR_MODE(pdas,timer,mode,value)\
                                das_tmr_mode(pdas->fd, timer, mode, value)

#define	DAS_TMR_SCAN(pdas,list,size)\
                                das_tmr_scan(pdas->fd, list, size)

#define	DAS_TMR_READ(pdas,data,size)\
                                das_tmr_read(pdas->fd, data, size)

#define	DAS_DIGITAL_IN(pdas,port,pbuf)\
                                das_digital_in(pdas->fd, port, pbuf)
#define DAS_DIGITAL_DIR(pdas,port,bits)\
                                das_digital_dir(pdas->fd, port, bits)
#define DAS_DIGITAL_OUT(pdas,port,bits,mask,pold_bits,pnew_bits)\
            das_digital_out(pdas->fd, port, bits, mask, pold_bits, pnew_bits)
#define DAS_GET_VOLTS(pdas)     (pdas->volts)

das_typ *das_init(char *ppath, unsigned long sample_rate,
	char *pscan, int num_scan, int chid);
bool_typ das_done(das_typ *pdas);
bool_typ das_set_da(das_typ *pdas, int channel, float value);

#endif
