/*\file
 *	Simplified from can4linux 3.5.4 to match only Janus MM board
 *	Timing values only for 8MHz included (assume 16MHz oscillator
 *	on Janus MM is halved to be 8MHz, if not we need to change.)
 */ 
#ifndef __CAN_SJA1000__
#define __CAN_SJA1000__

/* Standard definition with offset 1 */
union frame {
	struct {
	    u8	 canid1;
	    u8	 canid2;
	    u8	 canid3;
	    u8	 canid4;
	    u8   canxdata[8];
	} extframe;
	struct {
	    u8	 canid1;
	    u8	 canid2;
	    u8   candata[8];
	} stdframe;
};

typedef struct canregs {
	u8	canmode;		/* 0 */
	u8	cancmd;
	u8	canstat;
	u8	canirq;
	u8	canirq_enable;
	u8	reserved1;		/* 5 */
	u8	cantim0;
	u8	cantim1;
	u8	canoutc;
	u8	cantest;
	u8	reserved2;		/* 10 */
	u8	arbitrationlost;	/* read only */
	u8	errorcode;		/* read only */
	u8	errorwarninglimit;
	u8	rxerror;
	u8	txerror;		/* 15 */
	u8	frameinfo;
	union   frame frame;
	u8	reserved3;
	u8	canrxbufferadr		/* 30 */;
	u8	canclk; 	 
} __attribute__((packed)) canregs_t;

#define CAN_RANGE 0x20		/* default: 32 registers */

/*--- Mode Register -------- PeliCAN -------------------*/

#  define CAN_SLEEP_MODE		0x10    /* Sleep Mode */
#  define CAN_ACC_FILT_MASK		0x08    /* Acceptance Filter Mask */
#  define CAN_SELF_TEST_MODE		0x04    /* Self test mode */
#  define CAN_LISTEN_ONLY_MODE		0x02    /* Listen only mode */
#  define CAN_RESET_REQUEST		0x01	/* reset mode */
#  define CAN_MODE_DEF CAN_ACC_FILT_MASK	 /* Default ModeRegister Value*/

   /* bit numbers of mode register */
#  define CAN_SLEEP_MODE_BIT		4	/* Sleep Mode */
#  define CAN_ACC_FILT_MASK_BIT		3	/* Acceptance Filter Mask */
#  define CAN_SELF_TEST_MODE_BIT	2	/* Self test mode */
#  define CAN_LISTEN_ONLY_MODE_BIT	1	/* Listen only mode */
#  define CAN_RESET_REQUEST_BIT		0	/* reset mode */


/*--- Interrupt enable Reg -----------------------------*/
#define CAN_ERROR_BUSOFF_INT_ENABLE		(1<<7)
#define CAN_ARBITR_LOST_INT_ENABLE		(1<<6)
#define CAN_ERROR_PASSIVE_INT_ENABLE		(1<<5)
#define CAN_WAKEUP_INT_ENABLE			(1<<4)
#define CAN_OVERRUN_INT_ENABLE			(1<<3)
#define CAN_ERROR_INT_ENABLE			(1<<2)
#define CAN_TRANSMIT_INT_ENABLE			(1<<1)
#define CAN_RECEIVE_INT_ENABLE			(1<<0)

/*--- Frame information register -----------------------*/
#define CAN_EFF				0x80	/* extended frame */
#define CAN_SFF				0x00	/* standard fame format */


/*--- Command Register ------------------------------------*/
 
#define CAN_GOTO_SLEEP				(1<<4)
#define CAN_CLEAR_OVERRUN_STATUS		(1<<3)
#define CAN_RELEASE_RECEIVE_BUFFER		(1<<2)
#define CAN_ABORT_TRANSMISSION			(1<<1)
#define CAN_TRANSMISSION_REQUEST		(1<<0)

/*--- Status Register --------------------------------*/
 
#define CAN_BUS_STATUS 				(1<<7)
#define CAN_ERROR_STATUS			(1<<6)
#define CAN_TRANSMIT_STATUS			(1<<5)
#define CAN_RECEIVE_STATUS			(1<<4)
#define CAN_TRANSMISSION_COMPLETE_STATUS	(1<<3)
#define CAN_TRANSMIT_BUFFER_ACCESS		(1<<2)
#define CAN_DATA_OVERRUN			(1<<1)
#define CAN_RECEIVE_BUFFER_STATUS		(1<<0)

/*--- Status Register --------------------------------*/
 
