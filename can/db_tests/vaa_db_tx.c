/**\file
 *
 * vaa_db_tx.c
 *
 * Testing program: reads the specified DB variable from the DB server
 * and sends the appropriate messages to the specified CAN port. 
 * VAA message number determines DB variable number. 
 *
 * Function to use to convert DB variable to CAN sends is found
 * in array of function pointers indexed by DB variable number. 
 *
 * Usage: vaa_db_tx -p /dev/can1 -n <VAA msg number> 
 *
 * Code is built with different libraries to match the kind of
 * CAN card on the system. You must change the Makefile flags
 * for this to happen. 
 */

#include <sys_os.h>
#include "db_clt.h"
#include "db_utils.h"
#include "sys_rt.h"
#include "timestamp.h"
#include "can_defs.h"
#include "can_client.h"
#include "vaa_msg.h"
#include "vaa_clt_vars.h"

extern void snd_g(db_clt_typ *pclt, int can_fd, int extended, 
		void *psnd_msg_info);

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

/** Basic scheduling unit is set by the "interval" parameter.
 *  Each item is scheduled by counting these units module the "modulus"
 *  for that item, and sending it on the "send_index", e.g., for
 *  modulus 4, send item 3, the item would be sent when the 
 *  interval count was equal to 3 mod 4.
 */
typedef struct {
	unsigned char msg_id;
	unsigned char modulus;
	unsigned char send_index;
} schedule_item_t;

schedule_item_t snd_schedule[] = {
	{DB_SENSOR_CONFIG_ID, 1, 0},	/// sent every cycle 
	{DB_SENSOR_STATE_ID, 2, 0},	/// sent every even cycle
	{DB_POSITION_NORMAL_ID, 9, 5},	/// sent every 9 cycles 
	{DB_HMI_STATE_ID, 2, 1},	/// sent every odd cycle
};

int num_schedule_items = sizeof(snd_schedule)/sizeof(schedule_item_t);

int main(int argc, char **argv)
{
	float secs = 1.0;
	unsigned long id = 0;  
	unsigned char extended = 0;
	long count=0;
	int fd;
        char *port = "/dev/can1";
        int opt;
	int verbose = 0;	// by default silent
	int nmax = 0; // if zero, run forever
	int millisecs = 1000;	// interval for posix_timer_typ
	posix_timer_typ *ptmr;	
	db_clt_typ *pclt;              /* Database client pointer */
	char hostname[MAXHOSTNAMELEN+1];
	char *domain = DEFAULT_SERVICE; 
	int xport = COMM_OS_XPORT;	

        while ((opt = getopt(argc, argv, "e:i:n:p:t:v")) != -1) {
		switch (opt) {
		case 'e':
                        extended = 1;  
                        break;
		case 'i':
                        id = atoi(optarg);  // message ID 
                        break;
		case 'n':
			nmax = atoi(optarg);  
			break;
		case 'p':	// for Steinhoff, optarg should be 1-4
                        port = strdup(optarg);
                        break;
		case 't':
			secs = atof(optarg);  
			millisecs = secs*1000;
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
		fflush(stdout);
	}

	// second parameter of timer_init is no longer used
	if ((ptmr = timer_init(millisecs, 0))== NULL) {
                printf("timer_init failed in %s.\n", argv[0]);
                exit(EXIT_FAILURE);
        }

	get_local_name(hostname, MAXHOSTNAMELEN);
	if ((pclt = db_list_init( argv[0], hostname, domain, xport, 
		NULL, 0, NULL, 0)) == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
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
		int j;
		for (j = 0; j < num_schedule_items; j++) {
			int id = snd_schedule[j].msg_id;
			int modulus = snd_schedule[j].modulus;
			int send_index = snd_schedule[j].send_index;
			if (count % modulus == send_index)
				vaa_msg[id].snd(pclt, fd, extended, 
						(void *) &vaa_msg[id]);
		}	
		count++;
		if(count>nmax && nmax!=0)
			longjmp(exit_env, 2);

		TIMER_WAIT(ptmr);
		
	}
	return 0;
}



