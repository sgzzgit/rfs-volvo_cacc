/* This process receives can messages from both hmi and writes them
   to the database. It assumes that when it is started, the database
   variables are already created.
*/

#include <sys_os.h>
#include "db_clt.h"
#include "sys_rt.h"
#include "can_defs.h"
#include "can_client.h"
#include "messages.h"
#include "vaa_msg_struct.h"


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

static void sig_hand( int code );


int main(int argc, char *argv[])
{
    /* database stuff */
    db_clt_typ *pclt = NULL;
    char hostname[MAXHOSTNAMELEN+1];
    db_data_typ db_data;
    int recv_type;
    trig_info_typ trig_info;

    /* can stuff */
    int fd;
    char *port = "/dev/can1";

    /* Initialize the database. */
    get_local_name(hostname, MAXHOSTNAMELEN);
    if ((pclt = clt_login(argv[0], hostname,
			  DEFAULT_SERVICE, COMM_OS_XPORT)) == NULL)
    {
	fprintf(stderr, "Database initialization error in cc_hmi_recv\n");
	exit( EXIT_FAILURE );
    }

    /* initialize the can bus */
    fd = can_open(port, O_RDONLY);
    if (fd == -1)
    {
	printf("error opening %s\n", port);
	exit(EXIT_FAILURE);
    }
    else
	puts("can bus successfully opened in cc_hmi_recv");

    /* handle the jumps */
    if( setjmp( exit_env ) != 0 )
    {
	clt_logout(pclt);
	exit( EXIT_SUCCESS );
    }
    else
	sig_ign( sig_list, sig_hand );


    /* main loop */
    for (;;)
    {
	/* setup some variables first */
	can_basic_msg_t raw_msg;
	can_received_msg_t msg;
	
	/* read from the can */
	int size = can_read(fd, &raw_msg.id, (char*)NULL, raw_msg.d, 8);
	if (size <= 0)
	    continue;

	/* unpack the message */
	unpack_vaa_received_msg(&msg, &raw_msg);

	/* TODO: write msg to database */
	clt_update(pclt, TO_DBID(raw_msg.id), TO_DBID(raw_msg.id),
		   sizeof(msg.hmi_primary_msg), &msg.hmi_primary_msg);
    }
}

