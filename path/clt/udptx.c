/**\file 
 *
 * udptx.c   Send text data from stdin as UDP messages, one line per message.  
 *	     
 *      Usage: 	udptx -p PORT -a IP_ADDR_STR -u	/// send unicast  	  
 *      	udptx -p PORT -a IP_ADDR_STR 	/// send broadcast  	  
 *
 *
 *	Additional flags: -m milliseconds useful if reading lines from
 *		a piped file and want to slow down the sends
 *
 *  Copyright (c) 2010   Regents of the University of California
 *
 *  The signals SIGINT, SIGQUIT, and SIGTERM are trapped, and cause the
 *  process to terminate. SIGALRM is used for Posix timers on Linux.
 *
 *  Author: Sue Dickey
 *
 */


#include <sys_os.h>
#include <sys_rt.h>
#include <local.h>
#include <timing.h>
#include <timestamp.h>
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

#define MAX_PACKET_SIZE 1500

int main(int argc, char *argv[])
{
	int option;
	int status;
	timestamp_t ts;
	int sd_out;		/// socket descriptor for UDP send
	short udp_port = 7015;	/// set from command line option 
	char *udp_name = NULL;	/// address of UDP destination
	char *my_udp_name = "127.0.0.1";	/// address of UDP source 
	int do_broadcast = 1;	/// by default broacast information
	int verbose = 0;	/// if 1, print extra info for debugging	
	int bytes_sent;		/// returned from sendto
	struct sockaddr_in snd_addr;	/// used in sendto call
	int millisecs = 0;		/// minimum interval for send
	posix_timer_typ *ptimer; 	/// waits if millisecs > 0
	char buf[MAX_PACKET_SIZE];

	/* Read and interpret any user switches. */
	while ((option = getopt(argc, argv, "a:m:p:uv")) != EOF) {
		switch(option) {
	        case 'a':
			udp_name = strdup(optarg);
			break;
		case 'm':
			millisecs = atoi(optarg);
			break;
		case 'p':
			udp_port = atoi(optarg);;
			break;
		case 'u':
			do_broadcast = 0;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Usage %s: \n", argv[0]); 
			fprintf(stderr, " -a <dest IP (default broadcast)>\n");
			fprintf(stderr, " -m <millisecs between sends>\n");
			fprintf(stderr, " -p  (UDP port number for output) ");
			fprintf(stderr, " -u  (unicast)  \n");
			fprintf(stderr, " -v  (verbose) \n ");
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (udp_name == NULL) {
		do_broadcast = 1;
		udp_name = "255.255.255.255";
	}
	if (do_broadcast)  	
		sd_out = udp_broadcast_init(&snd_addr, udp_name, udp_port);
	else 
		sd_out = udp_unicast_init(&snd_addr, udp_name, udp_port);

	if (sd_out < 0) {
		printf("failure opening socket on %s %d\n",
				 udp_name, udp_port);
		exit(EXIT_FAILURE);
	}
	if (millisecs > 0) {
		if ((ptimer = timer_init(millisecs, 0))== NULL) {
			printf("%s: timer_init failed\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	} 

	if (setjmp(env) != 0) {
		exit(EXIT_SUCCESS);
	} else
               sig_ign(sig_list, sig_hand);

        memset(buf, 0, MAX_PACKET_SIZE);

        /// Safe to print buf and use strlen because full
        /// buffer was initialized to 0, final character always
        /// terminates string.
        while (fgets(buf, MAX_PACKET_SIZE-1, stdin) != NULL) {
		if (verbose)
			printf("%s %d\n", buf, strlen(buf));

		bytes_sent = sendto(sd_out, buf, strlen(buf), 0,
                                        (struct sockaddr *) &snd_addr,
                                        sizeof(snd_addr));
		if (verbose)
			printf("%d bytes sent\n", bytes_sent);
			

		if (bytes_sent < 0) {
			perror("sendto error");
			printf("port %d addr 0x%08x\n",
                                        ntohs(snd_addr.sin_port),
                                        ntohl(snd_addr.sin_addr.s_addr));
			fflush(stdout);
		}
		if (millisecs > 0)
			TIMER_WAIT(ptimer);
	}
        longjmp(env, 1);
}

static void sig_hand(int sig)
{
	longjmp(env, 1);
}
