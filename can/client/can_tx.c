/**\file
 *
 * can_tx.c
 *
 * Periodically sends a message to the specified CAN port. 
 * Messages have ID sent from the command line and data that
 * is just an incrementing integer.
 *
 * For SSV_CAN or ECAN, sample usage:
 *
 * can_tx -p /dev/can1 -i <integer id>  -t <seconds (flost)>
 *	  -e (if uses extended frames) -v debugging information
 *
 * For Steinhoff driver, sample usage:
 *
 * can_tx -p 1 -i <integer id>  -t <seconds (flost)>
 *	  -e (if uses extended frames) -v debugging information
 *
 * Code is built with different libraries to match the kind of
 * CAN card on the system. You must change the Makefile flags
 * for this to happen. 
 */

#include <sys_os.h>
#include "db_clt.h"
#include "sys_rt.h"
#include "timestamp.h"
#include "can_defs.h"
#include "can_client.h"

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

int main(int argc, char **argv)
{
	float secs = 1.0;
	unsigned long id = 0;  
	unsigned char extended = 0;
	unsigned char data[20];	
	volatile long count=0;
	int fd;
        char *port = "/dev/can1";
        int opt;
	int verbose = 0;	// by default silent
	int nmax = 0; // if zero, run forever
	int millisecs = 1000;	// timer interval is in milliseconds
	int repetitions = 1;
	posix_timer_typ *ptmr;	

        while ((opt = getopt(argc, argv, "e:i:n:p:t:v")) != -1) {
		switch (opt) {
		case 'e':
                        extended = 1;  
                        break;
		case 'i':
                        id = atoi(optarg);  
                        break;
		case 'n':
			nmax = atoi(optarg);  
			break;
		case 'p':	// for Steinhoff, optarg should be 1-4
                        port = strdup(optarg);
                        break;
		case 't':
			secs = atof(optarg);  
			millisecs = secs*1000.0;
			if (millisecs < 1) {
				millisecs = 1;
				repetitions = (1.0/(secs * 1000.0) + 0.5);
			} else {
				millisecs = secs * 1000.0 + 0.5;
			}
                        break;
		case 'v':
                        verbose = 1;
                        break;
		default:
			printf("Usage: %s -p <port> ", argv[0]);
			printf("-i <ID> -t <seconds> -e <extended frames>\n");
			printf("example: %s -p /dev/can -i 1000 -t 0.1\n",
				argv[0]);
			exit(1);
                }
        }

	fd = can_open(port,O_WRONLY);

	if(fd == -1) {
		printf("error opening %s\n", port);
		exit(EXIT_FAILURE);
	} else {
		printf("program %s, device name %s, fd: %d, extended %hhd\n",
			argv[0], port, fd, extended);
		printf("interval %d ms, %d repetitions\n",
			 millisecs, repetitions);
		fflush(stdout);
	}

	// second parameter of timer_init is no longer used
	if ((ptmr = timer_init(millisecs, 0))== NULL) {
                printf("timer_init failed in %s.\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        /* exit code on receiving signal */
        if (setjmp(exit_env) != 0) {
                fprintf(stderr, "%s exiting, send count %ld\n",
                        argv[0], count);
                fflush(stdout);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);


	for(;;) {
		int i;
		int j;

		for (j = 0; j < repetitions; j++) {
			count++;
			// Make artificial data from count, for debugging
			for (i = 0; i < 8; i++) {
				int shift = (i % 4) * 8;
				data[i] = ((count & (0xff << shift)) >> shift);
			}

			can_write(fd,id,extended,data,8);
		}

		if (verbose) { 
			timestamp_t ts;
			get_current_timestamp(&ts);
			print_timestamp(stdout, &ts);
			printf(" %ld ", id);
			for (i = 0; i < 8; i++) 
				printf("%02hhx ", data[i]);
			printf("\n");
			fflush(stdout);
		}
		
		if(count>nmax && nmax!=0)
			longjmp(exit_env, 2);

		TIMER_WAIT(ptmr);
		
	}

	return 0;

}



