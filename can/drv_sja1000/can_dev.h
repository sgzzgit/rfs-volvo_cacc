/**\file
 *	Device-dependent CAN definitions for the Phillips SJA 1000 driver.
 *	Assumes board is memory mapped at address given during initialization.
 */
#ifndef CAN_DEV_H
#define CAN_DEV_H

/** This must be initialized to the mapped address of the CAN channel.
 */ 
extern canregs_t *can_base_addr;

/** Reads from byte register on CAN chip at address "addr" and returns value 
 */
static inline unsigned char can_read_reg(unsigned char *addr)
{
	volatile unsigned char *preg = (volatile unsigned char *) (addr);
        return *preg;
}

/** Writes "value" to byte register on CAN chip at address "addr" 
 */
static inline void can_write_reg(unsigned char *addr, unsigned char value)
{
	volatile unsigned char *preg = (volatile unsigned char *) (addr);
        *preg = value;
}

/** Sets bits in "mask" in byte register on CAN chip at address "addr" 
 *  Register must be readable and then writable.
 */
static inline void can_set_reg(unsigned char *addr, unsigned char mask)
{
	volatile unsigned char *preg = (volatile unsigned char *) (addr);
	*preg |= mask;
}

/** Clears the bits in "mask" in byte register on CAN chip at address "addr" 
 *  Register must be readable and then writable.
 */
static inline void can_reset_reg(unsigned char *addr, unsigned char mask)
{
	volatile unsigned char *preg = (volatile unsigned char *) (addr);
	*preg &= ~(mask);
}

/*
 *  Board access macros, as used in can4linux, rely on a packed
 *  structure for canregs_t that mirrors the offsets of the actual
 *  registers. 
 *
 *  Unlike can4linux, each instance of the can_man resource manager
 *  corresponds to one SJA 1000 chip and one CAN port. Unused bd
 *  (board) parameter to macros is  retained for compatibility in
 *  sja1000funcs.c 
 */ 
#define CANin(bd,adr) can_read_reg(&can_base_addr->adr)
#define CANout(bd,adr,v) can_write_reg(&can_base_addr->adr, v)
#define CANset(bd,adr,m) can_set_reg(&can_base_addr->adr, m)
#define CANreset(bd,adr,m) can_reset_reg(&can_base_addr->adr, m)

#endif

