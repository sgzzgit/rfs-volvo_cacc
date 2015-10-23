/**\file
 * Database variable update:  sample_update
 *
 * (C) Copyright University of California 2006.  All rights reserved.
 *
 * Sample code: update a variable containing several different fields
 *
 * Usage:
 *	 -d [domain] specifies Cogent datahub on QNX6 
 *	 -e don't exit after update, stay alive for debugging 
 *	 -v database variable number to use
 *	 -i integer value to put in database
 *	 -f floating point value to put in database
 */

#include	<sys_os.h>
#include	<sys_rt.h>
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
	char *domain = DEFAULT_SERVICE;
	char hostname[MAXHOSTNAMELEN];
	db_clt_typ *pclt;
	new_type value;
	int opt;
	int var = 50;
	int stay_alive = FALSE;
	int xport = COMM_OS_XPORT;

	value.new_int = 9;
	value.new_float = 999.9;
	strcpy(value.str8,"abcdefg");

	while ((opt = getopt(argc, argv, "d:ei:f:v:x:")) != -1)
	{
		switch (opt)
		{
		  case 'd':
			domain = strdup(optarg);
			break;
		  case 'e':
			stay_alive = TRUE;
			break;
		  case 'i':
			value.new_int = atoi(optarg);	
			break;
		  case 'f':
			value.new_float = atof(optarg);	
			break;
		  case 'v':
			var = atoi(optarg);	
			break;
		  case 'x':
			xport = atoi(optarg);	
			break;
		  default:
			printf( "Usage %s: clt_update -d [domain] ", argv[0]);
			printf("-e  -x [xport] -v [var] -i [int] -f [float]\n");
			exit(1);
		}
	}
	get_local_name(hostname, MAXHOSTNAMELEN+1);
	if ((pclt = clt_login(argv[0], hostname, domain, xport))
			== NULL)
	{
		printf("clt login %s %s %s %d failed\n",
			argv[0], hostname, domain, xport);
		exit(EXIT_FAILURE);
	}

	printf("clt_create, pclt %0x, variable %d\n", (unsigned int) pclt, var);
	if (!clt_create(pclt, var, var, sizeof(new_type))) {
		printf("clt create failed\n");
		clt_logout(pclt);
		exit(EXIT_FAILURE);
	}

	/* Catch the signals SIGINT, SIGQUIT, and SIGTERM.  If signal occurs,
	 * log out from database and exit. */
	if( setjmp( exit_env ) != 0 )
	{
	    /* Log out from the database. */
	    if (pclt != NULL)
		clt_destroy( pclt, var, var); 
	        clt_logout( pclt );

		exit( EXIT_SUCCESS );
	}
	else
		sig_ign( sig_list, sig_hand );

	printf("clt_update, var %d\n", var);
	if (!clt_update(pclt, var, var, sizeof(value), &value))
		printf("clt update failed\n");

	if (stay_alive) while (1); 
	longjmp( exit_env, 1 );
}

