/**\file
 *
 *	Searches for CAN device at possible IO port locations.
 *	For I82527 chip configured with IO port address register, as SSV CAN
 */

#include <sys_os.h>
#include <x86/inout.h>
#include "i82527.h"
#include <sys/neutrino.h>

#define outp(p,v)       out8(p,v)

unsigned int BaseAddress;

unsigned int can1_list[]={0x200,0x210,0x220,0x230,
                        0x300,0x310,0x320,0x330,0};
unsigned int can2_list[]={0x202,0x212,0x222,0x232,
                        0x302,0x312,0x322,0x332,0};

void write_i82527_reg(unsigned char Addr,unsigned char value)
{
   out8(BaseAddress,Addr);     /* select CAN-Control-Reg. */
   out8(BaseAddress+1,value);  /* write Reg.              */
}

unsigned char read_i82527_reg(unsigned char Addr)
{
   out8(BaseAddress,Addr);
   return(in8(BaseAddress+1));
}

/***************************************************************************/
/* function : Search_CAN, automatic searching for CAN */
/* var.     : - save, to save the current value of reg.  */
/*            - Value to save the current value of reg.  */
/*            - V, counter */
/*            - BaseAddrList, base address list for CAN */
/* return   : TRUE,FALSE, result of searching for CAN */
/***************************************************************************/

int Search_CAN(unsigned char *ctrl_reg, unsigned int BaseAddrList[])
{
   unsigned char save,Value,rd0;
   int v, loop;
   /* Two loops: 1th id=CBh or id=CAh, 2ed ignore id */
   for (loop = 2; loop; loop--)
   {
      if (loop == 1) printf("Second time through, ignoring ID\n");
      for (v=0;BaseAddrList[v];v++)
      {
         BaseAddress = BaseAddrList[v];
         rd0 = in8(BaseAddress);
         if (rd0 == 0xCA || rd0 == 0xCB || loop == 1)
         {
            save=read_i82527_reg(CONTROL_REG);
	    *ctrl_reg = save;
	    /* check to see if bits 5 and 4 are 0 and not writable;
	     * if so, assume we are talking to the i82527 Control Register */
            if ((save & 0x30) == 0x00)
            {
               if (read_i82527_reg (0xFF) == 0xFF)
               {
                  write_i82527_reg(CONTROL_REG,0x70);
                  read_i82527_reg(CONTROL_REG); /* read twice, don't know why */
                  Value=read_i82527_reg(CONTROL_REG);
                  write_i82527_reg(CONTROL_REG,save);
                  if (Value==0x40)
                  {
                     return (BaseAddress);
                  }
               }
            }
         }
         outp (BaseAddress, rd0);
      }
   }
   return (-1);
}

int
main(int argc, char **argv)
{
	unsigned int base_addr;
	unsigned char ctrl_reg;
	ThreadCtl(_NTO_TCTL_IO, NULL); /// required to access I/O ports
	base_addr = Search_CAN(&ctrl_reg, can1_list);
	if (base_addr == -1)
		printf("No CAN1 controller found\n");
	else {
		printf("First CAN controller found at 0x%x\n", base_addr);
		printf("Value in control reg 0x%02x\n", ctrl_reg);
	}
	base_addr = Search_CAN(&ctrl_reg, can2_list);
	if (base_addr == -1)
		printf("No CAN2 controller found\n");
	else {
		printf("Second CAN controller found at 0x%x\n", base_addr);
		printf("Value in control reg 0x%02x\n", ctrl_reg);
	}
	return 0;
}
