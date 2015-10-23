/**\file
 *
 * can_rx.c
 *
 * Listens and prints all the messages on the CAN port..
 *
 *	Usage for SSV_CAN or ECAN cards (i82527): 
 *		can_rx  -p /dev/can1
 #
 *	Usage for Steinhoff driver with SJA1000 chips:
 *		can_rx  -p 1 
 *
 * Links against different libraries for different CAN cards;
 * must change Makefile to match.
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

int main(int argc, char **argv)
{
	int size;
	int fd;
	unsigned long id;  
	unsigned char data[8];
        char *port = "/dev/can1";	// use "1" to "4" for steinhoff
        int opt;
	int status;
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

	printf("can_rx: trying to open %s\n", port); 
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
		if (read_err > 0)
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
		int i;
		timestamp_t ts;
		id = 0;		
		size = can_read(fd,&id,(char *)NULL,data,8);

		if (size < 0) 
			read_err++;	
		else {
			get_current_timestamp(&ts);
			print_timestamp(stdout, &ts);
			printf(" %ld ", id);		
			printf(" %d bytes: ", size);		
			for (i = 0; i < size; i++)
				printf("%02hhx ",data[i]);
			printf("\n");

			fflush(stdout);
		}
	}
}

