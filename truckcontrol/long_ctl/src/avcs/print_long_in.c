/**\file
 * 
 * rddmm32.c - client for das_man compiled with dmm32.o
 *
 * 
 *  Copyright (c) 2009   Regents of the University of California
 *
*/

#include <db_include.h>
#include <db_comm.h>
#include "sys_mem.h"
#include "sys_das.h"
#include "db_clt.h"
#include "db_utils.h"
#include "clt_vars.h"
#include "veh_trk.h"
#include "long_ctl.h"

jmp_buf exit_env;

static void sig_hand( int sig);

static int sig_list[] = 
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
	SIGALRM,
        ERROR,
};

static void sig_hand( int code) {
	longjmp( exit_env, code );
}

int main(int argc, char *argv[]) {

	long_input_typ	long_input;
	db_clt_typ *pclt = NULL;	/* Database client pointer */
	char hostname[MAXHOSTNAMELEN + 1];
	char *domain = DEFAULT_SERVICE;
	int chid;
	int interval = 500;
	posix_timer_typ *ptmr;
	int option;

	while ((option = getopt(argc, argv, "i:")) != EOF) {
		switch(option) {
		case 'i':
			interval = atoi(optarg);
			break;
		default:
			printf("Usage: %s -i <interval>\n", argv[0]);
			break;
		}
	}

        get_local_name(hostname, MAXHOSTNAMELEN);                 
        if ((pclt = clt_login(argv[0], hostname, domain, COMM_OS_XPORT))  
                        == NULL)                                  
        {                                                         
                printf("clt login %s %s %s %d failed\n",          
                        argv[0], hostname, domain, COMM_OS_XPORT);        
                exit(EXIT_FAILURE);                               
        }                                                         

        if ( setjmp(exit_env) != 0) {
		if (pclt != NULL) 
			clt_logout(pclt);
                exit( EXIT_SUCCESS);
        } else 
                sig_ign( sig_list, sig_hand);

                                
	printf("DB_LONG_INPUT_VAR %d\n", DB_LONG_INPUT_VAR);
	while(1) {	
		db_clt_read(pclt, DB_LONG_INPUT_VAR, sizeof(long_input_typ),
				 (void *) &long_input);
                print_timestamp(stdout, &long_input.ts);           
                printf(" fb %.3lf rb %.3lf mb %.3lf\n",  
                        long_input.fb_axle, long_input.mb_axle,
                        long_input.rb_axle);
		sleep(1);
	}
}
