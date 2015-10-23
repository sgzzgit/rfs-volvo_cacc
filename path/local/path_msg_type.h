/**\file
 *	Message type identifier used by various sets of interacting programs
 */
#ifndef PATH_MSGE_TYPE_H
#define PATH_MSG_TYPE_H

/* Used by programs testing the DENSO WAVE radio modules
 * Message format is text strings, first two space separated fields
 * are message sequence ID and message type, other fields may
 * vary depending on type of message.
 *
 */
#define CLOCK_SET_MSG_TYPE	100	// sent by clock_snd_skew
#define CLOCK_SKW_MSG_TYPE	200	// sent by clock_set
#define CLOCK_SND_MSG_TYPE	300	// sent by clock_snd
#define CLOCK_MTR_MSG_TYPE	400	// sent by clock_rcv
#define CHG_TXOPT_MSG_TYPE	500	// sent by sndchan, received by chgchan

static inline void print_ip_address(int addr)
{
	printf("%03d.%03d.%03d.%03d ", (addr & 0xff000000) >> 24,
			(addr &0xff0000) >> 16, (addr & 0xff00) >> 8,
			addr & 0xff);
}
#endif
