/*\\file
 *	Utility functions to do common combinations of open, set options,
 *	bind and listen operations on UDP sockets, making the
 *	real calls and setting the address structures appropiately.
 *
 *	All functions return the socket descriptor if successful,
 *	a negative number if unsuccessful
 *
 *	Copyright (c) 2006   Regents of the University of California
 *
 */
#ifndef UDP_UTILS_H
#define UDP_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Fills in an AF_INET address structure with integer IP address and port 
// IPv4 address is in network order, port is not 
void set_inet_addr(struct sockaddr_in *paddr, int ip_nworder, short port);

// Sets up a UDP socket on a port for reception from any address 
extern int udp_allow_all(short port);

// Sets up a UDP socket on a port for reception from a particular address 
extern int udp_allow_from(int ip_nworder, short port);

// Sets up a UDP socket to send broadcast messages 
extern int udp_broadcast();

// Sets up a UDP socket to send unicast messages
extern int udp_unicast();

// Sets up a UDP socket for sending unicast messages to "port" at "ip_str"
// and initializes the sockaddr_in structure to be used for the sends.
extern int udp_unicast_init(struct sockaddr_in *paddr,char *remote_ip_str, short port);

/* Sets up a UDP socket for sending unicast messages to "port" at "ip_str"
** and initializes the sockaddr_in structure to be used for the sends.
** The difference between this initialization and udp_unicast_init is the optional
** use of the local IP address and a local port.  So why don't we use TCP/IP instead?
** Because the connection may be tenuous, and ITRI required the same port be used for local host (e.g. 128.32.234.123, NOT 127.0.0.1)
*/
extern int udp_peer2peer_init(struct sockaddr_in *paddr,char *remote_ip_str, char *local_ip_str, short remote_port, short local_port);



// Sets up a UDP socket for sending broadcast messages to "port"
// and initializes the sockaddr_in structure to be used for the sends.
extern int udp_broadcast_init(struct sockaddr_in *paddr, char *ip_str,
				short port);

#endif
