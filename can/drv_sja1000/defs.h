/**\file
 *
 *	Header file containing definitions needed to make can4linux
 *	file sja1000funcs.c compile. Linux-only definitions may
 *	be defined to no-ops or simple ints. Some definitions from can4linux.h
 *	as well as defs.h in can4linux-3.5.4 have been included.
 */
#include "string.h"
#include "sys/time.h"
#include "linux/delay.h"

typedef unsigned char u8; 

/** External structures may not be used, declared in can_dev.c for
 *  compatibility with can4linux sja1000funcs.c.
 *
 *  For PATH driver, each channel has a separate driver, so MAX_CHANNELS is 1
 */
#define MAX_CHANNELS 1
#define MY_CHANNEL 0

extern int Baud[MAX_CHANNELS];
extern int IRQ[MAX_CHANNELS]; 
extern int Baud[MAX_CHANNELS];
extern unsigned int AccCode[MAX_CHANNELS];
extern unsigned int AccMask[MAX_CHANNELS];
extern int Timeout[MAX_CHANNELS];
extern int Outc[MAX_CHANNELS];
extern int TxErr[MAX_CHANNELS];
extern int RxErr[MAX_CHANNELS];
extern int Overrun[MAX_CHANNELS];
extern int Options[MAX_CHANNELS];
extern atomic_t Can_isopen[MAX_CHANNELS];

// value for Janus MM board
#define CAN_OUTC_VAL	0xda

extern int Outc[1];

/** 
 *	Fancy leveled debugging not really needed in
 *	simpler QNX driver environment.
 */
#define DBGprint(ms,ar) { }
#define DBGin() { }
#define DBGout()        { }
#define DEBUG_TTY(n, args...)
extern unsigned int dbgMask;

/**
 IOCTL generic CAN controller status request parameter structure */
typedef struct CanStatusPar {
    unsigned int baud;                  /**< actual bit rate */
    unsigned int status;                /**< CAN controller status register */
    unsigned int error_warning_limit;   /**< the error warning limit */
    unsigned int rx_errors;             /**< content of RX error counter */
    unsigned int tx_errors;             /**< content of TX error counter */
    unsigned int error_code;            /**< content of error code register */
    unsigned int rx_buffer_size;        /**< size of rx buffer  */
    unsigned int rx_buffer_used;        /**< number of messages */
    unsigned int tx_buffer_size;        /**< size of tx buffer  */
    unsigned int tx_buffer_used;        /**< number of messages */
    unsigned long retval;               /**< return value */
    unsigned int type;                  /**< CAN controller / driver type */
} CanStatusPar_t;

/** More Linux-specific defines included for can4linux compatibility.
 *  so that all of sja1000funcs.c can compile without change, even
 *  the parts that aren't used by our driver.
 */
struct inode {
	int dummy;
};

struct _instance_data {
	int rx_index;
};
	
struct file {
	void *private_data;
};

#define iminor(x)	((unsigned int) (x))

#define CAN_TYPE_SJA1000	1
#define local_irq_save(flags)	
#define local_irq_restore(flags) 
#define schedule()
#define need_resched	0

#define CAN_MSG_LENGTH 8                /**< maximum length of a CAN frame */

#define MSG_ACTIVE      (0)             /**< Controller Error Active */
#define MSG_BASE        (0)             /**< Base Frame Format */
#define MSG_RTR         (1<<0)          /**< RTR Message */
#define MSG_OVR         (1<<1)          /**< CAN controller Msg overflow error*/
#define MSG_EXT         (1<<2)          /**< extended message format */
#define MSG_SELF        (1<<3)          /**< message received from own tx */
#define MSG_PASSIVE     (1<<4)          /**< controller in error passive */
#define MSG_BUSOFF      (1<<5)          /**< controller Bus Off  */
#define MSG_WARNING     (1<<6)          /**< CAN Warning Level reached */
#define MSG_BOVR        (1<<7)          /**< receive/transmit buffer overflow */

/**
* mask used for detecting CAN errors in the canmsg_t flags field
*/
#define MSG_ERR_MASK    (MSG_OVR+MSG_PASSIVE+MSG_BUSOFF+MSG_BOVR+MSG_WARNING)

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU        /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU        /* extended frame format (EFF) */
#define CANDRIVERERROR  0xFFFFFFFFul    /* invalid CAN ID == Error */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU        /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU        /* extended frame format (EFF) */
#define CANDRIVERERROR  0xFFFFFFFFul    /* invalid CAN ID == Error */

/**
* The CAN message structure used by can4linux, not used in PATH driver.
*/
typedef struct {
    /** flags, indicating or controlling special message properties */
    int             flags;
    int             cob;         /**< CAN object number, used in Full CAN  */
    unsigned   long id;          /**< CAN message ID, 4 bytes  */
    struct timeval  timestamp;   /**< time stamp for received messages */
    short      int  length;      /**< number of bytes in the CAN message */
    unsigned   char data[CAN_MSG_LENGTH]; /**< data, 0...8 bytes */
} canmsg_t;

extern canmsg_t last_Tx_object[MAX_CHANNELS];

#define MAX_BUFSIZE 1
#define CAN_MAX_OPEN 1

#define BUF_EMPTY    0
#define BUF_OK       1
#define BUF_FULL     BUF_OK
#define BUF_OVERRUN  2
#define BUF_UNDERRUN 3

typedef struct {
        int head;
        int tail;
        int status;
        int active;
        char free[MAX_BUFSIZE];
        canmsg_t data[MAX_BUFSIZE];
 } msg_fifo_t;

extern msg_fifo_t Rx_Buf[MAX_BUFSIZE][CAN_MAX_OPEN];
extern msg_fifo_t Tx_Buf[MAX_BUFSIZE];

extern int IRQ_requested[];
extern int Can_minors[];                        /* used as IRQ dev_id */
extern int selfreception[MAX_CHANNELS][CAN_MAX_OPEN];                   /* flag 
 1 = On */
extern int use_timestamp[MAX_CHANNELS];                 /* flag  1 = On */
extern int wakeup[];                            /* flag  1 = On */
extern int listenmode;                          /* true for listen only */

/** Bogus definitions to get simplest case to make Linux ISR 
 *  in sja1000funcs.c compile -- not used.
 */
#define LINUX_VERSION_CODE	1
#define KERNEL_VERSION(X,Y,Z)	0
typedef int irqreturn_t;
#define IRQ_NONE	0
#define IRQ_HANDLED	1
#define IRQ_RETVAL(x)	(x)

typedef int wait_queue_head_t;
extern wait_queue_head_t CanWait[MAX_CHANNELS][CAN_MAX_OPEN];
extern wait_queue_head_t CanOutWait[MAX_CHANNELS];
extern int CanWaitFlag[MAX_CHANNELS][CAN_MAX_OPEN];


#include "sja1000.h"
#include "can_dev.h"

/** Functions in sja1000funcs.c that may be called externally
 */
extern int CAN_SetTiming (int minor, int baud);
extern int CAN_SetMask (int minor, unsigned int code, unsigned int mask);
extern int CAN_ChipReset(int minor);
extern int CAN_StartChip(int minor);
