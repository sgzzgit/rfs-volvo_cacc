/**\file
 *
 * can_rx4.c
 *
 * Lighweight reception: no timestamp, print only if verbose. no timestamp.
 *
 *	Usage for SSV_CAN or ECAN cards (i82527): 
 *		can_rx  -p /dev/can1 [-v]
 #
 *	Usage for Steinhoff driver with SJA1000 chips:
 *		can_rx  -p 1 [-v] 
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
	int verbose = 0;
	int msg_count = 0;

        while ((opt = getopt(argc, argv, "p:v")) != -1) {
                switch (opt) {
                  case 'p':
                        port = strdup(optarg);
                        break;
                  case 'v':
                        verbose = 1; 
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

        if(setjmp(exit_env) != 0) {
		printf("%d messages, %d read errors\n", msg_count, read_err);
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
		id = 0;		
		size = can_read(fd,&id,(char *)NULL,data,8);
		if (size < 0) 
			read_err++;	
		else {
			msg_count++;
			if (verbose) {
				printf(" %5lu ", id);		
				printf(" %u ", size);		
				for (i = 0; i < size; i++)
					printf("%03hhu ",data[i]);
				printf("\n");
				fflush(stdout);
			}
		}
	}
}

