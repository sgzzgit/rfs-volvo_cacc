/*\file
 *
 * QNX6 version of functions to access CAN driver and set
 * up database for use with IBEO Laserscanner.
 *
 * OS-version-specific functions for QNX4 are in ibeo_qnx4.c
 */

#include "ibeo.h"

/*
 * can_open	Open the CAN bus device
 */
int can_open(char *devname)
{
	int fd;
	
	fd = open(devname, O_RDWR);
	if (fd<0) {
		perror("can_open");
		exit(1);
	} else {
		printf("device name %s, fd: %d\n", devname, fd);
		fflush(stdout);
	}

	return fd;
}

static IP_Msg *hmsg;
static IP_Task *htask = NULL;
static IP_MsgInfo msginfo;
static IP_MsgInfo *pmsginfo = &msginfo;

int init_channel(char **argv)
{
        char namebuf[256];
        char *qname;
        char *domain = "nissan"; //later set datahub domain from command line

        hmsg = IP_MsgCreate (NULL, IP_MsgDefaultSize(), 0);
        /*
         * Generate a queuename based on process id
         * process pid
         */
        sprintf (namebuf, "%s%d", "nissan", getpid());
        qname = strdup (namebuf);

        if (!(htask = IP_NserveInit (argv[0], domain, qname, 0, 0))) {
                printf("IP_NserveInit() failed:  qserve and nserve running?");
                exit(1);
        }
        printf("init_channel returning %d\n", htask->chid);
        fflush(stdout);
        return (htask->chid);
}

/*
 * init_can	Setup the CAN bus device
 *
 * Returns "channel id", either share with database or, if NULL, create
 *	channel specifically for CAN received
 *
 * On QNX4, "channel id" is a proxy
 */

int init_can(int fd, db_clt_typ *pclt, char **argv)
{
	int channel_id;

	if (pclt == NULL)
		channel_id = init_channel(argv);
	else {
		htask = pclt->htask;	// set for use by can_get_message
		hmsg = pclt->hmsg; 	// set for use by can_get_message
		pmsginfo = &pclt->msginfo; 	// set for use by can_get_message
		channel_id = pclt->htask->chid;
	}

	can_set_filter(fd,0,0); //listens to all messages
	can_arm(fd, channel_id);
}

/* 
 * can_get_message	Block on Receive, read data when available. 
 *
 * Receive calls on QNX4 and QNX6 are different, so this is made
 * into a function.  This function does a simplified read that assumes that it
 * is already known whether the message has a standard or extended ID.
 * Returns number of bytes in the message, 0 if no read was done.
 * May need more complicated return structure if used with database trigger.
 */
int can_get_message(int fd, int channel_id, unsigned long *pid,
			 unsigned char *data, int length)
{
	int size = 0;
	int type = IP_Receive (htask, hmsg, pmsginfo);

	if (msginfo.type == IP_PULSE && msginfo.rcvid == fd) 
		size = can_read(fd, pid, (char *)NULL,data,8);

	return size;
}

/*
 * ibeo_database_init	Login to local database and create variables
 *
 * Returns pointer to client's structure for the database.
 */
db_clt_typ *ibeo_database_init(char *progname, char *domain)
{
	int i;
	db_clt_typ *pclt;

        if ((pclt = clt_login(progname, NULL, domain, COMM_QNX_XPORT))
								 == NULL) {
		fprintf(stderr, "%s database open error, domain %s\n",
			 progname, domain);
		return NULL;
        }

	if (clt_create(pclt, DB_IBEO_LIST_VAR, DB_IBEO_LIST_TYPE,
		sizeof(ibeo_list_typ)) == FALSE) {
		fprintf(stderr, "IBEO DB create error: %d ?\n",
				DB_IBEO_LIST_VAR);
	}
	for (i = 0; i < IBEO_MAX_OBJECTS; i++) {
		int db_num = DB_IBEO_OBJ0_VAR + i;
		if (clt_create(pclt, db_num, db_num,
				 sizeof(ibeo_obj_typ)) == FALSE) {
			fprintf(stderr, 
				"IBEO DB create error: %d \n",
				db_num);
		}
	} 

        return pclt;
}

