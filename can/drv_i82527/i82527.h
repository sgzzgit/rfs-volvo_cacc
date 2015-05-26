/*
** i82527.h
**
**
** provided by SSV and updated by
** Jorge M. Estrela da Silva, 2002
** ISEP, Porto, Portugal
**
*/

#ifndef include_i82527_h
#define include_i82527_h

/* Definition the Registers of 82527 */
/*-----------------------------------*/
#define CONTROL_REG      0x00  /* Address Kontroll-Reg.                */
#define STATUS_REG       0x01  /* Address Status-Reg.                  */
#define CPU_IF_REG       0x02  /* Address CPU-Interface-Register       */
#define HIGH_SPEED_READ	 0x04  /* Has result of last read, for fast CPU */
#define G_MASK_S_REG0    0x06  /* Address Global Mask Standard Reg. 0  */
#define G_MASK_S_REG1    0x07  /* Address Global Mask Standard Reg. 1  */
#define G_MASK_E_REG0    0x08  /* Address Global Mask Extended Reg. 0  */
#define G_MASK_E_REG1    0x09  /* Address Global Mask Extended Reg. 1  */
#define G_MASK_E_REG2    0x0A  /* Address Global Mask Extended Reg. 2  */
#define G_MASK_E_REG3    0x0B  /* Address Global Mask Extended Reg. 3  */
#define M_MASK_REG0      0x0C  /* Address Message Mask Reg. 0          */
#define M_MASK_REG1      0x0D  /* Address Message Mask Reg. 1          */
#define M_MASK_REG2      0x0E  /* Address Message Mask Reg. 2          */
#define M_MASK_REG3      0x0F  /* Address Message Mask Reg. 3          */
#define CLKOUT_REG  	 0x1F  /* Clock Out Reg..      */
#define BUS_CON_REG      0x2F  /* Address Bus Konfigurations Reg.      */
#define BIT_TIMING_REG0  0x3F  /* Address Bit Timing Reg. 0            */
#define BIT_TIMING_REG1  0x4F  /* Address Bit Timing Reg. 1            */
#define INT_REG          0x5F  /* Address Interrupt Reg.               */
#define P1CONF_REG       0x9F  /* Address Port1 Konfig. Reg.           */
#define PCONF_REG       0x9F  /* Address Port1 Konfig. Reg.           */
#define PIN		       0xBF  /* Address Port1 Konfig. Reg.           */
#define POUT		       0xDF  /* Address Port1 Konfig. Reg.           */
#define MSG_1            0x10  /* Address Message 1;   0x10-0x1E       */
#define MSG_2            0x20  /* Address Message 2;   0x20-0x2E       */
#define MSG_3            0x30  /* Address Message 3;   0x30-0x3E       */
#define MSG_4            0x40  /* Address Message 4;   0x40-0x4E       */
#define MSG_5            0x50  /* Address Message 5;   0x50-0x5E       */
#define MSG_6            0x60  /* Address Message 6;   0x60-0x6E       */
#define MSG_7            0x70  /* Address Message 7;   0x70-0x7E       */
#define MSG_8            0x80  /* Address Message 8;   0x80-0x8E       */
#define MSG_9            0x90  /* Address Message 9;   0x90-0x9E       */
#define MSG_10           0xA0  /* Address Message 10;  0xA0-0xAE       */
#define MSG_11           0xB0  /* Address Message 11;  0xB0-0xBE       */
#define MSG_12           0xC0  /* Address Message 12;  0xC0-0xCE       */
#define MSG_13           0xD0  /* Address Message 13;  0xD0-0xDE       */
#define MSG_14           0xE0  /* Address Message 14;  0xE0-0xEE       */
#define MSG_15           0xF0  /* Address Message 15;  0xF0-0xFE       */

#define CTRL_0_REG       0x00  
#define CTRL_1_REG       0x01  
#define DATA0_REG 	0x07
#define CONF_REG 	0x06


#define _82527_XTD_FRAME 4
#define _DIR_TX 8

#endif

