/**\file
 *
 * vaa_db_rx.c
 *
 *	Reads messages on a CAN channel, assembles them and
 *	writes to DB data server variables.
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

extern int rcv_g(db_clt_typ *pclt, int can_fd, void *pmsg_funcs_array);

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
        char *port = "/dev/can1";	// use "1" to "4" for steinhoff
        int opt;
	int status;
	db_id_t *pdb_id_list = NULL;
	db_clt_typ *pclt;              /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	char *domain = DEFAULT_SERVICE; 
	int xport = COMM_OS_XPORT;	
	int count = 0;
	
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

	printf("program %s, device name %s, fd: %d\n", argv[0], port, fd);
	fflush(stdout);

	pdb_id_list = get_db_id_list();
	get_local_name(hostname, MAXHOSTNAMELEN);

	// Log in to the data server and create variables for all VAA messages
	if ((pclt = db_list_init( argv[0], hostname, domain, xport, 
			pdb_id_list, num_vaa_msg_ids, NULL, 0)) == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
        if(setjmp(exit_env) != 0) {
		printf("Successfully wrote %d messages to data server\n",
				count);
		if (fd != -1)
			status = can_close(&fd);
		if (status != -1)			
			exit(EXIT_SUCCESS);
		else
			exit(EXIT_FAILURE);	// can_close prints error info
        } else
		sig_ign(sig_list, sig_hand);

	for(;;) {
		int retval;
		retval = rcv_g(pclt, fd, (void *) &vaa_msg[0]);
		count += retval;
	}
}

