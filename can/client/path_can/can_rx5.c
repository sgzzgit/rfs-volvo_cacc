/**\file
 *
 * can_rx5.c
 *
 * Lighweight reception: timestamp only at beginning and end. 
 * Saves data from each message up to maximum (default maximum 1,500,000).
 * If maximum is 0, runs indefinitely cycling through default sized buffer
 * to save data. Prints data to stdout on exit.
 *
 *	Usage: 
 *		can_rx5 -p <device name> -n <max> 
 #
 * Links against different libraries for different CAN cards.
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
        SIGALRM,        // for timer
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

typedef struct {
        unsigned long id;   
        unsigned char data[8];	/// CAN data always has a maximum of 8 bytes
} save_data_t;

void print_err_counts(char *context, can_err_count_t *p)
{
	printf("%s: ", context);
	printf("SB %u  ", p->shadow_buffer_count);
	printf("IIH %u  ", p->intr_in_handler_count);
	printf("RXI %u  ", p->rx_interrupt_count);
	printf("RXML %u  ", p->rx_message_lost_count);
        printf("TXI %u\n", p->tx_interrupt_count);
}


int main(int argc, char **argv)
{
	int size;
	int fd;
        char *port = "/dev/can1";	/// use "1" to "4" for steinhoff
        int opt;
	int status;
	int read_err = 0;
	unsigned int msg_count = 0;
	unsigned int max_msg_count = 1500000;
	save_data_t *save_array;
	timestamp_t start_ts;
	timestamp_t end_ts;
	unsigned char ext_hdr;
	int assume_sequence = 0; /// if 1, integer sequence number in CAN data 
	can_err_count_t errs;

        while ((opt = getopt(argc, argv, "p:n:s")) != -1) {
                switch (opt) {
                case 'p':
                        port = strdup(optarg);
                        break;
                case 'n':
                        max_msg_count = atoi(optarg);  
                        break;
                case 's':
                        assume_sequence = 1; 
                        break;
                default:
                        printf("Usage: %s -p <port> -n <max>", argv[0]);
                        printf(" -s (assume sequence)\n");
                        exit(1);
                }
        }

	fd = can_open(port, O_RDONLY);

	if (fd == -1)
		exit(EXIT_FAILURE);	// error message printed by can_open 

	printf("program %s, device name %s, fd: %d\n", argv[0], port, fd);
	fflush(stdout);
	
	if (!(save_array = (save_data_t *) 
			malloc(max_msg_count * sizeof(save_data_t)))) {
		printf("Failed to allocate array to save data\n");
		exit(EXIT_FAILURE);
	}

        if(setjmp(exit_env) != 0) {
		int i;	/// cycle through saved data
		get_current_timestamp(&end_ts);
		print_timestamp(stdout, &start_ts);
		printf("-- ");
		print_timestamp(stdout, &end_ts);
		printf(": ");
		printf("%d messages, %d read errors\n", msg_count, read_err);
		for (i = 0; i < msg_count; i++) {
			int j;
			save_data_t *p = &save_array[i];
			printf("%d %lu ", i, p->id);
			if (assume_sequence) {
				printf("%u", *((unsigned int *) &p->data[0]));
			} else {
				for (j = 0; j < 8; j++) 
					printf("%3hhd ", p->data[j]);
			}
			printf("\n");
		}
		errs = can_clear_errs(fd);
		print_err_counts("Counts at end:", &errs);
			 
		if (fd != -1)
			status = can_close(&fd);
		if (status != -1)			
			exit(EXIT_SUCCESS);
		else
			exit(EXIT_FAILURE);	// can_close prints error info
        } else
		sig_ign(sig_list, sig_hand);

	get_current_timestamp(&start_ts);
	errs = can_clear_errs(fd);
	print_err_counts("Counts at start:", &errs);

	for(;;) {
		save_data_t *p = &save_array[msg_count];
		size = can_read(fd, &p->id, &ext_hdr, p->data, 8);
		if (size < 0) 
			read_err++;	// try again to fill same array element	
		else {
			msg_count++;
		}
		if (max_msg_count == 0) {
			if (msg_count == max_msg_count)
				msg_count = 0;
		} else {
			if (msg_count >= max_msg_count)
			longjmp(exit_env, 2);
		}
	}
}