#define CAN_BUS_STATUS_BIT 			(1<<7)
#define CAN_ERROR_STATUS_BIT			(1<<6)
#define CAN_TRANSMIT_STATUS_BIT			(1<<5)
#define CAN_RECEIVE_STATUS_BIT			(1<<4)
#define CAN_TRANSMISSION_COMPLETE_STATUS_BIT	(1<<3)
#define CAN_TRANSMIT_BUFFER_ACCESS_BIT		(1<<2)
#define CAN_DATA_OVERRUN_BIT			(1<<1)
#define CAN_RECEIVE_BUFFER_STATUS_BIT		(1<<0)

/*--- Interrupt Register -----------------------------------*/
 
#define CAN_WAKEUP_INT				(1<<4)
#define CAN_OVERRUN_INT				(1<<3)
#define CAN_ERROR_INT				(1<<2)
#define CAN_TRANSMIT_INT			(1<<1)
#define CAN_RECEIVE_INT 			(1<<0)

/*--- Output Control Register -----------------------------------------*/
/*
 *	7	6	5	4	3	2	1	0
 * 	OCTP1	OCTN1	OCPOL1	OCTP0	OCTN0	OCPOL0	OCMODE1	OCMODE0
 *	----------------------  ----------------------  ---------------
 *	    TX1 Output		    TX0 Output		  programmable
 *	  Driver Control	  Driver Control	  output functions
 *
 *	MODE
 *	OCMODE1	OCMODE0
 *	  0	  1	Normal Mode; TX0, TX1 bit sequenze TXData
 *	  1	  1	Normal Mode; TX0 bit sequenze, TX1 busclock TXCLK
 *	  0	  0	Biphase Mode
 *	  1	  0	Test Mode; TX0 bit sequenze, TX1 COMPOUT
 *
 *	In normal Mode Voltage Output Levels depend on 
 *	Driver Characteristic: OCTPx, OCTNx
 *	and programmed Output Polarity: OCPOLx
 *
 *	Driver Characteristic
 *	OCTPx	OCTNx
 *	  0	 0	always Floating Outputs,
 *	  0	 1	Pull Down
 *	  1	 0	Pull Up
 *	  1	 1	Push Pull
 */
 
/*--- Output control register --------------------------------*/

#define CAN_OCTP1			(1<<7)
#define CAN_OCTN1			(1<<6)
#define CAN_OCPOL1			(1<<5)
#define CAN_OCTP0			(1<<4)
#define CAN_OCTN0			(1<<3)
#define CAN_OCPOL0			(1<<2)
#define CAN_OCMODE1			(1<<1)
#define CAN_OCMODE0			(1<<0)

/*--- Clock Divider register ---------------------------------*/

#define CAN_MODE_BASICCAN		(0x00)
#define CAN_MODE_PELICAN		(0xC0)

/** Original code from can4linux only used CAN_MODE_CLK, which was
 *  defined to the CAN_MODE_CLK1 value shown here.
 */
#define CAN_MODE_CLK1			(0x07)		/* CLK-out = Fclk   */
#define CAN_MODE_CLK2			(0x00)		/* CLK-out = Fclk/2 */

/** On the Janus-MM board the correct CLKout is Fclk/2
 */
#define CAN_MODE_CLK	CAN_MODE_CLK2

/*--- Remote Request ---------------------------------*/
/*    Notes:
 *    Basic CAN: RTR is Bit 4 in TXDES1.
 *    Peli  CAN: RTR is Bit 6 in frameinfo.
 */
# define CAN_RTR				(1<<6)

/* these timings are valid for clock 8Mhz */
#  define CAN_TIM0_10K            49
#  define CAN_TIM1_10K          0x1c
#  define CAN_TIM0_20K            24
#  define CAN_TIM1_20K          0x1c
#  define CAN_TIM0_40K          0x89    /* Old Bit Timing Standard of port */
#  define CAN_TIM1_40K          0xEB    /* Old Bit Timing Standard of port */
#  define CAN_TIM0_50K             9
#  define CAN_TIM1_50K          0x1c
#  define CAN_TIM0_100K            4    /* sp 87%, 16 abtastungen, sjw 1 */
#  define CAN_TIM1_100K          0x1c
#  define CAN_TIM0_125K            3
#  define CAN_TIM1_125K         0x1c
#  define CAN_TIM0_250K            1
#  define CAN_TIM1_250K         0x1c
#  define CAN_TIM0_500K            0
#  define CAN_TIM1_500K         0x1c
#  define CAN_TIM0_800K            0
#  define CAN_TIM1_800K         0x16
#  define CAN_TIM0_1000K           0
#  define CAN_TIM1_1000K        0x14

#endif 	/* __CAN_SJA1000__ */
