/*\file
 *
 * QNX4 version of functions to access CAN driver and set
 * up database for use with IBEO Laserscanner.
 *
 * OS-version-specific functions for QNX6 are in ibeo_qnx6.c
 */
#include "ibeo.h"

/*
 * can_open	Open the CAN bus device
 */
int can_open(char *devname)
{
	int fd;
	
	/* Open and close -- to flush? */
	fd = open(devname, O_RDWR);
	if (fd<0) {
		perror("can_open");
		exit(1);
	} 
	close(fd);

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

/*
 * init_can	Setup the CAN bus device
 *
 * Returns "channel id", which is a proxy on QNX4
 */
int init_can(int fd, db_clt_typ *pclt, char **argv)
{
	int proxy;

        can_set_filter(fd,0,0); //listens all messages
        proxy = qnx_proxy_attach(0,0,0,0);
        can_arm(fd,proxy);
	return proxy;
}

/* 
 * can_get_message	Block on Receive, read data when available. 
 *
 * Receive calls on QNX4 and QNX6 are different, so this is made
 * into a function.  This function does a simplified read that assumes that it
 * is already known whether the message has a standard or extended ID.
 * Returns number of bytes in the message, 0 if no read was done.
 */
int can_get_message(int fd, int channel_id, unsigned long *p_id,
			 unsigned char *data, int length)
{
	int i;
	int pid;	// process id or proxy id

	pid = Receive(0,0,0); //blocking

	if(pid == channel_id) 
		i = can_read(fd, p_id, (char *)NULL, data, 8);
	else
		i = 0;

	return i;
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
        char hostname[MAXHOSTNAMELEN+1];

        sprintf(hostname, "%lu", getnid());
	domain = DEFAULT_SERVICE; //useful to have this set for debug
        if ((pclt = clt_login(progname, hostname, domain, COMM_QNX_XPORT))
								 == NULL) {
		fprintf(stderr, "%s database open error, domain %s\n",
			 progname, domain);
		return NULL;
        }

	if (clt_create(pclt, DB_IBEO_LIST_VAR, DB_IBEO_LIST_TYPE,
		sizeof(ibeo_list_typ)) == FALSE) {
		fprintf(stderr, "IBEO DB create error: %d already created?\n",
				DB_IBEO_LIST_VAR);
	}
	for (i = 0; i < IBEO_MAX_OBJECTS; i++) {
		int db_num = DB_IBEO_OBJ0_VAR + i;
		if (clt_create(pclt, db_num, db_num,
				 sizeof(ibeo_obj_typ)) == FALSE) {
			fprintf(stderr, 
				"IBEO DB create error: %d already created?\n",
				db_num);
		}
	} 

        return pclt;
}
