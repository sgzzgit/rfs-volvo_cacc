/**\file
 *
 * can_client_i82527.c	Functions to be called by higher-level
 *			protocols for specific CAN devices in
 *			order to get the basic CAN message from
 *			using the PATH driver for the I82527 chip.
 *
 *			Currently the SSV_CAN and ECAN boards 
 *			have been tested wth this driver (June 2009).	
 *
 */

#include <sys_os.h>
#include <devctl.h>
#include <sys/neutrino.h>
#include "local.h"
#include "das_clt.h"
#include "can_defs.h"
#include "can_client.h"

/* This structure type is specific to the I82527 driver and not
 * visible except to routines in this file.
 */
typedef struct {
	int fd;
	int channel_id;
	int flags;
	char *filename;
} can_dev_handle_t;


#define MAX_MSG_BUF 1000
/** Read information from the CAN card; return -1 if error encountered,
 *  otherwise number of bytes in the data segment.
 *  Blocking read, includes MsgReceive
 */
int can_read(int fd, unsigned long *id, char *extended, void *data, 
				unsigned char size) {
	can_msg_t msg;
	int status;
	can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 
	int real_fd = phdl->fd;
	int channel_id = phdl->channel_id;
	char msg_buf[MAX_MSG_BUF];
	struct _msg_info msginfo;
	int rcvid;

        memset(&msg, 0, sizeof(msg));

	rcvid = MsgReceive(channel_id, msg_buf, MAX_MSG_BUF, &msginfo);

	if (rcvid == 0) {
		status = devctl(real_fd, DCMD_CAN_I82527_READ, (void *) &msg,
                        sizeof(msg), NULL);
	} else {
		printf("rcvid %d channel_id %d chid %d pid %d ",
			rcvid, channel_id, msginfo.chid, msginfo.pid); 
 		printf("msglen %d coid %d scoid %d\n", 
			msginfo.msglen, msginfo.coid, msginfo.scoid);
		perror("MsgReceive");
		return (-1);
	}
	
	if (status != EOK) {
		printf("can_read: devctl error %d\n", status);
		return -1;
	}
	if (msg.error != 0) {
#ifdef DO_TRACE
		printf("can_read: failed to pop msg from in buffer\n");
#endif
		return -1;
	}

	if (id != NULL)
		*id = CAN_ID(msg);
	if (extended != NULL) {
		if (IS_EXTENDED_FRAME(msg))
			*extended = 1;
		else
			*extended = 0;
	}
	memcpy(data, msg.data, size > 8 ? 8 : size);

#ifdef DO_TRACE
	printf("can_read: msg.id 0x%08x msg.size %hhd\n", msg.id, msg.size);
#endif
	return(msg.size);
}

int can_write(int fd, unsigned long id, char extended, void *data, 
				unsigned char size) {
	can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 
	int real_fd = phdl->fd;
	can_msg_t msg;

	msg.size = size > 8 ? 8 : size;
	msg.id = id;

	if (extended) SET_EXTENDED_FRAME(msg);

	memcpy(msg.data, data, msg.size);


	return(devctl(real_fd, DCMD_CAN_I82527_WRITE, (void *) &msg,
			sizeof(msg), NULL));
}

/** We never actually used this function on QNX4, so it was
 *  untested at PATH with QNX4.
 *	Internal to the driver, set as part of open for read
 */
int can_set_filter(int fd, unsigned long id, unsigned long mask)
{
	can_filter_t filter_data;
	filter_data.id = id;
	filter_data.mask = mask;

	return(devctl(fd, DCMD_CAN_FILTER, (void *) &filter_data,
			sizeof(filter_data), NULL)); 
}

/* Enable pulses from the CAN driver to the client process.
 * Pulses are waited for with MsgReceive or IP_Receive.
 * channel_id can be one obtained from DB_CHANNEL(pclt) on
 * a the pointer returned by the database clt_login.
 *
 *	Internal to the driver, done as part of open for read
 */
int can_arm(int fd, int channel_id)
{
        int coid;
        static struct sigevent event;
	
        /* we need a connection to that channel for the pulse to be
             delivered on */
        coid = ConnectAttach(0, 0, channel_id, _NTO_SIDE_CHANNEL, 0);

        /** fill in the event structure for a pulse; use the file
         *  descriptor of the CAN device so that this pulse can be
         *  distinguished from other devices that may be sending
         *  pulses to this client
         */
        SIGEV_PULSE_INIT(&event, coid, SIGEV_PULSE_PRIO_INHERIT,
                    fd, 0);

        return (devctl(fd, DCMD_CAN_ARM, (void *) &event,
                         sizeof(struct sigevent), NULL));
}

