/**\file
 *
 * vaa_sim_sens_rx.c
 *
 *
 * 	Receives messages sent by Fan-ping's program running
 *	on the evaluation board that simulates 10 sensors
 *	each sending data every 2 ms.  
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
	unsigned char can_id;	// sensor number, 0-9
	unsigned int sensor_group_counter; // same for all 10 sensors
	unsigned int thread_counter; // changes about every 0.5 ms
} save_data_t;

/**
 *	Save 5 minutes of data, assuming 10 messages sent every .002 sec
 */
save_data_t saved_data[5 * 60 * 10 * 500];

static int max_count = sizeof(saved_data)/sizeof(save_data_t);

int main(int argc, char **argv)
{
	int size;
	int fd;
	unsigned long id;  
	unsigned char data[8];
        char *port = "/dev/can1";	/// use "1" to "4" for steinhoff
        int opt;
	int status;
	int read_err = 0;
	int msg_count = 0;
	timestamp_t start_ts, end_ts, diff_ts; /// report start and end times
	can_err_count_t errs;

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

	fd = can_open(port, O_RDONLY);

	if (fd == -1)
		exit(EXIT_FAILURE);	// error message printed by can_open 

	// Save starting time
	get_current_timestamp(&start_ts);

        if(setjmp(exit_env) != 0) {
		unsigned int i;
		get_current_timestamp(&end_ts);
		print_timestamp(stdout, &start_ts);
		printf(" ");
		print_timestamp(stdout, &end_ts);
		printf(" ");
		decrement_timestamp(&end_ts, &start_ts, &diff_ts);
		print_timestamp(stdout, &diff_ts); 	
		printf("\n");

		errs = can_get_errs(fd);

		if (read_err > 0)
			printf(" %d client read errors\n", read_err);

		printf("Error counts  at finish:\n");
		printf("intr_in_handler_count %u\n", errs.intr_in_handler_count);
		printf("rx_interrupt_count %u\n", errs.rx_interrupt_count);
		printf("rx_message_lost_count %u\n", errs.rx_message_lost_count);
		printf("tx_interrupt_count %u\n", errs.tx_interrupt_count);
		printf("shadow_buffer_count %u\n", errs.shadow_buffer_count);

		if (fd != -1)
			status = can_close(&fd);

		for (i = 0; i < msg_count; i++) { 
			save_data_t *p = &saved_data[i];
			printf("%u %hhu %u %u \n", i, p->can_id,
				p->sensor_group_counter, p->thread_counter); 
		}

		if (status != -1)			
			exit(EXIT_SUCCESS);
		else
			exit(EXIT_FAILURE);	// can_close prints error info
        } else
		sig_ign(sig_list, sig_hand);

	/// Clear error counts in driver
	errs =  can_clear_errs(fd);

	printf("Error counts at start:\n");
	printf("intr_in_handler_count %u\n", errs.intr_in_handler_count);
	printf("rx_interrupt_count %u\n", errs.rx_interrupt_count);
	printf("rx_message_lost_count %u\n", errs.rx_message_lost_count);
	printf("tx_interrupt_count %u\n", errs.tx_interrupt_count);
	printf("shadow_buffer_count %u\n", errs.shadow_buffer_count);

	for(;;) {
		id = 0;		
		size = can_read(fd,&id,(char *)NULL,data,8);

		if (size < 0) 
			read_err++;	
		else {
			saved_data[msg_count].can_id = id;
			saved_data[msg_count].sensor_group_counter = 
				(*(int *) &data[0]);
			saved_data[msg_count].thread_counter = 
				(*(int *) &data[4]);
		}
		msg_count++;
		if (msg_count >= max_count)
			longjmp(exit_env, 1);
	}
}

