/***************************************************************************
*                                                                         
*    STEINHOFF Automation & Fieldbus-Systems     D-65604 Elz               
*                                                                          
*    www.steinhoff-automation.com  --  support@steinhoff-automation.com     
*                 Phone: +49 6431 57099-70  Fax:  -80                      
*                                                                          
*   Copyright (C) STEINHOFF Automation & Fieldbus-Systems 1995-2008          
*              All Rights Reserved                                         
*                                                                          
****************************************************************************/
/****************************************************************************
*                                                                          
*   Filename    : SAJ1000.h                                                  
*   Version        : 1.0                                                   
*   Date              : 2001                                      
*   Author         : Armin Steinhoff                                            
*                                                                                                                                                                       
*****************************************************************************/

#ifndef __SAJINC__
#define __SAJINC__

#ifdef __cplusplus
extern "C" {
#endif

/* address and bit definitions for the Mode & Control Register */
#define RM_RR_Bit 0x01 /* reset mode (request) bit */
#define LOM_Bit 0x02 /* listen only mode bit */
#define STM_Bit 0x04 /* self test mode bit */
#define AFM_Bit 0x08 /* acceptance filter mode bit */
#define SM_Bit  0x10 /* enter sleep mode bit */

/* address and bit definitions for the
Interrupt Enable & Control Register */

#define RIE_Bit  0x01 /* receive interrupt enable bit */
#define TIE_Bit  0x02 /* transmit interrupt enable bit */
#define EIE_Bit  0x04 /* error warning interrupt enable bit */
#define DOIE_Bit 0x08 /* data overrun interrupt enable bit */
#define WUIE_Bit 0x10 /* wake-up interrupt enable bit */
#define EPIE_Bit 0x20 /* error passive interrupt enable bit */
#define ALIE_Bit 0x40 /* arbitration lost interr. enable bit*/
#define BEIE_Bit 0x80 /* bus error interrupt enable bit */

/* address and bit definitions for the Command Register */
#define TR_Bit  0x01 /* transmission request bit */
#define AT_Bit  0x02 /* abort transmission bit */
#define RRB_Bit 0x04 /* release receive buffer bit */
#define CDO_Bit 0x08 /* clear data overrun bit */

#define SRR_Bit 0x10 /* self reception request bit */

/* address and bit definitions for the Status Register */
#define RBS_Bit 0x01 /* receive buffer status bit */
#define DOS_Bit 0x02 /* data overrun status bit */
#define TBS_Bit 0x04 /* transmit buffer status bit */
#define TCS_Bit 0x08 /* transmission complete status bit */
#define RS_Bit  0x10 /* receive status bit */
#define TS_Bit  0x20 /* transmit status bit */
#define ES_Bit  0x40 /* error status bit */
#define BS_Bit  0x80 /* bus status bit */

/* address and bit definitions for the Interrupt Register */
#define RI_Bit  0x01 /* receive interrupt bit */
#define TI_Bit  0x02 /* transmit interrupt bit */
#define EI_Bit  0x04 /* error warning interrupt bit */
#define DOI_Bit 0x08 /* data overrun interrupt bit */
#define WUI_Bit 0x10 /* wake-up interrupt bit */

#define EPI_Bit 0x20 /* error passive interrupt bit */
#define ALI_Bit 0x40 /* arbitration lost interrupt bit */
#define BEI_Bit 0x80 /* bus error interrupt bit */

/* address and bit definitions for the Bus Timing Registers */
#define SAM_Bit 0x80 /* sample mode bit
1 == the bus is sampled 3 times
0 == the bus is sampled once */

/* OCMODE1, OCMODE0 */
#define BiPhaseMode 0x00 /* bi-phase output mode */
#define NormalMode  0x02 /* normal output mode */
#define ClkOutMode  0x03 /* clock output mode */

/* output pin configuration for TX1 */
#define OCPOL1_Bit 0x20 /* output polarity control bit */
#define Tx1Float   0x00 /* configured as float */
#define Tx1PullDn  0x40 /* configured as pull-down */
#define Tx1PullUp  0x80 /* configured as pull-up */
#define Tx1PshPull 0xC0 /* configured as push/pull */

/* output pin configuration for TX0 */
#define OCPOL0_Bit 0x04 /* output polarity control bit */
#define Tx0Float   0x00 /* configured as float */
#define Tx0PullDn  0x08 /* configured as pull-down */
#define Tx0PullUp  0x10 /* configured as pull-up */
#define Tx0PshPull 0x18 /* configured as push/pull */

/* address and bit definitions for the Clock Divider Register */
#define DivBy1      0x07 /* CLKOUT = oscillator frequency */
#define DivBy2      0x00 /* CLKOUT = 1/2 oscillator frequency */
#define ClkOff_Bit  0x08 /* clock off bit, control of the CLK OUT pin */
#define RXINTEN_Bit 0x20 /* pin TX1 used for receive interrupt */
#define CBP_Bit     0x40 /* CAN comparator bypass control bit */
#define CANMode_Bit 0x80 /* CAN mode definition bit */


#ifdef __cplusplus
};
#endif

#endif
