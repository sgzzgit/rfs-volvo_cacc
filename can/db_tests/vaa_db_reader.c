/**\file
 *
 * Database variable reader:  vaa_db_reader
 * Prints database variable as an unformated sequence of bytes.
 *
 * Usage: vaa_db_reader -n <DB VAR number> -s <DB VAR size> -i interval 
 */

#include	<sys_os.h>
#include 	<sys_rt.h>
#include	"db_clt.h"

static int sig_list[]=
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	SIGALRM,	// for timer
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
int main (int argc, char** argv)
{
	char *domain=DEFAULT_SERVICE;
	char hostname[MAXHOSTNAMELEN+1];
	db_clt_typ *pclt;
	db_data_typ db_data;
	int opt;
	int var = 512;		// DB_J1939_CCVS_VAR
	int db_size =  29;	// sizeof(j1939_ccvs_typ)
	int xport = COMM_OS_XPORT;
	int interval = 20;	// milliseconds
	posix_timer_typ *ptmr;

	while ((opt = getopt(argc, argv, "i:e:s:")) != -1) {
		switch (opt) {
		  case 'i':
			interval = atoi(optarg);
			break;
		  case 'n':
			var = atoi(optarg);
			break;
		  case 's':
			db_size = atoi(optarg);
			break;
		  default:
			printf("Usage:%s: -i[interval]\n", argv[0]);
			printf(" -n [DB VAR #] -s [DB VAR size]\n");
			exit(1);
		}
	}
	get_local_name(hostname, MAXHOSTNAMELEN);
	if ((pclt = clt_login(argv[0], hostname, domain, xport))
			== NULL) {
		printf("clt login %s %s %s %d failed\n",
			argv[0], hostname, domain, xport);
		exit(EXIT_FAILURE);
	}
	if ((ptmr = timer_init(interval, 0))== NULL) {
                printf("timer_init failed in %s.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if (setjmp(exit_env) != 0) {
	    /* Log out from the database. */
		if (pclt != NULL)
			clt_logout(pclt);
		exit(EXIT_SUCCESS);
	} else
		sig_ign(sig_list, sig_hand);

	while (1) {
		if (!clt_read(pclt, var, var, &db_data))
			printf("clt read failed\n");
		else {
			int i;
			for (i = 0; i < db_size; i++) 
				printf("%hhu ", db_data.value.user[i]);	
			printf("\n");	
		}
		TIMER_WAIT(ptmr);
	}
	longjmp(exit_env, 1);
}

