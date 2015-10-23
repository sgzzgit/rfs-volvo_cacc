/*\file
 *	Forward every message received on port specified	
 *	on the specified output interface. If no output interface
 *	is specified, broadcasts to 255.255.255.255
 *
 * 	Must set unicast flag if specifying a unicast output interface. 
 *	If no input interface is specified, will accept from any.
 *
 *	Copyright (c) Regents of the University of California
 *
 *	Sue Dickey, October 25, 2005
 */ 

#include <sys_os.h>	// link to file for appropriate OS
#include "udp_utils.h"
#include "timestamp.h"

#define UDP_PACKET_SIZE 1024	//max for this program

// debug with CICAS message sequence numbers
#define CICAS_HDR
void do_usage(char *progname)
{
	fprintf(stderr, "Usage %s:\n", progname);
	fprintf(stderr, "-b port for broadcast ");
	fprintf(stderr, "-o output interface name ");
	fprintf(stderr, "-i input interface name ");
	fprintf(stderr, "-p port for input ");
	fprintf(stderr, "-u (do unicast) ");
	fprintf(stderr, "-v verbose ");
	fprintf(stderr, "\n");
	exit(1);
}

int main (int argc, char **argv)
{
	int option;		/// for getopt
	int fdin;		/// input file or socket descriptor
	int fdout;		/// output file descriptor
	char *finname = NULL;	/// by default accept from any 
	char *foutname = "192.168.255.255";	/// default output file name
	int port_in = 6053;	/// Econolite broadcast address
	int port_out = 6054;	/// Use for broadcast to vehicles
	unsigned char buf[UDP_PACKET_SIZE];	/// input buffer 
	socklen_t socklen;	
	struct sockaddr_in src_addr;	/// filled in by recvfrom
	struct sockaddr_in snd_addr;	/// used in by sendto
	int bytes_received;	/// returned from recvfrom
	int bytes_sent;		/// returned from sendto
	int verbose = 0;	/// echo formatted output to stdout
	int unicast = 0;	/// by default, send to broadcast address
	int count = 0;		/// character count for formatting
//	unsigned int socklen;	/// holds sockaddr_in size for sendto call

  while ((option = getopt(argc, argv, "b:i:o:p:uv")) != EOF) 
	{
		switch( option )
		{
    case 'b':
			port_out = atoi(optarg);
			break;
		case 'i':
			finname = strdup(optarg);
			break;
		case 'o':
			foutname = strdup(optarg);
			break;
		case 'p':
			port_in = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		case 'u':
			unicast = 1;
			break;
		default:
			do_usage(argv[0]);
    break;
		}
	}
	fprintf(stderr, "Receive Port: %d\n", port_in);
	if (finname != NULL) {
		fdin = udp_allow_from(inet_addr(finname), port_in);
		fprintf(stderr, "Input: %s\n", finname);
	} else {
		fdin = udp_allow_all(port_in);
		fprintf(stderr, "Input: any IP\n");
	}

	if (fdin < 0) 
		do_usage(argv[0]);

	if (foutname == NULL) 
		foutname = "255.255.255.255";
	if (unicast)
		fdout = udp_unicast();
	else
		fdout = udp_broadcast();

	if (fdout < 0) 
		do_usage(argv[0]);
		
	fprintf(stderr, "Output: %s, %s\n", foutname,
				 unicast?"unicast":"broadcast");
	fflush(stderr);

	socklen = sizeof(src_addr);
	memset(&src_addr, 0, socklen);

	while (1) {
		count = 0;	// for formatting output
		memset(&buf[0],'\0',UDP_PACKET_SIZE*sizeof(unsigned char));
		socklen = sizeof(struct sockaddr);		
		bytes_received = recvfrom(fdin, buf,
				 UDP_PACKET_SIZE-1, 0,
				(struct sockaddr *) &src_addr,
				 &socklen);

		if (bytes_received  < 0) {
			perror("recvfrom failed\n");
			continue;
		}
		if (verbose) { 
#ifdef CICAS_HDR
			timestamp_t ts;
                        unsigned short seq = buf[1] << 8 | buf[0];
			get_current_timestamp(&ts);
			print_timestamp(stdout, &ts);
                        printf(" %hu  byte count %d UDP port %d ",
                                seq, bytes_received, ntohs(src_addr.sin_port));
                        printf("IP address 0x%08x\n",
                                 ntohl(src_addr.sin_addr.s_addr));
#else
			for (i = 0; i < bytes_received; i++) {
				printf("%02hhx ", buf[i]);
				fflush(stdout);
				count++;
				if (count % 20 == 0)
					printf("\n");
			}
			printf("\n");
#endif
			
		}

		set_inet_addr(&snd_addr, inet_addr(foutname), port_out);

		bytes_sent = sendto(fdout, buf, bytes_received, 0,
				 (struct sockaddr *) &snd_addr,
				 sizeof(struct sockaddr));
				 
		if (bytes_sent != bytes_received) {
			fprintf(stderr,
				 "%s: bytes sent %d not equal received %d\n",
					argv[0], bytes_sent, bytes_received);
			continue;
		}
	}
	exit(EXIT_FAILURE);
}
