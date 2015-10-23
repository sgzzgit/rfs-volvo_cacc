/* \file
 *
 *  Process to communicate with the MDL ILM150HR lidar.  
 *
 *
 */

#include "sys_os.h"
#include "sys_rt.h"
#include "timestamp.h"
//#include "data_log.h"
#include "db_clt.h"
#include "db_utils.h"
#include "mdl.h"

/** Usage:
 *  -d not needed for Linux, DEFAULT_SERVICE is Linux value
 *  -o output mask 1 trace 2 DB 4 MySQl (can OR)
 *  -t time in minutes for trace file duration 
 *  -v verbose, prints trace output
 *  -c create db variables here, instead of in a db-creating program
 *
 *  Typical usage: 
 *  mdl -o 2 < /dev/ser2
 *  will write the input from /dev/ser2 to the database
 */
static char *usage= "-d domain -v verbose -o output mask (1 trace, 2 DB, 4 MySQL, 8 init DB vars, can OR) -n db_number (660 default) -i (create db vars)"; 

/** Signal handling is required for clean exits when using the
 *  data server. SIGALRM is requird for use of timers.
 */
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

bool_typ mdl_ser_driver_read(unsigned short *prange, int debug)
{
	unsigned char msb = 0;
	unsigned char lsb = 0;
	unsigned short range;

	read(STDIN_FILENO, &msb, 1);
	if( !(msb & 0x80) )
		return FALSE;
	read(STDIN_FILENO, &lsb, 1);
	if(debug)
		printf("MDLREAD: msb %#0x lsb %#0x\n",msb,lsb);
	if( ~lsb & 0xC0 ) {
		range = ( (msb & 0x3F) << 6) + lsb;
		if( range == 0xBF3F )
			return FALSE;
		else
			*prange = range;
		}
	else
		return FALSE;
	return (TRUE);
}


/** Possible to do all three at once by setting -o to 7, or
 *  any two, e.g, 3 for TRACE_FILE | USE_DB will both write a
 *  trace file and write the DB server.
 */
#define TRACE_FILE	1
#define USE_DB		2
#define USE_MYSQL	4



int main(int argc, char *argv[])
{
	char hostname[MAXHOSTNAMELEN+1];
	mdl_lidar_typ mdl_lidar;
	mdl_lidar_typ mdl_lidar2;
	unsigned short range;
	db_clt_typ *pclt = NULL;
	int lidar_db_num = DB_MDL_LIDAR_VAR;
	static db_id_t db_vars_list[1];
	int NUM_DB_VARS;//     sizeof(db_vars_list)/sizeof(db_id_t)
	int output_mask = 0;	// 1 trace, 2 DB server, 4 MySQL
	int debug = 0;
	int verbose = 0;
	int option;
	char *domain = DEFAULT_SERVICE; //for QNX6, use e.g. "ids"
	int readerr = 0;
	int prtctr = 0;
	int create_db_vars = 0;


    while ((option = getopt(argc, argv, "d:o:n:gvc")) != EOF) {
      switch(option) {
	case 'd':
		domain = strdup(optarg);
		break;
	case 'o':
		output_mask = atoi(optarg);	
		break;
	case 'g':
		debug = 1;
		break;
	case 'v':
		verbose = 1;
		break;
	case 'c':
		create_db_vars = 1;
		break;
	case 'n':
		lidar_db_num = atoi(optarg);
		break;
	default:
		printf("Usage: %s %s\n",argv[0], usage);
		exit(EXIT_FAILURE);
		break;
	}
    }

	db_vars_list[0].id = lidar_db_num;
	db_vars_list[0].size = sizeof(mdl_lidar_typ);
	NUM_DB_VARS = sizeof(db_vars_list)/sizeof(db_id_t);

	if (output_mask & USE_DB) {
		get_local_name(hostname, MAXHOSTNAMELEN);
		if (create_db_vars) {
			if ((pclt = db_list_init(argv[0], hostname, DEFAULT_SERVICE, 
				COMM_OS_XPORT, db_vars_list, NUM_DB_VARS, NULL, 0)) 
				== NULL) {
				printf("Database variable initialization error in %s.\n", 					argv[0]);
				exit(EXIT_FAILURE);
				}
			    }
		else {
			if ((pclt = db_list_init(argv[0], hostname, DEFAULT_SERVICE, 
				COMM_OS_XPORT, NULL, 0, NULL, 0)) == NULL) {
				printf("Database initialization error in %s.\n", 					argv[0]);
				exit(EXIT_FAILURE);
				}
			}
		}
	

	/** Receiving signal will cause program to exit here
	*/
	if (setjmp(exit_env) != 0) {
		/** Log out from the DB data server. */
		if (pclt != NULL) {
			if (create_db_vars) 
				db_list_done(pclt, db_vars_list, NUM_DB_VARS, NULL, 0);
			else
				db_list_done(pclt, NULL, 0, NULL, 0);
			}
		printf("MDL: read error %d\n", readerr);
		printf("%s lidar exiting\n", argv[0]);
		fflush(NULL);
		exit(EXIT_SUCCESS);
	} else 
		sig_ign(sig_list, sig_hand);


	for (;;) {
		if (mdl_ser_driver_read(&range, debug)) {
			get_current_timestamp(&mdl_lidar.ts);
			mdl_lidar.range = range / 10.0;
			if (output_mask & USE_DB)
				// update the database and display the 
				// messages read from driver on the screen
				db_clt_write(pclt, lidar_db_num, 
					sizeof(mdl_lidar_typ), &mdl_lidar);
			if(verbose) {
				if( (++prtctr % 100) == 0 ) {
					if(output_mask & USE_DB) {
						db_clt_read(pclt, lidar_db_num, 
							sizeof(mdl_lidar_typ), &mdl_lidar2);
						mdl_print(&mdl_lidar2);
						}
					else
						mdl_print(&mdl_lidar);
						printf("m\n");
					}
				}
			} 
		else
			++readerr;
		} 
}

void mdl_print( mdl_lidar_typ *pmdl_lidar) {
	printf(" %02d:%02d:%02d:%03d %f ",
	pmdl_lidar->ts.hour, pmdl_lidar->ts.min,
	pmdl_lidar->ts.sec, pmdl_lidar->ts.millisec,
	pmdl_lidar->range);
}