/* Call this function to empty the queue of messages in
 * the CAN driver; returns number of messages that were dropped
 * from the queue.
 *	Internal to the driver, done as part of open for read
 */
int can_empty_q(int fd)
{
	int num_dropped = -1;
	(void) devctl(fd, DCMD_CAN_EMPTY_Q, (void *) &num_dropped, 
			sizeof(int), NULL);
	return (num_dropped);
}

/**
 *	Clears the error counters maintained by the driver.
 *	Returns their old value.
 */
can_err_count_t can_clear_errs(int fd)
{
	can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 
	int real_fd = phdl->fd;
	can_err_count_t errs;
	int status;
	memset(&errs, 0, sizeof(can_err_count_t));
	status =  devctl(real_fd, DCMD_CAN_CLEAR_ERRS, (void *) &errs, 
			sizeof(can_err_count_t), NULL);
	if (status != EOK) {
		perror("can clear errs");
		printf("fd %d, DCMD 0x%x, status %d\n", 
			fd, DCMD_CAN_CLEAR_ERRS, status);
	}
	return (errs);
}

/**
 *	Gets the current values of error counters maintained by the driver.
 */
can_err_count_t can_get_errs(int fd)
{
	can_dev_handle_t *phdl = (can_dev_handle_t *) fd; 
	int real_fd = phdl->fd;
	can_err_count_t errs;
	int status;
	memset(&errs, 0, sizeof(can_err_count_t));
	status =  devctl(real_fd, DCMD_CAN_GET_ERRS, (void *) &errs, 
			sizeof(can_err_count_t), NULL);
	if (status != EOK) {
		perror("can get errs");
		printf("fd %d, DCMD 0x%x, status %d\n", 
			fd, DCMD_CAN_GET_ERRS, status);
	}
	return (errs);
}

/* Wrapper for the open call; some drivers are not file structured
 * and different calls will be needed to "open"
 *
 * Returns "fd" that will be used in all subsequent calls
 * Filename string and flags contain any information needed to do the open
 */
int can_open(char *filename, int flags)
{
	int fd;
	int channel_id;
	can_dev_handle_t *phdl = (can_dev_handle_t *)
		 malloc(sizeof(can_dev_handle_t));
	fd = open(filename, flags);
	if (fd == -1) {
		perror("can_open");
		return(-1);
	}
	if (flags == O_RDONLY) {
		if ((channel_id = ChannelCreate(0)) == -1) {
			printf("can_open: ChannelCreate failed\n");
			return (-1);
		}
		can_set_filter(fd,0,0); //listens to all messages
		can_empty_q(fd);
		if (can_arm(fd, channel_id) == -1) {
			printf("can_arm failed\n");
			return (-1);         
		}
	}
	phdl->fd = fd;
	phdl->channel_id = channel_id;
	phdl->flags = flags;
	phdl->filename = filename;
	return ((int) phdl);
}

/* Wrapper for the close call; some drivers are not file structured
 * and may require disconnect functions to be called.
 *
 * Sets the input "file descriptor" to NULL, so that attempts
 * to close twice can be caught. Requires passing address of fd/handle
 * to this routine. 
 *
 * Returns -1 if handle is NULL, otherwise returns the value from
 * the close of the real fd.
 */
int can_close(int *pfd)
{
	can_dev_handle_t **pphdl = (can_dev_handle_t **) pfd; 
	can_dev_handle_t *phdl = *pphdl; 
	int real_fd;
	int retval = -1;
	
	if (phdl == NULL) {
		fprintf(stderr, "NULL handle passed to can_close\n"); 
		return retval;	 
	}

  	real_fd= phdl->fd;

	free(phdl);

	*pphdl = NULL;

	if ((retval = close(real_fd)) == -1)
		perror("can_close");

	return (retval);
	// Should there be a "can_disarm"?? Sue 2009
}

/**
 *	When I get a chance I will have this print the i82527 bit
 *	timing registers, at least. (Sue, August 2009)
 */
int can_print_config(FILE *fp, int fd)
{
	return 1;	// successful return, don't create error just for print	
}

/**
 *	Not yet implemented
 */
int can_set_config(int fd, int bitspeed)
{
	return 0;	// error return in case change in bitspeed is required 
}

/**
 *	
 *	Not yet implemented.	
 */
int can_print_status(FILE *fp, int fd)
{
	return 1;	// successful return, lack of print shows error
}

