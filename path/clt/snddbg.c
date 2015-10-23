/*\file
 *	For debugging connections and message formatting;
 *	sends bytes read from file to serial port or UDP socket
 *
 *	Default is broadcast UDP socket on port 6056
 *
 */ 

#include <sys_os.h>
#include <sys_rt.h>
#include "udp_utils.h"

/** global definitions for signal handling */
static int sig_list[] =
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,
        ERROR,
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
        if (code == SIGALRM)
                return;
        else
                longjmp(exit_env, code);
}

#define UDP_PACKET_SIZE 1024

void do_usage(char *progname)
{
	fprintf(stderr, "Usage %s:\n", progname);
	fprintf(stderr, "-b <send to broadcast addres> ");
	fprintf(stderr, "-c <number of messages to send (0 continuous)> ");
	fprintf(stderr, "-f <input name> ");
	fprintf(stderr, "-i <interval> ");
	fprintf(stderr, "-n <bytes per message (must be <= 1024) > ");
	fprintf(stderr, "-o <output name or address> ");
	fprintf(stderr, "-p <port> ");
	fprintf(stderr, "-s (serial, not socket) ");
	fprintf(stderr, "-v (verbose) ");
	fprintf(stderr, "\n");
	exit(1);
}

int main (int argc, char **argv)
{
	int option;		/// for getopt
	int fdin;		/// input file or socket descriptor
	int fdout;		/// output file descriptor
	char *finname = NULL;	/// set for serial port or single socket source 
	char *foutname = NULL;	/// default output file name
	int port = 6056;	/// default port 
	int do_serial = 0;	/// by default, receive from socket
	int verbose = 0;	/// echo formatted output to stdout
	int count = 0;		/// character count for formatting
	int bytes_to_send = 1;	/// number of bytes desired for sendto call
	int bytes_read;		/// number actually read from input file
	int bytes_sent;		/// return value from sento call 
	struct sockaddr_in snd_addr;	/// used in sendto call
	int count_to_send = 0;	/// if 0, send until CTRL-C 
	int loop_count = 0;	/// compared to non-zero count_to_send 
	int do_broadcast = 0;	/// set to 1 for broadcast to subnet
	int chid = 0;		/// only needed on QNX6
	int interval = 100;	/// milliseconds 
	int do_timer = 1;	/// if interval is 0, set to 0
	posix_timer_typ *ptimer; 		

	unsigned char buf[UDP_PACKET_SIZE];	/// input buffer 

        while ((option = getopt(argc, argv, "bc:f:i:n:o:p:sv")) != EOF) {
                switch(option) {
                        case 'b':
                                do_broadcast = 1;
				break;
                        case 'c':
                                count_to_send = atoi(optarg);
                                break;
                        case 'f':
                                finname = strdup(optarg);
                                break;
                        case 'i':
                                interval = atoi(optarg);
				if (interval == 0)
					do_timer = 0;
				break;
                        case 'n':
                                bytes_to_send = atoi(optarg);
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

       if ((ptimer = timer_init(interval, chid))== NULL) {
                printf("timer_init failed in accread.\n");
                longjmp(exit_env, 1);
        }

        /* exit code on receiving signal */
        if (setjmp(exit_env) != 0) {
		fprintf(stderr, "%s exiting, send count %d\n", 
			argv[0], loop_count);
                fflush(stdout);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);

	if (finname == NULL) {
		fdin = STDIN_FILENO;
		fprintf(stderr, "Input: standard input\n");
	} else {
		if ((fdin = open(finname, O_RDONLY)) < 0) {
			perror("input open");
			do_usage(argv[0]);
		}
		fprintf(stderr, "Input: %s\n", finname);
	}

	if (do_serial) {
		if (foutname == NULL)
			do_usage(argv[0]);
		fdout = open(foutname, O_WRONLY);
	} else {
		if (foutname == NULL) {
			foutname = "255.255.255.255";
			do_broadcast = 1;
		}
		if (do_broadcast)
			fdout = udp_broadcast();
		else 
			// finname must be IP numeric string x.x.x.x 
			fdout = udp_unicast();

		if (fdout < 0) 
			do_usage(argv[0]);
		
	}
	fprintf(stderr, "Output: %s, port %d\n", foutname, port);

	// will not exit at end of file, so can run easily from pipe, terminal 
	// option to exit after sending a count_to_send messages 
	while (1) {
		if (do_serial) {
			read(fdin, buf, 1);
			write(fdout, buf, 1);
			if (verbose) 
				printf("%02hhx ", buf[0]);
			count++;
			if (count % 20 == 0)
				printf("\n");
		} else { 
			int i;
			set_inet_addr(&snd_addr, inet_addr(foutname), port);

			bytes_read = read (fdin, buf, bytes_to_send);

			bytes_sent = sendto(fdout, buf, bytes_read, 0,
				 (struct sockaddr *) &snd_addr,
				 sizeof(snd_addr));
			if (bytes_sent != bytes_read) {
				fprintf(stderr,
					 "%s: bytes sent %d bytes read %d\n",
						argv[0], bytes_sent, bytes_read);
				break;
			}
			if (verbose) 
				for (i = 0; i < bytes_sent; i++) {
					printf("%02hhx ", buf[i]);
					fflush(stdout);
					count++;
					if (count % 20 == 0)
						printf("\n");
				}
			loop_count++;
			if (count_to_send && loop_count >= count_to_send)
				break; 
               }
	       if (do_timer) // may not want timer if reading from serial port
		       TIMER_WAIT(ptimer);	// wait betweeen sends
	}
	longjmp(exit_env, 2);
}
