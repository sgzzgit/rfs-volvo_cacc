/***************************************************************************
*                                                                         
*    STEINHOFF Automation & Fieldbus-Systems     D-65604 Elz               
*                                                                          
*    www.steinhoff-automation.com  --  support@steinhoff-automation.com     
*                 Phone: +49 6431 57099-70  Fax:  -80                      
*                                                                          
*   Copyright (C) STEINHOFF Automation & Fieldbus-Systems 1995-2004           
*              All Rights Reserved                                         
*                                                                          
****************************************************************************/
/****************************************************************************
*                                                                          
*   Filename    : canext.h                                                    
*   Version        : 1.0                                                   
*   Date              : 2001                                     
*   Author         : Armin Steinhoff                                            
*                                                                                                   
*****************************************************************************/

/**************canext.h **************/

#ifndef _CANEXT_
#define _CANEXT_

#ifdef __cplusplus
extern "C" {
#endif
	
extern BYTE * BAR[6];

extern struct userbuff User[2];
extern struct busbuff  Bus[2];

extern struct usermsg *RdInMsg[2];  // current read ptr
extern struct usermsg *WrOutMsg[2]; // current write ptr

extern short  WrInIx[2];  // wr index msg to bus
extern short  RdInIx[2];  // rd index msg to bus

extern short  WrOutIx[2]; // wr index bus to user
extern short  RdOutIx[2]; // rd index bus to user
extern BYTE   OverRun[2];
extern short  TxFrames[2];
extern short  RxFrames[2];
	
#ifdef __cplusplus
};

#endif
#endif
