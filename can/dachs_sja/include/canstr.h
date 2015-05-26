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
*   Filename    : canstr.h                                                    
*   Version        : 1.0                                                   
*   Date              : 2001                                      
*   Author         : Armin Steinhoff                                            
*                                                                                                                                         
*****************************************************************************/


#ifndef _CANSTR_
#define _CANSTR_

#ifdef __cplusplus
extern "C" {
#endif
	
struct frinf
{
   BYTE DLC:4;
   BYTE res:2;
   BYTE RTR:1;
   BYTE  FF:1;
};

struct can_object
{
    union
    {
	 struct frinf inf;
       BYTE octet;
    } frame_inf;

    unsigned int id;
	BYTE data[24];
};	

struct UserMsg 
{
   short cmd;
   short ch;
   struct can_object msg;
};

struct usermsg 
{
   unsigned char st;
   struct can_object msg;
};

struct userbuff
{
  struct usermsg toBus[MSGNB];
}; 

struct busbuff 
{
   struct usermsg toUser[MSGNB];
}; 

struct btr0
{
   BYTE brp:6;
   BYTE sjw:2;
};
struct btr1
{
   BYTE tseg1:3;
   BYTE tseg2:4;
   BYTE sam:1;
};

struct config
{
   BYTE MODE;
   union
   {
      struct btr0 BTR0;
      BYTE   oct;
   }ubtr0;

   union
   {
      struct btr1 BTR1;
      BYTE oct;
   }ubtr1;
   
   BYTE AFM;
   BYTE STM;
   BYTE LOM;
   BYTE OCR;
   BYTE EWL;
   BYTE RXERR;
   BYTE TXERR;
   BYTE ACR[4];
   BYTE AMR[4];
};


struct status
{
  BYTE  STR;
  BYTE  ErrState;
  BYTE  OverRunState;
  BYTE  WakeUpState;
  BYTE  ErrPassiveState;
  BYTE  ArbitLostState;
  BYTE  BusErrState;
  int  LostFrames;
};

struct _canhdl
{
	int con_id;
	short ch;
};

typedef struct _canhdl canhdl_t;

#define MSGLGT sizeof(struct can_object)

void sffid_to_arr(unsigned int id, BYTE *id_ptr);
void effid_to_arr(unsigned int id, BYTE *id_ptr);
void arr_to_sffid(unsigned int *id, BYTE *id_ptr);
void arr_to_effid(unsigned int * id, BYTE *id_ptr);

int ConnectDriver( short ch, char * name, canhdl_t * hdl );
short DisConnectDriver(canhdl_t * hdl);

short CanRead(canhdl_t hdl, struct can_object *msg, struct sigevent * pulse);
short CanWrite(canhdl_t hdl, struct can_object  *msg);

short CanGetConfig(canhdl_t hdl, struct config *msg);
short CanSetConfig(canhdl_t hdl, struct config *msg);

short CanRestart(canhdl_t hdl);
short ResetAccPattern(canhdl_t hdl );

short RegRdPulse(canhdl_t hdl,  struct sigevent * pulse);
short DeRegRdPulse(canhdl_t hdl);

short CanGetStatus(canhdl_t hdl, struct status *st);
    
short CanNbOfRxMessages (canhdl_t hdl);
short CanNbOfTxMessages (canhdl_t hdl);

short CanStart(canhdl_t hdl);
short CanStop(canhdl_t hdl);

#ifdef __cplusplus
};
#endif
#endif
