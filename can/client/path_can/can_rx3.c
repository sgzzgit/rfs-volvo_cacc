/**\file
 *
 * can_rx3.c
 *
 * Counts arrivals on the CAN port and checks for dropped messages
 *
 *	Usage for SSV_CAN or ECAN cards (i82527): 
 *		can_rx3  -p /dev/can1
 #
 *	Usage for Steinhoff driver with SJA1000 chips:
 *		can_rx3  -p 1 
 *
 * Links against different libraries for different CAN cards;
 * code is symbolically linked into different directories with
 * CAN driver specific Makefiles.
*/

#include <sys_os.h>
#include <sys/neutrino.h>
#include "local.h"
#include "sys_rt.h"
#include "timestamp.h"
#include "can_defs.h"
#include "can_client.h"

static int sig_list[]=
{
		SIGINT,
		SIGQUIT,
		SIGTERM,
		SIGALRM,		// for timer
		ERROR
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
	int size;
	int fd;
	unsigned long id;  
	unsigned char data[8];
	char *port = "/dev/can1";	// use "1" to "4" for steinhoff
	int opt;
	int status;
	
	int initiated = 0;
	timestamp_t start_time;
	unsigned int prevmsg = 0;
	int num_received = 0;
	int total_dropped = 0;
	int read_err = 0;
	
	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
		case 'p':
			port = strdup(optarg);
			break;
		default:
			printf("Usage: %s -p <port>\n", argv[0]);
			exit(1);
		}
	}
	
	printf("%s: trying to open %s\n", argv[0], port); 
	fflush(stdout);
	
	fd = can_open(port, O_RDONLY);
	
	if (fd == -1)
		exit(EXIT_FAILURE);	// error message printed by can_open 
	
	printf("program %s, device name %s, fd: %d\n", argv[0], port, fd);
	fflush(stdout);
	
	(void) can_print_config(stdout, fd);
	fflush(stdout);
	
	(void) can_print_status(stdout, fd);
	fflush(stdout);
	
	if(setjmp(exit_env) != 0) {
		timestamp_t elapsed_time;
		timestamp_t end_time;

		get_current_timestamp(&end_time);
		decrement_timestamp(&end_time, &start_time, &elapsed_time); 

		printf("%.3f secs ", TS_TO_SEC(&elapsed_time));
		printf(" %d msgs ", num_received);
		printf(" %.6f mean interval",
			TS_TO_SEC(&elapsed_time)/num_received);
		printf(" %d dropped (%.2f percent) ",
			total_dropped, 
			total_dropped *100.0/(total_dropped + num_received));
		printf(" %d read errors\n", read_err);

		if (fd != -1)
			status = can_close(&fd);
		if (status != -1)			
			exit(EXIT_SUCCESS);
		else
			exit(EXIT_FAILURE);	// can_close prints error info
	} else
		sig_ign(sig_list, sig_hand);
	
	
	for(;;) {
		id = 0;
		size = can_read(fd,&id,(char *)NULL,data,8);

		if (size < 0) 
			read_err++;
		else {
			// if not initiated, set prevtime as ts
			// else, continue as normal
			if (!initiated) {
				initiated = 1;
				get_current_timestamp(&start_time);
				prevmsg = *((unsigned*)data);
			} else {
				int dropped = *((unsigned*)data) - prevmsg - 1;
				num_received++;

				// Start computing difference and dropped totals
				// on second message 
				if (num_received > 1) {
					total_dropped += dropped;
				} 
				prevmsg = *((unsigned*)data);
			}
		}
	}
}

