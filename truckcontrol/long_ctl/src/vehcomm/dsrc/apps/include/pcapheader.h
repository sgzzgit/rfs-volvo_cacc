#ifndef _PCAPHEADERS_H
#define _PCAPHEADERS_H

#include <stdint.h>		// c++11 <cstdint>
#include <pcap.h>

//defines for the packet type code in an ETHERNET header
#define ETHER_TYPE_IPv4		(0x0800)
#define ETHER_TYPE_IPv6		(0x86dd)
#define ETHER_TYPE_8021Q	(0x8100)
#define ETHER_TYPE_80211	(0x2452) 
#define ETHER_TYPE_WSM		(0x88dc)

#if 0
struct pcap_pkthdr 		// in <pcap.h>
{
	struct timeval ts;	/* time stamp */
	bpf_u_int32 caplen;	/* length of portion present */
	bpf_u_int32 len;		/* length this packet (off wire) */
};

struct timeval	// in <sys/time.h>
{
	time_t			tv_sec;		// seconds
	suseconds_t	tv_usec;	// microseconds
};
#endif

struct	ether_header
{
	uint8_t		ether_dhost[6];
	uint8_t		ether_shost[6];
	uint16_t	ether_type;
};

struct llc_snap_hdr
{
	uint8_t		dsap;				// always 0xAA 
	uint8_t		ssap;				// always 0xAA 
	uint8_t		ctrl;				// always 0x03 
	uint8_t		oui[3];			// organizational universal id 
	uint16_t	ethertype;	// packet type ID field
};

#endif
