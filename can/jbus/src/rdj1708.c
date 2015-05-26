/**\file
 * Receive J1708 data from a serial port or trace file on stdin, extract
 * messages and print as hex bytes, one message per line. Command line
 * argument -m specifies MID for sync, default is 128. Used to identify
 * port type and for hardware checking of connections.
 *
 * Copyright (c) 2005   Regents of the University of California
 *
 * Ported to QNX6 May 2005
 */

#include "old_include/std_jbus.h"

int fpin;	
jmp_buf env;

static int sig_list[] = 
{
	SIGINT,
	SIGQUIT,
	SIGTERM,
	ERROR,
};


#ifdef QNX_DBASE
db_clt_typ *pclt;              
#endif


#define EXIT_SUCCESS 0

static void sig_hand( int sig)
{
	longjmp( env, 1);
}

/** Sync on MID given as input parameter.
 */ 
int sync_on_known_message(int fd, int mid, char *buf, int *msg_length)

{
	int i;
	int retval;
	unsigned char byteval;
	int checksum = 0;
	int msg_found = 0;
	printf("syncing on mid %d (0x%02x)\n", mid, mid);
	while (!msg_found) {
		i = 0;
		while ((retval = read(fd, &byteval, 1)) == 1)
			if (byteval == mid){
				buf[i] = byteval;
				i++;
				checksum = byteval;
				printf("Checksum %d ", checksum);
				break;
			}
		if (retval <= 0)
			return 0;
		while ((retval = read(fd, &byteval, 1)) == 1){
			buf[i] = byteval;
			i++;
			if (byteval == (256 - checksum)){
				msg_found = 1;
				break;
			}
			checksum += byteval;
			checksum &= 0xff;
			printf("%d ", checksum);
			if (i >= 21){
				printf("checksum failure\n");
				break;
			}
		}
		printf("\n");
		if (retval <= 0)
			return 0;
	}
	printf("i %d\n", i);
	*msg_length = i;
	printf("*msg_length %d\n", *msg_length);
	return 1;
}  

/** Read bytes until checksum matches from J1708 
 */
int get_j1708(int fd, char *buf, int *msg_length)

{
	int i = 0;
	int retval;
	unsigned char byteval;
	int checksum = 0;
	
	while ((retval = read(fd, &byteval, 1)) == 1){
		buf[i] = byteval;
		i++;
		if (byteval == (256 - (checksum & 0xff))){
			break;
		}
		checksum += byteval;
		if (i == 21)
			return 0;
	}
	if (retval <= 0)
		return 0;
	*msg_length = i;
	return 1;
}  

int main( int argc, char *argv[] )
{
	int fpin = STDIN_FILENO; /* file descriptor for serial port */
	int hexformat = 0;
	int ch;
	int known_mid = 128;
	int msg_length;
        while ((ch = getopt(argc, argv, "m:h")) != EOF) {
                switch (ch) {
                case 'h': hexformat=1;
                          break;
		case 'm': known_mid = atoi(optarg);
			  break;
                default: fprintf(stderr, "Usage: %s [-h hex output]\n",
					argv[0]);
                          break;
                }
        }

	/*	set jmp */
	if ( setjmp(env) != 0) {
		exit( EXIT_SUCCESS);
	}
	else 
		sig_ign( sig_list, sig_hand);

	while (1) {
		int i;
		unsigned char buffer[J1587_MSG_MAX];
		int got_message = 1;
		if (!sync_on_known_message(fpin, known_mid,
				 (char *) buffer, &msg_length))
			break; 
		printf("msg_length of known message %d\n", msg_length);
	        while (got_message) {			
			for (i = 0; i < msg_length; i++)
				if (hexformat)	
					printf("0x%02x ", buffer[i]);
				else
					printf("%3d ", buffer[i]);
			printf("\n");
			got_message = get_j1708(fpin, (char *) buffer, &msg_length);
		}
	}
	longjmp(env,1);	/* go to exit code if receive loop terminates */
}			
