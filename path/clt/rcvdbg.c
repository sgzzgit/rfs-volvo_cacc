/*\file
 *	For debugging connections and message formatting;
 *	echos bytes received from file, serial port or UDP socket
 *	to output file.
 *
 *	Default is to receive from any address on port 6056.
 *
 */ 

#include <sys_os.h>
#include "udp_utils.h"
#include <stdio.h>

#define UDP_PACKET_SIZE 1024

void do_usage(char *progname)
{
	fprintf(stderr, "Usage %s:\n", progname);
	fprintf(stderr, "-f input name ");
	fprintf(stderr, "-n output line length ");
	fprintf(stderr, "-o output name ");
	fprintf(stderr, "-p port ");
	fprintf(stderr, "-s serial (not socket) ");
	fprintf(stderr, "-v verbose ");
	fprintf(stderr, "\n");
	exit(1);
}

int main (int argc, char **argv)
{
	int option;		/// for getopt
	int fdin;		/// input file or socket descriptor
	int fdout;		/// output file descriptor
	char *finname = NULL;	/// set for serial port or single socket source 
	char *foutname = "stream.out";	/// default output file name
	int port = 6056;		/// default port 
	unsigned char buf[UDP_PACKET_SIZE];	/// input buffer 
	int do_serial = 0;	/// by default, receive from socket
	struct sockaddr_in src_addr;	/// filled in by recvfrom
	int bytes_received;	/// returned from recvfrom
	int verbose = 0;	/// echo formatted output to stdout
	int count = 0;		/// character count for formatting
	int linelength = 16;	/// default byte codes per line
	int i;			/// local counter for for loops
	unsigned int socklen;	/// holds sockaddr_in size for sendto call

        while ( (option = getopt( argc, argv, "f:o:p:sv" )) != EOF ) {
                switch( option ) {
                        case 'f':
                                finname = strdup(optarg);
                                break;
			case 'n':
				linelength = atoi(optarg);
				break;
                        case 'o':
                                foutname = strdup(optarg);
                                break;
                        case 'p':
                                port = atoi(optarg);
				break;
                        case 's':
                                do_serial = 1;
				break;
                        case 'v':
                                verbose = 1;
				break;
                        default:
				do_usage(argv[0]);
                                break;
                }
        }
	if (do_serial) {
		if (finname == NULL) 
			do_usage(argv[0]);
		if ((fdin = open(finname, O_RDONLY)) < 0) {
			perror("serial open");
			do_usage(argv[0]);
		}
	} else {
		if (finname == NULL) 
			fdin = udp_allow_all(port);
		else
			// finname must be IP numeric string x.x.x.x 
			fdin = udp_allow_from(inet_addr(finname), port);

		if (fdin < 0) 
			do_usage(argv[0]);
			
		
		fprintf(stderr, "Receive Port: %d\n", port);
	}

	if (finname == NULL)
		fprintf(stderr, "Input: any IP\n");
	else
		fprintf(stderr, "Input: %s\n", finname);

	fdout = open(foutname, O_WRONLY | O_CREAT);
	fprintf(stderr, "Output: %s\n", foutname);

	socklen = sizeof(src_addr);
	memset(&src_addr, 0, socklen);

	while (1) {
		if (do_serial) {
			read(fdin, buf, 1);
			write(fdout, buf, 1);
			if (verbose) 
				printf("%02hhx ", buf[0]);
			count++;
			if (count % linelength == 0)
				printf("\n");
		} else { 
			bytes_received = recvfrom(fdin, buf,
				 UDP_PACKET_SIZE, 0,
                                (struct sockaddr *) &src_addr, &socklen);

			if (bytes_received  < 0) {
				perror("recvfrom failed\n");
				break;
			}
			write(fdout, buf, bytes_received);

			if (verbose) 
				for (i = 0; i < bytes_received; i++) {
					printf("%02hhx ", buf[i]);
					fflush(stdout);
					count++;
					if (count % linelength == 0)
						printf("\n");
				}
                }
	}
	fprintf(stderr, "%s exiting on error\n", argv[0]);
	exit(EXIT_FAILURE);
}
