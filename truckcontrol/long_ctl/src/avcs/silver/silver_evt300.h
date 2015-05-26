/* FILE:  evt300.h (For the Intersection Decision Support project)
 *
 * Copyright (c) 2001, 2003  Regents of the University of California
 *
 * static char rcsid[] = "$Id: evt300.h 760 2007-02-07 22:59:30Z jspring $";
 *
 *
 * $Log$
 * Revision 1.4  2007/02/07 22:59:30  jspring
 * Made all message types IS_PACKEDl
 *
 * Revision 1.3  2007/02/01 03:54:30  jspring
 * Minor changes, commenting added.
 *
 * Revision 1.2  2007/01/20 03:40:33  jspring
 * Added ddu_disp_mess_typ struct for DDU display.
 *
 * Revision 1.1.1.1  2006/11/17 00:54:47  dickey
 * On Board Monitoring System
 *
 * Revision 1.2  2004/09/07 21:35:07  dickey
 *
 * Updated from cabinet Sep 7, 2004.
 * Paul's changes to ids_io.h and clt_vars.h
 * Added Joel's files alert.c and alert.h
 *
 *
 */

#include <sys_os.h>

typedef struct
{
	char          msgID;            /* 82 for front end targer report message */
	char          FFTframe;         /* LS 8 bits of FFT Frame Number */
	char          targ_count;       /* Number of targets (0-7) */
	unsigned char target_1_id;      /* Target ID number (1-255) */
	short int     target_1_range;   /* Range, LSB = 0.1 ft */
	short int     target_1_rate;    /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_1_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_1_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_1_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                             * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7. */
	unsigned char target_2_id;      /* Target ID number (1-255) */
	short int     target_2_range;   /* Range, LSB = 0.1 ft */
	short int     target_2_rate;    /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_2_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_2_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_2_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                             * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7. */
	unsigned char target_3_id;      /* Target ID number (1-255) */
	short int     target_3_range;   /* Range, LSB = 0.1 ft */
	short int     target_3_rate;    /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_3_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_3_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_3_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                             * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	unsigned char target_4_id;      /* Target ID number (1-255) */
	short int     target_4_range;   /* Range, LSB = 0.1 ft */
	short int     target_4_rate;    /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_4_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_4_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_4_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                             * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	unsigned char target_5_id;      /* Target ID number (1-255) */
	short int     target_5_range;   /* Range, LSB = 0.1 ft */
	short int     target_5_rate;    /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_5_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_5_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_5_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                             * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	unsigned char target_6_id;      /* Target ID number (1-255) */
	short int     target_6_range;   /* Range, LSB = 0.1 ft */
	short int     target_6_rate;    /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_6_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_6_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_6_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                             * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	unsigned char target_7_id;      /* Target ID number (1-255) */
	short int     target_7_range;   /* Range, LSB = 0.1 ft */
	short int     target_7_rate;    /* Relative velocity, LSB = 0.1 ft/sec */
	signed char   target_7_azimuth; /* Azimuth, LSB = 0.002 radians */
	unsigned char target_7_mag;	    /* Magnitude, LSB = -0.543 dB */
	unsigned char target_7_lock;    /* Bit mapped, 1=locked, 0-not locked,
	                                 * bit 0 is current FFT frame, bit 1 is
		                             * FFT from n-1, ... bit 7 is FFT frame
	                                 * n-7.  */
	char      checksum;
} IS_PACKED evt300_mess_typ;

typedef struct
{
	char    data[60];
} IS_PACKED gen_mess_typ;

typedef struct
{
	unsigned char	msgID;		/* 67 for driver ID message */
	unsigned char	status;		/* ID length & card reader status */
	char		drv_ID[10];	/* Driver ID */
	char      	checksum;
} IS_PACKED driver_ID_mess_typ;

typedef struct
{
	unsigned char	msgID;		/* 2 for DDU display update message */

   /* LED control byte*/
	unsigned char	light_ctl;	
     #define PWR_ON       0x02		/* Power on:       1 = light on 0 = off */
     #define SYS_FAIL     0x04		/* System failure: 1 = light on 0 = off */
     #define SMART_CRUISE 0x08		/* Smart cruise:   1 = light on 0 = off */
     #define BARCODE      0x10		/* Barcode:        1 = lights off 0 = on */
     #define TARG_DET     0x20		/* Target detect:  1 = light on 0 = off */
     #define ALERT1       0x40		/* Alert 1:        1 = light on 0 = off */
     #define ALERT2       0x80		/* Alert 2:        1 = light on 0 = off */

   /* Audio control byte */
   unsigned char	audio_ctl;	
     #define TONE_SEL_MASK    0X0F  /* tone select mask*/
     #define VOL_MASK         0X70  /* volume mask */
     #define START_TONE_MASK  0X80  /* start tone mask */

   char      	checksum;
} IS_PACKED ddu_disp_mess_typ;

typedef union
{
	driver_ID_mess_typ     driver_ID_mess;
	evt300_mess_typ        evt300_mess;
	gen_mess_typ           gen_mess;
} IS_PACKED mess_union_typ;
