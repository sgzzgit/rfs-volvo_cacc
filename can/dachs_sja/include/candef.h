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
*   Filename    : candef.h                                                    
*   Version        : 1.0                                                   
*   Date              : 2001                                      
*   Author         : Armin Steinhoff                                            
*                                                                                                                                                                       
*****************************************************************************/

#ifndef _CANDEF_
#define _CANDEF_

#ifdef __cplusplus
extern "C" {
#endif
	
#define BYTE  unsigned char
#define WORD  unsigned short
#define LWORD unsigned long

#define TRANSMIT 1
#define RECEIVE  0
#define EXTID    1
#define STDID    0

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

//#define TRUE 1
//#define FALSE 0 
#define MAXBUFF 1000 

#define SFF 1
#define ExtFF 1
#define StdFF 0

#define USED_IN_CANMSG_HEAP 0x0C
#define NO_HEAP_AVAILABLE   0x0B
#define SUCCESS 0x00
#define MSGNB  128

#define div_by_32   0  
#define div_by_64   1  
#define div_by_128  2  
#define div_by_256  3

#define ERR_OK        0
#define ERR_NOT_CON  -1
#define ERR_NO_DRV   -1
#define ERR_CHANNEL  -2
#define ERR_DEVCTL   -2
#define ERR_NOTIFY   -3

#define ERR_FULL     -10  
#define ERR_EMPTY    -11 
#define ERR_OVERRUN  -12

#define MY_CMD_CODE		1
#define MY_DEVCTL_SETGET	__DIOTF(_DCMD_MISC, MY_CMD_CODE + 2, struct UserMsg)

#ifdef __cplusplus
};
#endif

#endif
