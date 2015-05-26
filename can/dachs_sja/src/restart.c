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

/***************************************************************************
*                                                                         
*     Task      : CAN-Restart                                             
*                                                                         
*     Funktion  : Reset the CAN controller and restart of the driver      
*                                                                         
*     Author    : Armin Steinhoff                                         
*                                                                         
*     Copyright : Armin Steinhoff                                         
*                                                                         
*     Datum     : 26.8.2004                                                
*                                                                         
*    Changes  :                                                         
*                                                                         
***************************************************************************/

int main(void)
{

	canhdl_t hdl;
    
	if (ConnectDriver (1, "CANDRV", &hdl) < 0)
	{
		printf ("CAN restart: can't connect the CAN driver. STOP\n");
		exit (-1);
	}
    
    CanRestart(hdl);
	
	DisConnectDriver(&hdl);
}
