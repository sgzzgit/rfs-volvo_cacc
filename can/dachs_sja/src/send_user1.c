/*****************************************************************************
*                                                                          
*     STEINHOFF Automation & Fieldbus-Systems     D-65604 Elz               
*                                                                           
*     www.steinhoff-automation.com  --  support@steinhoff-automation.com     
*                  Phone: +49 6431 57099-70  Fax:  -80                     
*                                                                           
*    Copyright (C) STEINHOFF Automation & Fieldbus-Systems 1995-2008           
*               All Rights Reserved                                        
*                                                                        
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/neutrino.h>

#include "../include/candef.h"
#include "../include/canstr.h"
#include "../include/canglob.h"

char vers[]="Copyright A. Steinhoff, Produktions-Vers. 1.0";

int NameId;
short resp, k;
struct can_object rmsg, msg;
struct status st;
char p[32];
unsigned char OutByte;

/***************************************************************************
*                                                                         
*     Task      : CAN application example task                                 
*                                                                         
*     Funktion  : this task sends frames to a partner task                
*                                                                         
*     Author    : Armin Steinhoff                                         
*                                                                         
*     Copyright : Armin Steinhoff                                         
*                                                                         
*     Datum     : 26.8.2004                                                  
*                                                                         
*     Changes  :                                                         
*                                                                         
***************************************************************************/

int main(int argc, char **argv)
{
	canhdl_t hdl;
 
	if (ConnectDriver (1, "CANDRV", &hdl) < 0)
	{
		printf ("CAN send: can't connect the CAN driver. STOP\n");
		exit (-1);
	}
    
    printf("CAN status: %04x ", CanGetStatus(hdl, &st));
	
	msg.id = 0x701;   // CAN ID    
	// prepare output frame  
    msg.frame_inf.inf.DLC = 1;
    msg.frame_inf.inf.FF  = StdFF;
    msg.frame_inf.inf.RTR = 0;
    msg.data[0] = OutByte;

    for(;;)
	{
	   
	    resp = CanWrite(hdl, &msg);    // write msg 

		printf("CAN status: %04x ", CanGetStatus(hdl, &st));
		printf(" response: %d \n ", resp);		
        
        msg.data[0] = OutByte++;
        delay(2);
    }     
}
