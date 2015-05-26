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
*   Filename    : canglob.h                                                    
*   Version        : 1.0                                                   
*   Date              : 2001                                      
*   Author         : Armin Steinhoff                                            
*                                                                                                   
*****************************************************************************/

/***********  canglob.h *****************/
#ifndef _CANGLOB_
#define _CANGLOB_

#ifdef __cplusplus
extern "C" {
#endif
	
struct userbuff User;
struct busbuff  Bus;

struct usermsg *RdInMsg;  // actual read ptr
struct usermsg *WrOutMsg; // actual write ptr

short  WrInIx; // wr index msg to bus
short  RdInIx; // rd index msg to bus

short  WrOutIx;// wr index bus to user
short  RdOutIx;// rd index bus to user

BYTE   OverRun;

#ifdef __cplusplus
};
#endif
#endif



