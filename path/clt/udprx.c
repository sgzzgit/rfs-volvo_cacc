/**\file *
 *     Read message arriving over UDP on a specified port
 *     and print to screen, either as hex numbers or assuming it is a string
 *
 *	Usage: udprx -p PORT -h	/// hex bytes in message 
 *	udprx -p PORT -c	/// print message as text string 
 *	udprx -p PORT		/// just print count at the end	
 *	udprx -p PORT -v	/// a little extra info
 *
 *  Copyright (c) 2010   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 *  Author: Sue Dickey
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <local.h>
#include <timestamp.h>
#include <timing.h>
#include <udp_utils.h>

jmp_buf env;
static void sig_hand(int sig);

static int sig_list[] = {
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,
	ERROR,
};

static void sig_hand(int code)
{
	if (code == SIGALRM)
		return;
	else
		longjmp(env, code);
}

#define MAX_PACKET_SIZE 1500

int main(int argc, char *argv[])
{
	int option;
	int status;
	
	int verbose = 0;	/// if 1, print extra info for debugging	
	int udp_port = 7015;	/// port for receiving UDP message
	int sd_in;		/// socket descriptor for UDP receive
	unsigned char buf[MAX_PACKET_SIZE];
	int bytes_rcvd;		/// returned from recvfrom
	struct sockaddr_in src_addr;	/// used in recvfrom call
	unsigned int socklen;
	int message_count = 0;
	int do_string = 0;
	int do_hex = 0;

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "chp:v")) != EOF) {
		switch(option) {
		case 'c':
			do_string = 1;	
			break;
		case 'h':
			do_hex = 1;	
			break;
		case 'p':
			udp_port = atoi(optarg);
			break;
		case 'v':
			verbose = 1;	
			break;
		default:
			fprintf(stderr, "Usage %s: ", argv[0]); 
			fprintf(stderr, " -p  (UDP port number for input) ");
			fprintf(stderr, " -c  print message as string ");
			fprintf(stderr, " -h  print message as hex bytes ");
			fprintf(stderr, " -v  (verbose, info to stdout) ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
		}
	}

	sd_in = udp_allow_all(udp_port);

	if (sd_in < 0) {
		printf("failure opening socket on %d\n", udp_port);
		exit(EXIT_FAILURE);
	}

	if (verbose)
		printf("%s: Receiving on port %d\n", argv[0], udp_port);

	if (setjmp(env) != 0) {
		printf("%s: received %d UDP messages\n",
				 argv[0], message_count);
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

	while (1) {
		timestamp_t ts;
		memset(&buf, 0, MAX_PACKET_SIZE); /// keep a '\0' at the end
		socklen = sizeof(src_addr);

		bytes_rcvd = recvfrom(sd_in, &buf, MAX_PACKET_SIZE - 1, 0,
				(struct sockaddr *) &src_addr, &socklen);
		if (verbose) {
			get_current_timestamp(&ts); 
			print_timestamp(stdout, &ts);
			printf(" %d bytes\n", bytes_rcvd);
			fflush(stdout);
		}
			
		if (bytes_rcvd < 0) {
			perror("recvfrom failed\n");
			continue;
		}
		if (do_hex) {
			int i;
			for (i = 0; i < bytes_rcvd; i++) {
				if ((i % 16) == 0)
					printf("\n");
				printf("%02hhx ", buf[i]);
			}
			printf("\n");
			fflush(stdout);
		}
		if (do_string) { 
			printf("%s\n", buf);
			fflush(stdout);
		}

		if (verbose) {
			printf("s_addr 0x%08x ", 
				src_addr.sin_addr.s_addr);
			printf("\n");
		}
	}
}
