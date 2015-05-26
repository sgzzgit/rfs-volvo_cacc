/**\file
 *
 *	Searches for CAN card at possible memory mapped locations
 *	For memory mapped registers, as the ECAN 527.
 */
	
#include <sys_os.h>
#include <x86/inout.h>
#include "i82527.h"
#include "can_mm.h"
#include <sys/neutrino.h>
#include <sys/mman.h>

unsigned int BaseAddress;

/* More address possibilities to add? 
 */

unsigned int can1_list[]={0xd8000};
unsigned int can2_list[]={0xd8100};

void write_i82527_mm(unsigned char Addr,unsigned char value)
{
        volatile unsigned char *preg = (volatile unsigned char *)
                         (BaseAddress + Addr);
        *preg = value;

}

unsigned char read_i82627_mm(unsigned char Addr)
{
        volatile unsigned char *preg = (volatile unsigned char *)
                         (BaseAddress + Addr);
        return *preg;

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
         BaseAddress = (unsigned int) mmap_device_memory(NULL,
				I82527_MAP_SIZE,
				PROT_READ | PROT_WRITE | PROT_NOCACHE,
				0, BaseAddrList[v]);
         rd0 = read_i82627_mm(0);
         if (rd0 == 0xCA || rd0 == 0xCB || loop == 1)
         {
            save=read_i82627_mm(CONTROL_REG);
	    *ctrl_reg = save;
	    /* check to see if bits 5 and 4 are 0 and not writable;
	     * if so, assume we are talking to the i82527 Control Register */
            if ((save & 0x30) == 0x00)
            {
               if (read_i82627_mm (0xFF) == 0xFF)
               {
                  write_i82527_mm(CONTROL_REG,0x70);
                  read_i82627_mm(CONTROL_REG); /* read twice, don't know why */
                  Value=read_i82627_mm(CONTROL_REG);
                  write_i82527_mm(CONTROL_REG,save);
                  if (Value==0x40)
                  {
                     return (BaseAddrList[v]);
                  }
               }
            }
         }
         write_i82527_mm( 0, rd0);
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
