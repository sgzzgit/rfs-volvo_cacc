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

char vers[] = "Copyright A. Steinhoff, Produktions-Vers. 1.0";

int NameId;
short resp, k;
struct can_object msg, rmsg;
struct status st;

/***************************************************************************
*                                                                         
*     Task      : CAN-application example task                                 
*                                                                         
*     Funktion  : this task reads frames sent by  a partner task                
*                                                                         
*     Author    : Armin Steinhoff                                         
*                                                                         
*     Copyright : Armin Steinhoff                                         
*                                                                         
*     Date      : 26.8.2004                                               
*                                                                         
*     Changes   :                                                         
*                                                                         
***************************************************************************/

int main (void)
{
	unsigned short i;
	struct sigevent pulse_event;
	struct _pulse pulse;
	int chid, coid, rcvid;
	canhdl_t hdl;
  
	// create notification channel
	if ((chid = ChannelCreate(0)) == -1) exit( -1);

	if ((coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0)) == -1)
	{
		ChannelDestroy(chid);
		exit( -1);
	}
	// initialize notification event
	SIGEV_PULSE_INIT(&pulse_event, coid, getprio(0), _PULSE_CODE_MINAVAIL, 0);

	if (ConnectDriver (1, "CANDRV", &hdl) < 0)
	{
		printf ("CAN recv: can't connect the CAN driver. STOP\n");
		exit (-1);
	}

	printf ("\nCAN status: %04x\n", CanGetStatus(hdl, &st));

	DeRegRdPulse(hdl);
    RegRdPulse(hdl, &pulse_event);

	for (;;)
	{
	   // a pulse will be triggerd after a frame or more has been received
	   	rcvid = MsgReceivePulse(chid, &pulse, sizeof(struct _pulse), NULL);
		if(rcvid == -1)
		   perror("recv\n");
        else
		{
		    resp = CanRead (hdl, &rmsg, NULL);	// read the next frame
			if(resp == 0)
			{
 	           printf("CAN Status: %04x %d", CanGetStatus(hdl, &st), rmsg.frame_inf.inf.DLC);
  	           printf (".%04x", rmsg.id);
    	       for (i = 0; i < 7; i++)
     	          printf (".%02x", rmsg.data[i]);
      			printf (" 0: %d\n", resp);
			}else
			   printf("CAN Status: %04x\n", CanGetStatus(hdl, &st));
 
        }
	}
}
