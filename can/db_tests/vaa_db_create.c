/**\file
 *
 *	Creates alls the variables needed for VAA CAN messages
 *
*/

#include <sys_os.h>
#include "local.h"
#include "db_clt.h"
#include "db_utils.h"
#include "sys_rt.h"
#include "timestamp.h"
#include "can_defs.h"
#include "can_client.h"
#include "vaa_msg.h"
#include "vaa_clt_vars.h"

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

/* mallocs and initializes list used for creation of data server variables
 */
db_id_t *get_db_id_list()
{
	int i;
	db_id_t *p = (db_id_t *) malloc(num_vaa_msg_ids * sizeof(db_id_t));
	if (p == NULL){
		printf("malloc failed\n");
		return (p);
	}

	for (i =0; i < num_vaa_msg_ids; i++) {
		p[i].id =  vaa_msg[i].id;
		p[i].size = vaa_msg[i].size;
	}
	return (p);
}

int main(int argc, char **argv)
{
	int fd;
        int opt;
	int status;
	db_id_t *pdb_id_list = NULL;
	db_clt_typ *pclt;              /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	char *domain = DEFAULT_SERVICE; 
	int xport = COMM_OS_XPORT;	
	int interval = 60000;		// wake up once a minute
	posix_timer_typ *ptimer;
	int verbose;
	
        while ((opt = getopt(argc, argv, "v")) != -1) {
                switch (opt) {
                case 'v':
                        verbose = 1;
                        break;
                default:
                        printf("Usage: %s -v \n", argv[0]);
                        exit(1);
                }
        }

        // second parameter of timer_init is no longer used
        if ((ptimer = timer_init(interval, 0))== NULL) {
                printf("timer_init failed in %s.\n", argv[0]);
                exit(EXIT_FAILURE);
        }

	pdb_id_list = get_db_id_list();
	get_local_name(hostname, MAXHOSTNAMELEN);

	// Log in to the data server and create variables for all VAA messages
	if ((pclt = db_list_init( argv[0], hostname, domain, xport, 
			pdb_id_list, num_vaa_msg_ids, NULL, 0)) == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
        if(setjmp(exit_env) != 0) {
		if (fd != -1)
			status = can_close(&fd);
		if (status != -1)			
			exit(EXIT_SUCCESS);
		else
			exit(EXIT_FAILURE);	// can_close prints error info
        } else
		sig_ign(sig_list, sig_hand);

	for(;;) {
		if (verbose) {
			timestamp_t ts; 
			get_current_timestamp(&ts);
			print_timestamp(stdout, &ts);
		}
		TIMER_WAIT(ptimer);
	}
}

