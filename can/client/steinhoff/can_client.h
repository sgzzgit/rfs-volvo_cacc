/**\file
 * can_client.h
 *
 * Definition of the CAN API.
 * Header file to be included in the CAN client processes.
 *
 */

#ifndef INCLUDE_CAN_CLIENT_H
#define INCLUDE_CAN_CLIENT_H

#include <sys/types.h>

extern int can_open(char *filename, int flags);
extern int can_set_filter(int fd, unsigned long id, unsigned long mask);
extern int can_read(int fd, unsigned long *id, char *extended, void *data,
			unsigned char size);
extern int can_write(int fd, unsigned long id, char extended, void *data, 
			unsigned char size);
extern int can_empty_q(int fd);
extern int can_arm(int fd, pid_t proxy);
extern int can_print_config(FILE *fpout, int fd);
extern int can_set_config(int fd, int bitspeed);
extern int can_close(int *pfd);

extern int can_print_config(FILE *fpout, int fd);
extern int can_print_status(FILE *fpout, int fd);
extern can_err_count_t can_get_errs();
extern can_err_count_t can_clear_errs();

#endif


