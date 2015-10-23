/**\file
 *
 * Database variable reader:  sample_read
 *
 * Sample code: reads a variable with several fields from the Cascade DataHub
 * and displays the  result on the standard output.
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
static void sig_hand( int code )
{
	if (code == SIGALRM)
		return;
	else
		longjmp( exit_env, code );
}
typedef struct {
	int new_int;
	float new_float;
	char str8[8];
} new_type;

int main (int argc, char** argv)
{
	char *domain=DEFAULT_SERVICE;
	char hostname[MAXHOSTNAMELEN+1];
	db_clt_typ *pclt;
	db_data_typ db_data;
	new_type *presult;
	int stay_alive = 0;
	int opt;
	int var = 50;
	int xport = COMM_OS_XPORT;

	while ((opt = getopt(argc, argv, "d:ev:x:")) != -1)
	{
		switch (opt)
		{
		  case 'd':
			domain = optarg;
			if (strlen(domain) > 15)
				domain[15] = '\0';
			break;
		  case 'e':
			stay_alive = TRUE;
			break;
		  case 'v':
			var = atoi(optarg);
			break;
		  case 'x':
			xport = atoi(optarg);
			break;
		  default:
			printf("Usage: clt_read -d [domain]\n");
			exit(1);
		}
	}
	get_local_name(hostname, MAXHOSTNAMELEN);
	if ((pclt = clt_login(argv[0], hostname, domain, xport))
			== NULL)
	{
		printf("clt login %s %s %s %d failed\n",
			argv[0], hostname, domain, xport);
		exit(EXIT_FAILURE);
	}


	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if( setjmp( exit_env ) != 0 )
	{
	    /* Log out from the database. */
	    if (pclt != NULL)
	        clt_logout( pclt );

		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	if (!clt_read(pclt, var, var, &db_data))
		printf("clt read failed\n");
	else {
		presult = (new_type *) &db_data.value.user[0];
		printf("presult: %d %.3f %s\n", presult->new_int,
			 presult->new_float, presult->str8);
	}
	if (stay_alive) while (1);	// stay alive until killed
	longjmp( exit_env, 1 );
}

